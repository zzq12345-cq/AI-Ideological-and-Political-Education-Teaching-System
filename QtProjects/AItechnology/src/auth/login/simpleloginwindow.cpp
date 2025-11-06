#include "simpleloginwindow.h"
#include "../../dashboard/modernmainwindow.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QEvent>
#include <QTimer>
#include <QDebug>
#include <iostream>

SimpleLoginWindow::SimpleLoginWindow(QWidget *parent)
    : QWidget(parent)
    , m_supabaseClient(new SupabaseClient(this))
{
    setupUI();  // 设置UI组件
    setupStyle(); // 设置样式

    // 连接Supabase信号
    connect(m_supabaseClient, &SupabaseClient::loginSuccess, this, &SimpleLoginWindow::onLoginSuccess);
    connect(m_supabaseClient, &SupabaseClient::loginFailed, this, &SimpleLoginWindow::onLoginFailed);
}

SimpleLoginWindow::~SimpleLoginWindow()
{
    // 不再需要删除ui，因为我们不使用UI文件
}

bool SimpleLoginWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == passwordEdit && event->type() == QEvent::Resize) {
        // 重新定位眼睛按钮到右侧
        togglePasswordBtn->move(passwordEdit->width() - 40, (passwordEdit->height() - 30) / 2);
    }
    return QWidget::eventFilter(watched, event);
}

void SimpleLoginWindow::setupUI()
{
    qDebug() << "开始设置UI...";

    // 首先设置基本窗口属性
    setWindowTitle("思想政治智慧课堂");
    resize(1200, 700);
    setMinimumSize(800, 600);

    qDebug() << "窗口基本属性设置完成";

    // 创建主布局
    mainLayout = new QHBoxLayout(this);
    if (!mainLayout) {
        qDebug() << "错误：无法创建主布局";
        return;
    }

    qDebug() << "主布局创建成功";

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    qDebug() << "主布局属性设置完成";

    // 左侧面板 - 柔化红色调背景，显示口号和引言
    leftPanel = new QFrame();
    if (!leftPanel) {
        qDebug() << "错误：无法创建左侧面板";
        return;
    }
    leftPanel->setFixedWidth(720); // 60% of 1200
    leftLayout = new QVBoxLayout(leftPanel);
    if (!leftLayout) {
        qDebug() << "错误：无法创建左侧布局";
        return;
    }

    // 口号标签 - 使用暗金色点缀，体现庄重与典雅
    mottoLabel = new QLabel("\"不忘初心，牢记使命\"");
    mottoLabel->setStyleSheet("color: #C9A64E; font-size: 32px; font-weight: 900; text-align: center; text-shadow: 1px 1px 2px rgba(0,0,0,0.3);");

    // 英文翻译 - 使用浅色调搭配暗金色标题
    QLabel *mottoEnglish = new QLabel("\"Remain true to our original aspiration and keep our mission firmly in mind.\"");
    mottoEnglish->setStyleSheet("color: #E8D5B5; font-size: 18px; font-weight: 500; text-align: center;");

    // 分隔线 - 使用暗金色边框体现典雅
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #C9A64E; height: 1px; border: none;");

    // 引言 - 使用暗金色标题，突出主题
    quoteLabel = new QLabel("\"为中华之崛起而读书\"");
    quoteLabel->setStyleSheet("color: #C9A64E; font-size: 28px; font-weight: bold; text-align: center; text-shadow: 1px 1px 2px rgba(0,0,0,0.3);");

    // 作者 - 使用浅色调
    authorLabel = new QLabel("—— 周恩来 (Zhou Enlai)");
    authorLabel->setStyleSheet("color: #E8D5B5; font-size: 16px; font-weight: 500; font-style: italic;");

    // 英文翻译 - 使用淡雅色调
    translationLabel = new QLabel("\"Study for the rise of China.\"");
    translationLabel->setStyleSheet("color: #D4C5A0; font-size: 14px;");

    // 添加中心图片 - 适配您界面中的图片样式
    QLabel *imageLabel = new QLabel();
    imageLabel->setFixedSize(400, 400); // 更大的尺寸
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setScaledContents(true);

    // 加载实际图片
    QPixmap pixmap(":/images/download.png");
    if (!pixmap.isNull()) {
        imageLabel->setPixmap(pixmap);
        qDebug() << "图片加载成功，尺寸:" << pixmap.size();
    } else {
        // 如果图片加载失败，尝试绝对路径
        pixmap = QPixmap("/Users/zhouzhiqi/QtProjects/AItechnology/src/shared/resources/download.png");
        if (!pixmap.isNull()) {
            imageLabel->setPixmap(pixmap);
            qDebug() << "图片加载成功（绝对路径），尺寸:" << pixmap.size();
        } else {
            qDebug() << "图片加载失败，显示占位符";
            imageLabel->setText("图片加载失败");
            imageLabel->setStyleSheet(
                "QLabel {"
                "  color: #C9A64E;"
                "  font-size: 16px;"
                "  background-color: rgba(255, 255, 255, 0.2);"
                "  border: 1px solid #C9A64E;"
                "  border-radius: 8px;"
                "  padding: 10px;"
                "}"
            );
        }
    }

    // 左侧布局
    leftLayout->addStretch();
    leftLayout->addWidget(imageLabel);
    leftLayout->addSpacing(30);
    leftLayout->addWidget(mottoLabel);
    leftLayout->addWidget(mottoEnglish);
    leftLayout->addSpacing(40);
    leftLayout->addWidget(separator);
    leftLayout->addSpacing(40);
    leftLayout->addWidget(quoteLabel);
    leftLayout->addWidget(authorLabel);
    leftLayout->addWidget(translationLabel);
    leftLayout->addStretch();

    // 右侧面板 - 白色背景，登录表单，更紧凑的布局
    rightPanel = new QFrame();
    if (!rightPanel) {
        qDebug() << "错误：无法创建右侧面板";
        return;
    }
    rightLayout = new QVBoxLayout(rightPanel);
    if (!rightLayout) {
        qDebug() << "错误：无法创建右侧布局";
        return;
    }
    rightLayout->setContentsMargins(50, 50, 50, 50); // 稍微减少边距使界面更紧凑
    qDebug() << "右侧面板创建完成";

  
    // 品牌标题区域 - 【修改1】使用品牌红色
    titleLabel = new QLabel("思想政治智慧课堂");
    titleLabel->setStyleSheet("color: #C62828; font-size: 42px; font-weight: 900; text-align: center; margin: 10px 0; text-shadow: 1px 1px 3px rgba(0,0,0,0.4);");

    subtitleLabel = new QLabel("Ideological & Political Smart Classroom");
    subtitleLabel->setStyleSheet("color: #6B7280; font-size: 14px; text-align: center;");

    // 欢迎信息 - 作为主要操作提示，层级更高
    welcomeLabel = new QLabel("欢迎回来");
    welcomeLabel->setStyleSheet("color: #0F172A; font-size: 32px; font-weight: 900; text-align: center; margin-top: 20px;");

    descLabel = new QLabel("请登录您的账户以继续");
    descLabel->setStyleSheet("color: #6B7280; font-size: 14px; text-align: center;");

    // 用户名输入框
    usernameLabel = new QLabel("用户名或邮箱");
    usernameLabel->setStyleSheet("color: #0F172A; font-size: 16px; font-weight: 500;");

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("请输入您的用户名或邮箱");
    usernameEdit->setFixedHeight(56);
    usernameEdit->setAlignment(Qt::AlignLeft);  // 使用Qt内置对齐方法
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #CFD7E7;"
        "  border-radius: 8px;"
        "  padding: 16px 16px;"  // 明确设置左右padding
        "  font-size: 16px;"
        "  background-color: #F6F6F8;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #C62828;"
        "  outline: none;"
        "}"
    );

    // 密码输入框
    passwordLabel = new QLabel("密码");
    passwordLabel->setStyleSheet("color: #0F172A; font-size: 16px; font-weight: 500;");

    // 密码输入框 - 添加右侧眼睛按钮
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("请输入您的密码");
    passwordEdit->setFixedHeight(56);
    passwordEdit->setAlignment(Qt::AlignLeft);  // 使用Qt内置对齐方法
    // 为右侧按钮预留空间，调整右侧padding
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #CFD7E7;"
        "  border-radius: 8px;"
        "  padding: 16px 50px 16px 16px;"  // 右侧留出50px给眼睛按钮
        "  font-size: 16px;"
        "  background-color: #F6F6F8;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #C62828;"
        "  outline: none;"
        "}"
    );

    // 创建眼睛按钮，放在密码框内部右侧
    togglePasswordBtn = new QPushButton("👁", passwordEdit);  // 设置父对象为passwordEdit
    togglePasswordBtn->setFixedSize(30, 30);
    togglePasswordBtn->setCursor(Qt::PointingHandCursor);
    togglePasswordBtn->move(passwordEdit->width() - 40, (passwordEdit->height() - 30) / 2);  // 定位到右侧
    togglePasswordBtn->setStyleSheet(
        "QPushButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #6B7280;"
        "  font-size: 16px;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover {"
        "  color: #C62828;"
        "}"
    );

    // 记住我和忘记密码 - 重新设计平衡布局
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    rememberCheck = new QCheckBox("记住我");
    rememberCheck->setStyleSheet(
        "QCheckBox {"
        "  color: #0F172A;"
        "  font-size: 14px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 18px;"
        "  height: 18px;"
        "  border-radius: 4px;"
        "  border: 1px solid #CFD7E7;"
        "  background-color: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: #C62828;"
        "  border-color: #C62828;"
        "}"
    );

    forgotPasswordBtn = new QPushButton("忘记密码?");
    forgotPasswordBtn->setStyleSheet(
        "QPushButton {"
        "  color: #C62828;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton:hover {"
        "  color: #8E0000;"
        "  text-decoration: underline;"
        "}"
    );

    // 平衡布局：左侧记住我，右侧忘记密码，中间弹簧
    optionsLayout->addWidget(rememberCheck);
    optionsLayout->addStretch();
    optionsLayout->addWidget(forgotPasswordBtn);

    // 【修改2】登录按钮 - 彻底修改为红色主题
    loginButton = new QPushButton("登 录");
    loginButton->setFixedHeight(56);
    loginButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #C62828;"
        "  color: white;"
        "  border: 2px solid #C62828;"
        "  border-radius: 8px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #D32F2F;"
        "  border-color: #D32F2F;"
        "  color: white;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #B71C1C;"
        "  border-color: #B71C1C;"
        "}"
    );

    // 注册链接 - 重新设计为居中对齐，与忘记密码视觉平衡
    QHBoxLayout *signupLayout = new QHBoxLayout();
    signupLayout->setContentsMargins(0, 15, 0, 0); // 增加上边距

    signupLabel = new QLabel("没有账户?");
    signupLabel->setStyleSheet("color: #6B7280; font-size: 14px;");

    signupBtn = new QPushButton("立即注册");
    signupBtn->setStyleSheet(
        "QPushButton {"
        "  color: #C62828;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton:hover {"
        "  color: #8E0000;"
        "  text-decoration: underline;"
        "}"
    );

    // 居中对齐注册链接
    signupLayout->addStretch();
    signupLayout->addWidget(signupLabel);
    signupLayout->addWidget(signupBtn);
    signupLayout->addStretch();

    // 右侧布局组装 - 【修改4】使用Qt::AlignCenter显式居中对齐
    rightLayout->addStretch();

    // 品牌区域 - 较小间距，作为辅助信息，英文标题也居中对齐
    rightLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(subtitleLabel, 0, Qt::AlignCenter);
    rightLayout->addSpacing(20); // 减少间距

    // 欢迎区域 - 主要操作提示，增加突出感，对齐描述文字
    rightLayout->addWidget(welcomeLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(descLabel, 0, Qt::AlignCenter);
    rightLayout->addSpacing(30); // 适当间距

    // 表单区域
    rightLayout->addWidget(usernameLabel);
    rightLayout->addWidget(usernameEdit);
    rightLayout->addSpacing(16);
    rightLayout->addWidget(passwordLabel);

    // 密码输入区域 - 直接添加密码框，眼睛按钮已在内部
    rightLayout->addWidget(passwordEdit);
    rightLayout->addSpacing(16); // 减少选项区域间距

    // 选项区域 - 记住我/忘记密码
    rightLayout->addLayout(optionsLayout);
    rightLayout->addSpacing(24); // 登录按钮前间距

    // 主要操作按钮
    rightLayout->addWidget(loginButton);
    rightLayout->addLayout(signupLayout);
    rightLayout->addStretch();

    // 添加到主布局
    if (mainLayout && leftPanel && rightPanel) {
        qDebug() << "正在添加面板到主布局...";
        mainLayout->addWidget(leftPanel);
        mainLayout->addWidget(rightPanel);
        qDebug() << "面板添加完成";
    } else {
        qDebug() << "错误：主布局或面板为空";
    }

    // 连接信号
    connect(loginButton, &QPushButton::clicked, this, &SimpleLoginWindow::onLoginClicked);
    connect(signupBtn, &QPushButton::clicked, this, &SimpleLoginWindow::onSignupClicked);
    connect(togglePasswordBtn, &QPushButton::clicked, [this]() {
        if (passwordEdit->echoMode() == QLineEdit::Password) {
            passwordEdit->setEchoMode(QLineEdit::Normal);
            togglePasswordBtn->setText("👁‍🗨");
        } else {
            passwordEdit->setEchoMode(QLineEdit::Password);
            togglePasswordBtn->setText("👁");
        }
    });

    // 使用事件过滤器来监听密码框大小改变事件
    passwordEdit->installEventFilter(this);

    qDebug() << "UI设置完成！";
}

void SimpleLoginWindow::setupStyle()
{
    setStyleSheet(
        "QDialog {"
        "  background-color: white;"
        "}"
        "QFrame#leftPanel {"
        "  background-color: #B71C1C;"  // 【修改3】使用更亮的深红色
        "}"
        "QFrame#rightPanel {"
        "  background-color: white;"
        "}"
    );

    leftPanel->setObjectName("leftPanel");
    rightPanel->setObjectName("rightPanel");
}

void SimpleLoginWindow::onLoginClicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码！");
        return;
    }

    // 检查是否是测试账号
    if ((username == "teacher01" && password == "Teacher@2024") ||
        (username == "student01" && password == "Student@2024") ||
        (username == "admin01" && password == "Admin@2024")) {
        // 测试账号直接登录
        QString role = (username == "teacher01") ? "教师" :
                       (username == "student01") ? "学生" : "管理员";

        QMessageBox::information(this, "登录成功", "欢迎 " + username + "！\n\n正在进入" + role + "端...");
        this->close(); // 关闭登录窗口

        // 打开主界面
        openMainWindow(username, role);
        return;
    }

    // 如果不是测试账号，尝试Supabase登录
    // 检查输入的是否是邮箱格式
    if (username.contains("@")) {
        qDebug() << "尝试Supabase登录:" << username;
        loginButton->setEnabled(false);
        loginButton->setText("登录中...");

        m_supabaseClient->login(username, password);
    } else {
        QMessageBox::warning(this, "登录失败", "请使用正确的用户名或邮箱！\n\n提示：\n• 测试账号：teacher01\n• 或使用邮箱登录");
    }
}

void SimpleLoginWindow::onSignupClicked()
{
    qDebug() << "SimpleLoginWindow::onSignupClicked 调用";
    QMessageBox::information(this, "注册功能", "注册功能正在开发中，敬请期待！");
}

void SimpleLoginWindow::openMainWindow(const QString &username, const QString &role)
{
    qDebug() << "准备打开主窗口...";
    qDebug() << "用户名:" << username << "角色:" << role;

    qDebug() << "正在创建主窗口...";
    ModernMainWindow *mainWindow = new ModernMainWindow(role, username);
    qDebug() << "主窗口创建完成，准备显示...";
    mainWindow->show();
    qDebug() << "主窗口已显示!";
}

void SimpleLoginWindow::onLoginSuccess(const QString &userId, const QString &email)
{
    // 防止重复处理
    if (m_loginProcessed) {
        qDebug() << "登录已处理，跳过重复调用";
        return;
    }
    m_loginProcessed = true;

    qDebug() << "Supabase登录成功! 用户ID:" << userId << "邮箱:" << email;

    loginButton->setEnabled(false);
    loginButton->setText("登录中...");

    // 打开主界面，默认角色为教师
    openMainWindow(email, "教师");

    // 最后关闭登录窗口
    this->close();
}

void SimpleLoginWindow::onLoginFailed(const QString &errorMessage)
{
    qDebug() << "Supabase登录失败:" << errorMessage;

    QMessageBox::warning(this, "登录失败", errorMessage);

    loginButton->setEnabled(true);
    loginButton->setText("登 录");
}

