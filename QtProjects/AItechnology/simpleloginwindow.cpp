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
    setFixedSize(1200, 700);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // 主布局 - 60%左侧 + 40%右侧布局，使右侧登录模块更聚焦
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧面板 - 柔化红色调背景，显示口号和引言
    leftPanel = new QFrame();
    leftPanel->setFixedWidth(720); // 60% of 1200
    leftLayout = new QVBoxLayout(leftPanel);

    // 口号标签 - 使用更柔和的颜色搭配
    mottoLabel = new QLabel("\"不忘初心，牢记使命\"");
    mottoLabel->setStyleSheet("color: #FFF8F0; font-size: 32px; font-weight: 900; text-align: center;");

    // 英文翻译
    QLabel *mottoEnglish = new QLabel("\"Remain true to our original aspiration and keep our mission firmly in mind.\"");
    mottoEnglish->setStyleSheet("color: #FFE8E0; font-size: 18px; font-weight: 500; text-align: center;");

    // 分隔线 - 使用更柔和的透明度
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: rgba(255, 248, 240, 0.4);");

    // 引言 - 调整颜色使视觉更柔和
    quoteLabel = new QLabel("\"为中华之崛起而读书\"");
    quoteLabel->setStyleSheet("color: #FFF8F0; font-size: 28px; font-weight: bold; text-align: center;");

    // 作者
    authorLabel = new QLabel("—— 周恩来 (Zhou Enlai)");
    authorLabel->setStyleSheet("color: #FFE8E0; font-size: 16px; font-weight: 500; font-style: italic;");

    // 英文翻译
    translationLabel = new QLabel("\"Study for the rise of China.\"");
    translationLabel->setStyleSheet("color: rgba(255, 232, 224, 0.9); font-size: 14px;");

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

    // 关闭按钮 - 使用柔化红色
    closeButton = new QPushButton("✕");
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #C62828;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 15px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #B71C1C;"
        "}"
    );

    // 品牌标题区域 - 建立清晰的层级
    titleLabel = new QLabel("思想政治智慧课堂");
    titleLabel->setStyleSheet("color: #C62828; font-size: 28px; font-weight: bold; text-align: center;");

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
        "  padding: 16px 44px;"
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

    // 密码输入框容器
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("请输入您的密码");
    passwordEdit->setFixedHeight(56);
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #CFD7E7;"
        "  border-radius: 8px;"
        "  padding: 16px 44px;"
        "  font-size: 16px;"
        "  background-color: #F6F6F8;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #C62828;"
        "  outline: none;"
        "}"
    );

    togglePasswordBtn = new QPushButton("👁");
    togglePasswordBtn->setFixedSize(40, 40);
    togglePasswordBtn->setStyleSheet(
        "QPushButton {"
        "  border: none;"
        "  background: transparent;"
        "  font-size: 18px;"
        "}"
    );

    passwordLayout->addWidget(passwordEdit);
    passwordLayout->addWidget(togglePasswordBtn);

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

    // 登录按钮 - 使用柔化红色系
    loginButton = new QPushButton("登 录");
    loginButton->setFixedHeight(56);
    loginButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #C62828;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #8E0000;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #B71C1C;"
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

    // 右侧布局组装 - 优化间距和层级
    rightLayout->addWidget(closeButton);
    rightLayout->addStretch();

    // 品牌区域 - 较小间距，作为辅助信息
    rightLayout->addWidget(titleLabel);
    rightLayout->addWidget(subtitleLabel);
    rightLayout->addSpacing(20); // 减少间距

    // 欢迎区域 - 主要操作提示，增加突出感
    rightLayout->addWidget(welcomeLabel);
    rightLayout->addWidget(descLabel);
    rightLayout->addSpacing(30); // 适当间距

    // 表单区域
    rightLayout->addWidget(usernameLabel);
    rightLayout->addWidget(usernameEdit);
    rightLayout->addSpacing(16);
    rightLayout->addWidget(passwordLabel);
    rightLayout->addLayout(passwordLayout);
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
    connect(closeButton, &QPushButton::clicked, this, &SimpleLoginWindow::onCloseClicked);
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
        "  background-color: #C62828;"  // 使用柔化红色
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

void SimpleLoginWindow::onCloseClicked()
{
    close();
}