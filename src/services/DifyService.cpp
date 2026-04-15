#include "DifyService.h"
#include "../utils/NetworkRequestFactory.h"
#include <QNetworkRequest>
#include <QJsonArray>
#include <QUuid>
#include <QDebug>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QSettings>
#include <QCoreApplication>

DifyService::DifyService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_baseUrl("https://api.dify.ai/v1")
{
    // 初始化 SSE 解析器回调
    m_sseParser.setEventHandler([this](const QString &event, const QJsonObject &obj) {
        handleSseEvent(event, obj);
    });

    // 从 QSettings 读取持久化的用户 ID，如果不存在则生成新的并保存
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    m_userId = settings.value("dify/userId").toString();

    if (m_userId.isEmpty()) {
        m_userId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue("dify/userId", m_userId);
        qDebug() << "[DifyService] Created new persistent userId:" << m_userId;
    } else {
        qDebug() << "[DifyService] Loaded existing userId:" << m_userId;
    }
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

void DifyService::setUserId(const QString &userId)
{
    m_userId = userId;
    // 持久化到 QSettings
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("dify/userId", m_userId);
    qDebug() << "[DifyService] 用户 ID 已绑定:" << m_userId;
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

void DifyService::setCurrentConversationId(const QString &conversationId)
{
    m_conversationId = conversationId;
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
    m_sseParser.reset();
    resetStreamFilters();

    // 构建请求 URL
    QUrl url(m_baseUrl + "/chat-messages");
    QNetworkRequest request = createConfiguredRequest(url, 120000);

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

    qDebug() << "[DifyService] Sending message length:" << message.length();
    qDebug() << "[DifyService] Request URL:" << url.toString();
    qDebug() << "[DifyService] Request body size:" << jsonData.size();

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

    QByteArray data = m_currentReply->readAll();
    if (!data.isEmpty()) {
        m_sseParser.feed(data);
    }
}

void DifyService::onReplyFinished()
{
    qDebug() << "[DifyService] Reply finished, checking for errors...";

    if (!m_currentReply) {
        qDebug() << "[DifyService] No current reply";
        return;
    }

    // 对于流式响应，RemoteHostClosedError 是正常的（服务器完成发送后关闭连接）
    QNetworkReply::NetworkError error = m_currentReply->error();
    if (error != QNetworkReply::NoError && error != QNetworkReply::RemoteHostClosedError) {
        if (m_sseParser.hasPendingData()) {
            qDebug() << "[DifyService] Flush pending stream on error";
            m_sseParser.flush();
        }
        QString errorMsg = m_currentReply->errorString();
        int httpStatus = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = m_currentReply->readAll();

        qDebug() << "[DifyService] HTTP Error:" << httpStatus << "Error:" << errorMsg;
        qDebug() << "[DifyService] Network error code:" << m_currentReply->error();
        qDebug() << "[DifyService] Response data length:" << responseData.size();

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
        const bool hasStreamedText = !m_fullResponse.trimmed().isEmpty();
        qDebug() << "[DifyService] Request successful, response data length:" << responseData.length();

        if (m_sseParser.hasPendingData()) {
            qDebug() << "[DifyService] Flush pending stream on finish";
            m_sseParser.flush();
        }

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
                } else if (!hasStreamedText) {
                    qDebug() << "[DifyService] Warning: No answer field and no streamed text in response";
                    if (responseObj.contains("message")) {
                        emit errorOccurred(responseObj["message"].toString());
                    } else {
                        emit errorOccurred(QStringLiteral("AI 未返回有效内容，请稍后重试。"));
                    }
                } else {
                    qDebug() << "[DifyService] No answer field, but stream chunks were already received";
                }
            } else {
                qDebug() << "[DifyService] Failed to parse JSON response, using raw data";
                emit messageReceived(QString::fromUtf8(responseData));
            }
        } else {
            qDebug() << "[DifyService] Warning: Response data is empty!";
            if (!hasStreamedText) {
                emit errorOccurred(QStringLiteral("AI 未返回任何内容，请稍后重试。"));
            }
        }
    }

    emit requestFinished();

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void DifyService::onSslErrors(const QList<QSslError> &errors)
{
    if (!NetworkRequestFactory::handleSslErrors(m_currentReply, errors, "[DifyService]")) {
        emit errorOccurred("SSL 证书校验失败，已拒绝建立不安全连接");
    }
}

void DifyService::resetStreamFilters()
{
    m_tagRemainder.clear();
    m_hiddenTagName.clear();
    m_ignoreFurtherContent = false;
    m_hasTruncated = false;
}

// 统一创建已配置的网络请求，委托给 NetworkRequestFactory
QNetworkRequest DifyService::createConfiguredRequest(const QUrl &url, int timeout)
{
    return NetworkRequestFactory::createDifyRequest(url, m_apiKey, timeout);
}

// 统一处理流式文本，消除 message/agent_message/text_chunk 重复逻辑
void DifyService::handleStreamText(const QString &text)
{
    if (m_ignoreFurtherContent) {
        return;
    }

    // 过滤 think/analysis 标签
    QString filteredText = filterThinkTagsStreaming(text);
    if (filteredText.isEmpty()) {
        return;
    }

    // 检查是否超过最大字符数限制
    const int remaining = m_maxResponseChars - m_fullResponse.length();
    if (remaining <= 0) {
        if (!m_hasTruncated) {
            m_fullResponse += "…";
            emit streamChunkReceived("…");
            m_hasTruncated = true;
        }
        m_ignoreFurtherContent = true;
        return;
    }

    // 处理需要截断的情况
    if (filteredText.length() > remaining) {
        const QString clipped = filteredText.left(remaining);
        m_fullResponse += clipped + "…";
        emit streamChunkReceived(clipped + "…");
        m_hasTruncated = true;
        m_ignoreFurtherContent = true;
        return;
    }

    // 正常累加并发射信号
    m_fullResponse += filteredText;
    emit streamChunkReceived(filteredText);
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

    // 只压缩超过2个的连续换行，保留正常的段落分隔
    static const QRegularExpression multiNewlineRe("\\n{3,}");
    output.replace(multiNewlineRe, "\n\n");
    // 注意：不再调用 trimmed()，保留流式响应中的换行符
    return output;
}

// SSE 事件业务处理（由 SseStreamParser 回调）
void DifyService::handleSseEvent(const QString &event, const QJsonObject &obj)
{
    if (event == "message" || event == "agent_message") {
        QString answer = obj["answer"].toString();
        handleStreamText(answer);

    } else if (event == "message_end") {
        QString convId = obj["conversation_id"].toString();
        if (!convId.isEmpty() && convId != m_conversationId) {
            m_conversationId = convId;
            emit conversationCreated(convId);
        }

    } else if (event == "error") {
        QString errorMsg = obj["message"].toString();
        emit errorOccurred(errorMsg);

    } else if (event == "agent_thought") {
        QString thought = obj["thought"].toString();
        if (!thought.isEmpty()) {
            emit thinkingChunkReceived(thought);
        }

    } else if (event == "text_chunk") {
        QJsonObject dataObj = obj["data"].toObject();
        QString text = dataObj["text"].toString();
        if (text.isEmpty()) {
            text = obj["text"].toString();
        }
        handleStreamText(text);
    }
    // workflow_started / node_started / node_finished / workflow_finished: 忽略
}

void DifyService::fetchConversations(int limit)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API Key 未设置");
        return;
    }

    QUrl url(m_baseUrl + "/conversations");
    QUrlQuery query;
    query.addQueryItem("user", m_userId);
    query.addQueryItem("limit", QString::number(limit));
    query.addQueryItem("sort_by", "-updated_at");
    url.setQuery(query);

    QNetworkRequest request = createConfiguredRequest(url);

    qDebug() << "[DifyService] Fetching conversations from:" << url.toString();

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError && 
            reply->error() != QNetworkReply::RemoteHostClosedError) {
            qDebug() << "[DifyService] Fetch conversations error:" << reply->errorString();
            emit errorOccurred(QString("获取对话列表失败: %1").arg(reply->errorString()));
            return;
        }

        QByteArray data = reply->readAll();
        qDebug() << "[DifyService] Conversations response:" << data.left(500);

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("data")) {
                QJsonArray conversations = obj["data"].toArray();
                qDebug() << "[DifyService] Received" << conversations.size() << "conversations";
                emit conversationsReceived(conversations);
            }
        }
    });
}

void DifyService::fetchMessages(const QString &conversationId, int limit)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API Key 未设置");
        return;
    }

    if (conversationId.isEmpty()) {
        emit errorOccurred("对话ID不能为空");
        return;
    }

    QUrl url(m_baseUrl + "/messages");
    QUrlQuery query;
    query.addQueryItem("user", m_userId);
    query.addQueryItem("conversation_id", conversationId);
    query.addQueryItem("limit", QString::number(limit));
    url.setQuery(query);

    QNetworkRequest request = createConfiguredRequest(url);

    qDebug() << "[DifyService] Fetching messages for conversation:" << conversationId;

    QNetworkReply *reply = m_networkManager->get(request);
    QString convId = conversationId; // 捕获变量
    connect(reply, &QNetworkReply::finished, this, [this, reply, convId]() {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError && 
            reply->error() != QNetworkReply::RemoteHostClosedError) {
            qDebug() << "[DifyService] Fetch messages error:" << reply->errorString();
            emit errorOccurred(QString("获取消息历史失败: %1").arg(reply->errorString()));
            return;
        }

        QByteArray data = reply->readAll();
        qDebug() << "[DifyService] Messages response:" << data.left(500);

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("data")) {
                QJsonArray messages = obj["data"].toArray();
                qDebug() << "[DifyService] Received" << messages.size() << "messages";
                emit messagesReceived(messages, convId);
            }
        }
    });
}

void DifyService::fetchAppInfo()
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API Key 未设置");
        return;
    }

    // Dify API: GET /meta 获取应用元数据信息
    QUrl url(m_baseUrl + "/meta");
    QUrlQuery query;
    query.addQueryItem("user", m_userId);
    url.setQuery(query);

    QNetworkRequest request = createConfiguredRequest(url);

    qDebug() << "[DifyService] Fetching app info from:" << url.toString();

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError && 
            reply->error() != QNetworkReply::RemoteHostClosedError) {
            qDebug() << "[DifyService] Fetch app info error:" << reply->errorString();
            return;
        }

        QByteArray data = reply->readAll();
        qDebug() << "[DifyService] App info response:" << data.left(500);

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QString name = obj["name"].toString();
            QString introduction = obj["opening_statement"].toString();
            
            if (introduction.isEmpty()) {
                introduction = obj["user_input_form"].toString();
            }
            
            qDebug() << "[DifyService] App name:" << name << "Introduction:" << introduction.left(50);
            emit appInfoReceived(name, introduction);
        }
    });
}
