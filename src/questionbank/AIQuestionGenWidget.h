#ifndef AIQUESTIONGENWIDGET_H
#define AIQUESTIONGENWIDGET_H

#include <QWidget>

class ChatWidget;
class DifyService;
class QVBoxLayout;
class QFrame;
class QPushButton;
class PaperService;

/**
 * @brief AI 对话式出题组件
 *
 * 复用 ChatWidget 和 DifyService，通过自然语言对话生成试题。
 * 支持流式响应、快捷提示按钮和保存到题库。
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
    void onStreamChunk(const QString &chunk);
    void onThinkingChunk(const QString &thought);
    void onRequestStarted();
    void onRequestFinished();
    void onNewConversation();
    void onSaveToBank();
    void onErrorOccurred(const QString &error);

private:
    void setupUI();
    void setupDifyService();
    void showWelcome();

    // UI 组件
    ChatWidget *m_chatWidget = nullptr;
    QFrame *m_bottomBar = nullptr;
    QPushButton *m_newChatBtn = nullptr;
    QPushButton *m_saveBtn = nullptr;

    // 服务
    DifyService *m_difyService = nullptr;

    // 状态
    bool m_isGenerating = false;
    bool m_hasPendingAIPlaceholder = false;
    bool m_isSavingToBank = false;
    QString m_lastAIResponse;  // 最后一次完整 AI 回复（用于保存）
};

#endif // AIQUESTIONGENWIDGET_H
