#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QTextDocument>
#include <memory>

// 前向声明
class MarkdownRenderer;

/**
 * @brief 现代化气泡对话风格的聊天组件
 * 
 * 使用 QScrollArea + QVBoxLayout 实现消息堆叠
 * 支持用户消息（蓝色气泡靠右）和 AI 消息（白色气泡靠左）
 */
class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();

    /**
     * @brief 添加消息到聊天区域
     * @param text 消息文本内容
     * @param isUser true=用户消息(蓝色靠右)，false=AI消息(白色靠左)
     */
    void addMessage(const QString &text, bool isUser);

    /**
     * @brief 更新最后一条 AI 消息（用于流式响应）
     * @param text 新的消息内容
     */
    void updateLastAIMessage(const QString &text);

    /**
     * @brief 更新最后一条 AI 消息的思考过程
     * @param thought 思考内容
     */
    void updateLastAIThinking(const QString &thought);

    /**
     * @brief 折叠思考过程区域
     */
    void collapseThinking();

    /**
     * @brief 展开思考过程区域
     */
    void expandThinking();

    /**
     * @brief 清空所有消息
     */
    void clearMessages();

    /**
     * @brief 设置输入框占位符文本
     */
    void setPlaceholderText(const QString &text);

    /**
     * @brief 启用/禁用Markdown渲染
     * @param enabled true=启用Markdown渲染，false=纯文本模式
     */
    void setMarkdownEnabled(bool enabled);

    /**
     * @brief 预填输入框文本
     */
    void setInputText(const QString &text);

    /**
     * @brief 设置输入框是否启用
     */
    void setInputEnabled(bool enabled);

    /**
     * @brief 获取输入框文本
     */
    QString inputText() const;

    /**
     * @brief 清空输入框
     */
    void clearInput();

    /**
     * @brief 聚焦输入框
     */
    void focusInput();

signals:
    /**
     * @brief 用户发送消息时发出
     * @param message 消息内容
     */
    void messageSent(const QString &message);

private slots:
    void onSendClicked();

private:
    void setupUI();
    void setupStyles();
    QWidget* createMessageBubble(const QString &text, bool isUser);
    void scrollToBottom();

    // Markdown渲染相关
    QString renderMessage(const QString &text, bool isUser);

    // UI 组件
    QScrollArea *m_scrollArea;
    QWidget *m_messageContainer;
    QVBoxLayout *m_messageLayout;
    QLineEdit *m_inputEdit;
    QPushButton *m_sendBtn;

    // 用于流式更新的最后一条 AI 消息
    QLabel *m_lastAIMessageLabel;
    
    // 用于显示思考过程的组件
    QWidget *m_lastAIThinkingWidget;
    QLabel *m_lastAIThinkingLabel;
    QPushButton *m_lastAIThinkingToggle;

    // Markdown渲染器
    std::unique_ptr<MarkdownRenderer> m_markdownRenderer;

    // Markdown渲染开关
    bool m_markdownEnabled;
    
    // 样式常量
    static const QString USER_BUBBLE_COLOR;
    static const QString AI_BUBBLE_COLOR;
    static const QString USER_TEXT_COLOR;
    static const QString AI_TEXT_COLOR;
};

#endif // CHATWIDGET_H
