#ifndef AIQUESTIONGENWIDGET_H
#define AIQUESTIONGENWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>

class ChatWidget;
class QVBoxLayout;
class QFrame;
class QPushButton;
class PaperService;

/**
 * @brief AI 对话式出题组件
 *
 * 直接调用智谱 GLM-5.1 API（OpenAI 兼容格式），通过自然语言对话生成试题。
 * 支持流式 SSE 响应、快捷提示按钮和保存到题库。
 */
class AIQuestionGenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AIQuestionGenWidget(QWidget *parent = nullptr);
    ~AIQuestionGenWidget();
    void setSavingToBank(bool saving);
    void showSaveSuccessMessage(int savedCount, bool directInsert);
    void showSaveErrorMessage(const QString &error);

signals:
    /// 请求将 AI 生成的内容保存到题库
    void saveRequested(const QString &content);

private slots:
    void onUserMessageSent(const QString &message);
    void onNewConversation();
    void onSaveToBank();

private:
    void setupUI();
    void setupZhipuService();
    void showWelcome();

    // 智谱 API 调用（流式 SSE）
    void sendToZhipu(const QString &userMessage);
    void processSSEData(const QByteArray &data);

    // UI 组件
    ChatWidget *m_chatWidget = nullptr;
    QFrame *m_bottomBar = nullptr;
    QPushButton *m_newChatBtn = nullptr;
    QPushButton *m_saveBtn = nullptr;

    // 网络
    QNetworkAccessManager *m_networkManager = nullptr;
    QPointer<QNetworkReply> m_currentReply;
    QString m_apiKey;
    QString m_baseUrl;

    // 对话历史（OpenAI 格式 messages）
    struct ChatMessage {
        QString role;    // "system" / "user" / "assistant"
        QString content;
    };
    QList<ChatMessage> m_conversationHistory;

    // 状态
    bool m_isGenerating = false;
    bool m_hasPendingAIPlaceholder = false;
    bool m_isSavingToBank = false;
    QString m_lastAIResponse;       // 最后一次完整 AI 回复（用于保存）
    QByteArray m_sseBuffer;         // SSE 数据缓冲区

    // 常量
    static constexpr const char* MODEL_NAME = "glm-5.1";
    static const QString systemPrompt();
};

#endif // AIQUESTIONGENWIDGET_H
