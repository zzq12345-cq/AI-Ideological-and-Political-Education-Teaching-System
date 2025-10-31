#include "simpleloginwindow.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>

SimpleLoginWindow::SimpleLoginWindow(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setupStyle();
}

SimpleLoginWindow::~SimpleLoginWindow()
{
}

void SimpleLoginWindow::setupUI()
{
    setWindowTitle("思想政治智慧课堂");
    resize(1200, 700);  // 使用resize而不是setFixedSize，允许窗口调整大小
    setMinimumSize(800, 600);  // 设置最小尺寸限制

    // 设置窗口标志以显示完整的窗口控制按钮（关闭、最小化、最大化）
    // 使用Qt::Window确保所有原生控件都显示，并启用最大化
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint);

    // 启用窗口最大化功能
    setWindowModality(Qt::NonModal);

    // 确保窗口具有完整的窗口控件
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);

    // 主布局 - 60%左侧 + 40%右侧布局，使右侧登录模块更聚焦
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧面板 - 柔化红色调背景，显示口号和引言
    leftPanel = new QFrame();
    leftPanel->setFixedWidth(720); // 60% of 1200
    leftLayout = new QVBoxLayout(leftPanel);

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

    // 左侧布局
    leftLayout->addStretch();
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
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(50, 50, 50, 50); // 稍微减少边距使界面更紧凑

  
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
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #CFD7E7;"
        "  border-radius: 8px;"
        "  padding: 16px 16px;"  // 与密码输入框左侧padding保持一致
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

    // 密码输入框 - 使用内部按钮
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("请输入您的密码");
    passwordEdit->setFixedHeight(56);

    // 创建密码显示/隐藏按钮
    togglePasswordBtn = new QPushButton("👁");
    togglePasswordBtn->setFixedSize(24, 24);
    togglePasswordBtn->setCursor(Qt::PointingHandCursor);
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

    // 将按钮放在输入框右侧
    QHBoxLayout *passwordLayout = new QHBoxLayout(passwordEdit);
    passwordLayout->setContentsMargins(16, 0, 16, 0);
    passwordLayout->setSpacing(8);
    passwordLayout->addStretch();
    passwordLayout->addWidget(togglePasswordBtn);

    // 设置输入框样式，为右侧按钮留出空间
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #CFD7E7;"
        "  border-radius: 8px;"
        "  padding: 16px 16px;"  // 与用户名输入框完全一致的padding
        "  font-size: 16px;"
        "  background-color: #F6F6F8;"
        "  text-align: left;"  // 确保文本左对齐
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #C62828;"
        "  outline: none;"
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
    rightLayout->addWidget(passwordEdit);  // 直接添加密码输入框，已包含内部按钮
    rightLayout->addSpacing(16); // 减少选项区域间距

    // 选项区域 - 记住我/忘记密码
    rightLayout->addLayout(optionsLayout);
    rightLayout->addSpacing(24); // 登录按钮前间距

    // 主要操作按钮
    rightLayout->addWidget(loginButton);
    rightLayout->addLayout(signupLayout);
    rightLayout->addStretch();

    // 添加到主布局
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);

    // 连接信号
    connect(loginButton, &QPushButton::clicked, this, &SimpleLoginWindow::onLoginClicked);
    connect(togglePasswordBtn, &QPushButton::clicked, [this]() {
        if (passwordEdit->echoMode() == QLineEdit::Password) {
            passwordEdit->setEchoMode(QLineEdit::Normal);
            togglePasswordBtn->setText("👁‍🗨");
        } else {
            passwordEdit->setEchoMode(QLineEdit::Password);
            togglePasswordBtn->setText("👁");
        }
    });
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

    // 测试账号验证
    if ((username == "teacher01" && password == "Teacher@2024") ||
        (username == "student01" && password == "Student@2024") ||
        (username == "admin01" && password == "Admin@2024")) {

        QMessageBox::information(this, "登录成功", "欢迎 " + username + "！\n\n登录功能正常工作！");
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误！\n\n请使用：\n• teacher01 / Teacher@2024\n• student01 / Student@2024\n• admin01 / Admin@2024");
    }
}

