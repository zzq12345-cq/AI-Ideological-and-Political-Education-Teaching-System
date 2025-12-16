#ifndef DIFYSERVICE_H
#define DIFYSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief Dify Cloud API 服务类
 * 
 * 用于与 Dify Cloud 对话 API 进行通信
 */
class DifyService : public QObject
{
    Q_OBJECT

public:
    explicit DifyService(QObject *parent = nullptr);
    ~DifyService();

    /**
     * @brief 发送对话消息到 Dify
     * @param message 用户消息内容
     * @param conversationId 会话ID（可选，留空则创建新会话）
     */
    void sendMessage(const QString &message, const QString &conversationId = "");

    /**
     * @brief 设置 API Key
     * @param apiKey Dify 应用的 API Key
     */
    void setApiKey(const QString &apiKey);

    /**
     * @brief 设置 API 基础 URL
     * @param baseUrl API 基础地址
     */
    void setBaseUrl(const QString &baseUrl);

    /**
     * @brief 设置要使用的模型
     * @param model 模型名称（如 "glm-4", "gpt-4", "claude-3" 等）
     */
    void setModel(const QString &model);

    /**
     * @brief 获取当前会话 ID
     */
    QString currentConversationId() const;

    /**
     * @brief 清除当前会话
     */
    void clearConversation();

signals:
    /**
     * @brief 收到完整响应时发出
     * @param response AI 回复的完整内容
     */
    void messageReceived(const QString &response);

    /**
     * @brief 收到流式响应块时发出
     * @param chunk 响应片段
     */
    void streamChunkReceived(const QString &chunk);

    /**
     * @brief 收到思考过程时发出
     * @param thought 思考内容片段
     */
    void thinkingChunkReceived(const QString &thought);

    /**
     * @brief 新会话创建时发出
     * @param conversationId 新会话的 ID
     */
    void conversationCreated(const QString &conversationId);

    /**
     * @brief 请求开始时发出
     */
    void requestStarted();

    /**
     * @brief 请求完成时发出
     */
    void requestFinished();

    /**
     * @brief 发生错误时发出
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();
    void onReadyRead();
    void onSslErrors(const QList<QSslError> &errors);

private:
    void parseStreamResponse(const QByteArray &data);
    QString filterThinkTagsStreaming(const QString &text);
    void resetStreamFilters();

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply;
    QString m_apiKey;
    QString m_baseUrl;
    QString m_model;
    QString m_conversationId;
    QString m_userId;
    QString m_fullResponse;  // 累积完整响应
    QString m_streamBuffer;  // SSE 残留缓冲
    QString m_tagRemainder;  // 跨 chunk 的标签残留缓冲
    QString m_hiddenTagName; // 当前隐藏块标签名（如 think/analysis）
    bool m_ignoreFurtherContent = false;
    bool m_hasTruncated = false;
    int m_maxResponseChars = 10000;
};

#endif // DIFYSERVICE_H
