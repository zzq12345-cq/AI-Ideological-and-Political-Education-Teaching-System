#include "questionbankwindow.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QRadioButton>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace {
constexpr int kSidebarWidth = 360;
constexpr int kControlHeight = 48;
constexpr int kButtonHeight = 44;
constexpr int kContentMinWidth = 1280;
constexpr int kContentMaxWidth = 1440;
}

QuestionBankWindow::QuestionBankWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("questionBankWindow");
    buildUI();
    loadStyle();
}

void QuestionBankWindow::buildUI()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(32, 24, 32, 24);
    rootLayout->setSpacing(0);

    QWidget *contentWrapper = new QWidget(this);
    contentWrapper->setObjectName("contentWrapper");
    contentWrapper->setMinimumWidth(kContentMinWidth);
    contentWrapper->setMaximumWidth(kContentMaxWidth);

    auto *contentLayout = new QVBoxLayout(contentWrapper);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(24);

    contentLayout->addWidget(createNavBar());
    contentLayout->addWidget(createBody(), 1);

    rootLayout->addWidget(contentWrapper, 0, Qt::AlignHCenter);
}

QWidget *QuestionBankWindow::createNavBar()
{
    auto *navBar = new QFrame(this);
    navBar->setObjectName("navBar");

    auto *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(16);

    auto *backButton = new QPushButton("返回", navBar);
    backButton->setObjectName("navBackButton");
    backButton->setFixedHeight(36);

    QWidget *headerWrapper = new QWidget(navBar);
    headerWrapper->setObjectName("headerWrapper");
    auto *headerLayout = new QVBoxLayout(headerWrapper);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    auto *titleLabel = new QLabel("AI 智能备课 - PPT生成", headerWrapper);
    titleLabel->setObjectName("pageTitle");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *subtitleLabel = new QLabel("根据教材、章节和课时，智能生成教学PPT", headerWrapper);
    subtitleLabel->setObjectName("pageSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);

    QWidget *navPlaceholder = new QWidget(navBar);
    navPlaceholder->setFixedWidth(backButton->sizeHint().width());

    navLayout->addWidget(backButton, 0, Qt::AlignLeft);
    navLayout->addWidget(headerWrapper, 1);
    navLayout->addWidget(navPlaceholder, 0, Qt::AlignRight);

    return navBar;
}

QWidget *QuestionBankWindow::createBody()
{
    auto *bodyWrapper = new QWidget(this);
    bodyWrapper->setObjectName("bodyWrapper");

    auto *bodyLayout = new QHBoxLayout(bodyWrapper);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(24);

    bodyLayout->addWidget(createSidebar(), 0, Qt::AlignTop);
    bodyLayout->addWidget(createContentCard(), 1);
    bodyLayout->setStretch(0, 0);
    bodyLayout->setStretch(1, 1);

    return bodyWrapper;
}

QWidget *QuestionBankWindow::createSidebar()
{
    auto *sidebar = new QFrame(this);
    sidebar->setObjectName("sidebarPanel");
    sidebar->setFixedWidth(kSidebarWidth);

    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(24, 24, 24, 24);
    sidebarLayout->setSpacing(20);

    auto *sidebarTitle = new QLabel("智能筛选器", sidebar);
    sidebarTitle->setObjectName("sidebarTitle");

    auto *sidebarDescription = new QLabel("根据教材参数精准筛选题目", sidebar);
    sidebarDescription->setObjectName("sidebarDescription");
    sidebarDescription->setWordWrap(true);

    sidebarLayout->addWidget(sidebarTitle);
    sidebarLayout->addWidget(sidebarDescription);

    const QStringList comboPlaceholders = {
        "课程范围",
        "教材版本",
        "年级学期",
        "选择章节"
    };

    for (const QString &placeholder : comboPlaceholders) {
        auto *combo = new QComboBox(sidebar);
        combo->setProperty("role", "filterCombo");
        combo->setFixedHeight(kControlHeight);
        combo->addItem(placeholder);
        combo->addItem("示例选项 A");
        combo->addItem("示例选项 B");
        combo->setCurrentIndex(0);
        sidebarLayout->addWidget(combo);
    }

    sidebarLayout->addWidget(createFilterGroup("试卷类型",
                                               {"不限", "章节练习", "期中", "期末"},
                                               "paperTypeButton"));

    sidebarLayout->addWidget(createFilterGroup("题目题型",
                                               {"不限", "单选题", "多选题", "判断题", "简答题"},
                                               "questionTypeButton", 2));

    sidebarLayout->addWidget(createFilterGroup("题目难度",
                                               {"不限", "简单", "中等", "困难"},
                                               "difficultyButton"));

    auto *spacer = new QWidget(sidebar);
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sidebarLayout->addWidget(spacer);

    auto *generateButton = new QPushButton("开始生成", sidebar);
    generateButton->setObjectName("generateButton");
    generateButton->setFixedHeight(kControlHeight);
    sidebarLayout->addWidget(generateButton);

    return sidebar;
}

QWidget *QuestionBankWindow::createFilterGroup(const QString &title,
                                               const QStringList &options,
                                               const QString &groupName,
                                               int columns)
{
    auto *groupFrame = new QFrame(this);
    groupFrame->setObjectName("filterGroup");

    auto *groupLayout = new QVBoxLayout(groupFrame);
    groupLayout->setContentsMargins(0, 0, 0, 0);
    groupLayout->setSpacing(12);

    auto *groupLabel = new QLabel(title, groupFrame);
    groupLabel->setProperty("role", "sectionLabel");
    groupLayout->addWidget(groupLabel);

    auto *buttonsWrapper = new QWidget(groupFrame);
    auto *gridLayout = new QGridLayout(buttonsWrapper);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setHorizontalSpacing(12);
    gridLayout->setVerticalSpacing(12);

    auto *buttonGroup = new QButtonGroup(buttonsWrapper);
    buttonGroup->setExclusive(true);

    for (int i = 0; i < options.size(); ++i) {
        auto *filterButton = new QPushButton(options.at(i), buttonsWrapper);
        filterButton->setCheckable(true);
        filterButton->setProperty("role", "filterButton");
        filterButton->setObjectName(groupName + QString::number(i));
        filterButton->setFixedHeight(kButtonHeight);
        if (i == 0) {
            filterButton->setChecked(true);
        }
        buttonGroup->addButton(filterButton);

        const int row = i / columns;
        const int column = i % columns;
        gridLayout->addWidget(filterButton, row, column);
    }

    groupLayout->addWidget(buttonsWrapper);
    return groupFrame;
}

QWidget *QuestionBankWindow::createContentCard()
{
    auto *card = new QFrame(this);
    card->setObjectName("contentCard");

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setSpacing(24);

    auto *metaRow = new QFrame(card);
    metaRow->setObjectName("tagRow");
    auto *metaLayout = new QHBoxLayout(metaRow);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(12);

    auto *typeBadge = new QLabel("单选题", metaRow);
    typeBadge->setObjectName("badgePrimary");
    typeBadge->setStyleSheet("background-color: #E0F2FE; color: #075985;");

    auto *difficultyBadge = new QLabel("中等", metaRow);
    difficultyBadge->setObjectName("badgeSecondary");
    difficultyBadge->setStyleSheet("background-color: #FEF3C7; color: #B45309;");

    auto *progressLabel = new QLabel("进度: 3 / 20", metaRow);
    progressLabel->setObjectName("progressLabel");
    progressLabel->setStyleSheet("color: #475467; font-size: 14px; font-weight: 600;");

    metaLayout->addWidget(typeBadge, 0, Qt::AlignLeft);
    metaLayout->addWidget(difficultyBadge, 0, Qt::AlignLeft);
    metaLayout->addStretch();
    metaLayout->addWidget(progressLabel, 0, Qt::AlignRight);

    auto *questionStem = new QLabel("在“AI 智能备课 - PPT生成”系统中，教师如何保证课堂内容与教材章节精准对应？", card);
    questionStem->setObjectName("questionStem");
    questionStem->setWordWrap(true);

    auto *optionsPanel = new QFrame(card);
    optionsPanel->setObjectName("optionsPanel");
    auto *optionsLayout = new QVBoxLayout(optionsPanel);
    optionsLayout->setContentsMargins(16, 16, 16, 16);
    optionsLayout->setSpacing(12);

    struct OptionRow {
        QString key;
        QString value;
    };

    const QList<OptionRow> options = {
        {"A", "生成PPT后再逐步补充教学要点"},
        {"B", "将所有章节一次性导入模板"},
        {"C", "先完成教材版本、章节与题型的精准选择"},
        {"D", "完全依赖系统默认推荐内容"}
    };

    m_optionGroup = new QButtonGroup(optionsPanel);
    m_optionGroup->setExclusive(true);

    for (const OptionRow &option : options) {
        auto *optionItem = new QFrame(optionsPanel);
        optionItem->setProperty("role", "optionItem");
        auto *optionLayout = new QHBoxLayout(optionItem);
        optionLayout->setContentsMargins(12, 12, 12, 12);
        optionLayout->setSpacing(12);

        auto *optionRadio = new QRadioButton(optionItem);
        optionRadio->setProperty("role", "optionRadio");
        m_optionGroup->addButton(optionRadio);
        if (option.key == "C") {
            optionRadio->setChecked(true);
        }

        auto *optionKey = new QLabel(option.key, optionItem);
        optionKey->setProperty("role", "optionKey");

        auto *optionText = new QLabel(option.value, optionItem);
        optionText->setProperty("role", "optionText");
        optionText->setWordWrap(true);

        optionLayout->addWidget(optionRadio, 0, Qt::AlignTop);
        optionLayout->addWidget(optionKey, 0, Qt::AlignTop);
        optionLayout->addWidget(optionText, 1);

        optionsLayout->addWidget(optionItem);
    }

    auto *divider = new QFrame(card);
    divider->setObjectName("cardDivider");
    divider->setFixedHeight(1);

    auto *answerLabel = new QLabel("答案：C 先完成教材版本、章节与题型的精准选择", card);
    answerLabel->setObjectName("answerLabel");

    auto *analysisLabel = new QLabel(
        "解析：通过明确课程范围和章节，再结合题型与难度的筛选，AI 才能构建精准的知识图谱，从而生成与教学目标完全一致的PPT内容。",
        card);
    analysisLabel->setObjectName("analysisLabel");
    analysisLabel->setWordWrap(true);

    auto *actionsRow = new QFrame(card);
    actionsRow->setObjectName("actionsRow");
    auto *actionsLayout = new QHBoxLayout(actionsRow);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(16);

    auto *prevButton = new QPushButton("上一题", actionsRow);
    prevButton->setObjectName("prevButton");
    prevButton->setFixedHeight(kControlHeight);

    auto *revealButton = new QPushButton("查看答案", actionsRow);
    revealButton->setObjectName("revealButton");
    revealButton->setFixedHeight(kControlHeight);

    auto *exportButton = new QPushButton("导出试卷", actionsRow);
    exportButton->setObjectName("exportButton");
    exportButton->setFixedHeight(kControlHeight);

    actionsLayout->addWidget(prevButton);
    actionsLayout->addWidget(revealButton);
    actionsLayout->addWidget(exportButton);

    cardLayout->addWidget(metaRow);
    cardLayout->addWidget(questionStem);
    cardLayout->addWidget(optionsPanel);
    cardLayout->addWidget(divider);
    cardLayout->addWidget(answerLabel);
    cardLayout->addWidget(analysisLabel);
    cardLayout->addWidget(actionsRow);

    return card;
}

void QuestionBankWindow::loadStyle()
{
    QFile styleFile(":/styles/question_bank.qss");
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QString styleSheet = QString::fromUtf8(styleFile.readAll());
    setStyleSheet(styleSheet);
}
