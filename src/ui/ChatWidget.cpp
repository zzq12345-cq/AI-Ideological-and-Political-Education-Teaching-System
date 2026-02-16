#include "ChatWidget.h"
#include "../shared/StyleConfig.h"
#include "../utils/MarkdownRenderer.h"
#include <QScrollBar>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QFile>
#include <QDebug>

const QString ChatWidget::USER_BUBBLE_COLOR = StyleConfig::PATRIOTIC_RED_DARK;
const QString ChatWidget::AI_BUBBLE_COLOR = StyleConfig::BG_CARD;
const QString ChatWidget::USER_TEXT_COLOR = "#FFFFFF";
const QString ChatWidget::AI_TEXT_COLOR = StyleConfig::TEXT_PRIMARY;

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_inputEdit(nullptr)
    , m_sendBtn(nullptr)
    , m_lastAIMessageLabel(nullptr)
    , m_lastAIThinkingWidget(nullptr)
    , m_lastAIThinkingLabel(nullptr)
    , m_lastAIThinkingToggle(nullptr)
    , m_markdownRenderer(nullptr)
    , m_markdownEnabled(true)
    , m_typingIndicator(nullptr)
    , m_typingAnimTimer(nullptr)
    , m_typingAnimStep(0)
{
    // 初始化Markdown渲染器
    m_markdownRenderer = std::make_unique<MarkdownRenderer>();

    // 设置代码块主题
    m_markdownRenderer->setCodeTheme(QColor("#f6f8fa"), QColor("#d73a49"));

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
    QWidget *bottomWidget = new QWidget();
    bottomWidget->setObjectName("bottomWidget");
    bottomWidget->setAttribute(Qt::WA_TranslucentBackground); // 确保背景透明
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(20, 10, 20, 5);
    bottomLayout->setSpacing(4);

    // 输入框容器（胶囊状）
    QFrame *inputContainer = new QFrame();
    inputContainer->setObjectName("inputContainer");
    inputContainer->setFixedHeight(56); // 增加高度以适应胶囊形状
    
    // 移除阴影效果，避免产生底部阴影长方形
    /*
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 2);
    inputContainer->setGraphicsEffect(shadow);
    */

    QHBoxLayout *containerLayout = new QHBoxLayout(inputContainer);
    containerLayout->setContentsMargins(12, 8, 12, 8);
    containerLayout->setSpacing(12);

    // 加号按钮
    QPushButton *plusBtn = new QPushButton("+");
    plusBtn->setObjectName("plusBtn");
    plusBtn->setFixedSize(32, 32);
    plusBtn->setCursor(Qt::PointingHandCursor);

    // 输入框
    m_inputEdit = new QLineEdit();
    m_inputEdit->setObjectName("chatInputEdit");
    m_inputEdit->setPlaceholderText("向AI助手发送信息...");
    m_inputEdit->setFixedHeight(40);
    
    // 发送按钮
    m_sendBtn = new QPushButton("↑"); // 使用向上箭头
    m_sendBtn->setObjectName("chatSendBtn");
    m_sendBtn->setFixedSize(32, 32);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    
    containerLayout->addWidget(plusBtn);
    containerLayout->addWidget(m_inputEdit);
    containerLayout->addWidget(m_sendBtn);
    
    // 底部提示文字
    QLabel *tipLabel = new QLabel("AI可能产生错误信息，请核实重要内容。");
    tipLabel->setObjectName("tipLabel");
    tipLabel->setAlignment(Qt::AlignCenter);
    
    bottomLayout->addWidget(inputContainer);
    bottomLayout->addWidget(tipLabel);
    
    mainLayout->addWidget(bottomWidget);
    
    // 连接信号
    connect(m_sendBtn, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &ChatWidget::onSendClicked);
}

void ChatWidget::setupStyles()
{
    // 从外部 QSS 文件加载样式（样式与逻辑分离）
    QFile styleFile(":/styles/resources/styles/ChatWidget.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
        qDebug() << "[ChatWidget] Loaded external stylesheet successfully";
    } else {
        qWarning() << "[ChatWidget] Failed to load ChatWidget.qss, using fallback styles";
        // 如果加载失败，使用最小化的回退样式
        setStyleSheet(R"(
            ChatWidget { background-color: #f5f5f5; }
            QScrollArea#chatScrollArea { background-color: #f5f5f5; border: none; }
            QWidget#messageContainer { background-color: #f5f5f5; }
        )");
    }
}

QWidget* ChatWidget::createMessageBubble(const QString &text, bool isUser)
{
    // 消息行容器 - 设为透明，避免出现白色"金属块"
    QWidget *rowWidget = new QWidget();
    rowWidget->setAttribute(Qt::WA_TranslucentBackground);
    rowWidget->setStyleSheet("background: transparent;");

    QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 4, 0, 4);  // 增加垂直间距
    rowLayout->setSpacing(14);  // 增加头像与气泡间距

    // 头像 - 使用更精致的设计
    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(44, 44);  // 稍大一点
    avatarLabel->setAlignment(Qt::AlignCenter);

    if (isUser) {
        // 用户头像 - 思政红配色
        avatarLabel->setStyleSheet(
            "QLabel {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #C62828, stop:1 #E53935);"
            "   border-radius: 22px;"
            "   color: #ffffff;"
            "   font-size: 15px;"
            "   font-weight: 600;"
            "   border: 2px solid rgba(255, 255, 255, 0.3);"
            "}"
        );
        avatarLabel->setText("我");
    } else {
        // AI头像 - 金色渐变
        avatarLabel->setStyleSheet(
            "QLabel {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #D4A017, stop:1 #B8860B);"
            "   border-radius: 22px;"
            "   color: #ffffff;"
            "   font-size: 13px;"
            "   font-weight: 700;"
            "   border: 2px solid rgba(255, 255, 255, 0.3);"
            "}"
        );
        avatarLabel->setText("AI");
    }

    // 气泡容器 - 限制最大宽度为65%以提升阅读体验
    QWidget *bubbleWidget = new QWidget();
    bubbleWidget->setObjectName(isUser ? "userBubble" : "aiBubble");

    // 计算最大宽度：屏幕宽度的65%，但不超过600px
    int maxBubbleWidth = qMin(static_cast<int>(width() * 0.65), 600);
    if (maxBubbleWidth < 280) {
        maxBubbleWidth = 450;  // 合理的默认宽度
    }
    bubbleWidget->setMaximumWidth(maxBubbleWidth);

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(18, 14, 18, 14);  // 增加内边距
    bubbleLayout->setSpacing(0);
    
    // 消息文本
    QLabel *textLabel = new QLabel();
    textLabel->setWordWrap(true);
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    textLabel->setOpenExternalLinks(true);
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    textLabel->setTextFormat((!m_markdownEnabled || isUser) ? Qt::PlainText : Qt::RichText);

    // 渲染消息内容
    QString renderedText = renderMessage(text, isUser);
    textLabel->setText(renderedText);
    
    // 根据用户/AI设置不同样式
    if (isUser) {
        // 用户气泡 - 思政红渐变
        bubbleWidget->setStyleSheet(QString(
            "QWidget#userBubble {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #C62828, stop:1 %1);"
            "   border-radius: 18px;"
            "   border-top-right-radius: 6px;"
            "}"
        ).arg(USER_BUBBLE_COLOR));

        textLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1;"
            "   font-size: 15px;"
            "   line-height: 1.6;"
            "   background: transparent;"
            "   letter-spacing: 0.3px;"
            "}"
        ).arg(USER_TEXT_COLOR));
    } else {
        // AI气泡 - 白色卡片风格
        bubbleWidget->setStyleSheet(QString(
            "QWidget#aiBubble {"
            "   background-color: %1;"
            "   border: 1px solid #e5e7eb;"
            "   border-radius: 18px;"
            "   border-top-left-radius: 6px;"
            "}"
        ).arg(AI_BUBBLE_COLOR));

        textLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1;"
            "   font-size: 15px;"
            "   line-height: 1.6;"
            "   background: transparent;"
            "   letter-spacing: 0.3px;"
            "}"
        ).arg(AI_TEXT_COLOR));

        // 为 AI 消息添加可折叠的思考过程区域
        m_lastAIThinkingWidget = new QWidget();
        m_lastAIThinkingWidget->setVisible(false); // 默认隐藏
        QVBoxLayout *thinkingLayout = new QVBoxLayout(m_lastAIThinkingWidget);
        thinkingLayout->setContentsMargins(0, 8, 0, 0);
        thinkingLayout->setSpacing(4);
        
        // 思考过程标题和折叠按钮
        QWidget *thinkingHeaderWidget = new QWidget();
        QHBoxLayout *thinkingHeaderLayout = new QHBoxLayout(thinkingHeaderWidget);
        thinkingHeaderLayout->setContentsMargins(0, 0, 0, 0);
        thinkingHeaderLayout->setSpacing(4);
        
        m_lastAIThinkingToggle = new QPushButton("v");
        m_lastAIThinkingToggle->setFixedSize(20, 20);
        m_lastAIThinkingToggle->setStyleSheet(
            "QPushButton {"
            "   background: transparent;"
            "   border: none;"
            "   color: #6b7280;"
            "   font-size: 12px;"
            "   padding: 0;"
            "}"
            "QPushButton:hover {"
            "   color: #2b7de9;"
            "}"
        );

        QLabel *thinkingTitleLabel = new QLabel("思考过程");
        thinkingTitleLabel->setStyleSheet(
            "QLabel {"
            "   color: #6b7280;"
            "   font-size: 13px;"
            "   font-weight: 500;"
            "   background: transparent;"
            "}"
        );
        
        thinkingHeaderLayout->addWidget(m_lastAIThinkingToggle);
        thinkingHeaderLayout->addWidget(thinkingTitleLabel);
        thinkingHeaderLayout->addStretch();
        
        // 思考内容标签
        m_lastAIThinkingLabel = new QLabel();
        m_lastAIThinkingLabel->setWordWrap(true);
        m_lastAIThinkingLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_lastAIThinkingLabel->setVisible(false); // 默认折叠
        m_lastAIThinkingLabel->setStyleSheet(
            "QLabel {"
            "   color: #6b7280;"
            "   font-size: 13px;"
            "   line-height: 1.4;"
            "   background-color: #f9fafb;"
            "   border: 1px solid #e5e7eb;"
            "   border-radius: 8px;"
            "   padding: 8px;"
            "}"
        );
        
        thinkingLayout->addWidget(thinkingHeaderWidget);
        thinkingLayout->addWidget(m_lastAIThinkingLabel);
        
        bubbleLayout->addWidget(m_lastAIThinkingWidget);
        
        // 连接折叠按钮点击事件
        connect(m_lastAIThinkingToggle, &QPushButton::clicked, [this]() {
            bool isVisible = m_lastAIThinkingLabel->isVisible();
            m_lastAIThinkingLabel->setVisible(!isVisible);
            m_lastAIThinkingToggle->setText(isVisible ? ">" : "v");
        });
        
        // 保存引用用于流式更新
        m_lastAIMessageLabel = textLabel;
        qDebug() << "[ChatWidget] createMessageBubble: Set m_lastAIMessageLabel to" << (void*)textLabel << "for AI message";
    }
    
    bubbleLayout->addWidget(textLabel);

    // 注意：移除 QGraphicsDropShadowEffect 阴影效果
    // 原因：Qt 的阴影效果在多个控件紧密排列时会产生色块/伪影
    // 改用 CSS border 实现轻微的视觉层次感（已在上方 bubbleStyle 中设置）

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

    // 安全检查：确保布局已初始化
    if (!m_messageLayout) {
        qWarning() << "[ChatWidget] addMessage: m_messageLayout is null!";
        return;
    }

    // 移除底部弹簧（确保布局中有元素）
    if (m_messageLayout->count() > 0) {
        QLayoutItem *stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
        if (stretch) delete stretch;
    }

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
        m_lastAIMessageLabel->setTextFormat(m_markdownEnabled ? Qt::RichText : Qt::PlainText);
        QString currentText = m_lastAIMessageLabel->text();
        qDebug() << "[ChatWidget] Current label text length:" << currentText.length();

        // 渲染Markdown内容
        QString renderedText = renderMessage(text, false); // AI消息，isUser=false
        m_lastAIMessageLabel->setText(renderedText);
        qDebug() << "[ChatWidget] Text updated, new length:" << m_lastAIMessageLabel->text().length();

        scrollToBottom();
        qDebug() << "[ChatWidget] Scroll to bottom completed";
    } else {
        qDebug() << "[ChatWidget] Error: m_lastAIMessageLabel is null, cannot update!";
    }
}

void ChatWidget::updateLastAIThinking(const QString &thought)
{
    qDebug() << "[ChatWidget] updateLastAIThinking called with thought length:" << thought.length();
    
    if (m_lastAIThinkingLabel && m_lastAIThinkingWidget) {
        // 显示思考过程区域
        m_lastAIThinkingWidget->setVisible(true);
        
        // 流式更新时自动展开思考面板
        if (m_lastAIThinkingLabel && m_lastAIThinkingToggle) {
            m_lastAIThinkingLabel->setVisible(true);
            m_lastAIThinkingToggle->setText("v");
        }
        
        // 追加新的思考内容
        QString currentThought = m_lastAIThinkingLabel->text();
        if (!currentThought.isEmpty()) {
            currentThought += "\n";
        }
        currentThought += thought;
        m_lastAIThinkingLabel->setText(currentThought);
        
        qDebug() << "[ChatWidget] Thinking content updated, total length:" << currentThought.length();
        
        scrollToBottom();
    } else {
        qDebug() << "[ChatWidget] Error: Thinking widgets are null, cannot update!";
    }
}

void ChatWidget::collapseThinking()
{
    qDebug() << "[ChatWidget] collapseThinking called";
    if (m_lastAIThinkingLabel && m_lastAIThinkingToggle) {
        m_lastAIThinkingLabel->setVisible(false);
        m_lastAIThinkingToggle->setText(">");
        qDebug() << "[ChatWidget] Thinking collapsed";
    }
}

void ChatWidget::expandThinking()
{
    qDebug() << "[ChatWidget] expandThinking called";
    if (m_lastAIThinkingLabel && m_lastAIThinkingToggle) {
        m_lastAIThinkingLabel->setVisible(true);
        m_lastAIThinkingToggle->setText("v");
        qDebug() << "[ChatWidget] Thinking expanded";
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
    m_lastAIThinkingWidget = nullptr;
    m_lastAIThinkingLabel = nullptr;
    m_lastAIThinkingToggle = nullptr;
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

void ChatWidget::setMarkdownEnabled(bool enabled)
{
    m_markdownEnabled = enabled;
}

QString ChatWidget::renderMessage(const QString &text, bool isUser)
{
    if (!m_markdownEnabled || isUser) {
        // 用户消息或不启用Markdown时，返回纯文本
        return text;
    }

    if (!m_markdownRenderer) {
        qDebug() << "[ChatWidget] MarkdownRenderer is null, returning plain text";
        return text;
    }

    try {
        QString html = m_markdownRenderer->renderToHtml(text);
        qDebug() << "[ChatWidget] Markdown rendered successfully, input length:"
                 << text.length() << "output length:" << html.length();
        return html;
    } catch (const std::exception& e) {
        qDebug() << "[ChatWidget] Markdown rendering error:" << e.what();
        return text; // 渲染失败时返回纯文本
    } catch (...) {
        qDebug() << "[ChatWidget] Unknown Markdown rendering error";
        return text; // 渲染失败时返回纯文本
    }
}

void ChatWidget::showTypingIndicator()
{
    hideTypingIndicator();

    // 创建指示器行（与 AI 消息气泡布局一致）
    m_typingIndicator = new QWidget();
    m_typingIndicator->setAttribute(Qt::WA_TranslucentBackground);
    m_typingIndicator->setStyleSheet("background: transparent;");

    QHBoxLayout *rowLayout = new QHBoxLayout(m_typingIndicator);
    rowLayout->setContentsMargins(0, 4, 0, 4);
    rowLayout->setSpacing(14);

    // AI 头像（与 createMessageBubble 中的 AI 头像一致）
    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(44, 44);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet(
        "QLabel {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #D4A017, stop:1 #B8860B);"
        "   border-radius: 22px;"
        "   color: #ffffff;"
        "   font-size: 13px;"
        "   font-weight: 700;"
        "   border: 2px solid rgba(255, 255, 255, 0.3);"
        "}"
    );
    avatarLabel->setText("AI");

    // 气泡容器
    QWidget *bubbleWidget = new QWidget();
    bubbleWidget->setObjectName("typingBubble");
    bubbleWidget->setStyleSheet(
        "QWidget#typingBubble {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #e5e7eb;"
        "   border-radius: 18px;"
        "   border-top-left-radius: 6px;"
        "}"
    );

    QHBoxLayout *bubbleLayout = new QHBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(20, 16, 20, 16);
    bubbleLayout->setSpacing(8);

    // 三个脉冲圆点
    m_typingDots.clear();
    for (int i = 0; i < 3; ++i) {
        QLabel *dot = new QLabel("●");
        dot->setFixedSize(16, 16);
        dot->setAlignment(Qt::AlignCenter);
        dot->setStyleSheet("QLabel { color: #D1D5DB; font-size: 12px; background: transparent; }");
        m_typingDots.append(dot);
        bubbleLayout->addWidget(dot);
    }

    rowLayout->addWidget(avatarLabel);
    rowLayout->addWidget(bubbleWidget);
    rowLayout->addStretch();

    // 添加到消息布局
    if (m_messageLayout->count() > 0) {
        QLayoutItem *stretch = m_messageLayout->takeAt(m_messageLayout->count() - 1);
        if (stretch) delete stretch;
    }
    m_messageLayout->addWidget(m_typingIndicator);
    m_messageLayout->addStretch();

    // 启动脉冲动画
    m_typingAnimStep = 0;
    m_typingAnimTimer = new QTimer(this);
    connect(m_typingAnimTimer, &QTimer::timeout, this, [this]() {
        if (m_typingDots.isEmpty()) return;

        // 所有点先恢复浅色
        for (QLabel *dot : m_typingDots) {
            dot->setStyleSheet("QLabel { color: #D1D5DB; font-size: 12px; background: transparent; }");
        }
        // 当前活跃点高亮为思政红
        int activeIdx = m_typingAnimStep % 3;
        m_typingDots[activeIdx]->setStyleSheet(
            "QLabel { color: #E53935; font-size: 12px; background: transparent; }"
        );
        m_typingAnimStep++;
    });
    m_typingAnimTimer->start(350);

    scrollToBottom();
    qDebug() << "[ChatWidget] Typing indicator shown";
}

void ChatWidget::hideTypingIndicator()
{
    if (m_typingAnimTimer) {
        m_typingAnimTimer->stop();
        m_typingAnimTimer->deleteLater();
        m_typingAnimTimer = nullptr;
    }

    if (m_typingIndicator) {
        m_messageLayout->removeWidget(m_typingIndicator);
        m_typingIndicator->deleteLater();
        m_typingIndicator = nullptr;
    }

    m_typingDots.clear();
    m_typingAnimStep = 0;
    qDebug() << "[ChatWidget] Typing indicator hidden";
}
