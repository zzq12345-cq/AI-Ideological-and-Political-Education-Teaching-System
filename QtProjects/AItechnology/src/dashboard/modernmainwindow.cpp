#include "modernmainwindow.h"
#include "../auth/login/simpleloginwindow.h"
#include "../ui/aipreparationwidget.h"
#include "../questionbank/QuestionRepository.h"
#include <QApplication>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QLineEdit>
#include <QProgressBar>
#include <QDateTime>
#include <QTimer>
#include <QComboBox>
#include <QShortcut>
#include <QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QLegend>
#include <QBarLegendMarker>
#include <QPieLegendMarker>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>

// 颜色常量 (从 code.html 提取)
const QString PATRIOTIC_RED = "#d32f2f";
const QString PATRIOTIC_RED_LIGHT = "#d32f2f22";
const QString BACKGROUND_LIGHT = "#f6f6f8";
const QString BACKGROUND_DARK = "#101622";
const QString OFF_WHITE = "#FFFFFF";
const QString LIGHT_GRAY = "#F5F5F5";
const QString MEDIUM_GRAY = "#757575";
const QString DARK_GRAY = "#333333";

// 侧栏按钮样式常量
const QString SIDEBAR_BTN_NORMAL =
    R"(QPushButton { background-color: transparent; color: %1; border: none; padding: 10px 12px; font-size: 14px; text-align: left; border-radius: 8px; }
       QPushButton:hover { background-color: %2; })";
const QString SIDEBAR_BTN_ACTIVE =
    R"(QPushButton { background-color: %1; color: %2; border: none; padding: 10px 12px; font-size: 14px; font-weight: bold; text-align: left; border-radius: 8px; }
       QPushButton:hover { background-color: rgba(211, 47, 47, 0.2); })";

// 学情分析数据结构
struct LearningMetrics {
    int engagement;      // 课堂参与度 (%)
    int accuracy;        // 测验正确率 (%)
    int focus;           // 专注度 (%)
    int mastery;         // 掌握 (%)
    int partial;         // 基本掌握 (%)
    int needsWork;       // 需巩固 (%)
};

QMap<QString, LearningMetrics> createSampleData() {
    QMap<QString, LearningMetrics> data;
    LearningMetrics metrics7d = {92, 79, 88, 65, 28, 7};
    LearningMetrics metrics30d = {88, 82, 85, 68, 26, 6};
    LearningMetrics metricsSemester = {90, 85, 87, 70, 24, 6};
    data["近7天"] = metrics7d;
    data["近30天"] = metrics30d;
    data["本学期"] = metricsSemester;
    return data;
}

ModernMainWindow::ModernMainWindow(const QString &userRole, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , currentUserRole(userRole)
    , currentUsername(username)
{
    qDebug() << "=== ModernMainWindow 构造函数开始 ===";
    qDebug() << "用户角色:" << userRole << "用户名:" << username;

    setWindowTitle("思政智慧课堂 - 教师中心");
    setMinimumSize(1400, 900);
    resize(1600, 1000);

    // 初始化试题库数据仓库
    questionRepository = new QuestionRepository(this);
    questionRepository->loadQuestions("data/questions.json");

    initUI();
    setupMenuBar();
    setupStatusBar();
    setupCentralWidget();
    setupStyles();
    applyPatrioticRedTheme();

    // 创建默认页面
    createDashboard();
    contentStack->setCurrentWidget(dashboardWidget);

    qDebug() << "=== ModernMainWindow 构造函数完成 ===";
}

ModernMainWindow::~ModernMainWindow()
{
}

void ModernMainWindow::initUI()
{
    // 设置窗口基本属性
    setStyleSheet("QMainWindow { background-color: " + BACKGROUND_LIGHT + "; }");
}

void ModernMainWindow::setupMenuBar()
{
    QMenuBar* mainMenuBar = this->menuBar();
    mainMenuBar->setStyleSheet("QMenuBar { background-color: " + OFF_WHITE + "; border-bottom: 1px solid #E0E0E0; }");

    // 文件菜单
    QMenu *fileMenu = mainMenuBar->addMenu("文件(&F)");
    QAction *newAction = fileMenu->addAction("新建(&N)");
    newAction->setShortcut(QKeySequence::New);

    QAction *openAction = fileMenu->addAction("打开(&O)");
    openAction->setShortcut(QKeySequence::Open);

    fileMenu->addSeparator();
    logoutAction = fileMenu->addAction("注销(&L)");
    logoutAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(logoutAction, &QAction::triggered, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "注销",
            "确定要注销当前账户吗？",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
            loginWindow->show();
            this->close();
        }
    });

    // 工具菜单
    QMenu *toolsMenu = mainMenuBar->addMenu("工具(&T)");
    settingsAction = toolsMenu->addAction("设置(&S)");
    connect(settingsAction, &QAction::triggered, this, &ModernMainWindow::onSettingsClicked);

    // 帮助菜单
    QMenu *helpMenu = mainMenuBar->addMenu("帮助(&H)");
    helpAction = helpMenu->addAction("帮助文档(&H)");
    connect(helpAction, &QAction::triggered, this, &ModernMainWindow::onHelpClicked);

    helpMenu->addSeparator();
    aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, [](){ QMessageBox::about(nullptr, "关于", "思政智慧课堂 - 教师中心"); });
}

void ModernMainWindow::setupStatusBar()
{
    QStatusBar* mainStatusBar = this->statusBar();
    mainStatusBar->setStyleSheet("QStatusBar { background-color: " + OFF_WHITE + "; color: " + DARK_GRAY + "; border-top: 1px solid #E0E0E0; }");
    mainStatusBar->showMessage("就绪");

    // 添加永久状态信息
    QLabel *statusLabel = new QLabel(QString("当前用户: %1 (%2)").arg(currentUsername).arg(currentUserRole));
    statusLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 12px;");
    mainStatusBar->addPermanentWidget(statusLabel);

    QLabel *timeLabel = new QLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    timeLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 12px;");
    mainStatusBar->addPermanentWidget(timeLabel);

    // 定时更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [timeLabel]() {
        timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    });
    timer->start(1000);
}

void ModernMainWindow::setupCentralWidget()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建主内容区域
    contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建侧边栏 (按照 code.html 的 <aside>)
    sidebar = new QFrame();
    sidebar->setFixedWidth(256); // w-64 = 16rem = 256px
    sidebar->setStyleSheet("QFrame { background-color: " + OFF_WHITE + "; border-right: 1px solid #E0E0E0; }");

    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(16, 16, 16, 16); // p-4 = 16px
    sidebarLayout->setSpacing(16);

    // 创建侧边栏顶部用户资料
    createSidebarProfile();

    // 创建导航菜单
    teacherCenterBtn = new QPushButton("教师中心");
    contentAnalysisBtn = new QPushButton("智能内容分析");
    aiPreparationBtn = new QPushButton("AI智能备课");
    resourceManagementBtn = new QPushButton("试题库");
    learningAnalysisBtn = new QPushButton("学情与教评");
    dataReportBtn = new QPushButton("数据分析报告");

    // 底部按钮
    settingsBtn = new QPushButton("系统设置");
    helpBtn = new QPushButton("帮助中心");

    // 设置侧边栏按钮样式 - 使用统一样式常量
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    dataReportBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    settingsBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    helpBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));

    // 连接信号
    connect(teacherCenterBtn, &QPushButton::clicked, this, &ModernMainWindow::onTeacherCenterClicked);
    connect(contentAnalysisBtn, &QPushButton::clicked, this, &ModernMainWindow::onContentAnalysisClicked);
    connect(aiPreparationBtn, &QPushButton::clicked, this, &ModernMainWindow::onAIPreparationClicked);
    connect(resourceManagementBtn, &QPushButton::clicked, this, &ModernMainWindow::onResourceManagementClicked);
    connect(learningAnalysisBtn, &QPushButton::clicked, this, &ModernMainWindow::onLearningAnalysisClicked);
    connect(dataReportBtn, &QPushButton::clicked, this, &ModernMainWindow::onDataReportClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &ModernMainWindow::onSettingsClicked);
    connect(helpBtn, &QPushButton::clicked, this, &ModernMainWindow::onHelpClicked);

    // 添加按钮到侧边栏
    sidebarLayout->addWidget(teacherCenterBtn);
    sidebarLayout->addWidget(contentAnalysisBtn);
    sidebarLayout->addWidget(aiPreparationBtn);
    sidebarLayout->addWidget(resourceManagementBtn);
    sidebarLayout->addWidget(learningAnalysisBtn);
    sidebarLayout->addWidget(dataReportBtn);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(settingsBtn);
    sidebarLayout->addWidget(helpBtn);

    // 创建内容堆栈窗口
    contentStack = new QStackedWidget();
    contentStack->setStyleSheet("background-color: " + BACKGROUND_LIGHT + ";");

    dashboardWidget = new QWidget();
    contentStack->addWidget(dashboardWidget);

    // 创建 AI 智能备课页面
    aiPreparationWidget = new AIPreparationWidget();
    contentStack->addWidget(aiPreparationWidget);

    // 创建试题库QML页面
    questionBankQuickWidget = new QQuickWidget(this);
    questionBankQuickWidget->setSource(QUrl("qrc:/src/ui/qml/questionbank/QuestionBankPage.qml"));
    questionBankQuickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    questionBankQuickWidget->engine()->rootContext()->setContextProperty("questionRepository", questionRepository);
    contentStack->addWidget(questionBankQuickWidget);

    // 添加到主布局
    contentLayout->addWidget(sidebar);
    contentLayout->addWidget(contentStack);

    mainLayout->addLayout(contentLayout);
}

void ModernMainWindow::createSidebarProfile()
{
    profileWidget = new QFrame();
    QVBoxLayout *profileLayout = new QVBoxLayout(profileWidget);
    profileLayout->setContentsMargins(0, 0, 0, 8);
    profileLayout->setSpacing(8);

    // 创建头像容器 (水平布局)
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->setSpacing(12);

    // 头像占位符
    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(48, 48); // size-12 = 48px
    avatarLabel->setStyleSheet(R"(
        QLabel {
            background-color: )" + PATRIOTIC_RED + R"(;
            border-radius: 24px;
            color: white;
            font-size: 18px;
            font-weight: bold;
        }
    )");
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setText("王");

    // 用户信息
    QVBoxLayout *userInfoLayout = new QVBoxLayout();
    userInfoLayout->setContentsMargins(0, 0, 0, 0);
    userInfoLayout->setSpacing(2);

    QLabel *nameLabel = new QLabel("王老师");
    nameLabel->setStyleSheet("color: " + DARK_GRAY + "; font-size: 16px; font-weight: bold;");

    QLabel *roleLabel = new QLabel("思政教研组");
    roleLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 14px;");

    userInfoLayout->addWidget(nameLabel);
    userInfoLayout->addWidget(roleLabel);

    avatarLayout->addWidget(avatarLabel);
    avatarLayout->addLayout(userInfoLayout);
    avatarLayout->addStretch();

    profileLayout->addLayout(avatarLayout);
    sidebarLayout->addWidget(profileWidget);
}

void ModernMainWindow::createHeaderWidget()
{
    headerWidget = new QFrame();
    headerWidget->setFixedHeight(64); // py-3 = 12px * 2 + line-height ≈ 64px
    headerWidget->setStyleSheet("QFrame { background-color: " + OFF_WHITE + "; border-bottom: 1px solid #E0E0E0; }");

    headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(24, 12, 24, 12); // px-6 py-3
    headerLayout->setSpacing(16);

    // 左侧标题
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(16);

    QLabel *starIcon = new QLabel("⭐");
    starIcon->setStyleSheet("color: " + PATRIOTIC_RED + "; font-size: 24px;");

    titleLabel = new QLabel("思政智慧课堂");
    titleLabel->setStyleSheet("color: " + DARK_GRAY + "; font-size: 18px; font-weight: bold;");

    titleLayout->addWidget(starIcon);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // 搜索框
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(0);

    QLabel *searchIcon = new QLabel("🔍");
    searchIcon->setFixedSize(40, 40);
    searchIcon->setStyleSheet(R"(
        QLabel {
            background-color: )" + LIGHT_GRAY + R"(;
            color: )" + MEDIUM_GRAY + R"(;
            border-radius: 8px 0px 0px 8px;
            font-size: 16px;
            padding-left: 12px;
        }
    )");
    searchIcon->setAlignment(Qt::AlignCenter);

    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("搜索资源、学生...");
    searchInput->setFixedHeight(40);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            background-color: )" + LIGHT_GRAY + R"(;
            border: none;
            border-radius: 0px 8px 8px 0px;
            padding: 0px 16px;
            font-size: 16px;
            color: )" + DARK_GRAY + R"(;
        }
        QLineEdit:focus {
            outline: none;
            border: 2px solid rgba(211, 47, 47, 0.3);
        }
    )");

    searchLayout->addWidget(searchIcon);
    searchLayout->addWidget(searchInput);
    searchLayout->addSpacing(24);

    // 通知按钮
    notificationBtn = new QPushButton("🔔");
    notificationBtn->setFixedSize(40, 40);
    notificationBtn->setStyleSheet(R"(
        QPushButton {
            background-color: )" + LIGHT_GRAY + R"(;
            color: )" + MEDIUM_GRAY + R"(;
            border: none;
            border-radius: 8px;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #E0E0E0;
        }
    )");

    // 头部头像
    headerProfileBtn = new QPushButton();
    headerProfileBtn->setFixedSize(40, 40);
    headerProfileBtn->setStyleSheet(R"(
        QPushButton {
            background-color: )" + PATRIOTIC_RED + R"(;
            color: white;
            border: none;
            border-radius: 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #B71C1C;
        }
    )");
    headerProfileBtn->setText("王");

    headerLayout->addLayout(searchLayout);
    headerLayout->addWidget(notificationBtn);
    headerLayout->addWidget(headerProfileBtn);

    // 搜索框快捷键
    auto slashShortcut = new QShortcut(QKeySequence("/"), this);
    connect(slashShortcut, &QShortcut::activated, this, [this](){ this->searchInput->setFocus(); this->searchInput->selectAll(); });

    // Ctrl+K 快捷键
    auto ctrlKShortcut = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(ctrlKShortcut, &QShortcut::activated, this, [this](){ this->searchInput->setFocus(); this->searchInput->selectAll(); });
}

void ModernMainWindow::createQuickActions()
{
    quickActionsFrame = new QFrame();
    quickActionsFrame->setStyleSheet(R"(
        QFrame {
            background-color: )" + OFF_WHITE + R"(;
            border: 1px solid #E0E0E0;
            border-radius: 12px;
            padding: 24px;
        }
    )");

    QHBoxLayout *quickActionsLayout = new QHBoxLayout(quickActionsFrame);
    quickActionsLayout->setSpacing(16);

    QLabel *quickLabel = new QLabel("需要开启新的课堂活动吗？");
    quickLabel->setStyleSheet("color: " + DARK_GRAY + "; font-size: 16px; font-weight: 500;");

    quickPreparationBtn = new QPushButton("快速备课");
    quickPreparationBtn->setStyleSheet(R"(
        QPushButton {
            background-color: )" + PATRIOTIC_RED_LIGHT + R"(;
            color: )" + PATRIOTIC_RED + R"(;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: bold;
            min-width: 84px;
        }
        QPushButton:hover {
            background-color: rgba(211, 47, 47, 0.2);
        }
    )");

    startClassBtn = new QPushButton("开始授课");
    startClassBtn->setStyleSheet(R"(
        QPushButton {
            background-color: )" + PATRIOTIC_RED + R"(;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: bold;
            min-width: 84px;
        }
        QPushButton:hover {
            background-color: #B71C1C;
        }
    )");

    connect(quickPreparationBtn, &QPushButton::clicked, this, &ModernMainWindow::onQuickPreparationClicked);
    connect(startClassBtn, &QPushButton::clicked, this, &ModernMainWindow::onStartClassClicked);

    quickActionsLayout->addWidget(quickLabel);
    quickActionsLayout->addStretch();
    quickActionsLayout->addWidget(quickPreparationBtn);
    quickActionsLayout->addWidget(startClassBtn);
}

void ModernMainWindow::createCoreFeatures()
{
    coreFeaturesFrame = new QFrame();
    coreFeaturesLayout = new QGridLayout(coreFeaturesFrame);
    coreFeaturesLayout->setSpacing(24);

    // 四个核心功能卡片
    psychologyCard = new QPushButton();
    editDocumentCard = new QPushButton();
    slideshowCard = new QPushButton();
    folderOpenCard = new QPushButton();

    QString cardStyle = R"(
        QPushButton {
            background-color: )" + OFF_WHITE + R"(;
            border: 1px solid #E0E0E0;
            border-radius: 12px;
            padding: 20px;
            text-align: left;
        }
        QPushButton:hover {
            border: 1px solid rgba(211, 47, 47, 0.3);
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        }
    )";

    QStringList icons = {"🧠", "📝", "📊", "📁"};
    QStringList titles = {"智能内容分析", "AI智能备课", "互动教学工具", "资源库管理"};
    QStringList descriptions = {
        "深挖思政元素，把握正确导向",
        "按章节自动生成PPT，一键生成试卷",
        "创新互动形式，激活红色课堂",
        "汇聚权威材料，构筑精神高地"
    };

    QList<QPushButton*> cards = {psychologyCard, editDocumentCard, slideshowCard, folderOpenCard};

    for (int i = 0; i < 4; ++i) {
        QVBoxLayout *cardLayout = new QVBoxLayout(cards[i]);
        cardLayout->setSpacing(8);

        QLabel *iconLabel = new QLabel(icons[i]);
        iconLabel->setStyleSheet("color: " + PATRIOTIC_RED + "; font-size: 24px; font-weight: bold;");

        QLabel *titleLabel = new QLabel(titles[i]);
        titleLabel->setStyleSheet("color: " + DARK_GRAY + "; font-size: 16px; font-weight: bold;");

        QLabel *descLabel = new QLabel(descriptions[i]);
        descLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 14px;");
        descLabel->setWordWrap(true);

        cardLayout->addWidget(iconLabel);
        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(descLabel);
        cardLayout->addStretch();

        cards[i]->setStyleSheet(cardStyle);
        cards[i]->setMinimumHeight(140);
    }

    coreFeaturesLayout->addWidget(psychologyCard, 0, 0);
    coreFeaturesLayout->addWidget(editDocumentCard, 0, 1);
    coreFeaturesLayout->addWidget(slideshowCard, 0, 2);
    coreFeaturesLayout->addWidget(folderOpenCard, 0, 3);
}

void ModernMainWindow::createRecentCourses()
{
    recentCoursesFrame = new QFrame();
    recentCoursesFrame->setStyleSheet(R"(
        QFrame {
            background-color: )" + OFF_WHITE + R"(;
            border: 1px solid #E0E0E0;
            border-radius: 12px;
            padding: 24px;
        }
    )");

    QVBoxLayout *coursesLayout = new QVBoxLayout(recentCoursesFrame);
    coursesLayout->setSpacing(16);

    QLabel *coursesTitle = new QLabel("近期课程");
    coursesTitle->setStyleSheet("color: " + DARK_GRAY + "; font-size: 18px; font-weight: bold;");

    QHBoxLayout *courseInfoLayout = new QHBoxLayout();
    courseInfoLayout->setSpacing(16);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    QLabel *timeLabel = new QLabel("今日, 10:00 AM");
    timeLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 14px;");

    QLabel *courseTitle = new QLabel("当代思潮与青年担当");
    courseTitle->setStyleSheet("color: " + PATRIOTIC_RED + "; font-size: 20px; font-weight: bold;");

    QLabel *classLabel = new QLabel("高二 (2) 班");
    classLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 14px;");

    infoLayout->addWidget(timeLabel);
    infoLayout->addWidget(courseTitle);
    infoLayout->addWidget(classLabel);

    enterClassBtn = new QPushButton("进入课堂");
    enterClassBtn->setStyleSheet(R"(
        QPushButton {
            background-color: )" + PATRIOTIC_RED + R"(;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: bold;
            min-width: 100px;
            min-height: 44px;
        }
        QPushButton:hover {
            background-color: #B71C1C;
        }
    )");

    connect(enterClassBtn, &QPushButton::clicked, this, &ModernMainWindow::onEnterClassClicked);

    courseInfoLayout->addLayout(infoLayout);
    courseInfoLayout->addStretch();
    courseInfoLayout->addWidget(enterClassBtn);

    coursesLayout->addWidget(coursesTitle);
    coursesLayout->addLayout(courseInfoLayout);

    // 设置SizePolicy以避免被高卡片撑出空白
    recentCoursesFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

// 创建指标项组件 - 紧凑的单行信息
QWidget* ModernMainWindow::createMetricItem(const QString& name, const QString& value, const QString& color, const QString& tooltip)
{
    // 容器：单行、高度56px、圆角10、轻底色
    QWidget *row = new QWidget();
    row->setObjectName("metricItem");
    row->setFixedHeight(56);
    row->setAutoFillBackground(false);  // 禁止自动填充背景
    row->setAttribute(Qt::WA_NoSystemBackground, true);  // 禁用系统背景
    row->setStyleSheet(QString(
        "QWidget#metricItem {"
        "  background-color: #F6F6F8;"
        "  border-radius: 10px;"
        "  padding: 0 12px;"
        "}"
        "QWidget#metricItem:hover {"
        "  background-color: rgba(0,0,0,0.05);"
        "}"
    ));

    // 水平布局
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(12, 0, 12, 0);
    rowLayout->setSpacing(8);

    // 左侧：彩色圆点 + 名称
    QHBoxLayout *leftLayout = new QHBoxLayout();
    leftLayout->setSpacing(8);

    // 彩色圆点
    QLabel *dotLabel = new QLabel();
    dotLabel->setFixedSize(10, 10);
    dotLabel->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(color));

    // 名称 - 降一阶与中灰
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("color: #757575; font-size: 13px;");
    nameLabel->setToolTip(tooltip);

    leftLayout->addWidget(dotLabel);
    leftLayout->addWidget(nameLabel);
    leftLayout->addStretch();

    // 右侧：数值 - 等宽字体、右对齐、深色
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setObjectName("valueLabel");
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    valueLabel->setAutoFillBackground(false);  // 确保数值标签也无背景

    // 使用 Consolas 等宽字体
    QFont valueFont("Consolas");
    valueFont.setPointSize(20);
    valueFont.setBold(true);
    valueLabel->setFont(valueFont);
    valueLabel->setStyleSheet("color: #222222;");

    // 添加到行布局
    rowLayout->addLayout(leftLayout);
    rowLayout->addWidget(valueLabel);

    return row;
}

void ModernMainWindow::createLearningAnalytics()
{
    learningAnalyticsFrame = new QFrame();
    learningAnalyticsFrame->setStyleSheet(R"(
        QFrame {
            background-color: #FFFFFF;
            border: none;
            border-radius: 12px;
            padding: 24px;
        }
    )");

    // 添加柔和阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 25));
    learningAnalyticsFrame->setGraphicsEffect(shadow);

    QVBoxLayout *analyticsLayout = new QVBoxLayout(learningAnalyticsFrame);
    analyticsLayout->setSpacing(16);

    // 标题和筛选器的水平布局
    QHBoxLayout *titleLayout = new QHBoxLayout();

    QLabel *analyticsTitle = new QLabel("学情分析");
    analyticsTitle->setStyleSheet("color: #222222; font-size: 18px; font-weight: bold;");
    analyticsTitle->setAlignment(Qt::AlignLeft);

    titleLayout->addWidget(analyticsTitle);
    titleLayout->addStretch();

    // 时间范围选择器 - 统一浅灰底+细描边
    QComboBox *timeRangeCombo = new QComboBox();
    timeRangeCombo->addItems({"近7天", "近30天", "本学期"});
    timeRangeCombo->setCurrentText("近7天");
    timeRangeCombo->setStyleSheet(R"(
        QComboBox {
            background-color: #F6F6F8;
            border: 1px solid #E6E6E6;
            border-radius: 8px;
            padding: 6px 10px;
            font-size: 14px;
            min-width: 112px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #757575;
        }
    )");

    titleLayout->addWidget(timeRangeCombo);

    analyticsLayout->addLayout(titleLayout);

    // 顶部区域：环形图（主视觉） + 指标网格（次要视觉）
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(24);
    topRow->setAlignment(Qt::AlignTop);

    // 左侧：主视觉 - 环形图（Donut Chart）
    QWidget *donutContainer = new QWidget();
    donutContainer->setFixedSize(208, 208);

    // 使用 QStackedLayout（StackAll）实现完美叠加
    QStackedLayout *stackedLayout = new QStackedLayout(donutContainer);
    stackedLayout->setStackingMode(QStackedLayout::StackAll);
    stackedLayout->setContentsMargins(0, 0, 0, 0);

    // 创建环形图 - 设置孔径与尺寸
    QPieSeries *donutSeries = new QPieSeries();
    donutSeries->append("已完成", 85);
    donutSeries->append("未完成", 15);
    donutSeries->setHoleSize(0.66);
    donutSeries->setPieSize(0.90);
    donutSeries->setPieStartAngle(270);

    // 设置slice颜色与边框 - 完全透明边框
    QPieSlice *completedSlice = donutSeries->slices().at(0);
    QPieSlice *remainingSlice = donutSeries->slices().at(1);
    completedSlice->setColor(QColor(PATRIOTIC_RED));
    completedSlice->setBorderColor(Qt::transparent);
    remainingSlice->setColor(QColor("#e6e6e6"));
    remainingSlice->setBorderColor(Qt::transparent);

    QChart *donutChart = new QChart();
    donutChart->addSeries(donutSeries);
    donutChart->setBackgroundBrush(Qt::NoBrush);
    donutChart->setBackgroundRoundness(0);  // 去圆角裁剪
    donutChart->legend()->hide();
    donutChart->setTitle("");

    QChartView *donutChartView = new QChartView(donutChart);
    donutChartView->setRenderHint(QPainter::Antialiasing);
    donutChartView->setFixedSize(208, 208);
    donutChartView->setStyleSheet("QChartView { border: none; background: transparent; }");
    donutChartView->setAutoFillBackground(false);  // 禁止自动填充背景

    // 中心文字叠加容器 - WA_TranslucentBackground
    QWidget *centerTextContainer = new QWidget();
    centerTextContainer->setFixedSize(208, 208);
    centerTextContainer->setAttribute(Qt::WA_TranslucentBackground, true);  // 关键：透明背景
    centerTextContainer->setAutoFillBackground(false);
    QVBoxLayout *centerTextLayout = new QVBoxLayout(centerTextContainer);
    centerTextLayout->setContentsMargins(0, 0, 0, 0);
    centerTextLayout->setAlignment(Qt::AlignCenter);
    centerTextLayout->setSpacing(2);

    QLabel *donutPercentLabel = new QLabel("85%");
    donutPercentLabel->setObjectName("donutPercentLabel");
    donutPercentLabel->setAlignment(Qt::AlignCenter);
    QFont donutPercentFont("Consolas");
    donutPercentFont.setPointSize(32);
    donutPercentFont.setBold(true);
    donutPercentLabel->setFont(donutPercentFont);
    donutPercentLabel->setStyleSheet("color: #333333;");

    QLabel *donutTitleLabel = new QLabel("综合完成度");
    donutTitleLabel->setStyleSheet("color: #757575; font-size: 13px;");
    donutTitleLabel->setAlignment(Qt::AlignCenter);

    centerTextLayout->addWidget(donutPercentLabel);
    centerTextLayout->addWidget(donutTitleLabel);

    // StackAll：图表在下，文字在上
    stackedLayout->addWidget(donutChartView);
    stackedLayout->addWidget(centerTextContainer);
    centerTextContainer->raise();  // 确保文字在最上层

    // 设置容器布局
    donutContainer->setLayout(stackedLayout);

    // 右侧：次要视觉 - 2×2 指标网格 - 固定间距 16×12
    QGridLayout *statsLayout = new QGridLayout();
    statsLayout->setHorizontalSpacing(16);
    statsLayout->setVerticalSpacing(12);

    // 指标项颜色语义固定
    QStringList statLabels = {"课堂参与度", "专注度", "测验正确率", "提问次数"};
    QStringList statValues = {"92%", "88%", "79%", "12"};
    QStringList statColors = {"#2196F3", "#4CAF50", "#FF9800", "#F44336"};
    QStringList tooltips = {
        "参与度=到课率×互动率；时间范围受右上角选择影响（默认近7天）",
        "根据课堂行为数据计算；范围0-100%",
        "近7天滚动口径；样本量=人×题量",
        "统计期间内师生提问次数总和"
    };

    QList<QLabel*> statValueLabels;

    // 创建 2×2 指标网格
    for (int i = 0; i < 4; ++i) {
        QWidget *metricItem = createMetricItem(statLabels[i], statValues[i], statColors[i], tooltips[i]);
        statValueLabels.append(metricItem->findChild<QLabel*>("valueLabel"));
        statsLayout->addWidget(metricItem, i / 2, i % 2);
    }

    // 将环形图和指标网格放入同一行 - 左主右次
    topRow->addWidget(donutContainer, 1, Qt::AlignTop);
    topRow->addLayout(statsLayout, 2);

    analyticsLayout->addLayout(topRow);

    // 图表区域 - 使用 Charts 或降级方案
    chartsContainer = new QWidget();
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsContainer);
    chartsLayout->setSpacing(16);

    // 设置learningAnalyticsFrame的SizePolicy
    learningAnalyticsFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 图表1：柱状图 - 课堂参与度/测验正确率/专注度对比
    QWidget *barChartContainer = new QWidget();
    barChartContainer->setObjectName("barChart");
    QVBoxLayout *barLayout = new QVBoxLayout(barChartContainer);
    barLayout->setContentsMargins(16, 16, 16, 16);
    barLayout->setSpacing(8);

    QLabel *barTitle = new QLabel("三维度评分对比");
    barTitle->setStyleSheet("color: #333333; font-size: 16px; font-weight: bold;");

    QChartView *barChartView = new QChartView();
    barChartView->setRenderHint(QPainter::Antialiasing);
    barChartView->setObjectName("barChartView");
    barChartView->setStyleSheet("QChartView { border: none; background: transparent; }");
    barChartView->setAutoFillBackground(false);  // 禁止自动填充背景

    // 创建柱状图数据
    QBarSet *set0 = new QBarSet("参与度");
    QBarSet *set1 = new QBarSet("正确率");
    QBarSet *set2 = new QBarSet("专注度");
    *set0 << 92 << 79 << 88;
    *set1 << 88 << 82 << 85;
    *set2 << 90 << 85 << 87;

    set0->setColor(QColor(PATRIOTIC_RED));
    set1->setColor(QColor("#2196F3"));
    set2->setColor(QColor("#4CAF50"));

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(set0);
    barSeries->append(set1);
    barSeries->append(set2);

    QChart *barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setBackgroundBrush(Qt::NoBrush);
    barChart->setBackgroundRoundness(0);  // 去圆角裁剪
    barChart->setTitle("");
    barChart->setAnimationOptions(QChart::SeriesAnimations);

    barChart->createDefaultAxes();
    barChart->axisY()->setRange(0, 100);
    QFont axisFont("Microsoft YaHei", 10);
    barChart->axisX()->setLabelsFont(axisFont);
    barChart->axisY()->setLabelsFont(axisFont);
    // 轴标签用深灰 #333
    barChart->axisX()->setLabelsColor(QColor("#333333"));
    barChart->axisY()->setLabelsColor(QColor("#333333"));

    barChartView->setChart(barChart);

    barLayout->addWidget(barTitle);
    barLayout->addWidget(barChartView);

    // 图表2：饼图 - 知识点掌握分布
    QWidget *pieChartContainer = new QWidget();
    pieChartContainer->setObjectName("pieChart");
    pieChartContainer->setAutoFillBackground(false);  // 禁止自动填充背景
    QVBoxLayout *pieLayout = new QVBoxLayout(pieChartContainer);
    pieLayout->setContentsMargins(16, 16, 16, 16);
    pieLayout->setSpacing(8);

    QLabel *pieTitle = new QLabel("知识点掌握分布");
    pieTitle->setStyleSheet("color: #333333; font-size: 16px; font-weight: bold;");

    QChartView *pieChartView = new QChartView();
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setObjectName("pieChartView");
    pieChartView->setStyleSheet("QChartView { border: none; background: transparent; }");
    pieChartView->setAutoFillBackground(false);  // 禁止自动填充背景

    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->append("掌握", 65);
    pieSeries->append("基本掌握", 28);
    pieSeries->append("需巩固", 7);

    QPieSlice *slice0 = pieSeries->slices().at(0);
    QPieSlice *slice1 = pieSeries->slices().at(1);
    QPieSlice *slice2 = pieSeries->slices().at(2);
    slice0->setColor(QColor("#4CAF50"));
    slice1->setColor(QColor("#2196F3"));
    slice2->setColor(QColor("#F44336"));

    slice0->setLabelVisible(true);
    slice1->setLabelVisible(true);
    slice2->setLabelVisible(true);

    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setBackgroundBrush(Qt::NoBrush);
    pieChart->setBackgroundRoundness(0);  // 去圆角裁剪
    pieChart->setTitle("");
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    pieChart->legend()->show();
    pieChart->legend()->setColor(QColor("#333333"));

    pieChartView->setChart(pieChart);

    pieLayout->addWidget(pieTitle);
    pieLayout->addWidget(pieChartView);

    // 图表容器布局
    chartsLayout->addWidget(barChartContainer);
    chartsLayout->addWidget(pieChartContainer);

    // 将图表容器添加到学情分析布局（供外部调用者使用）
    analyticsLayout->addWidget(chartsContainer);

    // 降级提示
    QLabel *fallbackNote = new QLabel("未启用 Qt Charts，已降级为基础视图");
    fallbackNote->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 12px; font-style: italic;");
    fallbackNote->setVisible(false);

    // 连接时间范围选择器的信号
    connect(timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this, timeRangeCombo, donutPercentLabel, statValueLabels, barSeries, pieSeries, donutSeries](int index) {
            QString range = timeRangeCombo->currentText();
            QMap<QString, LearningMetrics> data = createSampleData();
            LearningMetrics metrics = data[range];

            // 更新环形图
            int overallScore = (int)((metrics.engagement * 0.4 + metrics.accuracy * 0.4 + metrics.focus * 0.2));
            donutPercentLabel->setText(QString::number(overallScore) + "%");
            QList<QPieSlice*> donutSlices = donutSeries->slices();
            donutSlices[0]->setValue(overallScore);
            donutSlices[1]->setValue(100 - overallScore);

            // 更新统计数据
            statValueLabels[0]->setText(QString::number(metrics.engagement) + "%");
            statValueLabels[1]->setText(QString::number(metrics.focus) + "%");
            statValueLabels[2]->setText(QString::number(metrics.accuracy) + "%");

            // 更新柱状图数据
            QList<QBarSet*> sets = barSeries->barSets();
            sets[0]->remove(0, sets[0]->count());
            sets[1]->remove(0, sets[1]->count());
            sets[2]->remove(0, sets[2]->count());
            *sets[0] << metrics.engagement << (metrics.engagement + 5) << (metrics.engagement - 3);
            *sets[1] << metrics.accuracy << (metrics.accuracy + 3) << (metrics.accuracy - 4);
            *sets[2] << metrics.focus << (metrics.focus + 2) << (metrics.focus - 5);

            // 更新饼图数据
            QList<QPieSlice*> slices = pieSeries->slices();
            slices[0]->setValue(metrics.mastery);
            slices[1]->setValue(metrics.partial);
            slices[2]->setValue(metrics.needsWork);

            // 更新状态栏
            this->statusBar()->showMessage("学情分析 · " + range);
        });

    // 图表交互
    connect(barSeries, &QBarSeries::clicked, this, [this](int index, QBarSet *set) {
        this->statusBar()->showMessage("柱状图点击：可查看班级/学生下钻（示例）");
    });

    connect(pieSeries, &QPieSeries::clicked, this, [this](QPieSlice *slice) {
        this->statusBar()->showMessage("饼图点击：可查看知识点详细分析（示例）");
    });
}

void ModernMainWindow::createRecentActivities()
{
    recentActivitiesFrame = new QFrame();
    recentActivitiesFrame->setStyleSheet(R"(
        QFrame {
            background-color: )" + OFF_WHITE + R"(;
            border: 1px solid #E0E0E0;
            border-radius: 12px;
            padding: 24px;
        }
    )");

    QVBoxLayout *activitiesLayout = new QVBoxLayout(recentActivitiesFrame);
    activitiesLayout->setSpacing(20);

    QLabel *activitiesTitle = new QLabel("近期活动");
    activitiesTitle->setStyleSheet("color: " + DARK_GRAY + "; font-size: 18px; font-weight: bold;");

    // 活动列表
    QList<QStringList> activities = {
        {QString("《全球化与民族主义》的教案已创建"), "2小时前", "📄", PATRIOTIC_RED_LIGHT},
        {QString("新生\"李明\"已加入高二(2)班"), "昨天, 4:30 PM", "👤", "#4CAF5010"},
        {QString("已有15名学生提交\"历史分析论文\"作业"), "昨天, 11:00 AM", "📤", "#F4433610"},
        {QString("\"冷战纪录片\"已添加至资源库"), "2天前", "📹", "#FF980010"}
    };

    // 先添加标题
    activitiesLayout->addWidget(activitiesTitle);

    for (const auto &activity : activities) {
        QHBoxLayout *activityLayout = new QHBoxLayout();
        activityLayout->setSpacing(12);

        // 活动图标
        QLabel *iconLabel = new QLabel(activity[2]);
        iconLabel->setFixedSize(40, 40);
        iconLabel->setStyleSheet(QString(R"(
            QLabel {
                background-color: %1;
                color: %2;
                border-radius: 20px;
                font-size: 16px;
            }
        )").arg(activity[3], PATRIOTIC_RED));
        iconLabel->setAlignment(Qt::AlignCenter);

        // 活动内容
        QVBoxLayout *contentLayout = new QVBoxLayout();
        contentLayout->setSpacing(2);

        QLabel *descLabel = new QLabel(activity[0]);
        descLabel->setStyleSheet("color: " + DARK_GRAY + "; font-size: 14px;");
        descLabel->setWordWrap(true);

        QLabel *timeLabel = new QLabel(activity[1]);
        timeLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 12px;");

        contentLayout->addWidget(descLabel);
        contentLayout->addWidget(timeLabel);

        activityLayout->addWidget(iconLabel);
        activityLayout->addLayout(contentLayout);
        activityLayout->addStretch();

        activitiesLayout->addLayout(activityLayout);
    }

    activitiesLayout->addStretch();

    // 设置近期活动侧栏的SizePolicy和最大宽度
    recentActivitiesFrame->setMaximumWidth(360);
    recentActivitiesFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void ModernMainWindow::createDashboard()
{
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardWidget);
    dashboardLayout->setContentsMargins(0, 0, 0, 0);
    dashboardLayout->setSpacing(0);

    // 创建顶部工具栏
    createHeaderWidget();
    dashboardLayout->addWidget(headerWidget);

    // 创建滚动区域
    dashboardScrollArea = new QScrollArea();
    dashboardScrollArea->setWidgetResizable(true);
    dashboardScrollArea->setStyleSheet("QScrollArea { border: none; background-color: " + BACKGROUND_LIGHT + "; }");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(32, 32, 32, 32); // p-8 = 32px
    scrollLayout->setSpacing(24);

    // 欢迎标题
    QHBoxLayout *welcomeLayout = new QHBoxLayout();
    welcomeLayout->setSpacing(8);

    welcomeLabel = new QLabel("欢迎回来，王老师！");
    welcomeLabel->setStyleSheet("color: " + DARK_GRAY + "; font-size: 32px; font-weight: bold;");

    subtitleLabel = new QLabel("这是您的课堂活动与教学工具概览。");
    subtitleLabel->setStyleSheet("color: " + MEDIUM_GRAY + "; font-size: 16px;");

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(4);
    titleLayout->addWidget(welcomeLabel);
    titleLayout->addWidget(subtitleLabel);

    welcomeLayout->addLayout(titleLayout);
    welcomeLayout->addStretch();

    scrollLayout->addLayout(welcomeLayout);

    // 快速操作
    createQuickActions();
    scrollLayout->addWidget(quickActionsFrame);

    // 核心功能标题
    QLabel *coreTitle = new QLabel("核心功能");
    coreTitle->setStyleSheet("color: " + DARK_GRAY + "; font-size: 22px; font-weight: bold;");
    scrollLayout->addWidget(coreTitle);

    // 核心功能卡片
    createCoreFeatures();
    scrollLayout->addWidget(coreFeaturesFrame);

    // 按顺序创建组件
    createRecentCourses();         // 左列上侧卡片
    createLearningAnalytics();     // 左列下侧卡片
    createRecentActivities();      // 右列侧栏卡片

    // 创建两列网格：左列堆叠两个卡片，右列一个侧栏
    QFrame *dashboardGridFrame = new QFrame();
    QGridLayout *grid = new QGridLayout(dashboardGridFrame);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(24);
    grid->setColumnStretch(0, 2);   // 左列：占2份宽度（近期课程+学情分析垂直堆叠）
    grid->setColumnStretch(1, 1);   // 右列：占1份宽度（近期活动侧栏）

    // 左列：垂直堆叠容器
    QFrame *leftStackFrame = new QFrame();
    QVBoxLayout *leftStack = new QVBoxLayout(leftStackFrame);
    leftStack->setContentsMargins(0, 0, 0, 0);
    leftStack->setSpacing(24);
    leftStack->addWidget(recentCoursesFrame);
    leftStack->addWidget(learningAnalyticsFrame);

    // 放入网格
    grid->addWidget(leftStackFrame, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    grid->addWidget(recentActivitiesFrame, 0, 1, Qt::AlignTop | Qt::AlignLeft);

    // 添加到滚动布局
    scrollLayout->addWidget(dashboardGridFrame);

    // 设置滚动布局间距
    scrollLayout->setSpacing(24);

    // 不在底部重复显示近期活动

    scrollLayout->addStretch();

    dashboardScrollArea->setWidget(scrollContent);
    dashboardLayout->addWidget(dashboardScrollArea);
}

void ModernMainWindow::setupStyles()
{
    // 应用整体样式
    this->setStyleSheet(R"(
        QMainWindow {
            background-color: )" + BACKGROUND_LIGHT + R"(;
            font-family: "Microsoft YaHei", "PingFang SC", sans-serif;
        }
        QMenuBar {
            background-color: )" + OFF_WHITE + R"(;
            color: )" + DARK_GRAY + R"(;
            font-size: 14px;
            border-bottom: 1px solid #E0E0E0;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
        }
        QMenuBar::item:selected {
            background-color: rgba(0, 0, 0, 0.05);
        }
        QStatusBar {
            background-color: )" + OFF_WHITE + R"(;
            color: )" + MEDIUM_GRAY + R"(;
            font-size: 12px;
            border-top: 1px solid #E0E0E0;
        }
        QScrollArea {
            background-color: )" + BACKGROUND_LIGHT + R"(;
            border: none;
        }
        QScrollBar:vertical {
            background-color: #F0F0F0;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: )" + MEDIUM_GRAY + R"(;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: )" + DARK_GRAY + R"(;
        }
    )");
}

void ModernMainWindow::applyPatrioticRedTheme()
{
    // 确保主题一致性
    this->update();
}

// 槽函数实现
void ModernMainWindow::onTeacherCenterClicked()
{
    // 重置所有按钮样式
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    dataReportBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));

    contentStack->setCurrentWidget(dashboardWidget);
    this->statusBar()->showMessage("教师中心");
}

void ModernMainWindow::onContentAnalysisClicked()
{
    onTeacherCenterClicked();
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    this->statusBar()->showMessage("智能内容分析");
}

void ModernMainWindow::onAIPreparationClicked()
{
    // 重置所有按钮样式
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    dataReportBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));

    // 切换到AI智能备课页面
    contentStack->setCurrentWidget(aiPreparationWidget);
    this->statusBar()->showMessage("AI智能备课");
}

void ModernMainWindow::onResourceManagementClicked()
{
    // 重置所有按钮样式
    contentAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    aiPreparationBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    resourceManagementBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    dataReportBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));
    teacherCenterBtn->setStyleSheet(SIDEBAR_BTN_NORMAL.arg(DARK_GRAY, LIGHT_GRAY));

    // 切换到试题库页面
    contentStack->setCurrentWidget(questionBankQuickWidget);
    this->statusBar()->showMessage("试题库");
}

void ModernMainWindow::onLearningAnalysisClicked()
{
    onTeacherCenterClicked();
    learningAnalysisBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    this->statusBar()->showMessage("学情与教评");
}

void ModernMainWindow::onDataReportClicked()
{
    onTeacherCenterClicked();
    dataReportBtn->setStyleSheet(SIDEBAR_BTN_ACTIVE.arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    this->statusBar()->showMessage("数据分析报告");
}

void ModernMainWindow::onSettingsClicked()
{
    QMessageBox::information(this, "系统设置", "系统设置功能正在开发中...");
}

void ModernMainWindow::onHelpClicked()
{
    QMessageBox::information(this, "帮助中心", "帮助中心功能正在开发中...");
}

void ModernMainWindow::onQuickPreparationClicked()
{
    QMessageBox::information(this, "快速备课", "快速备课功能正在开发中...");
}

void ModernMainWindow::onStartClassClicked()
{
    QMessageBox::information(this, "开始授课", "开始授课功能正在开发中...");
}

void ModernMainWindow::onEnterClassClicked()
{
    QMessageBox::information(this, "进入课堂", "进入课堂功能正在开发中...");
}