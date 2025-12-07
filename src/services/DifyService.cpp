#include "DifyService.h"
#include <QNetworkRequest>
#include <QJsonArray>
#include <QUuid>
#include <QDebug>

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

    // 构建请求 URL
    QUrl url(m_baseUrl + "/chat-messages");
    QNetworkRequest request(url);

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    // 构建请求体
    QJsonObject body;
    body["inputs"] = QJsonObject();
    body["query"] = message;
    body["response_mode"] = "streaming";  // 使用流式响应
    body["user"] = m_userId;

    // 如果有会话 ID，添加到请求中
    QString convId = conversationId.isEmpty() ? m_conversationId : conversationId;
    if (!convId.isEmpty()) {
        body["conversation_id"] = convId;
    }

    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson();

    qDebug() << "[DifyService] Sending message:" << message;
    qDebug() << "[DifyService] Request URL:" << url.toString();

    emit requestStarted();

    // 发送 POST 请求
    m_currentReply = m_networkManager->post(request, jsonData);

    // 连接信号
    connect(m_currentReply, &QNetworkReply::finished, this, &DifyService::onReplyFinished);
    connect(m_currentReply, &QNetworkReply::readyRead, this, &DifyService::onReadyRead);
    connect(m_currentReply, &QNetworkReply::sslErrors, this, &DifyService::onSslErrors);
}

void DifyService::onReadyRead()
{
    if (!m_currentReply) return;

    QByteArray data = m_currentReply->readAll();
    parseStreamResponse(data);
}

void DifyService::onReplyFinished()
{
    if (!m_currentReply) return;

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString errorMsg = m_currentReply->errorString();
        QByteArray responseData = m_currentReply->readAll();

        // 尝试解析错误响应
        QJsonDocument errorDoc = QJsonDocument::fromJson(responseData);
        if (!errorDoc.isNull() && errorDoc.isObject()) {
            QJsonObject errorObj = errorDoc.object();
            if (errorObj.contains("message")) {
                errorMsg = errorObj["message"].toString();
            }
        }

        qDebug() << "[DifyService] Error:" << errorMsg;
        emit errorOccurred(errorMsg);
    } else {
        // 发送完整响应
        if (!m_fullResponse.isEmpty()) {
            emit messageReceived(m_fullResponse);
        }
    }

    emit requestFinished();

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void DifyService::onSslErrors(const QList<QSslError> &errors)
{
    for (const QSslError &error : errors) {
        qDebug() << "[DifyService] SSL Error:" << error.errorString();
    }
    // 忽略 SSL 错误（仅用于开发环境）
    if (m_currentReply) {
        m_currentReply->ignoreSslErrors();
    }
}

void DifyService::parseStreamResponse(const QByteArray &data)
{
    // Dify 使用 SSE (Server-Sent Events) 格式
    // 每行以 "data: " 开头，后跟 JSON 数据
    QString dataStr = QString::fromUtf8(data);
    QStringList lines = dataStr.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        if (!line.startsWith("data: ")) {
            continue;
        }

        QString jsonStr = line.mid(6);  // 去掉 "data: " 前缀

        if (jsonStr.trimmed().isEmpty()) {
            continue;
        }

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (doc.isNull() || !doc.isObject()) {
            continue;
        }

        QJsonObject obj = doc.object();
        QString event = obj["event"].toString();

        if (event == "message") {
            // 普通消息块
            QString answer = obj["answer"].toString();
            m_fullResponse += answer;
            emit streamChunkReceived(answer);

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

        } else if (event == "agent_message") {
            // Agent 消息（如果使用 Agent 类型应用）
            QString answer = obj["answer"].toString();
            m_fullResponse += answer;
            emit streamChunkReceived(answer);
        }
    }
}
