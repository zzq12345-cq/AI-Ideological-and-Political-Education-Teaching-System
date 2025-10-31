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

    // 主布局
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧面板 - 红色背景，显示口号和引言
    leftPanel = new QFrame();
    leftPanel->setFixedWidth(600);
    leftLayout = new QVBoxLayout(leftPanel);

    // 口号标签
    mottoLabel = new QLabel("\"不忘初心，牢记使命\"");
    mottoLabel->setStyleSheet("color: #FEF3C7; font-size: 32px; font-weight: 900; text-align: center;");

    // 英文翻译
    QLabel *mottoEnglish = new QLabel("\"Remain true to our original aspiration and keep our mission firmly in mind.\"");
    mottoEnglish->setStyleSheet("color: #FEF3C7; font-size: 18px; font-weight: 500; text-align: center;");

    // 分隔线
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: rgba(254, 243, 199, 0.3);");

    // 引言
    quoteLabel = new QLabel("\"为中华之崛起而读书\"");
    quoteLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold; text-align: center;");

    // 作者
    authorLabel = new QLabel("—— 周恩来 (Zhou Enlai)");
    authorLabel->setStyleSheet("color: #FEF3C7; font-size: 16px; font-weight: 500; font-style: italic;");

    // 英文翻译
    translationLabel = new QLabel("\"Study for the rise of China.\"");
    translationLabel->setStyleSheet("color: rgba(254, 243, 199, 0.8); font-size: 14px;");

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

    // 右侧面板 - 白色背景，登录表单
    rightPanel = new QFrame();
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(60, 60, 60, 60);

    // 关闭按钮
    closeButton = new QPushButton("✕");
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #EF4444;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 15px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #DC2626;"
        "}"
    );

    // 标题
    titleLabel = new QLabel("思想政治智慧课堂");
    titleLabel->setStyleSheet("color: #DC2626; font-size: 36px; font-weight: bold; text-align: center;");

    subtitleLabel = new QLabel("Ideological & Political Smart Classroom");
    subtitleLabel->setStyleSheet("color: #6B7280; font-size: 18px; text-align: center;");

    // 欢迎信息
    welcomeLabel = new QLabel("欢迎回来");
    welcomeLabel->setStyleSheet("color: #0F172A; font-size: 42px; font-weight: 900; text-align: center;");

    descLabel = new QLabel("请登录您的账户以继续");
    descLabel->setStyleSheet("color: #6B7280; font-size: 16px; text-align: center;");

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
        "  border: 2px solid #DC2626;"
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
        "  border: 2px solid #DC2626;"
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

    // 记住我和忘记密码
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    rememberCheck = new QCheckBox("记住我");
    rememberCheck->setStyleSheet(
        "QCheckBox {"
        "  color: #0F172A;"
        "  font-size: 14px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 20px;"
        "  height: 20px;"
        "  border-radius: 4px;"
        "  border: 1px solid #CFD7E7;"
        "  background-color: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: #DC2626;"
        "  border-color: #DC2626;"
        "}"
    );

    forgotPasswordBtn = new QPushButton("忘记密码?");
    forgotPasswordBtn->setStyleSheet(
        "QPushButton {"
        "  color: #DC2626;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton:hover {"
        "  color: #991B1B;"
        "}"
    );

    optionsLayout->addWidget(rememberCheck);
    optionsLayout->addStretch();
    optionsLayout->addWidget(forgotPasswordBtn);

    // 登录按钮
    loginButton = new QPushButton("登 录");
    loginButton->setFixedHeight(56);
    loginButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #DC2626;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #991B1B;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #7F1D1D;"
        "}"
    );

    // 注册链接
    QHBoxLayout *signupLayout = new QHBoxLayout();
    signupLabel = new QLabel("没有账户?");
    signupLabel->setStyleSheet("color: #6B7280; font-size: 14px;");

    signupBtn = new QPushButton("立即注册");
    signupBtn->setStyleSheet(
        "QPushButton {"
        "  color: #DC2626;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton:hover {"
        "  color: #991B1B;"
        "}"
    );

    signupLayout->addWidget(signupLabel);
    signupLayout->addWidget(signupBtn);

    // 右侧布局组装
    rightLayout->addWidget(closeButton);
    rightLayout->addStretch();
    rightLayout->addWidget(titleLabel);
    rightLayout->addWidget(subtitleLabel);
    rightLayout->addSpacing(40);
    rightLayout->addWidget(welcomeLabel);
    rightLayout->addWidget(descLabel);
    rightLayout->addSpacing(40);
    rightLayout->addWidget(usernameLabel);
    rightLayout->addWidget(usernameEdit);
    rightLayout->addSpacing(20);
    rightLayout->addWidget(passwordLabel);
    rightLayout->addLayout(passwordLayout);
    rightLayout->addLayout(optionsLayout);
    rightLayout->addSpacing(20);
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
        "  background-color: #DC2626;"
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