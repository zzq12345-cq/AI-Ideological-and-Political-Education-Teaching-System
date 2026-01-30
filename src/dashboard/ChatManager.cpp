#include "ChatManager.h"
#include "../services/DifyService.h"
#include "../ui/ChatWidget.h"
#include "../ui/ChatHistoryWidget.h"
#include "../services/PPTXGenerator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDebug>
#include <QRegularExpression>

namespace {
    constexpr int STREAM_THROTTLE_MS = 50;  // 流式更新节流间隔（毫秒）
}

ChatManager::ChatManager(QWidget *parentWidget, QObject *parent)
    : QObject(parent)
    , m_parentWidget(parentWidget)
    , m_chatContainer(nullptr)
    , m_difyService(nullptr)
    , m_bubbleChatWidget(nullptr)
    , m_chatHistoryWidget(nullptr)
    , m_pptxGenerator(nullptr)
    , m_streamUpdateTimer(nullptr)
    , m_streamUpdatePending(false)
    , m_isConversationStarted(false)
    , m_pptSimulationTimer(nullptr)
    , m_pptSimulationStep(0)
    , m_pptQuestionStep(0)
    , m_pptTypingTimer(nullptr)
    , m_pptTypingIndex(0)
{
    setupChatWidget();
    setupConnections();
}

ChatManager::~ChatManager()
{
    // DifyService 和其他组件会随父对象自动销毁
}

void ChatManager::setupChatWidget()
{
    // 创建聊天容器
    m_chatContainer = new QWidget(m_parentWidget);
    m_chatContainer->setObjectName("chatContainer");

    QVBoxLayout *containerLayout = new QVBoxLayout(m_chatContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // 创建气泡样式聊天组件
    m_bubbleChatWidget = new ChatWidget(m_chatContainer);
    containerLayout->addWidget(m_bubbleChatWidget);

    // 创建 DifyService
    m_difyService = new DifyService(this);

    // 创建历史记录组件
    m_chatHistoryWidget = new ChatHistoryWidget(m_parentWidget);

    // 创建 PPT 生成器
    m_pptxGenerator = new PPTXGenerator(this);

    // 流式更新定时器（节流）
    m_streamUpdateTimer = new QTimer(this);
    m_streamUpdateTimer->setSingleShot(true);
    m_streamUpdateTimer->setInterval(STREAM_THROTTLE_MS);

    qDebug() << "[ChatManager] Chat components initialized";
}

void ChatManager::setupConnections()
{
    // 连接 ChatWidget 发送消息信号
    connect(m_bubbleChatWidget, &ChatWidget::messageSent,
            this, &ChatManager::onSendChatMessage);

    // 连接 DifyService 信号
    connect(m_difyService, &DifyService::messageReceived,
            this, &ChatManager::onAIResponseReceived);
    connect(m_difyService, &DifyService::streamChunkReceived,
            this, &ChatManager::onAIStreamChunk);
    connect(m_difyService, &DifyService::thinkingChunkReceived,
            this, &ChatManager::onAIThinkingChunk);
    connect(m_difyService, &DifyService::errorOccurred,
            this, &ChatManager::onAIError);
    connect(m_difyService, &DifyService::requestStarted,
            this, &ChatManager::onAIRequestStarted);
    connect(m_difyService, &DifyService::requestFinished,
            this, &ChatManager::onAIRequestFinished);

    // 连接流式更新定时器（修复：在此处一次性连接，避免重复连接）
    connect(m_streamUpdateTimer, &QTimer::timeout, this, [this]() {
        if (m_bubbleChatWidget) {
            m_bubbleChatWidget->updateLastAIMessage(m_currentAIResponse);
        }
        m_streamUpdatePending = false;
    });

    // 连接历史记录选择信号
    connect(m_chatHistoryWidget, &ChatHistoryWidget::historyItemSelected,
            this, [this](const QString &convId) {
        qDebug() << "[ChatManager] Conversation selected:" << convId;
        // 加载选中的对话历史
        m_difyService->fetchMessages(convId);
    });

    qDebug() << "[ChatManager] Signal connections established";
}

void ChatManager::sendMessage(const QString &message)
{
    if (message.trimmed().isEmpty()) {
        return;
    }

    // 添加用户消息到界面
    m_bubbleChatWidget->addMessage(message, true);

    // 检查是否是 PPT 生成请求
    if (isPPTGenerationRequest(message)) {
        // TODO: 处理 PPT 生成流程
        qDebug() << "[ChatManager] PPT generation request detected";
    }

    // 发送到 DifyService
    m_difyService->sendMessage(message);

    if (!m_isConversationStarted) {
        m_isConversationStarted = true;
        emit conversationStarted();
    }
}

void ChatManager::clearConversation()
{
    m_difyService->clearConversation();
    m_bubbleChatWidget->clearMessages();
    m_currentAIResponse.clear();
    m_isConversationStarted = false;
    emit conversationCleared();
    qDebug() << "[ChatManager] Conversation cleared";
}

void ChatManager::setApiKey(const QString &apiKey)
{
    m_difyService->setApiKey(apiKey);
    qDebug() << "[ChatManager] API Key set";
}

bool ChatManager::isPPTGenerationRequest(const QString &message)
{
    // 检测 PPT 生成相关关键词
    static const QRegularExpression pptPattern(
        "(?:生成|创建|制作|做一个?).*(?:PPT|ppt|幻灯片|演示文稿)",
        QRegularExpression::CaseInsensitiveOption
    );
    return pptPattern.match(message).hasMatch();
}

void ChatManager::onSendChatMessage()
{
    QString message = m_bubbleChatWidget->inputText();
    if (!message.isEmpty()) {
        m_bubbleChatWidget->clearInput();
        sendMessage(message);
    }
}

void ChatManager::onAIResponseReceived(const QString &response)
{
    qDebug() << "[ChatManager] AI response received, length:" << response.length();
    emit messageReceived(response);
}

void ChatManager::onAIStreamChunk(const QString &chunk)
{
    m_currentAIResponse += chunk;

    // 节流更新 UI（连接已在 setupConnections 中完成）
    if (!m_streamUpdatePending) {
        m_streamUpdatePending = true;
        m_streamUpdateTimer->start();
    }

    emit streamChunkReceived(chunk);
}

void ChatManager::onAIThinkingChunk(const QString &thought)
{
    m_bubbleChatWidget->updateLastAIThinking(thought);
    emit thinkingChunkReceived(thought);
}

void ChatManager::onAIError(const QString &error)
{
    qDebug() << "[ChatManager] AI error:" << error;
    m_bubbleChatWidget->addMessage("错误: " + error, false);
    emit errorOccurred(error);
}

void ChatManager::onAIRequestStarted()
{
    qDebug() << "[ChatManager] AI request started";
    m_currentAIResponse.clear();

    // 添加空的 AI 消息作为占位符
    m_bubbleChatWidget->addMessage("", false);
    m_bubbleChatWidget->setInputEnabled(false);

    emit requestStarted();
}

void ChatManager::onAIRequestFinished()
{
    qDebug() << "[ChatManager] AI request finished";

    // 最后一次更新确保完整内容显示
    if (!m_currentAIResponse.isEmpty()) {
        m_bubbleChatWidget->updateLastAIMessage(m_currentAIResponse);
    }

    // 折叠思考过程
    m_bubbleChatWidget->collapseThinking();
    m_bubbleChatWidget->setInputEnabled(true);
    m_bubbleChatWidget->focusInput();

    emit requestFinished();
}
