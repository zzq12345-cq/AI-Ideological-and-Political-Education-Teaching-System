#ifndef AICHATDIALOG_H
#define AICHATDIALOG_H

#include <QDialog>
#include "ChatWidget.h"

class DifyService;

/**
 * @brief AI 对话框 - 独立的弹窗式聊天窗口
 */
class AIChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AIChatDialog(DifyService *difyService, QWidget *parent = nullptr);
    ~AIChatDialog();

    /**
     * @brief 添加用户消息
     */
    void addUserMessage(const QString &message);

    /**
     * @brief 添加 AI 消息
     */
    void addAIMessage(const QString &message);

    /**
     * @brief 更新最后一条 AI 消息（流式响应）
     */
    void updateAIMessage(const QString &message);

    /**
     * @brief 清空对话
     */
    void clearChat();

private slots:
    void onUserSendMessage(const QString &message);
    void onAIStreamChunk(const QString &chunk);
    void onAIThinkingChunk(const QString &thought);
    void onAIResponseReceived(const QString &response);
    void onAIError(const QString &error);
    void onAIRequestStarted();
    void onAIRequestFinished();

private:
    void setupUI();
    void connectDifyService();

    ChatWidget *m_chatWidget;
    DifyService *m_difyService;
    QString m_currentResponse;  // 累积流式响应
    bool m_isStreaming;
};

#endif // AICHATDIALOG_H
