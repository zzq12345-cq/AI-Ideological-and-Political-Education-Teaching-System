#include "AIQuestionGenWidget.h"
#include "../ui/ChatWidget.h"
#include "../services/DifyService.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMessageBox>

namespace {
// 绿色教育风格配色（与 SmartPaperWidget / QuestionBankWindow 统一）
const QString ACCENT_GREEN = "#2E7D32";
const QString ACCENT_GREEN_HOVER = "#1B5E20";
const QString ACCENT_GREEN_LIGHT = "#E8F5E9";
const QString ACCENT_RED = "#D32F2F";
const QString ACCENT_RED_HOVER = "#B71C1C";
const QString ACCENT_BLUE = "#1565C0";
const QString ACCENT_BLUE_LIGHT = "#E3F2FD";
const QString TEXT_SECONDARY = "#6B7280";
const QString BORDER_SUBTLE = "#E5E7EB";
const QString BG_PAGE = "#F8F9FA";
}

AIQuestionGenWidget::AIQuestionGenWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupDifyService();
    showWelcome();
}

AIQuestionGenWidget::~AIQuestionGenWidget() = default;

void AIQuestionGenWidget::setSavingToBank(bool saving)
{
    m_isSavingToBank = saving;
    if (!m_saveBtn) {
        return;
    }

    m_saveBtn->setEnabled(!saving && !m_lastAIResponse.trimmed().isEmpty());
    m_saveBtn->setText(saving ? "保存中..." : "💾 保存到题库");
}

void AIQuestionGenWidget::showSaveSuccessMessage(int savedCount, bool directInsert)
{
    setSavingToBank(false);
    QMessageBox::information(
        this,
        "保存成功",
        directInsert
            ? QString("AI 生成的试题已通过工作流直接入库（共 %1 题）。").arg(savedCount)
            : QString("AI 生成的试题已保存到题库（共 %1 题）。").arg(savedCount)
    );
}

void AIQuestionGenWidget::showSaveErrorMessage(const QString &error)
{
    setSavingToBank(false);
    QMessageBox::warning(this, "保存失败", error);
}

void AIQuestionGenWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setStyleSheet(QString("AIQuestionGenWidget { background-color: %1; }").arg(BG_PAGE));

    // ===================== 聊天区域 =====================
    m_chatWidget = new ChatWidget(this);
    m_chatWidget->setMarkdownEnabled(true);
    m_chatWidget->setPlaceholderText("输入出题需求，例如：帮我出5道关于宪法的选择题...");
    mainLayout->addWidget(m_chatWidget, 1);

    // 连接用户消息信号
    connect(m_chatWidget, &ChatWidget::messageSent,
            this, &AIQuestionGenWidget::onUserMessageSent);

    // ===================== 底部操作栏 =====================
    m_bottomBar = new QFrame(this);
    m_bottomBar->setObjectName("aiGenBottomBar");
    m_bottomBar->setStyleSheet(
        "QFrame#aiGenBottomBar {"
        "    background-color: #FFFFFF;"
        "    border-top: 1px solid " + BORDER_SUBTLE + ";"
        "    border-radius: 0px;"
        "}"
    );
    m_bottomBar->setFixedHeight(56);

    auto *barShadow = new QGraphicsDropShadowEffect(m_bottomBar);
    barShadow->setBlurRadius(12);
    barShadow->setOffset(0, -2);
    barShadow->setColor(QColor(0, 0, 0, 15));
    m_bottomBar->setGraphicsEffect(barShadow);

    auto *barLayout = new QHBoxLayout(m_bottomBar);
    barLayout->setContentsMargins(20, 8, 20, 8);
    barLayout->setSpacing(12);

    // 新对话按钮
    m_newChatBtn = new QPushButton("🔄 新对话");
    m_newChatBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: %1;"
        "    border: 1.5px solid %2;"
        "    border-radius: 10px;"
        "    padding: 8px 20px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F5F5F5;"
        "}"
    ).arg(TEXT_SECONDARY, BORDER_SUBTLE));
    m_newChatBtn->setCursor(Qt::PointingHandCursor);
    connect(m_newChatBtn, &QPushButton::clicked,
            this, &AIQuestionGenWidget::onNewConversation);

    barLayout->addWidget(m_newChatBtn);
    barLayout->addStretch();

    // 保存到题库按钮
    m_saveBtn = new QPushButton("💾 保存到题库");
    m_saveBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %1, stop:1 #388E3C);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 8px 24px;"
        "    font-size: 13px;"
        "    font-weight: 700;"
        "    letter-spacing: 0.3px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %2, stop:1 #2E7D32);"
        "}"
        "QPushButton:disabled {"
        "    background-color: #E5E7EB;"
        "    color: #9CA3AF;"
        "}"
    ).arg(ACCENT_GREEN, ACCENT_GREEN_HOVER));
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setEnabled(false);  // 初始禁用，等 AI 生成完再启用
    connect(m_saveBtn, &QPushButton::clicked,
            this, &AIQuestionGenWidget::onSaveToBank);

    barLayout->addWidget(m_saveBtn);

    mainLayout->addWidget(m_bottomBar);
}

void AIQuestionGenWidget::setupDifyService()
{
    m_difyService = new DifyService(this);

    // 优先使用专用出题 API Key，否则回退到通用 Key
    QString apiKey = qEnvironmentVariable("DIFY_QUESTION_GEN_API_KEY");
    if (apiKey.isEmpty()) {
        apiKey = qEnvironmentVariable("DIFY_API_KEY");
    }
    m_difyService->setApiKey(apiKey);

    // 连接信号
    connect(m_difyService, &DifyService::streamChunkReceived,
            this, &AIQuestionGenWidget::onStreamChunk);
    connect(m_difyService, &DifyService::thinkingChunkReceived,
            this, &AIQuestionGenWidget::onThinkingChunk);
    connect(m_difyService, &DifyService::requestStarted,
            this, &AIQuestionGenWidget::onRequestStarted);
    connect(m_difyService, &DifyService::requestFinished,
            this, &AIQuestionGenWidget::onRequestFinished);
    connect(m_difyService, &DifyService::errorOccurred,
            this, &AIQuestionGenWidget::onErrorOccurred);
    connect(m_difyService, &DifyService::conversationCreated,
            this, [](const QString &id) {
                qDebug() << "[AIQuestionGen] 新会话创建:" << id;
            });
}

void AIQuestionGenWidget::showWelcome()
{
    // AI 欢迎消息
    const QString welcome =
        "你好！我是 **AI 出题助手** 💡\n\n"
        "我可以帮你生成各类思政试题。请告诉我你的出题需求，例如：\n\n"
        "- 帮我出 **5道关于宪法** 的选择题\n"
        "- 生成一道 **中等难度** 的材料论述题\n"
        "- 围绕 **\"法治与道德\"** 出一套综合练习\n"
        "- 出 **3道判断说理题**，主题：青少年法律意识\n\n"
        "直接输入需求，或点击下方快捷按钮开始 👇";

    m_chatWidget->addMessage(welcome, false);

    // 快捷回复按钮
    m_chatWidget->addQuickReplyOptions({
        "帮我出5道选择题，主题：宪法",
        "生成一道材料论述题，难度中等",
        "围绕'法治'出综合练习（选择+判断+论述）",
        "出3道判断说理题，主题：青少年法律意识"
    });
}

// ===================== 消息处理 =====================

void AIQuestionGenWidget::onUserMessageSent(const QString &message)
{
    if (message.trimmed().isEmpty() || m_isGenerating) {
        return;
    }

    qDebug() << "[AIQuestionGen] 用户消息:" << message.left(50);

    // 发送到 Dify
    m_difyService->sendMessage(message, m_difyService->currentConversationId());
}

void AIQuestionGenWidget::onStreamChunk(const QString &chunk)
{
    m_lastAIResponse += chunk;
    m_chatWidget->updateLastAIMessage(m_lastAIResponse);
}

void AIQuestionGenWidget::onThinkingChunk(const QString &thought)
{
    m_chatWidget->updateLastAIThinking(thought);
}

void AIQuestionGenWidget::onRequestStarted()
{
    m_isGenerating = true;
    m_hasPendingAIPlaceholder = true;
    m_lastAIResponse.clear();
    m_saveBtn->setEnabled(false);
    m_chatWidget->setInputEnabled(false);

    // 创建空的 AI 消息气泡用于流式填充
    m_chatWidget->addMessage("", false);
    m_chatWidget->showTypingIndicator();
}

void AIQuestionGenWidget::onRequestFinished()
{
    m_isGenerating = false;
    m_hasPendingAIPlaceholder = false;
    m_chatWidget->setInputEnabled(true);
    m_chatWidget->hideTypingIndicator();
    m_chatWidget->collapseThinking();

    // 有内容时启用保存按钮
    if (!m_lastAIResponse.trimmed().isEmpty() && !m_isSavingToBank) {
        m_saveBtn->setEnabled(true);
    }

    qDebug() << "[AIQuestionGen] AI 回复完成，长度:" << m_lastAIResponse.length();
}

void AIQuestionGenWidget::onErrorOccurred(const QString &error)
{
    m_isGenerating = false;
    const bool hadPlaceholder = m_hasPendingAIPlaceholder;
    m_hasPendingAIPlaceholder = false;
    m_chatWidget->setInputEnabled(true);
    m_chatWidget->hideTypingIndicator();
    m_chatWidget->collapseThinking();

    const QString errorMessage = "⚠️ 出题失败：" + error + "\n\n请稍后重试。";
    if (hadPlaceholder) {
        m_chatWidget->updateLastAIMessage(errorMessage);
    } else {
        m_chatWidget->addMessage(errorMessage, false);
    }
    qWarning() << "[AIQuestionGen] 错误:" << error;
}

// ===================== 操作按钮 =====================

void AIQuestionGenWidget::onNewConversation()
{
    m_chatWidget->clearMessages();
    m_difyService->clearConversation();
    m_lastAIResponse.clear();
    m_saveBtn->setEnabled(false);

    showWelcome();
    qDebug() << "[AIQuestionGen] 新对话已创建";
}

void AIQuestionGenWidget::onSaveToBank()
{
    if (m_lastAIResponse.trimmed().isEmpty() || m_isSavingToBank) {
        return;
    }

    setSavingToBank(true);
    emit saveRequested(m_lastAIResponse);
    qDebug() << "[AIQuestionGen] 开始保存 AI 生成内容";
}
