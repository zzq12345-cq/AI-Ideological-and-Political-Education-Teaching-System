#include "simpleloginwindow.h"
#include "../../dashboard/modernmainwindow.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QSize>
#include <QEvent>
#include <QTimer>
#include <QDebug>
#include <iostream>

SimpleLoginWindow::SimpleLoginWindow(QWidget *parent)
    : QWidget(parent)
    , m_supabaseClient(new SupabaseClient(this))
    , m_settings(new QSettings(this))
    , m_eyeShowIcon(QIcon(QStringLiteral(":/images/眼睛_显示.png")))
    , m_eyeHideIcon(QIcon(QStringLiteral(":/images/眼睛_隐藏.png")))
{
    setupUI();  // 设置UI组件
    setupStyle(); // 设置样式

    // 连接Supabase信号
    connect(m_supabaseClient, &SupabaseClient::loginSuccess, this, &SimpleLoginWindow::onLoginSuccess);
    connect(m_supabaseClient, &SupabaseClient::loginFailed, this, &SimpleLoginWindow::onLoginFailed);

    // 检查是否有记住的凭证
    if (hasRememberedCredentials()) {
        loadRememberedCredentials();
    }
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
    resize(1400, 900);
    setMinimumSize(1000, 800);

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
    mottoLabel->setStyleSheet("color: #C9A64E; font-size: 32px; font-weight: 900; text-align: center; background-color: transparent;");

    // 英文翻译 - 使用浅色调搭配暗金色标题
    QLabel *mottoEnglish = new QLabel("\"Remain true to our original aspiration and keep our mission firmly in mind.\"");
    mottoEnglish->setStyleSheet("color: #E8D5B5; font-size: 18px; font-weight: 500; text-align: center; background-color: transparent;");

    // 分隔线 - 使用暗金色边框体现典雅
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #C9A64E; height: 1px; border: none;");

    // 引言 - 使用暗金色标题，突出主题
    quoteLabel = new QLabel("\"为中华之崛起而读书\"");
    quoteLabel->setStyleSheet("color: #C9A64E; font-size: 28px; font-weight: bold; text-align: center; background-color: transparent;");

    // 作者 - 使用浅色调
    authorLabel = new QLabel("—— 周恩来 (Zhou Enlai)");
    authorLabel->setStyleSheet("color: #E8D5B5; font-size: 16px; font-weight: 500; font-style: italic; background-color: transparent;");

    // 英文翻译 - 使用淡雅色调
    translationLabel = new QLabel("\"Study for the rise of China.\"");
    translationLabel->setStyleSheet("color: #D4C5A0; font-size: 14px; background-color: transparent;");

    // 添加小logo图片到左上方
    QLabel *imageLabel = new QLabel();
    imageLabel->setFixedSize(160, 160); // 放大到160x80
    imageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    imageLabel->setScaledContents(true);
    imageLabel->setStyleSheet("background-color: transparent; border: none;"); // 确保Logo区域透明

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
            imageLabel->setText("Logo");
            imageLabel->setStyleSheet(
                "QLabel {"
                "  color: #C9A64E;"
                "  font-size: 12px;"
                "  background-color: transparent;"
                "  border: none;"
                "  padding: 5px;"
                "}"
            );
        }
    }

    // 左侧布局 - logo在左上方，文字在中间
    leftLayout->addWidget(imageLabel, 0, Qt::AlignLeft | Qt::AlignTop);
    leftLayout->addStretch(); // 上方弹簧

    // 中间区域放置文字内容
    leftLayout->addWidget(mottoLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(mottoEnglish, 0, Qt::AlignCenter);
    leftLayout->addSpacing(30);
    leftLayout->addWidget(separator, 0, Qt::AlignCenter);
    leftLayout->addSpacing(30);
    leftLayout->addWidget(quoteLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(authorLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(translationLabel, 0, Qt::AlignCenter);
    leftLayout->addStretch(); // 中间弹簧

    // 添加天安门图片到底部 - 显示完整的城楼建筑，零缝隙
    QLabel *tiananmenLabel = new QLabel();
    tiananmenLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tiananmenLabel->setAlignment(Qt::AlignCenter);
    tiananmenLabel->setScaledContents(true); // 允许缩放，显示完整的楼

    // 移除QLabel默认边距 - 关键修复
    tiananmenLabel->setContentsMargins(0, 0, 0, 0);

    // 加载天安门图片
    QPixmap tiananmenPixmap(":/images/天安门.png");
    if (!tiananmenPixmap.isNull()) {
        tiananmenLabel->setPixmap(tiananmenPixmap);
        qDebug() << "天安门图片加载成功，原始尺寸:" << tiananmenPixmap.size();
        // 零缝隙样式 - 完全贴合背景
        tiananmenLabel->setStyleSheet(
            "QLabel {"
            "  margin: 0px;"          // 外边距=0
            "  padding: 0px;"         // 内边距=0
            "  border: none;"         // 无边框
            "  background-color: transparent;" // 透明背景
            "}"
        );
    } else {
        qDebug() << "天安门图片加载失败";
        tiananmenLabel->setText("天安门");
        tiananmenLabel->setStyleSheet(
            "QLabel {"
            "  color: #C9A64E;"
            "  font-size: 24px;"
            "  margin: 0px;"         // 零边距
            "  padding: 0px;"        // 零边距
            "  border: none;"
            "}"
        );
    }

    // 确保左侧布局零间距 - 关键修复
    leftLayout->setSpacing(0);          // 布局元素间距=0
    leftLayout->setContentsMargins(0, 0, 0, 0); // 布局边距=0

    // 让天安门图片占据底部更大空间，无缝贴合
    leftLayout->addWidget(tiananmenLabel, 6);

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
    titleLabel->setStyleSheet("color: #C62828; font-size: 42px; font-weight: 900; text-align: center; margin: 10px 0;");

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
    togglePasswordBtn = new QPushButton(passwordEdit);  // 设置父对象为passwordEdit
    togglePasswordBtn->setFixedSize(30, 30);
    togglePasswordBtn->setCursor(Qt::PointingHandCursor);
    togglePasswordBtn->move(passwordEdit->width() - 40, (passwordEdit->height() - 30) / 2);  // 定位到右侧

    // 测试图标加载，如果资源路径不行就用绝对路径
    QIcon showIcon, hideIcon;
    if (!m_eyeShowIcon.isNull()) {
        showIcon = m_eyeShowIcon;
        qDebug() << "使用资源路径加载显示图标";
    } else {
        showIcon = QIcon(":/images/眼睛_显示.png");
        qDebug() << "使用绝对路径加载显示图标";
    }

    if (!m_eyeHideIcon.isNull()) {
        hideIcon = m_eyeHideIcon;
        qDebug() << "使用资源路径加载隐藏图标";
    } else {
        hideIcon = QIcon(":/images/眼睛_隐藏.png");
        qDebug() << "使用绝对路径加载隐藏图标";
    }

    togglePasswordBtn->setIcon(passwordEdit->echoMode() == QLineEdit::Password ? hideIcon : showIcon);
    togglePasswordBtn->setIconSize(QSize(28, 28));
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

    auto *rememberLayout = new QHBoxLayout();
    rememberLayout->setContentsMargins(0, 0, 0, 0);
    rememberLayout->setSpacing(8);

    rememberMeCheck = new QCheckBox("记住我", this);

    rememberLayout->addWidget(rememberMeCheck);

    // 创建忘记密码按钮
    forgotPasswordBtn = new QPushButton("忘记密码？", this);

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
    optionsLayout->addLayout(rememberLayout);
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
    connect(rememberMeCheck, &QCheckBox::toggled, this, &SimpleLoginWindow::onRememberMeToggled);
    connect(togglePasswordBtn, &QPushButton::clicked, [this]() {
        if (passwordEdit->echoMode() == QLineEdit::Password) {
            passwordEdit->setEchoMode(QLineEdit::Normal);
            // 使用相同的逻辑加载显示图标
            QIcon showIcon = !m_eyeShowIcon.isNull() ?
                           m_eyeShowIcon :
                           QIcon(":/images/眼睛_显示.png");
            togglePasswordBtn->setIcon(showIcon);
        } else {
            passwordEdit->setEchoMode(QLineEdit::Password);
            // 使用相同的逻辑加载隐藏图标
            QIcon hideIcon = !m_eyeHideIcon.isNull() ?
                           m_eyeHideIcon :
                           QIcon(":/images/眼睛_隐藏.png");
            togglePasswordBtn->setIcon(hideIcon);
        }
    });

    // 使用事件过滤器来监听密码框大小改变事件
    passwordEdit->installEventFilter(this);

    qDebug() << "UI设置完成！";
}

void SimpleLoginWindow::setupStyle()
{
    setStyleSheet(
        "QWidget {"
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

        // 保存记住的凭证
        saveRememberedCredentials();

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

    // 关闭当前登录窗口
    this->close();

    // 创建并显示注册窗口
    SignUpWindow *signupWindow = new SignUpWindow();
    signupWindow->show();

    qDebug() << "已打开注册窗口";
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

    // 保存记住的凭证
    saveRememberedCredentials();

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


void SimpleLoginWindow::onRememberMeToggled(bool checked)
{
    qDebug() << "记住我状态切换:" << checked;
    
    if (!checked) {
        // 如果取消记住我，清除保存的凭证
        clearRememberedCredentials();
    }
}

bool SimpleLoginWindow::hasRememberedCredentials()
{
    return m_settings->value("rememberMe", false).toBool() &&
           !m_settings->value("savedUsername", "").toString().isEmpty();
}

void SimpleLoginWindow::loadRememberedCredentials()
{
    QString username = m_settings->value("savedUsername", "").toString();
    QString password = m_settings->value("savedPassword", "").toString();
    
    if (!username.isEmpty() && !password.isEmpty()) {
        usernameEdit->setText(username);
        passwordEdit->setText(password);
        rememberMeCheck->setChecked(true);
        qDebug() << "已加载记住的用户凭证:" << username;
    }
}

void SimpleLoginWindow::saveRememberedCredentials()
{
    if (rememberMeCheck->isChecked()) {
        m_settings->setValue("rememberMe", true);
        m_settings->setValue("savedUsername", usernameEdit->text());
        m_settings->setValue("savedPassword", passwordEdit->text());
        qDebug() << "已保存用户凭证";
    }
}

void SimpleLoginWindow::clearRememberedCredentials()
{
    m_settings->remove("rememberMe");
    m_settings->remove("savedUsername");
    m_settings->remove("savedPassword");
    qDebug() << "已清除记住的凭证";
}
