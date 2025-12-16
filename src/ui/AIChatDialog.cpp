#include "AIChatDialog.h"
#include "../services/DifyService.h"
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

AIChatDialog::AIChatDialog(DifyService *difyService, QWidget *parent)
    : QDialog(parent)
    , m_chatWidget(nullptr)
    , m_historyWidget(nullptr)
    , m_difyService(difyService)
    , m_isStreaming(false)
{
    setupUI();
    connectDifyService();
    loadMockHistory();
}

AIChatDialog::~AIChatDialog()
{
}

void AIChatDialog::setupUI()
{
    // 窗口属性
    setWindowTitle("AI 智能助手");
    setMinimumSize(800, 600); // 加宽以容纳侧边栏
    resize(900, 700);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 标题栏
    QFrame *titleBar = new QFrame();
    titleBar->setObjectName("dialogTitleBar");
    titleBar->setFixedHeight(56);
    titleBar->setStyleSheet(
        "QFrame#dialogTitleBar {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "       stop:0 #2b7de9, stop:1 #1a5fc4);"
        "   border-top-left-radius: 8px;"
        "   border-top-right-radius: 8px;"
        "}"
    );
    
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);
    
    // AI 图标
    QLabel *iconLabel = new QLabel("🤖");
    iconLabel->setStyleSheet("font-size: 24px; background: transparent;");
    
    // 标题
    QLabel *titleLabel = new QLabel("AI 智能助手");
    titleLabel->setStyleSheet(
        "QLabel {"
        "   color: #ffffff;"
        "   font-size: 18px;"
        "   font-weight: 600;"
        "   background: transparent;"
        "}"
    );
    
    titleLayout->addWidget(iconLabel);
    titleLayout->addSpacing(8);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    
    mainLayout->addWidget(titleBar);
    
    // 内容区域布局（侧边栏 + 聊天窗口）
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // 1. 侧边栏
    m_historyWidget = new ChatHistoryWidget();
    contentLayout->addWidget(m_historyWidget);
    
    // 2. 聊天组件
    m_chatWidget = new ChatWidget();
    m_chatWidget->setPlaceholderText("向AI助手提问...");
    contentLayout->addWidget(m_chatWidget, 1);
    
    mainLayout->addLayout(contentLayout);
    
    // 连接侧边栏信号
    connect(m_historyWidget, &ChatHistoryWidget::newChatRequested,
            this, &AIChatDialog::onNewChatRequested);
    connect(m_historyWidget, &ChatHistoryWidget::historyItemSelected,
            this, &AIChatDialog::onHistoryItemSelected);

    // 连接聊天组件信号
    connect(m_chatWidget, &ChatWidget::messageSent, 
            this, &AIChatDialog::onUserSendMessage);
    
    // 窗口整体样式
    setStyleSheet(
        "AIChatDialog {"
        "   background-color: #f5f7fa;"
        "   border-radius: 8px;"
        "}"
    );
    
    // 添加窗口阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 60));
    setGraphicsEffect(shadow);
}

void AIChatDialog::connectDifyService()
{
    if (!m_difyService) return;
    
    connect(m_difyService, &DifyService::streamChunkReceived,
            this, &AIChatDialog::onAIStreamChunk);
    connect(m_difyService, &DifyService::thinkingChunkReceived,
            this, &AIChatDialog::onAIThinkingChunk);
    connect(m_difyService, &DifyService::messageReceived,
            this, &AIChatDialog::onAIResponseReceived);
    connect(m_difyService, &DifyService::errorOccurred,
            this, &AIChatDialog::onAIError);
    connect(m_difyService, &DifyService::requestStarted,
            this, &AIChatDialog::onAIRequestStarted);
    connect(m_difyService, &DifyService::requestFinished,
            this, &AIChatDialog::onAIRequestFinished);
}

void AIChatDialog::loadMockHistory()
{
    if (!m_historyWidget) return;
    
    m_historyWidget->clearHistory();
    
    m_historyWidget->addHistoryItem("chat_1", "审批流程助手 对话 6", "12月12日 15:12");
    m_historyWidget->addHistoryItem("chat_2", "审批流程助手 对话 5", "12月12日 15:11");
    m_historyWidget->addHistoryItem("chat_3", "审批流程助手 对话 4", "12月11日 22:41");
    m_historyWidget->addHistoryItem("chat_4", "审批流程助手 对话 3", "12月11日 22:05");
    m_historyWidget->addHistoryItem("chat_5", "审批流程助手 对话 2", "11月27日 21:36");
    m_historyWidget->addHistoryItem("chat_6", "审批流程助手 对话", "11月27日 21:35");
}

void AIChatDialog::onNewChatRequested()
{
    clearChat();
    // 可以在这里触发 Dify 新建会话的逻辑，或仅重置 UI
    // 目前 clearChat 会清除 Dify conversationId
    // 可以在 UI 上添加一个新的空条目（可选）
}

void AIChatDialog::onHistoryItemSelected(const QString &conversationId)
{
    // 切换历史记录的逻辑
    // 目前仅作演示，清空当前聊天并显示提示
    m_chatWidget->clearMessages();
    addAIMessage(QString("已切换到历史对话: %1 (模拟功能)").arg(conversationId));
    
    // 在真实场景中，这里应该调用 DifyService 获取历史记录接口
}

void AIChatDialog::addUserMessage(const QString &message)
{
    m_chatWidget->addMessage(message, true);
}

void AIChatDialog::addAIMessage(const QString &message)
{
    m_chatWidget->addMessage(message, false);
}

void AIChatDialog::updateAIMessage(const QString &message)
{
    m_chatWidget->updateLastAIMessage(message);
}

void AIChatDialog::clearChat()
{
    m_chatWidget->clearMessages();
    if (m_difyService) {
        m_difyService->clearConversation();
    }
}

void AIChatDialog::onUserSendMessage(const QString &message)
{
    if (message.trimmed().isEmpty()) return;
    
    // 显示用户消息
    addUserMessage(message);
    
    // 清空累积响应
    m_currentResponse.clear();
    m_isStreaming = false;
    
    // 发送到 Dify
    if (m_difyService) {
        const QString concisePrefix = "请用简洁中文回答（不超过120字），不要使用Markdown/标签/代码块，直接回答：";
        m_difyService->sendMessage(concisePrefix + message);
    }
}

void AIChatDialog::onAIStreamChunk(const QString &chunk)
{
    // 首次收到流式响应时，添加一个空的 AI 消息气泡
    if (!m_isStreaming) {
        m_isStreaming = true;
        addAIMessage("");
    }
    
    // 累积响应并更新
    m_currentResponse += chunk;
    updateAIMessage(m_currentResponse);
}

void AIChatDialog::onAIThinkingChunk(const QString &thought)
{
    // 首次收到时，确保有 AI 消息气泡
    if (!m_isStreaming) {
        m_isStreaming = true;
        addAIMessage("");
    }
    
    // 更新思考过程区域
    m_chatWidget->updateLastAIThinking(thought);
}

void AIChatDialog::onAIResponseReceived(const QString &response)
{
    // 如果没有流式响应（非流式模式），直接添加完整消息
    if (!m_isStreaming) {
        addAIMessage(response);
    }
    // 如果是流式模式，消息已经通过 chunk 更新完毕
    
    m_currentResponse.clear();
    m_isStreaming = false;
}

void AIChatDialog::onAIError(const QString &error)
{
    addAIMessage("⚠️ 错误：" + error);
    m_currentResponse.clear();
    m_isStreaming = false;
}

void AIChatDialog::onAIRequestStarted()
{
    m_chatWidget->setInputEnabled(false);
}

void AIChatDialog::onAIRequestFinished()
{
    // 响应完成，自动折叠思考过程
    m_chatWidget->collapseThinking();
    
    m_chatWidget->setInputEnabled(true);
    m_chatWidget->focusInput();
}
