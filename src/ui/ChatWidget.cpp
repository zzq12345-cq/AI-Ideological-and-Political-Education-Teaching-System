#include "ChatWidget.h"
#include <QScrollBar>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>

// 样式常量定义
const QString ChatWidget::USER_BUBBLE_COLOR = "#2b7de9";
const QString ChatWidget::AI_BUBBLE_COLOR = "#ffffff";
const QString ChatWidget::USER_TEXT_COLOR = "#ffffff";
const QString ChatWidget::AI_TEXT_COLOR = "#1a1a1a";

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_inputEdit(nullptr)
    , m_sendBtn(nullptr)
    , m_lastAIMessageLabel(nullptr)
{
    setupUI();
    setupStyles();
}

ChatWidget::~ChatWidget()
{
}

void ChatWidget::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // ========== 消息显示区域 ==========
    m_scrollArea = new QScrollArea();
    m_scrollArea->setObjectName("chatScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    // 消息容器
    m_messageContainer = new QWidget();
    m_messageContainer->setObjectName("messageContainer");
    
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(20, 20, 20, 20);
    m_messageLayout->setSpacing(16);
    m_messageLayout->setAlignment(Qt::AlignTop);
    
    // 添加弹性空间，使消息从顶部开始
    m_messageLayout->addStretch();
    
    m_scrollArea->setWidget(m_messageContainer);
    mainLayout->addWidget(m_scrollArea, 1);
    
    // ========== 底部输入区域 ==========
    QFrame *inputFrame = new QFrame();
    inputFrame->setObjectName("inputFrame");
    inputFrame->setFixedHeight(80);
    
    QHBoxLayout *inputLayout = new QHBoxLayout(inputFrame);
    inputLayout->setContentsMargins(20, 16, 20, 16);
    inputLayout->setSpacing(12);
    
    // 输入框
    m_inputEdit = new QLineEdit();
    m_inputEdit->setObjectName("chatInputEdit");
    m_inputEdit->setPlaceholderText("输入消息...");
    m_inputEdit->setFixedHeight(48);
    
    // 发送按钮
    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setObjectName("chatSendBtn");
    m_sendBtn->setFixedSize(80, 48);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_sendBtn);
    
    mainLayout->addWidget(inputFrame);
    
    // 连接信号
    connect(m_sendBtn, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &ChatWidget::onSendClicked);
}

void ChatWidget::setupStyles()
{
    // 整体样式
    setStyleSheet(R"(
        ChatWidget {
            background-color: #f5f7fa;
        }
        
        /* 滚动区域 */
        QScrollArea#chatScrollArea {
            background-color: #f5f7fa;
            border: none;
        }
        
        /* 消息容器 */
        QWidget#messageContainer {
            background-color: #f5f7fa;
        }
        
        /* 滚动条样式 */
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(0, 0, 0, 0.2);
            border-radius: 4px;
            min-height: 40px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(0, 0, 0, 0.3);
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
            height: 0;
        }
        
        /* 输入框区域 */
        QFrame#inputFrame {
            background-color: #ffffff;
            border-top: 1px solid #e5e7eb;
        }
        
        /* 输入框 */
        QLineEdit#chatInputEdit {
            background-color: #f3f4f6;
            border: 1px solid #e5e7eb;
            border-radius: 24px;
            padding: 0 20px;
            font-size: 15px;
            color: #1a1a1a;
        }
        QLineEdit#chatInputEdit:focus {
            border-color: #2b7de9;
            background-color: #ffffff;
        }
        QLineEdit#chatInputEdit::placeholder {
            color: #9ca3af;
        }
        
        /* 发送按钮 */
        QPushButton#chatSendBtn {
            background-color: #2b7de9;
            color: #ffffff;
            border: none;
            border-radius: 24px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton#chatSendBtn:hover {
            background-color: #1e6fd9;
        }
        QPushButton#chatSendBtn:pressed {
            background-color: #1a5fc4;
        }
        QPushButton#chatSendBtn:disabled {
            background-color: #9ca3af;
        }
    )");
}

QWidget* ChatWidget::createMessageBubble(const QString &text, bool isUser)
{
    // 消息行容器
    QWidget *rowWidget = new QWidget();
    QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);
    
    // 头像
    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet(QString(
        "QLabel {"
        "   background-color: %1;"
        "   border-radius: 20px;"
        "   color: #ffffff;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
    ).arg(isUser ? "#2b7de9" : "#6b7280"));
    avatarLabel->setText(isUser ? "我" : "AI");
    
    // 气泡容器
    QWidget *bubbleWidget = new QWidget();
    bubbleWidget->setObjectName(isUser ? "userBubble" : "aiBubble");
    bubbleWidget->setMaximumWidth(static_cast<int>(width() * 0.7));
    if (bubbleWidget->maximumWidth() < 200) {
        bubbleWidget->setMaximumWidth(400); // 默认最大宽度
    }
    
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(16, 12, 16, 12);
    bubbleLayout->setSpacing(0);
    
    // 消息文本
    QLabel *textLabel = new QLabel(text);
    textLabel->setWordWrap(true);
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    textLabel->setOpenExternalLinks(true);
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    
    // 根据用户/AI设置不同样式
    if (isUser) {
        bubbleWidget->setStyleSheet(QString(
            "QWidget#userBubble {"
            "   background-color: %1;"
            "   border-radius: 16px;"
            "   border-top-right-radius: 4px;"
            "}"
        ).arg(USER_BUBBLE_COLOR));
        
        textLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1;"
            "   font-size: 15px;"
            "   line-height: 1.5;"
            "   background: transparent;"
            "}"
        ).arg(USER_TEXT_COLOR));
    } else {
        bubbleWidget->setStyleSheet(QString(
            "QWidget#aiBubble {"
            "   background-color: %1;"
            "   border: 1px solid #e5e7eb;"
            "   border-radius: 16px;"
            "   border-top-left-radius: 4px;"
            "}"
        ).arg(AI_BUBBLE_COLOR));
        
        textLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1;"
            "   font-size: 15px;"
            "   line-height: 1.5;"
            "   background: transparent;"
            "}"
        ).arg(AI_TEXT_COLOR));
        
        // 保存引用用于流式更新
        m_lastAIMessageLabel = textLabel;
        qDebug() << "[ChatWidget] createMessageBubble: Set m_lastAIMessageLabel to" << (void*)textLabel << "for AI message";
    }
    
    bubbleLayout->addWidget(textLabel);
    
    // 添加阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(bubbleWidget);
    shadow->setBlurRadius(8);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 20));
    bubbleWidget->setGraphicsEffect(shadow);
    
    // 根据用户/AI调整布局顺序
    if (isUser) {
        // 用户消息：弹簧 -> 气泡 -> 头像
        rowLayout->addStretch();
        rowLayout->addWidget(bubbleWidget);
        rowLayout->addWidget(avatarLabel);
    } else {
        // AI消息：头像 -> 气泡 -> 弹簧
        rowLayout->addWidget(avatarLabel);
        rowLayout->addWidget(bubbleWidget);
        rowLayout->addStretch();
    }
    
    return rowWidget;
}

void ChatWidget::addMessage(const QString &text, bool isUser)
{
    qDebug() << "[ChatWidget] addMessage called with text length:" << text.length() << "isUser:" << isUser;
    // 注意：允许空的 AI 消息作为流式响应的占位符
    if (text.trimmed().isEmpty() && isUser) {
        qDebug() << "[ChatWidget] addMessage: text is empty for user message, returning";
        return;
    }

    // 移除底部弹簧
    QLayoutItem *stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
    if (stretch) delete stretch;

    // 创建并添加消息气泡
    qDebug() << "[ChatWidget] addMessage: About to create message bubble";
    QWidget *bubble = createMessageBubble(text, isUser);
    m_messageLayout->addWidget(bubble);
    qDebug() << "[ChatWidget] addMessage: Message bubble added to layout";
    
    // 重新添加底部弹簧
    m_messageLayout->addStretch();
    
    // 滚动到底部
    scrollToBottom();
}

void ChatWidget::updateLastAIMessage(const QString &text)
{
    qDebug() << "[ChatWidget] updateLastAIMessage called with text length:" << text.length();
    qDebug() << "[ChatWidget] m_lastAIMessageLabel is null?" << (m_lastAIMessageLabel == nullptr);

    if (m_lastAIMessageLabel) {
        QString currentText = m_lastAIMessageLabel->text();
        qDebug() << "[ChatWidget] Current label text length:" << currentText.length();

        m_lastAIMessageLabel->setText(text);
        qDebug() << "[ChatWidget] Text updated, new length:" << m_lastAIMessageLabel->text().length();

        scrollToBottom();
        qDebug() << "[ChatWidget] Scroll to bottom completed";
    } else {
        qDebug() << "[ChatWidget] Error: m_lastAIMessageLabel is null, cannot update!";
    }
}

void ChatWidget::clearMessages()
{
    // 清除所有消息组件
    while (m_messageLayout->count() > 0) {
        QLayoutItem *item = m_messageLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    
    // 重新添加弹簧
    m_messageLayout->addStretch();
    
    m_lastAIMessageLabel = nullptr;
}

void ChatWidget::setPlaceholderText(const QString &text)
{
    if (m_inputEdit) {
        m_inputEdit->setPlaceholderText(text);
    }
}

void ChatWidget::setInputText(const QString &text)
{
    if (m_inputEdit) {
        m_inputEdit->setText(text);
    }
}

void ChatWidget::setInputEnabled(bool enabled)
{
    if (m_inputEdit) m_inputEdit->setEnabled(enabled);
    if (m_sendBtn) m_sendBtn->setEnabled(enabled);
}

QString ChatWidget::inputText() const
{
    return m_inputEdit ? m_inputEdit->text() : QString();
}

void ChatWidget::clearInput()
{
    if (m_inputEdit) m_inputEdit->clear();
}

void ChatWidget::focusInput()
{
    if (m_inputEdit) m_inputEdit->setFocus();
}

void ChatWidget::onSendClicked()
{
    QString text = m_inputEdit->text().trimmed();
    qDebug() << "[ChatWidget] Send button clicked, text:" << text;

    if (text.isEmpty()) {
        qDebug() << "[ChatWidget] Text is empty, not sending";
        return;
    }

    qDebug() << "[ChatWidget] Emitting messageSent signal with text:" << text;
    emit messageSent(text);
    m_inputEdit->clear();
    qDebug() << "[ChatWidget] Message sent and input cleared";
}

void ChatWidget::scrollToBottom()
{
    // 延迟执行以确保布局已更新
    QTimer::singleShot(50, this, [this]() {
        if (m_scrollArea && m_scrollArea->verticalScrollBar()) {
            m_scrollArea->verticalScrollBar()->setValue(
                m_scrollArea->verticalScrollBar()->maximum()
            );
        }
    });
}
