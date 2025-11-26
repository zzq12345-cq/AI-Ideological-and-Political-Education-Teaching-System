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
#include <QScrollArea>
#include <QStyle>
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

    setupLayout();
    loadStyleSheet();
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

    layout->addWidget(createFilterButtons(QStringLiteral("题目题型"),
                                          {QStringLiteral("不限"), QStringLiteral("单选"), QStringLiteral("多选"), QStringLiteral("判断"), QStringLiteral("简答"), QStringLiteral("综合"), QStringLiteral("材料分析")},
                                          QStringLiteral("questionType"),
                                          3));

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
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("contentScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *scrollWidget = new QWidget(scrollArea);
    scrollWidget->setObjectName("scrollWrapper");

    auto *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setContentsMargins(0, 0, 4, 0);
    scrollLayout->setSpacing(16);

    scrollLayout->addWidget(createQuestionCard());
    scrollLayout->addStretch(1);

    scrollArea->setWidget(scrollWidget);
    return scrollArea;
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

QWidget *QuestionBankWindow::createQuestionCard()
{
    auto *card = new QFrame(this);
    card->setObjectName("questionCard");
    applyShadow(card, 36, QPointF(0, 14), QColor(0, 0, 0, 18));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(kCardPadding, kCardPadding, kCardPadding, kCardPadding);
    layout->setSpacing(20);

    auto *header = new QFrame(card);
    header->setObjectName("cardHeader");

    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);

    auto *titleWrapper = new QWidget(header);
    auto *titleLayout = new QVBoxLayout(titleWrapper);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(6);

    auto *title = new QLabel(QStringLiteral("智能出题中心"), titleWrapper);
    title->setObjectName("cardTitle");

    auto *subtitle = new QLabel(QStringLiteral("依据选择的教材和章节即时生成题目"), titleWrapper);
    subtitle->setObjectName("cardSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);

    auto *progressWrapper = new QWidget(header);
    auto *progressLayout = new QVBoxLayout(progressWrapper);
    progressLayout->setContentsMargins(0, 0, 0, 0);
    progressLayout->setSpacing(6);
    progressLayout->setAlignment(Qt::AlignRight);

    auto *progressLabel = new QLabel(QStringLiteral("当前进度"), progressWrapper);
    progressLabel->setObjectName("progressLabel");

    m_progressBar = new QProgressBar(progressWrapper);
    m_progressBar->setObjectName("progressBar");
    m_progressBar->setRange(0, m_totalQuestions);
    m_progressBar->setTextVisible(false);

    m_progressValueLabel = new QLabel(progressWrapper);
    m_progressValueLabel->setObjectName("progressValue");
    m_progressValueLabel->setAlignment(Qt::AlignRight);

    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(m_progressBar);
    progressLayout->addWidget(m_progressValueLabel);

    headerLayout->addWidget(titleWrapper, 1);
    headerLayout->addWidget(progressWrapper, 0, Qt::AlignTop);

    layout->addWidget(header);
    layout->addWidget(createTagRow());

    auto *questionStem = new QLabel(
        QStringLiteral("在智能备课系统中，如何确保生成的题目能够精准匹配教材章节与难度？"),
        card);
    questionStem->setObjectName("questionStem");
    questionStem->setWordWrap(true);
    layout->addWidget(questionStem);

    auto *optionsPanel = new QFrame(card);
    optionsPanel->setObjectName("optionsPanel");

    auto *optionsLayout = new QVBoxLayout(optionsPanel);
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    optionsLayout->setSpacing(12);

    m_optionGroup = new QButtonGroup(optionsPanel);
    m_optionGroup->setExclusive(true);

    struct OptionDef {
        QString key;
        QString value;
    };

    const QList<OptionDef> options = {
        {QStringLiteral("A"), QStringLiteral("先配置课程范围、版本、年级与章节后再生成")},
        {QStringLiteral("B"), QStringLiteral("直接使用系统推荐而不进行任何筛选")},
        {QStringLiteral("C"), QStringLiteral("只选择难度即可，其他参数忽略")},
        {QStringLiteral("D"), QStringLiteral("在导出后再人工编辑匹配章节")}
    };

    for (int i = 0; i < options.size(); ++i) {
        optionsLayout->addWidget(createOptionItem(options.at(i).key, options.at(i).value));
        if (i == 0) {
            const auto buttons = m_optionGroup->buttons();
            if (!buttons.isEmpty()) {
                buttons.constLast()->setChecked(true);
            }
        }
    }

    layout->addWidget(optionsPanel);

    m_answerSection = createAnswerSection();
    layout->addWidget(m_answerSection);

    m_analysisSection = createAnalysisSection();
    layout->addWidget(m_analysisSection);

    layout->addWidget(createActionRow());

    updateProgress(0);
    return card;
}

QWidget *QuestionBankWindow::createTagRow()
{
    auto *tagRow = new QFrame(this);
    tagRow->setObjectName("tagRow");

    auto *layout = new QHBoxLayout(tagRow);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto createTag = [tagRow](const QString &text, const QString &objectName) {
        auto *label = new QLabel(text, tagRow);
        label->setObjectName(objectName);
        return label;
    };

    layout->addWidget(createTag(QStringLiteral("单选题"), QStringLiteral("tagType")));
    layout->addWidget(createTag(QStringLiteral("难度 · 中等"), QStringLiteral("tagDifficulty")));
    layout->addWidget(createTag(QStringLiteral("教材：第一单元"), QStringLiteral("tagChapter")));
    layout->addStretch(1);

    return tagRow;
}

QWidget *QuestionBankWindow::createOptionItem(const QString &key, const QString &text)
{
    auto *container = new QWidget(this);
    container->setProperty("role", "optionItem");
    container->setProperty("hovered", false);
    container->setProperty("selected", false);
    container->setCursor(Qt::PointingHandCursor);
    container->setMinimumHeight(kOptionMinHeight);
    container->setMouseTracking(true);

    auto *layout = new QHBoxLayout(container);
    layout->setContentsMargins(20, 12, 20, 12);  // 默认边距
    layout->setSpacing(16);

    // 现代化复选框
    auto *checkbox = new ModernCheckBox(container);
    checkbox->setProperty("role", "optionCheckbox");
    checkbox->setCursor(Qt::PointingHandCursor);
    m_optionGroup->addButton(checkbox);
    checkbox->setAutoExclusive(true);

    // 选项标签（A, B, C, D）
    auto *keyLabel = new QLabel(key, container);
    keyLabel->setProperty("role", "optionKey");
    keyLabel->setAlignment(Qt::AlignCenter);
    keyLabel->setFixedSize(32, 32);
    keyLabel->setStyleSheet(QString(
        "QLabel[role=\"optionKey\"] {"
        "  background: #F8F9FA;"
        "  color: #495057;"
        "  border-radius: 16px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "}"
    ));

    // 选项文本
    auto *textLabel = new QLabel(text, container);
    textLabel->setProperty("role", "optionText");
    textLabel->setWordWrap(true);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    layout->addWidget(checkbox, 0, Qt::AlignTop);
    layout->addWidget(keyLabel, 0, Qt::AlignTop);
    layout->addWidget(textLabel, 1);

    // 连接选择信号
    connect(checkbox, &QCheckBox::toggled, this, [this, container, layout, key, text](bool checked) {
        container->setProperty("selected", checked);

        // 调整选中状态的边距（4px留白效果）
        if (checked) {
            layout->setContentsMargins(16, 8, 16, 8);  // 缩小4px形成留白
        } else {
            layout->setContentsMargins(20, 12, 20, 12);  // 恢复默认边距
        }

        // 强制样式重绘
        container->style()->unpolish(container);
        container->style()->polish(container);
        container->update();

        if (checked) {
            qInfo() << "Selected option" << key << text;
        }
    });

    // 安装事件过滤器用于hover效果
    container->installEventFilter(this);
    checkbox->installEventFilter(this);
    keyLabel->installEventFilter(this);
    textLabel->installEventFilter(this);

    m_optionFrames.append(container);
    return container;
}

QFrame *QuestionBankWindow::createAnswerSection()
{
    auto *section = new QFrame(this);
    section->setObjectName("answerSection");

    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(4);

    auto *title = new QLabel(QStringLiteral("正确答案"), section);
    title->setObjectName("answerTitle");

    auto *value = new QLabel(QStringLiteral("A"), section);
    value->setObjectName("answerValue");

    auto *reason = new QLabel(
        QStringLiteral("通过完整配置课程范围、版本、年级与章节，系统才能生成与教学目标匹配的题目。"),
        section);
    reason->setObjectName("answerDescription");
    reason->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(value);
    layout->addWidget(reason);

    return section;
}

QFrame *QuestionBankWindow::createAnalysisSection()
{
    auto *section = new QFrame(this);
    section->setObjectName("analysisSection");

    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(6);

    auto *title = new QLabel(QStringLiteral("解析"), section);
    title->setObjectName("analysisTitle");

    auto *description = new QLabel(
        QStringLiteral("系统通过匹配教材版本、年级与章节，结合题型与难度标签对题库进行聚合检索，再依据教学目标进行二次筛选，保证题目语境与教学内容完全一致。"),
        section);
    description->setObjectName("analysisDescription");
    description->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(description);

    return section;
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
    QFile file(QStringLiteral(":/styles/question_bank.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to load question bank stylesheet from :/styles/question_bank.qss";

        // 尝试备用路径
        QFile fallbackFile(QStringLiteral(":/styles/resources/styles/question_bank.qss"));
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
