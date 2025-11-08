#include "questionbankwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFile>
#include <QSizePolicy>
#include <QList>

namespace {
constexpr int kSidebarWidth = 360;
constexpr int kControlHeight = 48;
constexpr int kButtonHeight = 44;
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
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(32, 24, 32, 24);
    rootLayout->setSpacing(0);

    QWidget *contentWrapper = new QWidget(this);
    contentWrapper->setObjectName("contentWrapper");
    contentWrapper->setMaximumWidth(1440);
    contentWrapper->setMinimumWidth(1280);

    QVBoxLayout *contentLayout = new QVBoxLayout(contentWrapper);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(24);

    QFrame *navBar = new QFrame(contentWrapper);
    navBar->setObjectName("navBar");
    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(16);

    QPushButton *backButton = new QPushButton("返回主界面", navBar);
    backButton->setObjectName("navBackButton");

    QWidget *headerWrapper = new QWidget(navBar);
    headerWrapper->setObjectName("headerWrapper");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWrapper);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    QLabel *titleLabel = new QLabel("AI 智能备课 - PPT生成", headerWrapper);
    titleLabel->setObjectName("pageTitle");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subtitleLabel = new QLabel("根据教材、章节和课时，智能生成教学PPT", headerWrapper);
    subtitleLabel->setObjectName("pageSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);

    QWidget *navPlaceholder = new QWidget(navBar);
    navPlaceholder->setObjectName("navPlaceholder");
    navPlaceholder->setFixedWidth(144);

    navLayout->addWidget(backButton, 0, Qt::AlignLeft);
    navLayout->addWidget(headerWrapper, 1);
    navLayout->addWidget(navPlaceholder, 0, Qt::AlignRight);

    QWidget *bodyWrapper = new QWidget(contentWrapper);
    bodyWrapper->setObjectName("bodyWrapper");
    QHBoxLayout *bodyLayout = new QHBoxLayout(bodyWrapper);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(24);

    QWidget *sidebar = createSidebar();
    QWidget *contentCard = createContentCard();

    bodyLayout->addWidget(sidebar, 0);
    bodyLayout->addWidget(contentCard, 1);

    contentLayout->addWidget(navBar);
    contentLayout->addWidget(bodyWrapper);

    rootLayout->addWidget(contentWrapper, 0, Qt::AlignHCenter);
}

QWidget *QuestionBankWindow::createSidebar()
{
    QFrame *sidebar = new QFrame(this);
    sidebar->setObjectName("sidebarPanel");
    sidebar->setFixedWidth(kSidebarWidth);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(24, 24, 24, 24);
    sidebarLayout->setSpacing(20);

    QLabel *sidebarTitle = new QLabel("智能筛选器", sidebar);
    sidebarTitle->setObjectName("sidebarTitle");

    QLabel *sidebarDescription = new QLabel("根据教材参数精准筛选题目", sidebar);
    sidebarDescription->setObjectName("sidebarDescription");
    sidebarDescription->setWordWrap(true);

    sidebarLayout->addWidget(sidebarTitle);
    sidebarLayout->addWidget(sidebarDescription);

    const QStringList comboPlaceholders = {
        "选择课程范围",
        "选择教材版本",
        "选择年级学期",
        "选择章节"
    };

    for (const QString &placeholder : comboPlaceholders) {
        QComboBox *combo = new QComboBox(sidebar);
        combo->setProperty("role", "filterCombo");
        combo->setFixedHeight(kControlHeight);
        combo->addItem(placeholder);
        combo->addItem("示例选项 A");
        combo->addItem("示例选项 B");
        combo->setCurrentIndex(0);
        sidebarLayout->addWidget(combo);
    }

    sidebarLayout->addWidget(
        createFilterGroup("试卷类型",
                          {"不限", "章节练习", "期中", "期末"},
                          "paperTypeButton"));

    sidebarLayout->addWidget(
        createFilterGroup("题目题型",
                          {"不限", "单选题", "多选题", "判断题", "简答题"},
                          "questionTypeButton"));

    sidebarLayout->addWidget(
        createFilterGroup("题目难度",
                          {"不限", "简单", "中等", "困难"},
                          "difficultyButton"));

    QWidget *spacer = new QWidget(sidebar);
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sidebarLayout->addWidget(spacer);

    QPushButton *generateButton = new QPushButton("开始生成", sidebar);
    generateButton->setObjectName("generateButton");
    generateButton->setFixedHeight(kControlHeight);
    sidebarLayout->addWidget(generateButton);

    return sidebar;
}

QWidget *QuestionBankWindow::createFilterGroup(const QString &title,
                                               const QStringList &options,
                                               const QString &groupName)
{
    QFrame *groupFrame = new QFrame(this);
    groupFrame->setObjectName("filterGroup");

    QVBoxLayout *groupLayout = new QVBoxLayout(groupFrame);
    groupLayout->setContentsMargins(0, 0, 0, 0);
    groupLayout->setSpacing(12);

    QLabel *groupLabel = new QLabel(title, groupFrame);
    groupLabel->setProperty("role", "sectionLabel");
    groupLayout->addWidget(groupLabel);

    QWidget *buttonsWrapper = new QWidget(groupFrame);
    QGridLayout *gridLayout = new QGridLayout(buttonsWrapper);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setHorizontalSpacing(12);
    gridLayout->setVerticalSpacing(12);

    QButtonGroup *buttonGroup = new QButtonGroup(buttonsWrapper);
    buttonGroup->setExclusive(true);

    const int columns = 2;

    for (int i = 0; i < options.size(); ++i) {
        QPushButton *filterButton = new QPushButton(options.at(i), buttonsWrapper);
        filterButton->setCheckable(true);
        filterButton->setProperty("role", "filterButton");
        filterButton->setObjectName(groupName + QString::number(i));
        filterButton->setFixedHeight(kButtonHeight);
        buttonGroup->addButton(filterButton);
        if (i == 0) {
            filterButton->setChecked(true);
        }
        const int row = i / columns;
        const int col = i % columns;
        gridLayout->addWidget(filterButton, row, col);
    }

    groupLayout->addWidget(buttonsWrapper);
    return groupFrame;
}

QWidget *QuestionBankWindow::createContentCard()
{
    QFrame *card = new QFrame(this);
    card->setObjectName("contentCard");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setSpacing(24);

    QFrame *tagRow = new QFrame(card);
    tagRow->setObjectName("tagRow");
    QHBoxLayout *tagLayout = new QHBoxLayout(tagRow);
    tagLayout->setContentsMargins(0, 0, 0, 0);
    tagLayout->setSpacing(12);

    QLabel *typeBadge = new QLabel("单选题", tagRow);
    typeBadge->setObjectName("badgePrimary");

    QLabel *levelBadge = new QLabel("中等", tagRow);
    levelBadge->setObjectName("badgeSecondary");

    tagLayout->addWidget(typeBadge, 0, Qt::AlignLeft);
    tagLayout->addWidget(levelBadge, 0, Qt::AlignLeft);
    tagLayout->addStretch();

    QLabel *questionStem = new QLabel("在“AI 智能备课 - PPT生成”系统中，为了保证课堂内容与教材章节精准对应，教师应重点关注的步骤是？", card);
    questionStem->setObjectName("questionStem");
    questionStem->setWordWrap(true);

    QFrame *optionsPanel = new QFrame(card);
    optionsPanel->setObjectName("optionsPanel");
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsPanel);
    optionsLayout->setContentsMargins(16, 16, 16, 16);
    optionsLayout->setSpacing(12);

    struct OptionRow {
        QString key;
        QString text;
    };

    const QList<OptionRow> optionData = {
        {"A", "在生成PPT后再手动补充教学要点"},
        {"B", "将所有章节一次性导入模板"},
        {"C", "先选择教材版本、章节并确认题型偏好"},
        {"D", "只使用系统默认的推荐内容"}
    };

    m_optionGroup = new QButtonGroup(optionsPanel);
    m_optionGroup->setExclusive(true);

    for (const OptionRow &option : optionData) {
        QFrame *optionItem = new QFrame(optionsPanel);
        optionItem->setProperty("role", "optionItem");
        QHBoxLayout *optionLayout = new QHBoxLayout(optionItem);
        optionLayout->setContentsMargins(12, 12, 12, 12);
        optionLayout->setSpacing(12);

        QRadioButton *optionRadio = new QRadioButton(optionItem);
        optionRadio->setProperty("role", "optionRadio");
        m_optionGroup->addButton(optionRadio);
        if (option.key == "C") {
            optionRadio->setChecked(true);
        }

        QLabel *optionKey = new QLabel(option.key, optionItem);
        optionKey->setProperty("role", "optionKey");

        QLabel *optionText = new QLabel(option.text, optionItem);
        optionText->setProperty("role", "optionText");
        optionText->setWordWrap(true);

        optionLayout->addWidget(optionRadio, 0, Qt::AlignTop);
        optionLayout->addWidget(optionKey, 0, Qt::AlignTop);
        optionLayout->addWidget(optionText, 1);
        optionsLayout->addWidget(optionItem);
    }

    QFrame *divider = new QFrame(card);
    divider->setObjectName("cardDivider");
    divider->setFixedHeight(1);

    QLabel *answerLabel = new QLabel("正确答案：C 先选择教材版本、章节并确认题型偏好", card);
    answerLabel->setObjectName("answerLabel");

    QLabel *analysisLabel = new QLabel("解析：系统通过教材范围、章节和题目难度的精细筛选来确保生成内容与教学目标一致。提前设定这些参数能帮助AI构建精准知识图谱，从而输出符合课堂语境的PPT。", card);
    analysisLabel->setObjectName("analysisLabel");
    analysisLabel->setWordWrap(true);

    QFrame *actionsRow = new QFrame(card);
    actionsRow->setObjectName("actionsRow");
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsRow);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(16);

    QPushButton *prevButton = new QPushButton("上一题", actionsRow);
    prevButton->setObjectName("prevButton");
    prevButton->setFixedHeight(kControlHeight);

    QPushButton *revealButton = new QPushButton("查看答案", actionsRow);
    revealButton->setObjectName("revealButton");
    revealButton->setFixedHeight(kControlHeight);

    QPushButton *exportButton = new QPushButton("导出试卷", actionsRow);
    exportButton->setObjectName("exportButton");
    exportButton->setFixedHeight(kControlHeight);

    actionsLayout->addWidget(prevButton);
    actionsLayout->addWidget(revealButton);
    actionsLayout->addWidget(exportButton);

    cardLayout->addWidget(tagRow);
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
