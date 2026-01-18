#include "questionbankwindow.h"
#include "../ui/moderncheckbox.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QStyle>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <algorithm>

namespace {
constexpr int kPageMaxWidth = 1280;
constexpr int kSidebarWidth = 320;
constexpr int kComboHeight = 48;
constexpr int kFilterButtonHeight = 40;
constexpr int kGenerateButtonHeight = 40;
constexpr int kPrimaryActionHeight = 52;
constexpr int kActionButtonHeight = 44;
constexpr int kOptionMinHeight = 60;
constexpr int kCardPadding = 32;

const QColor kPrimaryColor("#D9001B");
const QColor kSidebarShadowColor(18, 24, 38, 30);
const QColor kButtonShadowColor(217, 0, 27, 90);

QWidget *resolveOptionFrame(QObject *node)
{
    QObject *current = node;
    while (current) {
        if (current->property("role").toString() == QLatin1String("optionItem")) {
            return qobject_cast<QWidget *>(current);
        }
        current = current->parent();
    }
    return nullptr;
}

void applyShadow(QWidget *target, qreal blurRadius, const QPointF &offset, const QColor &color)
{
    if (!target) {
        return;
    }
    auto *shadow = new QGraphicsDropShadowEffect(target);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(offset);
    shadow->setColor(color);
    target->setGraphicsEffect(shadow);
}
}

QuestionBankWindow::QuestionBankWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("questionBankWindow");

    QFont baseFont("Lexend", 12);
    baseFont.setStyleHint(QFont::SansSerif);
    baseFont.setStyleStrategy(QFont::PreferAntialias);
    setFont(baseFont);
    
    // 初始化 PaperService
    m_paperService = new PaperService(this);
    
    // 连接信号
    connect(m_paperService, &PaperService::searchCompleted,
            this, &QuestionBankWindow::onQuestionsReceived);
    connect(m_paperService, &PaperService::questionError,
            this, &QuestionBankWindow::onQuestionsError);

    setupLayout();
    loadStyleSheet();
    
    // 页面加载时自动获取题目
    loadQuestions();
}

void QuestionBankWindow::setupLayout()
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *pageContainer = new QWidget(this);
    pageContainer->setObjectName("pageContainer");

    auto *pageLayout = new QVBoxLayout(pageContainer);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    pageLayout->addWidget(buildHeader());
    pageLayout->addWidget(buildBody(), 1);

    rootLayout->addWidget(pageContainer);
}

QWidget *QuestionBankWindow::buildHeader()
{
    auto *header = new QFrame(this);
    header->setObjectName("pageHeader");

    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);

    auto *backButton = new QPushButton(header);
    backButton->setObjectName("backButton");
    backButton->setCursor(Qt::PointingHandCursor);

    // 使用QtTheme图标 - 完全安全的图标集成方式
    backButton->setIcon(QIcon(":/QtTheme/icon/chevron_left/#424242.svg"));
    backButton->setIconSize(QSize(20, 20));
    backButton->setText(QStringLiteral("返回主界面"));

    connect(backButton, &QPushButton::clicked, this, [] {
        qInfo() << "Navigate back to dashboard";
    });

    auto *titleWrapper = new QWidget(header);
    auto *titleLayout = new QVBoxLayout(titleWrapper);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(6);

    auto *title = new QLabel(QStringLiteral("AI 智能备课 · 试题库"), titleWrapper);
    title->setObjectName("pageTitle");

    auto *subtitle = new QLabel(QStringLiteral("根据教材、章节与难度智能筛选题目"), titleWrapper);
    subtitle->setObjectName("pageSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);

    layout->addWidget(backButton, 0, Qt::AlignLeft);
    layout->addWidget(titleWrapper, 1);

    applyShadow(header, 24, QPointF(0, 8), QColor(0, 0, 0, 20));
    return header;
}

QWidget *QuestionBankWindow::buildBody()
{
    auto *body = new QFrame(this);
    body->setObjectName("mainBody");

    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(18);

    bodyLayout->addWidget(buildSidebar(), 0);
    bodyLayout->addWidget(buildContentArea(), 1);

    return body;
}

QWidget *QuestionBankWindow::buildSidebar()
{
    auto *sidebar = new QFrame(this);
    sidebar->setObjectName("filterSidebar");
    sidebar->setFixedWidth(kSidebarWidth);
    sidebar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    applyShadow(sidebar, 30, QPointF(0, 10), kSidebarShadowColor);

    auto *outerLayout = new QVBoxLayout(sidebar);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(sidebar);
    scrollArea->setObjectName("filterSidebarScroll");
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *content = new QWidget(scrollArea);
    content->setObjectName("filterSidebarContent");
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto *sidebarTitle = new QLabel(QStringLiteral("筛选条件"), content);
    sidebarTitle->setObjectName("sidebarTitle");

    auto *sidebarHint = new QLabel(QStringLiteral("按照 HTML 模板规范筛选参数，生成一致的试题体验"), content);
    sidebarHint->setObjectName("sidebarHint");
    sidebarHint->setWordWrap(true);

    layout->addWidget(sidebarTitle);
    layout->addWidget(sidebarHint);

    struct ComboField {
        QString label;
        QStringList options;
    };

    const QList<ComboField> comboFields = {
        {QStringLiteral("课程范围"), {QStringLiteral("思想道德与法治"), QStringLiteral("中国近现代史纲要"), QStringLiteral("马克思主义基本原理"), QStringLiteral("形势与政策")}},
        {QStringLiteral("教材版本"), {QStringLiteral("人教版"), QStringLiteral("部编版"), QStringLiteral("自编教材")}},
        {QStringLiteral("年级学期"), {QStringLiteral("七年级上学期"), QStringLiteral("八年级下学期"), QStringLiteral("高中必修一")}},
        {QStringLiteral("章节"), {QStringLiteral("第一章"), QStringLiteral("第二章"), QStringLiteral("第三章"), QStringLiteral("综合复习")}}
    };

    for (const ComboField &field : comboFields) {
        layout->addWidget(createComboField(field.label, field.options));
    }

    layout->addSpacing(4);

    layout->addWidget(createFilterButtons(QStringLiteral("试卷类型"),
                                          {QStringLiteral("不限"), QStringLiteral("章节练习"), QStringLiteral("课后作业"), QStringLiteral("期中"), QStringLiteral("期末"), QStringLiteral("模拟卷")},
                                          QStringLiteral("paperType"),
                                          3));

    m_questionTypeGroup = createFilterButtonsWithGroup(QStringLiteral("题目题型"),
                                          {QStringLiteral("不限"), QStringLiteral("选择题"), QStringLiteral("填空题"), QStringLiteral("材料论述题"), QStringLiteral("判断说理题")},
                                          QStringLiteral("questionType"),
                                          3,
                                          layout);  // 传递 layout 供添加 widget

    layout->addWidget(createFilterButtons(QStringLiteral("题目难度"),
                                          {QStringLiteral("不限"), QStringLiteral("简单"), QStringLiteral("中等"), QStringLiteral("困难")},
                                          QStringLiteral("difficulty"),
                                          2));

    layout->addStretch(1);

    auto *actionPanel = new QWidget(sidebar);
    actionPanel->setObjectName("filterSidebarActions");
    actionPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    actionPanel->setFixedHeight(kGenerateButtonHeight + 48); // 按钮高度 + 边距

    auto *actionLayout = new QVBoxLayout(actionPanel);
    actionLayout->setContentsMargins(24, 0, 24, 24);
    actionLayout->setSpacing(12);

    m_generateButton = new QPushButton(actionPanel);
    m_generateButton->setObjectName("generateButton");
    m_generateButton->setFixedHeight(kGenerateButtonHeight);
    m_generateButton->setCursor(Qt::PointingHandCursor);

    // 使用QtTheme图标 - 与主题色调一致的红色三角形图标
    m_generateButton->setIcon(QIcon(":/QtTheme/icon/triangle_right/#ef5350.svg"));
    m_generateButton->setIconSize(QSize(20, 20));
    m_generateButton->setText(QStringLiteral("开始生成"));
    m_generateButton->setProperty("hovered", false);
    m_generateButton->installEventFilter(this);
    applyShadow(m_generateButton, 20, QPointF(0, 4), kButtonShadowColor);

    actionLayout->addWidget(m_generateButton);

    connect(m_generateButton, &QPushButton::clicked, this, [this] {
        qInfo() << "Generate paper with" << m_currentQuestion << "current question";
        updateProgress(1);
    });

    // 设置滚动区域内容
    scrollArea->setWidget(content);

    // 更新主布局：滚动区域在上方，操作面板固定在底部
    outerLayout->addWidget(scrollArea, 1);  // 可伸缩
    outerLayout->addWidget(actionPanel, 0); // 固定高度

    return sidebar;
}

QWidget *QuestionBankWindow::buildContentArea()
{
    m_questionScrollArea = new QScrollArea(this);
    m_questionScrollArea->setObjectName("contentScrollArea");
    m_questionScrollArea->setWidgetResizable(true);
    m_questionScrollArea->setFrameShape(QFrame::NoFrame);
    m_questionScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *scrollWidget = new QWidget(m_questionScrollArea);
    scrollWidget->setObjectName("scrollWrapper");

    m_questionListLayout = new QVBoxLayout(scrollWidget);
    m_questionListLayout->setContentsMargins(0, 0, 4, 0);
    m_questionListLayout->setSpacing(16);
    
    // 状态提示标签
    m_statusLabel = new QLabel("正在加载题目...", scrollWidget);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { color: #6c757d; font-size: 14px; padding: 40px; }");
    m_questionListLayout->addWidget(m_statusLabel);

    m_questionListLayout->addStretch(1);

    m_questionScrollArea->setWidget(scrollWidget);
    return m_questionScrollArea;
}

QWidget *QuestionBankWindow::createComboField(const QString &labelText, const QStringList &options)
{
    auto *wrapper = new QWidget(this);
    wrapper->setObjectName("comboField");

    auto *layout = new QVBoxLayout(wrapper);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *label = new QLabel(labelText, wrapper);
    label->setProperty("role", "comboLabel");
    layout->addWidget(label);

    auto *combo = new QComboBox(wrapper);
    combo->setProperty("role", "filterSelect");
    combo->setCursor(Qt::PointingHandCursor);
    combo->setFixedHeight(kComboHeight);
    combo->addItems(options);

    layout->addWidget(combo);

    m_filterCombos.append(combo);
    connectFilterCombo(combo, labelText);

    return wrapper;
}

QWidget *QuestionBankWindow::createFilterButtons(const QString &labelText,
                                                 const QStringList &options,
                                                 const QString &groupId,
                                                 int columns)
{
    const int columnCount = std::max(1, columns);

    auto *section = new QWidget(this);
    section->setObjectName(groupId + "Section");
    section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *label = new QLabel(labelText, section);
    label->setProperty("role", "comboLabel");
    layout->addWidget(label);

    auto *gridHost = new QWidget(section);
    gridHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto *grid = new QGridLayout(gridHost);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    grid->setSizeConstraint(QLayout::SetMinimumSize);

    // 设置网格列拉伸，确保按钮平均分布
    for (int col = 0; col < columnCount; ++col) {
        grid->setColumnStretch(col, 1);
    }

    auto *group = new QButtonGroup(gridHost);
    group->setExclusive(true);

    for (int i = 0; i < options.size(); ++i) {
        auto *button = new QPushButton(options.at(i), gridHost);
        button->setProperty("role", "filterButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(kFilterButtonHeight);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        if (i == 0) {
            button->setChecked(true);
        }

        group->addButton(button);
        grid->addWidget(button, i / columnCount, i % columnCount);
    }

    layout->addWidget(gridHost);

    m_filterGroups.append(group);
    connectFilterButton(group, labelText);

    return section;
}

QButtonGroup *QuestionBankWindow::createFilterButtonsWithGroup(const QString &labelText,
                                                               const QStringList &options,
                                                               const QString &groupId,
                                                               int columns,
                                                               QVBoxLayout *parentLayout)
{
    const int columnCount = std::max(1, columns);

    auto *section = new QWidget(this);
    section->setObjectName(groupId + "Section");
    section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    auto *label = new QLabel(labelText, section);
    label->setProperty("role", "comboLabel");
    layout->addWidget(label);

    auto *gridHost = new QWidget(section);
    gridHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto *grid = new QGridLayout(gridHost);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    grid->setSizeConstraint(QLayout::SetMinimumSize);

    for (int col = 0; col < columnCount; ++col) {
        grid->setColumnStretch(col, 1);
    }

    auto *group = new QButtonGroup(gridHost);
    group->setExclusive(true);

    for (int i = 0; i < options.size(); ++i) {
        auto *button = new QPushButton(options.at(i), gridHost);
        button->setProperty("role", "filterButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(kFilterButtonHeight);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        if (i == 0) {
            button->setChecked(true);
        }

        group->addButton(button);
        grid->addWidget(button, i / columnCount, i % columnCount);
        
        // 连接按钮点击信号到题型变化处理
        connect(button, &QPushButton::clicked, this, &QuestionBankWindow::onQuestionTypeChanged);
    }

    layout->addWidget(gridHost);

    // 添加到传入的布局
    if (parentLayout) {
        parentLayout->addWidget(section);
    }

    m_filterGroups.append(group);

    return group;
}

QString QuestionBankWindow::getSelectedQuestionType()
{
    if (!m_questionTypeGroup) {
        return QString();
    }
    
    QAbstractButton *checkedButton = m_questionTypeGroup->checkedButton();
    if (checkedButton) {
        return checkedButton->text();
    }
    
    return QString();
}

void QuestionBankWindow::onQuestionTypeChanged()
{
    QString selectedType = getSelectedQuestionType();
    qDebug() << "[QuestionBankWindow] Question type changed to:" << selectedType;
    loadQuestions(selectedType);
}


QWidget *QuestionBankWindow::createActionRow()
{
    auto *row = new QFrame(this);
    row->setObjectName("actionRow");

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto *primaryAction = new QPushButton(QStringLiteral("下一题"), row);
    primaryAction->setObjectName("primaryActionButton");
    primaryAction->setFixedHeight(kPrimaryActionHeight);
    primaryAction->setCursor(Qt::PointingHandCursor);

    auto *prevAction = new QPushButton(QStringLiteral("上一题"), row);
    prevAction->setObjectName("secondaryButton");
    prevAction->setFixedHeight(kActionButtonHeight);
    prevAction->setCursor(Qt::PointingHandCursor);

    auto *revealAction = new QPushButton(QStringLiteral("查看解析"), row);
    revealAction->setObjectName("secondaryButton");
    revealAction->setFixedHeight(kActionButtonHeight);
    revealAction->setCursor(Qt::PointingHandCursor);

    auto *exportAction = new QPushButton(QStringLiteral("导出试卷"), row);
    exportAction->setObjectName("ghostButton");
    exportAction->setFixedHeight(kActionButtonHeight);
    exportAction->setCursor(Qt::PointingHandCursor);

    layout->addWidget(primaryAction, 0, Qt::AlignLeft);
    layout->addWidget(prevAction);
    layout->addWidget(revealAction);
    layout->addWidget(exportAction);
    layout->addStretch(1);

    connect(primaryAction, &QPushButton::clicked, this, [this] {
        updateProgress(1);
    });

    connect(prevAction, &QPushButton::clicked, this, [this] {
        updateProgress(-1);
    });

    connect(revealAction, &QPushButton::clicked, this, [this] {
        const bool currentlyVisible = m_answerSection && m_answerSection->isVisible();
        if (m_answerSection) {
            m_answerSection->setVisible(!currentlyVisible);
        }
        if (m_analysisSection) {
            m_analysisSection->setVisible(!currentlyVisible);
        }
    });

    connect(exportAction, &QPushButton::clicked, this, [] {
        qInfo() << "Export paper request";
    });

    return row;
}

void QuestionBankWindow::connectFilterCombo(QComboBox *combo, const QString &labelText)
{
    connect(combo, &QComboBox::currentTextChanged, this, [labelText](const QString &value) {
        qInfo() << labelText << "changed to" << value;
    });
}

void QuestionBankWindow::connectFilterButton(QButtonGroup *group, const QString &labelText)
{
    const QList<QAbstractButton *> buttons = group->buttons();
    for (QAbstractButton *button : buttons) {
        connect(button, &QAbstractButton::toggled, this, [labelText, button](bool checked) {
            if (checked) {
                qInfo() << labelText << "selected" << button->text();
            }
        });
    }
}

void QuestionBankWindow::updateProgress(int delta)
{
    m_currentQuestion = std::clamp(m_currentQuestion + delta, 1, m_totalQuestions);
    if (m_progressBar) {
        m_progressBar->setValue(m_currentQuestion);
    }
    if (m_progressValueLabel) {
        m_progressValueLabel->setText(QStringLiteral("%1 / %2").arg(m_currentQuestion).arg(m_totalQuestions));
    }
}

void QuestionBankWindow::loadStyleSheet()
{
    QFile file(QStringLiteral(":/styles/resources/styles/question_bank.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to load question bank stylesheet from :/styles/resources/styles/question_bank.qss";

        // 尝试备用路径 (兼容旧的资源结构)
        QFile fallbackFile(QStringLiteral(":/styles/question_bank.qss"));
        if (!fallbackFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Unable to load question bank stylesheet from fallback path";
            return;
        }

        setStyleSheet(QString::fromUtf8(fallbackFile.readAll()));
        qDebug() << "Loaded question bank stylesheet from fallback path";
        return;
    }

    setStyleSheet(QString::fromUtf8(file.readAll()));
    qDebug() << "Loaded question bank stylesheet from primary path";
}

void QuestionBankWindow::refreshOptionFrame(QWidget *frame, bool hovered)
{
    if (!frame) {
        return;
    }
    frame->setProperty("hovered", hovered);
    frame->style()->unpolish(frame);
    frame->style()->polish(frame);
    frame->update();
}

bool QuestionBankWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (!watched) {
        return QWidget::eventFilter(watched, event);
    }

    if (m_generateButton && watched == m_generateButton) {
        if (event->type() == QEvent::Enter) {
            m_generateButton->setProperty("hovered", true);
            m_generateButton->style()->unpolish(m_generateButton);
            m_generateButton->style()->polish(m_generateButton);
            m_generateButton->update();
        } else if (event->type() == QEvent::Leave) {
            m_generateButton->setProperty("hovered", false);
            m_generateButton->style()->unpolish(m_generateButton);
            m_generateButton->style()->polish(m_generateButton);
            m_generateButton->update();
        }
    }

    auto *optionFrame = resolveOptionFrame(watched);
    if (optionFrame) {
        if (event->type() == QEvent::Enter) {
            refreshOptionFrame(optionFrame, true);
        } else if (event->type() == QEvent::Leave) {
            refreshOptionFrame(optionFrame, false);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            // 点击容器时触发现代化复选框点击
            if (auto *checkbox = optionFrame->findChild<ModernCheckBox *>()) {
                checkbox->setChecked(true);
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

// ==================== 动态加载题目相关方法 ====================

void QuestionBankWindow::loadQuestions(const QString &questionType)
{
    if (!m_paperService) {
        qWarning() << "[QuestionBankWindow] PaperService is null";
        return;
    }

    if (m_statusLabel) {
        m_statusLabel->setText("正在加载题目...");
        m_statusLabel->show();
    }

    // 构建搜索条件
    QuestionSearchCriteria criteria;
    criteria.visibility = "public";  // 只显示公共题目

    // 如果指定了题型，添加筛选条件
    QString typeToSearch = questionType;
    if (typeToSearch.isEmpty()) {
        typeToSearch = getSelectedQuestionType();
    }

    if (!typeToSearch.isEmpty() && typeToSearch != "不限") {
        // 中文题型映射到英文（与数据库存储格式一致）
        static const QMap<QString, QString> typeMapping = {
            {"选择题", "single_choice"},
            {"填空题", "fill_blank"},
            {"判断说理题", "true_false"},
            {"判断题", "true_false"},
            {"材料论述题", "material_essay"},
            {"简答题", "short_answer"},
            {"论述题", "short_answer"},
            {"材料分析题", "material_essay"}
        };

        QString mappedType = typeMapping.value(typeToSearch, typeToSearch);
        criteria.questionType = mappedType;
        qDebug() << "[QuestionBankWindow] Filtering by question type:" << typeToSearch << "->" << mappedType;
    }

    qDebug() << "[QuestionBankWindow] Loading questions with criteria...";
    m_paperService->searchQuestions(criteria);
}

void QuestionBankWindow::onSearchClicked()
{
    loadQuestions();
}

void QuestionBankWindow::onQuestionsReceived(const QList<PaperQuestion> &questions)
{
    qDebug() << "[QuestionBankWindow] Received" << questions.size() << "questions";
    
    m_questions = questions;
    m_totalQuestions = questions.size();
    m_currentQuestion = 1;
    
    displayQuestions(questions);
}

void QuestionBankWindow::onQuestionsError(const QString &type, const QString &error)
{
    qWarning() << "[QuestionBankWindow] Error loading questions:" << type << error;
    
    if (m_statusLabel) {
        m_statusLabel->setText(QString("加载失败: %1").arg(error));
        m_statusLabel->show();
    }
}

void QuestionBankWindow::clearQuestionCards()
{
    if (!m_questionListLayout) return;
    
    // 移除所有子项（除了 stretch）
    while (m_questionListLayout->count() > 0) {
        QLayoutItem *item = m_questionListLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // 重置 statusLabel 指针，因为它可能已被删除
    m_statusLabel = nullptr;
}

void QuestionBankWindow::displayQuestions(const QList<PaperQuestion> &questions)
{
    clearQuestionCards();

    if (questions.isEmpty()) {
        m_statusLabel = new QLabel("暂无题目，请先导入试题或调整筛选条件", this);
        m_statusLabel->setObjectName("statusLabel");
        m_statusLabel->setAlignment(Qt::AlignCenter);
        m_statusLabel->setStyleSheet("QLabel { color: #6c757d; font-size: 14px; padding: 40px; }");
        m_questionListLayout->addWidget(m_statusLabel);
        m_questionListLayout->addStretch(1);
        return;
    }

    // 定义题型顺序和中文名称（数据库存储英文键）
    const QList<QPair<QString, QString>> questionTypeOrder = {
        // 英文键 -> 中文显示名
        {"single_choice", "选择题"},
        {"true_false", "判断题"},
        {"fill_blank", "填空题"},
        {"short_answer", "简答题"},
        {"essay", "论述题"},
        {"material_essay", "材料论述题"},
        {"material", "材料分析题"}
    };

    const QStringList chineseNumbers = {"一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};

    // 按题型分组题目
    QMap<QString, QList<PaperQuestion>> groupedQuestions;
    for (const PaperQuestion &q : questions) {
        groupedQuestions[q.questionType].append(q);
    }

    // 按预定顺序显示各题型
    int globalIndex = 1;
    int sectionIndex = 0;

    for (const auto &typePair : questionTypeOrder) {
        const QString &typeKey = typePair.first;
        const QString &typeName = typePair.second;

        if (!groupedQuestions.contains(typeKey) || groupedQuestions[typeKey].isEmpty()) {
            continue;
        }

        const QList<PaperQuestion> &typeQuestions = groupedQuestions[typeKey];

        // 创建题型标题
        QString sectionTitle = QString("%1、%2（共%3题）")
            .arg(sectionIndex < chineseNumbers.size() ? chineseNumbers[sectionIndex] : QString::number(sectionIndex + 1))
            .arg(typeName)
            .arg(typeQuestions.size());

        auto *sectionLabel = new QLabel(sectionTitle, this);
        sectionLabel->setObjectName("sectionTitle");
        sectionLabel->setStyleSheet(
            "QLabel {"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "  color: #D9001B;"
            "  padding: 16px 0 8px 0;"
            "  border-bottom: 2px solid #D9001B;"
            "  margin-bottom: 12px;"
            "}"
        );
        m_questionListLayout->addWidget(sectionLabel);

        // 为该题型下的每道题目创建卡片
        for (const PaperQuestion &q : typeQuestions) {
            QWidget *card = createQuestionCard(q, globalIndex);
            m_questionListLayout->addWidget(card);
            globalIndex++;
        }

        // 移除已处理的题型
        groupedQuestions.remove(typeKey);
        sectionIndex++;
    }

    // 处理未在预定义列表中的其他题型
    for (auto it = groupedQuestions.begin(); it != groupedQuestions.end(); ++it) {
        if (it.value().isEmpty()) {
            continue;
        }

        QString sectionTitle = QString("%1、%2（共%3题）")
            .arg(sectionIndex < chineseNumbers.size() ? chineseNumbers[sectionIndex] : QString::number(sectionIndex + 1))
            .arg(it.key())
            .arg(it.value().size());

        auto *sectionLabel = new QLabel(sectionTitle, this);
        sectionLabel->setObjectName("sectionTitle");
        sectionLabel->setStyleSheet(
            "QLabel {"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "  color: #D9001B;"
            "  padding: 16px 0 8px 0;"
            "  border-bottom: 2px solid #D9001B;"
            "  margin-bottom: 12px;"
            "}"
        );
        m_questionListLayout->addWidget(sectionLabel);

        for (const PaperQuestion &q : it.value()) {
            QWidget *card = createQuestionCard(q, globalIndex);
            m_questionListLayout->addWidget(card);
            globalIndex++;
        }

        sectionIndex++;
    }

    m_questionListLayout->addStretch(1);

    // 更新进度
    if (m_progressBar) {
        m_progressBar->setRange(0, m_totalQuestions);
        m_progressBar->setValue(m_currentQuestion);
    }
    if (m_progressValueLabel) {
        m_progressValueLabel->setText(QString("%1 / %2").arg(m_currentQuestion).arg(m_totalQuestions));
    }

    qDebug() << "[QuestionBankWindow] Displayed" << questions.size() << "question cards in" << sectionIndex << "sections";
}

QWidget *QuestionBankWindow::createQuestionCard(const PaperQuestion &question, int index)
{
    auto *card = new QFrame(this);
    card->setObjectName("questionCard");
    applyShadow(card, 36, QPointF(0, 14), QColor(0, 0, 0, 18));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(kCardPadding, kCardPadding, kCardPadding, kCardPadding);
    layout->setSpacing(16);

    // 标签行（题型、难度、学科）
    layout->addWidget(createTagRow(question.tags));

    // 材料论述题特殊处理
    bool isMaterialEssay = (question.questionType == "material_essay" ||
                            question.questionType == "material");

    if (isMaterialEssay) {
        // 智能解析材料论述题内容
        MaterialEssayParsed parsed = parseMaterialEssay(question);

        // 题号
        auto *titleLabel = new QLabel(QString("<b>第 %1 题</b>（材料论述题）").arg(index), card);
        titleLabel->setObjectName("questionTitle");
        titleLabel->setTextFormat(Qt::RichText);
        layout->addWidget(titleLabel);

        if (!parsed.material.isEmpty()) {
            // 材料内容区域
        auto *materialFrame = new QFrame(card);
        materialFrame->setObjectName("materialFrame");
        materialFrame->setStyleSheet(
            "QFrame#materialFrame {"
            "  background: #f8f9fa;"
            "  border-left: 4px solid #D9001B;"
            "  border-radius: 8px;"
            "  padding: 16px;"
            "}"
        );

        auto *materialLayout = new QVBoxLayout(materialFrame);
        materialLayout->setContentsMargins(16, 12, 16, 12);
        materialLayout->setSpacing(8);

        auto *materialTitle = new QLabel("【阅读材料】", materialFrame);
        materialTitle->setStyleSheet("QLabel { font-weight: bold; color: #D9001B; font-size: 14px; }");
        materialLayout->addWidget(materialTitle);

        // 使用 QTextBrowser 支持完整 HTML（表格、图片等）
        auto *materialBrowser = new QTextBrowser(materialFrame);
        materialBrowser->setObjectName("materialContent");
        materialBrowser->setOpenExternalLinks(true);
        materialBrowser->setHtml(parsed.material);
        materialBrowser->setStyleSheet(
            "QTextBrowser {"
            "  background: transparent;"
            "  border: none;"
            "  color: #333;"
            "  font-size: 14px;"
            "}"
        );
        // 自动调整高度以适应内容
        materialBrowser->document()->setDocumentMargin(0);
        materialBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        materialBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        materialBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        // 计算合适的高度
        QSize docSize = materialBrowser->document()->size().toSize();
        materialBrowser->setMinimumHeight(docSize.height() + 20);
        materialLayout->addWidget(materialBrowser);

        layout->addWidget(materialFrame);
        }  // end if (!parsed.material.isEmpty())

        // 显示题干说明（如果有，例如"阅读材料，回答下列问题"）
        if (!parsed.stem.isEmpty()) {
            auto *stemLabel = new QLabel(QString("<b>【题目要求】</b> %1").arg(parsed.stem), card);
            stemLabel->setObjectName("questionStem");
            stemLabel->setWordWrap(true);
            stemLabel->setTextFormat(Qt::RichText);
            stemLabel->setStyleSheet("QLabel { color: #333; font-size: 14px; padding: 8px 0; }");
            layout->addWidget(stemLabel);
        }

        // 小问列表
        if (!parsed.subQuestions.isEmpty()) {
            auto *questionsFrame = new QFrame(card);
            questionsFrame->setObjectName("subQuestionsFrame");

            auto *questionsLayout = new QVBoxLayout(questionsFrame);
            questionsLayout->setContentsMargins(0, 8, 0, 0);
            questionsLayout->setSpacing(12);

            for (int i = 0; i < parsed.subQuestions.size(); ++i) {
                auto *subFrame = new QFrame(questionsFrame);
                auto *subLayout = new QVBoxLayout(subFrame);
                subLayout->setContentsMargins(0, 0, 0, 0);
                subLayout->setSpacing(4);

                // 小问题目 - 先去除已有编号前缀，避免重复显示
                QString questionText = parsed.subQuestions[i].trimmed();
                // 去除开头的编号格式如 (1) （1） 1. 1、 ⑴ 等
                static QRegularExpression numPrefixRe(R"(^[\(（⑴⑵⑶⑷⑸]?\s*\d*\s*[\)）]?[\s\.\、\：\:]*)");;
                questionText.remove(numPrefixRe);
                QString subText = QString("<b>（%1）</b> %2").arg(i + 1).arg(questionText.trimmed());
                auto *subLabel = new QLabel(subText, subFrame);
                subLabel->setWordWrap(true);
                subLabel->setTextFormat(Qt::RichText);
                subLayout->addWidget(subLabel);

                // 小问答案（如果有）
                if (i < parsed.subAnswers.size() && !parsed.subAnswers[i].isEmpty()) {
                    auto *answerFrame = new QFrame(subFrame);
                    answerFrame->setStyleSheet(
                        "QFrame { background: #e8f5e9; border-radius: 6px; padding: 8px; }"
                    );
                    auto *answerLayout = new QVBoxLayout(answerFrame);
                    answerLayout->setContentsMargins(12, 8, 12, 8);

                    auto *answerTitle = new QLabel("正确答案", answerFrame);
                    answerTitle->setStyleSheet("QLabel { color: #2e7d32; font-weight: bold; font-size: 12px; }");
                    answerLayout->addWidget(answerTitle);

                    auto *answerContent = new QLabel(parsed.subAnswers[i], answerFrame);
                    answerContent->setWordWrap(true);
                    answerContent->setStyleSheet("QLabel { color: #1b5e20; }");
                    answerLayout->addWidget(answerContent);

                    subLayout->addWidget(answerFrame);
                }

                questionsLayout->addWidget(subFrame);
            }

            layout->addWidget(questionsFrame);
        }

        // 总答案（如果有且没有小问答案）
        if (parsed.subAnswers.isEmpty() && !question.answer.isEmpty()) {
            layout->addWidget(createAnswerSection(question.answer));
        }
    } else {
        // 普通题目处理（原逻辑）
        QString stemText = QString("<b>第 %1 题</b> %2").arg(index).arg(question.stem);
        auto *questionStem = new QLabel(stemText, card);
        questionStem->setObjectName("questionStem");
        questionStem->setWordWrap(true);
        questionStem->setTextFormat(Qt::RichText);
        layout->addWidget(questionStem);

        // 选项（如果有）
        if (!question.options.isEmpty()) {
            auto *optionsPanel = new QFrame(card);
            optionsPanel->setObjectName("optionsPanel");

            auto *optionsLayout = new QVBoxLayout(optionsPanel);
            optionsLayout->setContentsMargins(0, 0, 0, 0);
            optionsLayout->setSpacing(8);

            QStringList optionKeys = {"A", "B", "C", "D", "E", "F"};
            for (int i = 0; i < question.options.size() && i < optionKeys.size(); ++i) {
                QString optionText = question.options[i];
                // 检测选项是否已包含字母前缀（如 "A." "A、" "A:" "A " 等）
                static QRegularExpression prefixPattern("^[A-Fa-f][.、:：\\s]");
                if (!prefixPattern.match(optionText).hasMatch()) {
                    // 没有前缀，添加一个
                    optionText = QString("%1. %2").arg(optionKeys[i]).arg(optionText);
                }
                auto *optionLabel = new QLabel(optionText, optionsPanel);
                optionLabel->setWordWrap(true);
                optionLabel->setStyleSheet("QLabel { padding: 8px 12px; background: #f8f9fa; border-radius: 6px; }");
                optionsLayout->addWidget(optionLabel);
            }

            layout->addWidget(optionsPanel);
        }

        // 答案区域
        layout->addWidget(createAnswerSection(question.answer));
    }

    // 解析区域
    if (!question.explanation.isEmpty()) {
        layout->addWidget(createAnalysisSection(question.explanation));
    }

    return card;
}

QWidget *QuestionBankWindow::createTagRow(const QStringList &tags)
{
    auto *tagRow = new QFrame(this);
    tagRow->setObjectName("tagRow");

    auto *layout = new QHBoxLayout(tagRow);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto createTag = [tagRow](const QString &text, const QString &objectName) {
        auto *label = new QLabel(text, tagRow);
        label->setObjectName(objectName);
        label->setStyleSheet("QLabel { background: #e9ecef; color: #495057; padding: 4px 12px; border-radius: 12px; font-size: 12px; }");
        return label;
    };

    for (const QString &tag : tags) {
        layout->addWidget(createTag(tag, "tagItem"));
    }
    
    layout->addStretch(1);

    return tagRow;
}

QFrame *QuestionBankWindow::createAnswerSection(const QString &answer)
{
    auto *section = new QFrame(this);
    section->setObjectName("answerSection");

    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(4);

    auto *title = new QLabel(QStringLiteral("正确答案"), section);
    title->setObjectName("answerTitle");

    auto *value = new QLabel(answer, section);
    value->setObjectName("answerValue");
    value->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(value);

    return section;
}

QFrame *QuestionBankWindow::createAnalysisSection(const QString &explanation)
{
    auto *section = new QFrame(this);
    section->setObjectName("analysisSection");

    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(6);

    auto *title = new QLabel(QStringLiteral("解析"), section);
    title->setObjectName("analysisTitle");

    auto *description = new QLabel(explanation, section);
    description->setObjectName("analysisDescription");
    description->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(description);

    return section;
}

// 智能解析材料论述题内容
// 尝试从 stem 或 material 中提取：材料内容、小问列表、小问答案
MaterialEssayParsed QuestionBankWindow::parseMaterialEssay(const PaperQuestion &question)
{
    MaterialEssayParsed result;

    // 优先使用已有的结构化数据
    result.material = question.material;
    result.subQuestions = question.subQuestions;
    result.subAnswers = question.subAnswers;
    result.stem = question.stem;

    // 如果已有结构化数据，直接返回
    if (!result.material.isEmpty() && !result.subQuestions.isEmpty()) {
        return result;
    }

    // 否则尝试从 stem 中智能解析
    QString content = question.stem;
    if (content.isEmpty()) {
        content = question.material;
    }
    if (content.isEmpty()) {
        return result;
    }

    // 尝试提取材料内容（通常在开头，到第一个小问之前）
    // 小问通常以 (1) （1） ⑴ 1. 1、等格式开始
    QRegularExpression subQuestionRe(
        R"([\(（]?\s*[1-9⑴⑵⑶⑷⑸]\s*[\)）]?[\s\.、：:]+)"
    );

    int firstSubPos = -1;
    QRegularExpressionMatch firstMatch = subQuestionRe.match(content);
    if (firstMatch.hasMatch()) {
        firstSubPos = firstMatch.capturedStart();
    }

    if (firstSubPos > 0) {
        // 材料内容是第一个小问之前的部分
        result.material = content.left(firstSubPos).trimmed();

        // 提取所有小问
        QString questionsText = content.mid(firstSubPos);

        // 使用正则匹配所有小问
        QRegularExpression splitRe(
            R"([\(（]?\s*([1-9⑴⑵⑶⑷⑸])\s*[\)）]?[\s\.、：:]+)"
        );

        QStringList parts;
        int lastEnd = 0;
        QRegularExpressionMatchIterator it = splitRe.globalMatch(questionsText);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            if (lastEnd > 0) {
                // 上一个小问的内容
                QString part = questionsText.mid(lastEnd, match.capturedStart() - lastEnd).trimmed();
                if (!part.isEmpty()) {
                    parts.append(part);
                }
            }
            lastEnd = match.capturedEnd();
        }

        // 最后一个小问
        if (lastEnd > 0 && lastEnd < questionsText.length()) {
            QString lastPart = questionsText.mid(lastEnd).trimmed();
            if (!lastPart.isEmpty()) {
                parts.append(lastPart);
            }
        }

        result.subQuestions = parts;
        result.stem.clear();  // 已解析到材料和小问，不需要显示 stem
    } else {
        // 没有找到小问格式，整个内容作为材料
        result.material = content;
        result.stem.clear();
    }

    return result;
}
