#include "DifyService.h"
#include <QNetworkRequest>
#include <QJsonArray>
#include <QUuid>
#include <QDebug>
#include <QRegularExpression>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QtGlobal>

DifyService::DifyService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_baseUrl("https://api.dify.ai/v1")
    , m_userId(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

DifyService::~DifyService()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void DifyService::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void DifyService::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = baseUrl;
}

void DifyService::setModel(const QString &model)
{
    m_model = model;
}

QString DifyService::currentConversationId() const
{
    return m_conversationId;
}

void DifyService::clearConversation()
{
    m_conversationId.clear();
}

void DifyService::sendMessage(const QString &message, const QString &conversationId)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API Key 未设置");
        return;
    }

    if (message.trimmed().isEmpty()) {
        emit errorOccurred("消息内容不能为空");
        return;
    }

    // 如果有正在进行的请求，先取消
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    // 清空累积响应
    m_fullResponse.clear();
    m_streamBuffer.clear();
    resetStreamFilters();

    // 构建请求 URL
    QUrl url(m_baseUrl + "/chat-messages");
    QNetworkRequest request(url);

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    
    // 配置 SSL
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);  // 暂时禁用 SSL 验证用于调试
    request.setSslConfiguration(sslConfig);

    // 设置超时时间（给 AI 足够的思考时间）
    request.setTransferTimeout(120000);  // 120秒超时

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#endif

    // 构建请求体 - Agent 应用使用 streaming 模式
    QJsonObject body;
    body["query"] = message;
    body["response_mode"] = "streaming";
    body["user"] = m_userId;
    body["inputs"] = QJsonObject();  // Agent 应用的 inputs 可以是空对象
    
    const bool useStreaming = true;  // Agent 应用强制使用 streaming 模式

    // 暂时移除模型参数，让 Dify 使用默认配置
    // if (!m_model.isEmpty()) {
    //     body["model"] = m_model;
    //     qDebug() << "[DifyService] Using model:" << m_model;
    // }

    // 如果有会话 ID，添加到请求中
    QString convId = conversationId.isEmpty() ? m_conversationId : conversationId;
    if (!convId.isEmpty()) {
        body["conversation_id"] = convId;
    }

    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson();

    qDebug() << "[DifyService] Sending message:" << message;
    qDebug() << "[DifyService] Request URL:" << url.toString();
    qDebug() << "[DifyService] API Key:" << m_apiKey.left(10) + "...";
    qDebug() << "[DifyService] Request body:" << doc.toJson(QJsonDocument::Compact);

    emit requestStarted();

    // 发送 POST 请求
    m_currentReply = m_networkManager->post(request, jsonData);

    // 连接信号
    connect(m_currentReply, &QNetworkReply::finished, this, &DifyService::onReplyFinished);
    if (useStreaming) {
        connect(m_currentReply, &QNetworkReply::readyRead, this, &DifyService::onReadyRead);
    }
    connect(m_currentReply, &QNetworkReply::sslErrors, this, &DifyService::onSslErrors);
}

void DifyService::onReadyRead()
{
    if (!m_currentReply) return;

    // 立即读取所有可用数据
    QByteArray data = m_currentReply->readAll();
    if (!data.isEmpty()) {
        parseStreamResponse(data);
    }
}

void DifyService::onReplyFinished()
{
    qDebug() << "[DifyService] Reply finished, checking for errors...";

    if (!m_currentReply) {
        qDebug() << "[DifyService] No current reply";
        return;
    }

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString errorMsg = m_currentReply->errorString();
        int httpStatus = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = m_currentReply->readAll();

        qDebug() << "[DifyService] HTTP Error:" << httpStatus << "Error:" << errorMsg;
        qDebug() << "[DifyService] Network error code:" << m_currentReply->error();
        qDebug() << "[DifyService] Response data:" << responseData;
        qDebug() << "[DifyService] Response data (as string):" << QString::fromUtf8(responseData);

        // 特殊处理连接错误
        if (m_currentReply->error() == QNetworkReply::ConnectionRefusedError ||
            m_currentReply->error() == QNetworkReply::RemoteHostClosedError ||
            m_currentReply->error() == QNetworkReply::NetworkSessionFailedError) {
            qDebug() << "[DifyService] Connection error detected - possible network or SSL issues";
            errorMsg = QString("网络连接错误: %1\n请检查网络连接或API配置（API Key/代理/防火墙）").arg(errorMsg);
        }

        // 尝试解析错误响应
        QJsonDocument errorDoc = QJsonDocument::fromJson(responseData);
        if (!errorDoc.isNull() && errorDoc.isObject()) {
            QJsonObject errorObj = errorDoc.object();
            if (errorObj.contains("message")) {
                errorMsg = errorObj["message"].toString();
            }
        }

        if (httpStatus > 0) {
            errorMsg = QString("%1 (HTTP %2)").arg(errorMsg).arg(httpStatus);
        }

        qDebug() << "[DifyService] Error:" << errorMsg;
        emit errorOccurred(errorMsg);
    } else {
        // 发送完整响应
        QByteArray responseData = m_currentReply->readAll();
        qDebug() << "[DifyService] Request successful, response data length:" << responseData.length();

        // 对于 blocking 模式，直接解析响应
        if (!responseData.isEmpty()) {
            // 尝试解析 JSON 响应
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
            if (!responseDoc.isNull() && responseDoc.isObject()) {
                QJsonObject responseObj = responseDoc.object();
                const QString convId = responseObj["conversation_id"].toString();
                if (!convId.isEmpty() && convId != m_conversationId) {
                    m_conversationId = convId;
                    emit conversationCreated(convId);
                }
                QString answer = responseObj["answer"].toString();
                if (!answer.isEmpty()) {
                    // 过滤 think/analysis 标签
                    QString filteredAnswer = filterThinkTagsStreaming(answer);
                    qDebug() << "[DifyService] Emitting messageReceived with content:" << filteredAnswer.left(100) + "...";
                    emit messageReceived(filteredAnswer);
                } else {
                    qDebug() << "[DifyService] Warning: No answer field in response!";
                    qDebug() << "[DifyService] Full response:" << responseData;
                    if (responseObj.contains("message")) {
                        emit errorOccurred(responseObj["message"].toString());
                    }
                }
            } else {
                qDebug() << "[DifyService] Failed to parse JSON response, using raw data";
                emit messageReceived(QString::fromUtf8(responseData));
            }
        } else {
            qDebug() << "[DifyService] Warning: Response data is empty!";
        }
    }

    emit requestFinished();

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void DifyService::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "[DifyService] SSL Errors occurred:";
    for (const QSslError &error : errors) {
        qDebug() << "[DifyService] SSL Error:" << error.errorString();
        qDebug() << "[DifyService] SSL Error certificate:" << error.certificate().subjectInfo(QSslCertificate::CommonName);
    }

    // 忽略 SSL 错误（仅用于开发环境）
    if (m_currentReply) {
        m_currentReply->ignoreSslErrors(errors);
        qDebug() << "[DifyService] Ignoring SSL errors and continuing...";
    }
}

void DifyService::resetStreamFilters()
{
    m_tagRemainder.clear();
    m_hiddenTagName.clear();
    m_ignoreFurtherContent = false;
    m_hasTruncated = false;
}

static int partialSuffixLength(const QString &text, const QStringList &tokens)
{
    int best = 0;
    for (const QString &token : tokens) {
        const int tokenLen = token.length();
        const int maxK = qMin(tokenLen - 1, text.length());
        for (int k = maxK; k > best; --k) {
            if (text.endsWith(token.left(k), Qt::CaseInsensitive)) {
                best = k;
                break;
            }
        }
    }
    return best;
}

QString DifyService::filterThinkTagsStreaming(const QString &text)
{
    static const QStringList hiddenTags = {"think", "analysis"};
    static const QStringList allTokens = {"<think>", "</think>", "<analysis>", "</analysis>"};

    QString input = m_tagRemainder + text;
    m_tagRemainder.clear();

    QString output;
    int i = 0;

    auto startTokenFor = [](const QString &tag) { return QString("<%1>").arg(tag); };
    auto endTokenFor = [](const QString &tag) { return QString("</%1>").arg(tag); };

    while (i < input.length()) {
        if (!m_hiddenTagName.isEmpty()) {
            const QString endToken = endTokenFor(m_hiddenTagName);
            const int endIdx = input.indexOf(endToken, i, Qt::CaseInsensitive);
            if (endIdx == -1) {
                break;
            }
            i = endIdx + endToken.length();
            m_hiddenTagName.clear();
            continue;
        }

        int bestStartIdx = -1;
        QString bestTag;
        for (const QString &tag : hiddenTags) {
            const QString startToken = startTokenFor(tag);
            const int idx = input.indexOf(startToken, i, Qt::CaseInsensitive);
            if (idx != -1 && (bestStartIdx == -1 || idx < bestStartIdx)) {
                bestStartIdx = idx;
                bestTag = tag;
            }
        }

        if (bestStartIdx == -1) {
            output += input.mid(i);
            i = input.length();
            break;
        }

        output += input.mid(i, bestStartIdx - i);
        i = bestStartIdx + startTokenFor(bestTag).length();
        m_hiddenTagName = bestTag;
    }

    const int keep = partialSuffixLength(input, allTokens);
    if (keep > 0) {
        m_tagRemainder = input.right(keep);
        if (output.endsWith(m_tagRemainder, Qt::CaseInsensitive)) {
            output.chop(keep);
        }
    }

    output.replace(QRegularExpression("[ \\t]+"), " ");
    output.replace(QRegularExpression("\\n{3,}"), "\n\n");
    return output.trimmed();
}

void DifyService::parseStreamResponse(const QByteArray &data)
{
    // Dify 使用 SSE (Server-Sent Events) 格式，部分 JSON 可能跨分片
    QString buffer = m_streamBuffer + QString::fromUtf8(data);
    QStringList lines = buffer.split('\n');
    m_streamBuffer.clear();

    // 如果最后一行不完整，暂存到缓冲区，等待下一个分片
    if (!buffer.endsWith('\n') && !lines.isEmpty()) {
        QString lastLine = lines.takeLast();
        if (!lastLine.trimmed().isEmpty()) {
            m_streamBuffer = lastLine;
        }
    }

    for (QString line : lines) {
        line = line.trimmed();
        if (!line.startsWith("data: ")) {
            continue;
        }

        QString jsonStr = line.mid(6);  // 去掉 "data: " 前缀
        if (jsonStr.isEmpty() || jsonStr == "[DONE]") {
            qDebug() << "[DifyService] Stream finished [DONE]";
            continue;
        }

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (doc.isNull() || !doc.isObject()) {
            qDebug() << "[DifyService] Failed to parse JSON:" << jsonStr.left(100);
            continue;
        }

        QJsonObject obj = doc.object();
        QString event = obj["event"].toString();
        qDebug() << "[DifyService] Event:" << event;

        if (event == "message") {
            // 普通消息块
            if (m_ignoreFurtherContent) {
                continue;
            }
            QString answer = obj["answer"].toString();
            // 过滤 think/analysis 标签（支持跨 chunk）
            QString filteredAnswer = filterThinkTagsStreaming(answer);
            if (filteredAnswer.isEmpty()) {
                continue;
            }

            const int remaining = m_maxResponseChars - m_fullResponse.length();
            if (remaining <= 0) {
                if (!m_hasTruncated) {
                    m_fullResponse += "…";
                    emit streamChunkReceived("…");
                    m_hasTruncated = true;
                }
                m_ignoreFurtherContent = true;
                continue;
            }

            if (filteredAnswer.length() > remaining) {
                const QString clipped = filteredAnswer.left(remaining);
                m_fullResponse += clipped + "…";
                emit streamChunkReceived(clipped + "…");
                m_hasTruncated = true;
                m_ignoreFurtherContent = true;
                continue;
            }

            m_fullResponse += filteredAnswer;
            emit streamChunkReceived(filteredAnswer);

        } else if (event == "message_end") {
            // 消息结束
            QString convId = obj["conversation_id"].toString();
            if (!convId.isEmpty() && convId != m_conversationId) {
                m_conversationId = convId;
                emit conversationCreated(convId);
            }

        } else if (event == "error") {
            // 错误事件
            QString errorMsg = obj["message"].toString();
            emit errorOccurred(errorMsg);

        } else if (event == "agent_thought") {
            // Agent 思考过程
            qDebug() << "[DifyService] Processing agent thought event";
            QString thought = obj["thought"].toString();
            if (!thought.isEmpty()) {
                emit thinkingChunkReceived(thought);
            }
            continue;
            
        } else if (event == "agent_message") {
            // Agent 消息（如果使用 Agent 类型应用）
            if (m_ignoreFurtherContent) {
                continue;
            }
            QString answer = obj["answer"].toString();
            // 过滤 think/analysis 标签（支持跨 chunk）
            QString filteredAnswer = filterThinkTagsStreaming(answer);
            if (filteredAnswer.isEmpty()) {
                continue;
            }

            const int remaining = m_maxResponseChars - m_fullResponse.length();
            if (remaining <= 0) {
                if (!m_hasTruncated) {
                    m_fullResponse += "…";
                    emit streamChunkReceived("…");
                    m_hasTruncated = true;
                }
                m_ignoreFurtherContent = true;
                continue;
            }

            if (filteredAnswer.length() > remaining) {
                const QString clipped = filteredAnswer.left(remaining);
                m_fullResponse += clipped + "…";
                emit streamChunkReceived(clipped + "…");
                m_hasTruncated = true;
                m_ignoreFurtherContent = true;
                continue;
            }

            m_fullResponse += filteredAnswer;
            emit streamChunkReceived(filteredAnswer);
            
        } else if (event == "workflow_started" || event == "node_started" || event == "node_finished") {
            // 跳过工作流相关事件
            qDebug() << "[DifyService] Skipping workflow event:" << event;
            continue;
        }
    }
}
