#include "mainwindow.h"
#include "../login/simpleloginwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QFont>
#include <QSplitter>
#include <QScrollArea>

MainWindow::MainWindow(const QString &username, const QString &role, QWidget *parent)
    : QMainWindow(parent)
    , m_username(username)
    , m_role(role)
    , m_currentPage("dashboard")
{
    setupUI();
    setupStyles();

    // 启动时间更新定时器
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    timer->start(1000);
    updateDateTime();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 窗口设置
    setWindowTitle(QString("AI思政智慧课堂系统 - %1").arg(m_role));
    setMinimumSize(1200, 800);
    resize(1400, 900);

    // 创建中心widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(m_splitter);

    // 创建各个组件
    createSidebar();
    createHeader();
    createContentArea();

    // 将组件添加到分割器
    m_splitter->addWidget(m_sidebar);

    // 右侧主内容区域
    QFrame *rightFrame = new QFrame();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    rightLayout->addWidget(m_header);
    rightLayout->addWidget(m_contentArea);

    m_splitter->addWidget(rightFrame);

    // 设置分割器属性
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setHandleWidth(1);

    // 创建默认内容
    createDashboardContent();
}

void MainWindow::createSidebar()
{
    m_sidebar = new QFrame();
    m_sidebar->setObjectName("sidebar");
    m_sidebar->setFixedWidth(256);
    m_sidebar->setStyleSheet(
        "QFrame#sidebar {"
        "  background-color: #ffffff;"
        "  border-right: 1px solid #e5e7eb;"
        "}"
    );

    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(16, 24, 16, 24);
    sidebarLayout->setSpacing(16);

    // 用户信息区域
    QFrame *userInfoFrame = new QFrame();
    QVBoxLayout *userInfoLayout = new QVBoxLayout(userInfoFrame);
    userInfoLayout->setContentsMargins(0, 0, 0, 0);
    userInfoLayout->setSpacing(8);
    userInfoLayout->setAlignment(Qt::AlignCenter);

    // 头像
    m_userAvatar = new QLabel();
    m_userAvatar->setFixedSize(64, 64);
    m_userAvatar->setStyleSheet(
        "QLabel {"
        "  background-color: #C62828;"
        "  border-radius: 32px;"
        "  color: white;"
        "  font-size: 24px;"
        "  font-weight: bold;"
        "}"
    );
    m_userAvatar->setAlignment(Qt::AlignCenter);
    m_userAvatar->setText(m_username.left(1).toUpper());

    // 用户名和角色
    m_userName = new QLabel(m_username);
    m_userName->setStyleSheet("font-weight: bold; font-size: 16px; color: #1f2937;");
    m_userName->setAlignment(Qt::AlignCenter);

    m_userRole = new QLabel(m_role);
    m_userRole->setStyleSheet("color: #6b7280; font-size: 14px;");
    m_userRole->setAlignment(Qt::AlignCenter);

    userInfoLayout->addWidget(m_userAvatar);
    userInfoLayout->addWidget(m_userName);
    userInfoLayout->addWidget(m_userRole);

    // 导航按钮
    sidebarLayout->addWidget(userInfoFrame);
    sidebarLayout->addSpacing(24);

    // 仪表板按钮
    m_dashboardBtn = new QPushButton("📊 仪表板");
    m_dashboardBtn->setObjectName("navButton");
    m_dashboardBtn->setProperty("active", true);
    m_dashboardBtn->setStyleSheet(
        "QPushButton#navButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  padding: 12px 16px;"
        "  border-radius: 8px;"
        "  text-align: left;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  color: #374151;"
        "}"
        "QPushButton#navButton:hover {"
        "  background-color: #f3f4f6;"
        "}"
        "QPushButton#navButton[active=\"true\"] {"
        "  background-color: rgba(198, 40, 40, 0.1);"
        "  color: #C62828;"
        "}"
    );
    connect(m_dashboardBtn, &QPushButton::clicked, this, &MainWindow::onDashboardClicked);

    // 课程管理按钮
    m_coursesBtn = new QPushButton("📚 课程管理");
    m_coursesBtn->setObjectName("navButton");
    m_coursesBtn->setStyleSheet(m_dashboardBtn->styleSheet());
    connect(m_coursesBtn, &QPushButton::clicked, this, &MainWindow::onCoursesClicked);

    // 学生管理按钮（仅教师和管理员可见）
    if (m_role == "教师" || m_role == "管理员") {
        m_studentsBtn = new QPushButton("👥 学生管理");
        m_studentsBtn->setObjectName("navButton");
        m_studentsBtn->setStyleSheet(m_dashboardBtn->styleSheet());
        connect(m_studentsBtn, &QPushButton::clicked, this, &MainWindow::onStudentsClicked);
        sidebarLayout->addWidget(m_studentsBtn);
    }

    // 作业管理按钮
    m_assignmentsBtn = new QPushButton("📝 作业管理");
    m_assignmentsBtn->setObjectName("navButton");
    m_assignmentsBtn->setStyleSheet(m_dashboardBtn->styleSheet());
    connect(m_assignmentsBtn, &QPushButton::clicked, this, &MainWindow::onAssignmentsClicked);

    // 数据分析按钮
    m_analyticsBtn = new QPushButton("📈 数据分析");
    m_analyticsBtn->setObjectName("navButton");
    m_analyticsBtn->setStyleSheet(m_dashboardBtn->styleSheet());
    connect(m_analyticsBtn, &QPushButton::clicked, this, &MainWindow::onAnalyticsClicked);

    // 设置按钮
    m_settingsBtn = new QPushButton("⚙️ 设置");
    m_settingsBtn->setObjectName("navButton");
    m_settingsBtn->setStyleSheet(m_dashboardBtn->styleSheet());
    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

    // 添加按钮到布局
    sidebarLayout->addWidget(m_dashboardBtn);
    sidebarLayout->addWidget(m_coursesBtn);
    sidebarLayout->addWidget(m_assignmentsBtn);
    sidebarLayout->addWidget(m_analyticsBtn);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(m_settingsBtn);

    // 登出按钮
    QPushButton *logoutBtn = new QPushButton("🚪 登出");
    logoutBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #fee2e2;"
        "  color: #dc2626;"
        "  border: none;"
        "  padding: 12px 16px;"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "  background-color: #fecaca;"
        "}"
    );
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    sidebarLayout->addWidget(logoutBtn);
}

void MainWindow::createHeader()
{
    m_header = new QFrame();
    m_header->setObjectName("header");
    m_header->setFixedHeight(64);
    m_header->setStyleSheet(
        "QFrame#header {"
        "  background-color: #ffffff;"
        "  border-bottom: 1px solid #e5e7eb;"
        "}"
    );

    QHBoxLayout *headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    // Logo
    m_logoLabel = new QLabel("思政智慧课堂");
    m_logoLabel->setStyleSheet(
        "font-size: 20px;"
        "font-weight: bold;"
        "color: #C62828;"
    );

    // 搜索框
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索课程、学生或内容...");
    m_searchEdit->setFixedWidth(300);
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #f9fafb;"
        "  border: 1px solid #e5e7eb;"
        "  border-radius: 8px;"
        "  padding: 8px 16px;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #C62828;"
        "  outline: none;"
        "}"
    );

    // 通知按钮
    m_notificationBtn = new QPushButton("🔔");
    m_notificationBtn->setFixedSize(40, 40);
    m_notificationBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 20px;"
        "  font-size: 18px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #f3f4f6;"
        "}"
    );

    // 用户头像按钮
    m_profileBtn = new QPushButton();
    m_profileBtn->setFixedSize(40, 40);
    m_profileBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #C62828;"
        "  border: none;"
        "  border-radius: 20px;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #b71c1c;"
        "}"
    );
    m_profileBtn->setText(m_username.left(1).toUpper());
    connect(m_profileBtn, &QPushButton::clicked, this, &MainWindow::onProfileClicked);

    // 添加到布局
    headerLayout->addWidget(m_logoLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_searchEdit);
    headerLayout->addSpacing(16);
    headerLayout->addWidget(m_notificationBtn);
    headerLayout->addSpacing(8);
    headerLayout->addWidget(m_profileBtn);
}

void MainWindow::createContentArea()
{
    m_contentArea = new QScrollArea();
    m_contentArea->setObjectName("contentArea");
    m_contentArea->setWidgetResizable(true);
    m_contentArea->setStyleSheet(
        "QScrollArea {"
        "  background-color: #f6f6f8;"
        "  border: none;"
        "}"
    );

    m_contentWidget = new QWidget();
    m_contentWidget->setObjectName("contentWidget");
    m_contentArea->setWidget(m_contentWidget);
}

void MainWindow::createDashboardContent()
{
    clearContent();
    m_currentPage = "dashboard";

    QVBoxLayout *layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(24);

    // 标题
    QLabel *titleLabel = new QLabel(QString("欢迎回来，%1").arg(m_username));
    titleLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #1f2937;"
        "margin-bottom: 8px;"
    );
    layout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel("这是您的思政教学仪表板");
    subtitleLabel->setStyleSheet(
        "font-size: 16px;"
        "color: #6b7280;"
        "margin-bottom: 32px;"
    );
    layout->addWidget(subtitleLabel);

    // 统计卡片网格
    QGridLayout *statsLayout = new QGridLayout();
    statsLayout->setSpacing(20);

    // 创建统计卡片
    struct StatCard {
        QString title;
        QString value;
        QString icon;
        QString color;
    };

    QList<StatCard> stats = {
        {"课程总数", "12", "📚", "#3b82f6"},
        {"学生人数", "45", "👥", "#10b981"},
        {"作业发布", "8", "📝", "#f59e0b"},
        {"完成率", "85%", "✅", "#ef4444"}
    };

    for (int i = 0; i < stats.size(); ++i) {
        const auto &stat = stats[i];
        QFrame *card = createStatCard(stat.title, stat.value, stat.icon, stat.color);
        statsLayout->addWidget(card, i / 2, i % 2);
    }

    layout->addLayout(statsLayout);

    // 近期活动
    QLabel *activityTitle = new QLabel("近期活动");
    activityTitle->setStyleSheet(
        "font-size: 24px;"
        "font-weight: bold;"
        "color: #1f2937;"
        "margin: 24px 0 16px 0;"
    );
    layout->addWidget(activityTitle);

    // 活动列表
    QVBoxLayout *activityLayout = new QVBoxLayout();
    activityLayout->setSpacing(12);

    QStringList activities = {
        "发布了新的作业《马克思主义基本原理》",
        "学生张三提交了《毛泽东思想概论》作业",
        "课程《中国特色社会主义理论体系》已开启",
        "收到了3条新的学生反馈"
    };

    for (const QString &activity : activities) {
        QLabel *activityLabel = new QLabel(QString("• %1").arg(activity));
        activityLabel->setStyleSheet(
            "padding: 12px 16px;"
            "background-color: white;"
            "border-radius: 8px;"
            "color: #374151;"
            "border: 1px solid #e5e7eb;"
        );
        activityLayout->addWidget(activityLabel);
    }

    layout->addLayout(activityLayout);
    layout->addStretch();
}

QFrame *MainWindow::createStatCard(const QString &title, const QString &value, const QString &icon, const QString &color)
{
    QFrame *card = new QFrame();
    card->setStyleSheet(
        QString(
        "QFrame {"
        "  background-color: white;"
        "  border-radius: 12px;"
        "  border: 1px solid #e5e7eb;"
        "  padding: 20px;"
        "}"
        )
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(12);

    // 图标和数值
    QHBoxLayout *topLayout = new QHBoxLayout();

    QLabel *iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet(
        QString(
        "font-size: 24px;"
        "background-color: %1;"
        "color: white;"
        "padding: 8px;"
        "border-radius: 8px;"
        "min-width: 40px;"
        "text-align: center;"
        ).arg(color)
    );

    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet(
        "font-size: 28px;"
        "font-weight: bold;"
        "color: #1f2937;"
    );

    topLayout->addWidget(iconLabel);
    topLayout->addStretch();
    topLayout->addWidget(valueLabel);

    cardLayout->addLayout(topLayout);

    // 标题
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(
        "font-size: 14px;"
        "color: #6b7280;"
        "font-weight: 500;"
    );
    cardLayout->addWidget(titleLabel);

    return card;
}

void MainWindow::clearContent()
{
    if (m_contentWidget) {
        delete m_contentWidget;
        m_contentWidget = new QWidget();
        m_contentArea->setWidget(m_contentWidget);
    }
}

void MainWindow::setupStyles()
{
    // 设置全局样式
    setStyleSheet(
        "QMainWindow {"
        "  background-color: #f6f6f8;"
        "}"
        "QWidget {"
        "  font-family: 'Arial', sans-serif;"
        "}"
    );
}

void MainWindow::setupMenuBar()
{
    // 创建菜单栏（如果需要的话）
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪");
}

void MainWindow::updateDateTime()
{
    if (m_timeLabel) {
        m_timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
}

// 槽函数实现
void MainWindow::onLogoutClicked()
{
    int ret = QMessageBox::question(this, "登出确认", "确定要登出系统吗？",
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        // 关闭主窗口，返回登录窗口
        close();

        // 显示登录窗口
        SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
        loginWindow->show();
    }
}

void MainWindow::onProfileClicked()
{
    QMessageBox::information(this, "个人信息",
        QString("用户名：%1\n角色：%2\n状态：在线").arg(m_username).arg(m_role));
}

void MainWindow::onDashboardClicked()
{
    if (m_currentPage != "dashboard") {
        // 重置所有按钮状态
        resetNavButtons();
        m_dashboardBtn->setProperty("active", true);
        m_dashboardBtn->style()->unpolish(m_dashboardBtn);
        m_dashboardBtn->style()->polish(m_dashboardBtn);

        createDashboardContent();
    }
}

void MainWindow::onCoursesClicked()
{
    if (m_currentPage != "courses") {
        resetNavButtons();
        m_coursesBtn->setProperty("active", true);
        m_coursesBtn->style()->unpolish(m_coursesBtn);
        m_coursesBtn->style()->polish(m_coursesBtn);

        createCoursesContent();
    }
}

void MainWindow::onStudentsClicked()
{
    if (m_currentPage != "students") {
        resetNavButtons();
        m_studentsBtn->setProperty("active", true);
        m_studentsBtn->style()->unpolish(m_studentsBtn);
        m_studentsBtn->style()->polish(m_studentsBtn);

        createStudentsContent();
    }
}

void MainWindow::onAssignmentsClicked()
{
    if (m_currentPage != "assignments") {
        resetNavButtons();
        m_assignmentsBtn->setProperty("active", true);
        m_assignmentsBtn->style()->unpolish(m_assignmentsBtn);
        m_assignmentsBtn->style()->polish(m_assignmentsBtn);

        createAssignmentsContent();
    }
}

void MainWindow::onAnalyticsClicked()
{
    if (m_currentPage != "analytics") {
        resetNavButtons();
        m_analyticsBtn->setProperty("active", true);
        m_analyticsBtn->style()->unpolish(m_analyticsBtn);
        m_analyticsBtn->style()->polish(m_analyticsBtn);

        createAnalyticsContent();
    }
}

void MainWindow::onSettingsClicked()
{
    QMessageBox::information(this, "系统设置", "系统设置功能正在开发中...");
}

void MainWindow::resetNavButtons()
{
    QList<QPushButton*> buttons = {m_dashboardBtn, m_coursesBtn, m_studentsBtn,
                                   m_assignmentsBtn, m_analyticsBtn, m_settingsBtn};

    for (QPushButton *btn : buttons) {
        if (btn) {
            btn->setProperty("active", false);
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
        }
    }
}

void MainWindow::createCoursesContent()
{
    clearContent();
    m_currentPage = "courses";

    QVBoxLayout *layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(24);

    QLabel *title = new QLabel("课程管理");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #1f2937;");
    layout->addWidget(title);

    QLabel *desc = new QLabel("管理您的所有思政课程内容");
    desc->setStyleSheet("font-size: 16px; color: #6b7280; margin-bottom: 24px;");
    layout->addWidget(desc);

    // 课程卡片
    QGridLayout *coursesLayout = new QGridLayout();
    coursesLayout->setSpacing(20);

    QStringList courses = {
        "马克思主义基本原理",
        "毛泽东思想概论",
        "中国特色社会主义理论体系",
        "习近平新时代中国特色社会主义思想"
    };

    for (int i = 0; i < courses.size(); ++i) {
        QFrame *courseCard = new QFrame();
        courseCard->setStyleSheet(
            "QFrame {"
            "  background-color: white;"
            "  border-radius: 12px;"
            "  border: 1px solid #e5e7eb;"
            "  padding: 24px;"
            "}"
        );

        QVBoxLayout *cardLayout = new QVBoxLayout(courseCard);

        QLabel *courseTitle = new QLabel(courses[i]);
        courseTitle->setStyleSheet(
            "font-size: 18px;"
            "font-weight: bold;"
            "color: #1f2937;"
            "margin-bottom: 8px;"
        );

        QLabel *courseDesc = new QLabel("点击管理课程详情");
        courseDesc->setStyleSheet("color: #6b7280;");

        QPushButton *manageBtn = new QPushButton("管理课程");
        manageBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #C62828;"
            "  color: white;"
            "  border: none;"
            "  padding: 8px 16px;"
            "  border-radius: 6px;"
            "  font-weight: 500;"
            "}"
            "QPushButton:hover {"
            "  background-color: #b71c1c;"
            "}"
        );

        cardLayout->addWidget(courseTitle);
        cardLayout->addWidget(courseDesc);
        cardLayout->addStretch();
        cardLayout->addWidget(manageBtn);

        coursesLayout->addWidget(courseCard, i / 2, i % 2);
    }

    layout->addLayout(coursesLayout);
    layout->addStretch();
}

void MainWindow::createStudentsContent()
{
    clearContent();
    m_currentPage = "students";

    QVBoxLayout *layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(24);

    QLabel *title = new QLabel("学生管理");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #1f2937;");
    layout->addWidget(title);

    QLabel *desc = new QLabel("查看和管理您的学生信息");
    desc->setStyleSheet("font-size: 16px; color: #6b7280; margin-bottom: 24px;");
    layout->addWidget(desc);

    QLabel *content = new QLabel("学生管理功能正在开发中...");
    content->setStyleSheet(
        "font-size: 18px;"
        "color: #6b7280;"
        "text-align: center;"
        "padding: 60px;"
        "background-color: white;"
        "border-radius: 12px;"
        "border: 1px solid #e5e7eb;"
    );
    content->setAlignment(Qt::AlignCenter);
    layout->addWidget(content);
    layout->addStretch();
}

void MainWindow::createAssignmentsContent()
{
    clearContent();
    m_currentPage = "assignments";

    QVBoxLayout *layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(24);

    QLabel *title = new QLabel("作业管理");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #1f2937;");
    layout->addWidget(title);

    QLabel *desc = new QLabel("创建、发布和批改学生作业");
    desc->setStyleSheet("font-size: 16px; color: #6b7280; margin-bottom: 24px;");
    layout->addWidget(desc);

    QLabel *content = new QLabel("作业管理功能正在开发中...");
    content->setStyleSheet(
        "font-size: 18px;"
        "color: #6b7280;"
        "text-align: center;"
        "padding: 60px;"
        "background-color: white;"
        "border-radius: 12px;"
        "border: 1px solid #e5e7eb;"
    );
    content->setAlignment(Qt::AlignCenter);
    layout->addWidget(content);
    layout->addStretch();
}

void MainWindow::createAnalyticsContent()
{
    clearContent();
    m_currentPage = "analytics";

    QVBoxLayout *layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(24);

    QLabel *title = new QLabel("数据分析");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #1f2937;");
    layout->addWidget(title);

    QLabel *desc = new QLabel("查看教学数据统计和分析报告");
    desc->setStyleSheet("font-size: 16px; color: #6b7280; margin-bottom: 24px;");
    layout->addWidget(desc);

    QLabel *content = new QLabel("数据分析功能正在开发中...");
    content->setStyleSheet(
        "font-size: 18px;"
        "color: #6b7280;"
        "padding: 60px;"
        "background-color: white;"
        "border-radius: 12px;"
        "border: 1px solid #e5e7eb;"
    );
    content->setAlignment(Qt::AlignCenter);
    layout->addWidget(content);
    layout->addStretch();
}