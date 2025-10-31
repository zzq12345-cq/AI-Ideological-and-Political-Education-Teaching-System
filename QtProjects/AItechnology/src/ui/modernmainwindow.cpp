#include "modernmainwindow.h"
#include "../login/identicalloginwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QPixmap>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QProgressBar>
#include <QDateTime>
#include <QTimer>

ModernMainWindow::ModernMainWindow(const QString &userRole, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , currentUserRole(userRole)
    , currentUsername(username)
{
    setWindowTitle("AI思政智慧课堂系统");
    setMinimumSize(1400, 900);
    resize(1600, 1000);

    initUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
    setupStyles();

    // 创建默认页面
    createDashboard();
    contentStack->setCurrentWidget(dashboardWidget);
}

ModernMainWindow::~ModernMainWindow()
{
}

void ModernMainWindow::initUI()
{
    // 设置窗口图标和基本属性
    setWindowIcon(QIcon(":/icons/app_icon.png"));
    setStyleSheet("QMainWindow { background-color: #F5F5F5; }");
}

void ModernMainWindow::setupMenuBar()
{
    QMenuBar* mainMenuBar = this->menuBar();

    // 文件菜单
    QMenu *fileMenu = mainMenuBar->addMenu("文件(&F)");

    QAction *newAction = fileMenu->addAction("新建(&N)");
    newAction->setShortcut(QKeySequence::New);

    QAction *openAction = fileMenu->addAction("打开(&O)");
    openAction->setShortcut(QKeySequence::Open);

    fileMenu->addSeparator();

    logoutAction = fileMenu->addAction("注销(&L)");
    logoutAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(logoutAction, &QAction::triggered, this, &ModernMainWindow::onLogoutClicked);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // 编辑菜单
    QMenu *editMenu = mainMenuBar->addMenu("编辑(&E)");
    QAction *undoAction = editMenu->addAction("撤销(&U)");
    undoAction->setShortcut(QKeySequence::Undo);

    QAction *redoAction = editMenu->addAction("重做(&R)");
    redoAction->setShortcut(QKeySequence::Redo);

    // 视图菜单
    QMenu *viewMenu = mainMenuBar->addMenu("视图(&V)");
    QAction *fullscreenAction = viewMenu->addAction("全屏(&F)");
    fullscreenAction->setShortcut(QKeySequence::FullScreen);

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
}

void ModernMainWindow::setupToolBar()
{
    mainToolBar = addToolBar("主工具栏");
    mainToolBar->setMovable(false);
    mainToolBar->setIconSize(QSize(24, 24));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // 添加工具栏按钮
    QAction *dashboardAction = mainToolBar->addAction("🏠 仪表板");
    connect(dashboardAction, &QAction::triggered, this, &ModernMainWindow::onDashboardClicked);

    QAction *coursesAction = mainToolBar->addAction("📚 课程");
    connect(coursesAction, &QAction::triggered, this, &ModernMainWindow::onCoursesClicked);

    QAction *assignmentsAction = mainToolBar->addAction("📝 作业");
    connect(assignmentsAction, &QAction::triggered, this, &ModernMainWindow::onAssignmentsClicked);

    QAction *analyticsAction = mainToolBar->addAction("📊 分析");
    connect(analyticsAction, &QAction::triggered, this, &ModernMainWindow::onAnalyticsClicked);

    mainToolBar->addSeparator();

    QAction *messagesAction = mainToolBar->addAction("💬 消息");
    connect(messagesAction, &QAction::triggered, this, &ModernMainWindow::onMessagesClicked);

    mainToolBar->addSeparator();

    profileAction = mainToolBar->addAction("👤 个人资料");
    connect(profileAction, &QAction::triggered, this, &ModernMainWindow::onProfileClicked);
}

void ModernMainWindow::setupStatusBar()
{
    QStatusBar* mainStatusBar = this->statusBar();
    mainStatusBar->showMessage("就绪");

    // 添加永久状态信息
    QLabel *statusLabel = new QLabel(QString("当前用户: %1 (%2)").arg(currentUsername).arg(currentUserRole));
    mainStatusBar->addPermanentWidget(statusLabel);

    QLabel *timeLabel = new QLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
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

    // 创建顶部欢迎栏
    QFrame *topBar = new QFrame();
    topBar->setFixedHeight(80);
    topBar->setStyleSheet("background-color: #C62828; color: white;");

    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(30, 0, 30, 0);

    welcomeLabel = new QLabel(QString("欢迎回来，%1").arg(currentUsername));
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");

    userRoleLabel = new QLabel(QString("身份：%1").arg(currentUserRole));
    userRoleLabel->setStyleSheet("font-size: 16px; color: #FFE082;");

    usernameLabel = new QLabel(QString("用户名：%1").arg(currentUsername));
    usernameLabel->setStyleSheet("font-size: 16px; color: #FFE082;");

    topBarLayout->addWidget(welcomeLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(userRoleLabel);
    topBarLayout->addSpacing(20);
    topBarLayout->addWidget(usernameLabel);

    mainLayout->addWidget(topBar);

    // 创建主内容区域
    contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 创建侧边栏
    sidebar = new QFrame();
    sidebar->setFixedWidth(250);
    sidebar->setStyleSheet("background-color: #263238; color: white;");

    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 20, 0, 20);
    sidebarLayout->setSpacing(5);

    // 侧边栏按钮
    dashboardBtn = new QPushButton("🏠 仪表板");
    coursesBtn = new QPushButton("📚 课程管理");
    assignmentsBtn = new QPushButton("📝 作业系统");
    analyticsBtn = new QPushButton("📊 学情分析");
    messagesBtn = new QPushButton("💬 消息中心");
    settingsBtn = new QPushButton("⚙️ 系统设置");
    logoutBtn = new QPushButton("🚪 注销登录");

    // 设置按钮样式
    QString sidebarButtonStyle = R"(
        QPushButton {
            background-color: transparent;
            color: white;
            border: none;
            padding: 15px 20px;
            font-size: 14px;
            text-align: left;
            border-radius: 0;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.1);
            border-left: 4px solid #C62828;
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )";

    dashboardBtn->setStyleSheet(sidebarButtonStyle);
    coursesBtn->setStyleSheet(sidebarButtonStyle);
    assignmentsBtn->setStyleSheet(sidebarButtonStyle);
    analyticsBtn->setStyleSheet(sidebarButtonStyle);
    messagesBtn->setStyleSheet(sidebarButtonStyle);
    settingsBtn->setStyleSheet(sidebarButtonStyle);
    logoutBtn->setStyleSheet(sidebarButtonStyle);

    // 连接信号
    connect(dashboardBtn, &QPushButton::clicked, this, &ModernMainWindow::onDashboardClicked);
    connect(coursesBtn, &QPushButton::clicked, this, &ModernMainWindow::onCoursesClicked);
    connect(assignmentsBtn, &QPushButton::clicked, this, &ModernMainWindow::onAssignmentsClicked);
    connect(analyticsBtn, &QPushButton::clicked, this, &ModernMainWindow::onAnalyticsClicked);
    connect(messagesBtn, &QPushButton::clicked, this, &ModernMainWindow::onMessagesClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &ModernMainWindow::onSettingsClicked);
    connect(logoutBtn, &QPushButton::clicked, this, &ModernMainWindow::onLogoutClicked);

    // 添加按钮到侧边栏
    sidebarLayout->addWidget(dashboardBtn);
    sidebarLayout->addWidget(coursesBtn);
    sidebarLayout->addWidget(assignmentsBtn);
    sidebarLayout->addWidget(analyticsBtn);
    sidebarLayout->addWidget(messagesBtn);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(settingsBtn);
    sidebarLayout->addWidget(logoutBtn);

    // 创建内容堆栈窗口
    contentStack = new QStackedWidget();
    contentStack->setStyleSheet("background-color: white;");

    // 创建各个页面
    dashboardWidget = new QWidget();
    coursesWidget = new QWidget();
    assignmentsWidget = new QWidget();
    analyticsWidget = new QWidget();
    messagesWidget = new QWidget();

    contentStack->addWidget(dashboardWidget);
    contentStack->addWidget(coursesWidget);
    contentStack->addWidget(assignmentsWidget);
    contentStack->addWidget(analyticsWidget);
    contentStack->addWidget(messagesWidget);

    // 添加到主布局
    contentLayout->addWidget(sidebar);
    contentLayout->addWidget(contentStack);

    mainLayout->addLayout(contentLayout);
}

void ModernMainWindow::setupStyles()
{
    // 设置整体样式
    this->setStyleSheet(R"(
        QMainWindow {
            background-color: #F5F5F5;
        }
        QMenuBar {
            background-color: #263238;
            color: white;
            font-size: 14px;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
        }
        QMenuBar::item:selected {
            background-color: rgba(255, 255, 255, 0.1);
        }
        QToolBar {
            background-color: #37474F;
            color: white;
            spacing: 3px;
            border: none;
        }
        QToolBar::handle {
            background-color: rgba(255, 255, 255, 0.2);
            width: 8px;
            margin: 4px;
        }
        QStatusBar {
            background-color: #263238;
            color: white;
            font-size: 12px;
        }
    )");
}

void ModernMainWindow::createDashboard()
{
    QVBoxLayout *layout = new QVBoxLayout(dashboardWidget);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    // 标题
    QLabel *title = new QLabel("仪表板概览");
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #263238; margin-bottom: 20px;");
    layout->addWidget(title);

    // 统计卡片区域
    QGridLayout *cardsLayout = new QGridLayout();
    cardsLayout->setSpacing(20);

    // 创建统计卡片
    QStringList cardTitles = {"总课程数", "待完成作业", "已完成学习", "学习进度"};
    QStringList cardValues = {"12", "5", "28", "78%"};
    QStringList cardColors = {"#4CAF50", "#FF9800", "#2196F3", "#9C27B0"};

    for (int i = 0; i < 4; ++i) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString(
            "QFrame {"
            "   background-color: white;"
            "   border: 1px solid #E0E0E0;"
            "   border-radius: 8px;"
            "   padding: 20px;"
            "}"
            "QFrame:hover {"
            "   border: 2px solid %1;"
            "}"
        ).arg(cardColors[i]));

        QVBoxLayout *cardLayout = new QVBoxLayout(card);

        QLabel *valueLabel = new QLabel(cardValues[i]);
        valueLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(cardColors[i]));
        valueLabel->setAlignment(Qt::AlignCenter);

        QLabel *titleLabel = new QLabel(cardTitles[i]);
        titleLabel->setStyleSheet("font-size: 16px; color: #666666;");
        titleLabel->setAlignment(Qt::AlignCenter);

        cardLayout->addWidget(valueLabel);
        cardLayout->addWidget(titleLabel);

        cardsLayout->addWidget(card, i / 2, i % 2);
    }

    layout->addLayout(cardsLayout);

    // 最近活动区域
    QLabel *activityTitle = new QLabel("最近活动");
    activityTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #263238; margin: 20px 0;");
    layout->addWidget(activityTitle);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: 1px solid #E0E0E0; border-radius: 8px; }");

    QWidget *activityWidget = new QWidget();
    QVBoxLayout *activityLayout = new QVBoxLayout(activityWidget);

    QStringList activities = {
        "完成了《马克思主义基本原理》第3章学习",
        "提交了《毛泽东思想概论》作业",
        "参与了《习近平新时代中国特色社会主义思想》课堂讨论",
        "查看了《中国近现代史纲要》课程资料",
        "完成了《思想道德修养》在线测验"
    };

    for (const QString &activity : activities) {
        QLabel *activityLabel = new QLabel(QString("• %1").arg(activity));
        activityLabel->setStyleSheet("font-size: 14px; color: #333333; padding: 8px; border-bottom: 1px solid #F0F0F0;");
        activityLayout->addWidget(activityLabel);
    }

    activityLayout->addStretch();
    scrollArea->setWidget(activityWidget);
    layout->addWidget(scrollArea);
}

void ModernMainWindow::createCoursesPage()
{
    QVBoxLayout *layout = new QVBoxLayout(coursesWidget);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("课程管理");
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #263238;");
    layout->addWidget(title);

    QLabel *content = new QLabel("课程管理功能正在开发中...");
    content->setStyleSheet("font-size: 16px; color: #666666;");
    content->setAlignment(Qt::AlignCenter);
    layout->addWidget(content);
}

void ModernMainWindow::createAssignmentsPage()
{
    QVBoxLayout *layout = new QVBoxLayout(assignmentsWidget);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("作业系统");
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #263238;");
    layout->addWidget(title);

    QLabel *content = new QLabel("作业系统功能正在开发中...");
    content->setStyleSheet("font-size: 16px; color: #666666;");
    content->setAlignment(Qt::AlignCenter);
    layout->addWidget(content);
}

void ModernMainWindow::createAnalyticsPage()
{
    QVBoxLayout *layout = new QVBoxLayout(analyticsWidget);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("学情分析");
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #263238;");
    layout->addWidget(title);

    QLabel *content = new QLabel("学情分析功能正在开发中...");
    content->setStyleSheet("font-size: 16px; color: #666666;");
    content->setAlignment(Qt::AlignCenter);
    layout->addWidget(content);
}

void ModernMainWindow::createMessagesPage()
{
    QVBoxLayout *layout = new QVBoxLayout(messagesWidget);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("消息中心");
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #263238;");
    layout->addWidget(title);

    QLabel *content = new QLabel("消息中心功能正在开发中...");
    content->setStyleSheet("font-size: 16px; color: #666666;");
    content->setAlignment(Qt::AlignCenter);
    layout->addWidget(content);
}

// 槽函数实现
void ModernMainWindow::onLogoutClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "注销",
        "确定要注销当前账户吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 显示登录窗口
        IdenticalLoginWindow *loginWindow = new IdenticalLoginWindow();
        loginWindow->show();
        this->close();
    }
}

void ModernMainWindow::onProfileClicked()
{
    QMessageBox::information(this, "个人资料",
        QString("用户名：%1\n角色：%2\n\n个人资料功能正在开发中...")
        .arg(currentUsername).arg(currentUserRole));
}

void ModernMainWindow::onSettingsClicked()
{
    QMessageBox::information(this, "系统设置", "系统设置功能正在开发中...");
}

void ModernMainWindow::onDashboardClicked()
{
    contentStack->setCurrentWidget(dashboardWidget);
    this->statusBar()->showMessage("仪表板");
}

void ModernMainWindow::onCoursesClicked()
{
    contentStack->setCurrentWidget(coursesWidget);
    this->statusBar()->showMessage("课程管理");
}

void ModernMainWindow::onAssignmentsClicked()
{
    contentStack->setCurrentWidget(assignmentsWidget);
    this->statusBar()->showMessage("作业系统");
}

void ModernMainWindow::onAnalyticsClicked()
{
    contentStack->setCurrentWidget(analyticsWidget);
    this->statusBar()->showMessage("学情分析");
}

void ModernMainWindow::onMessagesClicked()
{
    contentStack->setCurrentWidget(messagesWidget);
    this->statusBar()->showMessage("消息中心");
}

void ModernMainWindow::onHelpClicked()
{
    QMessageBox::information(this, "帮助",
        "AI思政智慧课堂系统 v1.0\n\n"
        "这是一个基于人工智能的思想政治教育智慧课堂系统。\n\n"
        "主要功能：\n"
        "• 智能备课\n"
        "• 课堂教学\n"
        "• 作业评价\n"
        "• 学情分析\n\n"
        "技术支持：support@aiedu.com");
}