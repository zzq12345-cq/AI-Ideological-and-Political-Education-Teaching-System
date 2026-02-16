#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QTimer>

// 前向声明
class DifyService;
class ChatWidget;
class ChatHistoryWidget;
class PPTXGenerator;

/**
 * @brief AI 对话管理器
 *
 * 从 ModernMainWindow 中提取的 AI 对话相关功能，
 * 负责管理 DifyService、ChatWidget 和对话历史。
 */
class ChatManager : public QObject
{
    Q_OBJECT

public:
    explicit ChatManager(QWidget *parentWidget, QObject *parent = nullptr);
    ~ChatManager();

    // 获取聊天容器组件
    QWidget* chatContainer() const { return m_chatContainer; }

    // 获取历史记录组件
    ChatHistoryWidget* historyWidget() const { return m_chatHistoryWidget; }

    // 获取 DifyService 实例
    DifyService* difyService() const { return m_difyService; }

    // 发送消息
    void sendMessage(const QString &message);

    // 清除当前对话
    void clearConversation();

    // 设置 API Key
    void setApiKey(const QString &apiKey);

    // 检测是否是 PPT 生成请求
    bool isPPTGenerationRequest(const QString &message);

signals:
    // AI 请求状态信号
    void requestStarted();
    void requestFinished();

    // 消息接收信号
    void messageReceived(const QString &response);
    void streamChunkReceived(const QString &chunk);
    void thinkingChunkReceived(const QString &thought);

    // 错误信号
    void errorOccurred(const QString &error);

    // 对话状态信号
    void conversationStarted();
    void conversationCleared();

private slots:
    void onSendChatMessage();
    void onAIResponseReceived(const QString &response);
    void onAIStreamChunk(const QString &chunk);
    void onAIThinkingChunk(const QString &thought);
    void onAIError(const QString &error);
    void onAIRequestStarted();
    void onAIRequestFinished();

private:
    void setupChatWidget();
    void setupConnections();

    // 核心组件
    QWidget *m_parentWidget;
    QWidget *m_chatContainer;
    DifyService *m_difyService;
    ChatWidget *m_bubbleChatWidget;
    ChatHistoryWidget *m_chatHistoryWidget;
    PPTXGenerator *m_pptxGenerator;

    // 流式响应状态
    QString m_currentAIResponse;
    QTimer *m_streamUpdateTimer;
    bool m_streamUpdatePending;
    bool m_isConversationStarted;
    bool m_waitingForFirstChunk;

    // PPT 工作流状态
    QTimer *m_pptSimulationTimer;
    int m_pptSimulationStep;
    QString m_pendingPPTPath;
    int m_pptQuestionStep;
    QStringList m_pptUserAnswers;
    QTimer *m_pptTypingTimer;
    QString m_pptTypingText;
    int m_pptTypingIndex;
};

#endif // CHATMANAGER_H
