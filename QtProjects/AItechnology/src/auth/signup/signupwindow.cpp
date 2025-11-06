#include "signupwindow.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

SignUpWindow::SignUpWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("注册 - AI智慧课堂");
    setFixedSize(1000, 600);
    setAttribute(Qt::WA_DeleteOnClose);

    // 初始化Supabase客户端
    m_supabaseClient = new SupabaseClient(this);

    // 连接Supabase信号
    connect(m_supabaseClient, &SupabaseClient::signupSuccess,
            this, &SignUpWindow::onSignupSuccess);
    connect(m_supabaseClient, &SupabaseClient::signupFailed,
            this, &SignUpWindow::onSignupFailed);

    qDebug() << "SignUpWindow 构造函数";
    setupUI();
    setupStyle();
}

SignUpWindow::~SignUpWindow()
{
    qDebug() << "SignUpWindow 析构函数";
}

void SignUpWindow::setupUI()
{
    qDebug() << "开始设置注册窗口UI...";

    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("signupWindow");

    auto createIconPixmap = [](const QString &type) -> QPixmap {
        const int size = 32;
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen pen(QColor("#4A90E2"));
        pen.setWidthF(2.0);
        painter.setPen(pen);

        if (type == "person") {
            painter.drawEllipse(QPointF(size / 2.0, size * 0.32), size * 0.2, size * 0.2);
            QPainterPath body;
            body.moveTo(size * 0.22, size * 0.76);
            body.quadTo(size * 0.5, size * 0.58, size * 0.78, size * 0.76);
            body.quadTo(size * 0.5, size * 0.94, size * 0.22, size * 0.76);
            painter.drawPath(body);
        } else if (type == "mail") {
            QRectF rect(4.0, 8.0, size - 8.0, size - 14.0);
            painter.drawRoundedRect(rect, 6.0, 6.0);
            QPointF center(rect.left() + rect.width() / 2.0, rect.top() + rect.height() / 2.2);
            painter.drawLine(rect.topLeft(), center);
            painter.drawLine(rect.topRight(), center);
        } else if (type == "lock") {
            QRectF bodyRect(6.0, size * 0.48, size - 12.0, size * 0.36);
            painter.drawRoundedRect(bodyRect, 6.0, 6.0);
            QPainterPath shackle;
            shackle.moveTo(size * 0.28, size * 0.48);
            shackle.cubicTo(size * 0.28, size * 0.16, size * 0.72, size * 0.16, size * 0.72, size * 0.48);
            painter.drawPath(shackle);
            painter.drawLine(QPointF(size / 2.0, size * 0.58), QPointF(size / 2.0, size * 0.70));
        }

        painter.end();
        return pixmap;
    };

    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    leftPanel = new QFrame(this);
    leftPanel->setObjectName("leftPanel");
    leftPanel->setFixedWidth(420);
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(48, 64, 48, 64);
    leftLayout->setSpacing(18);
    leftLayout->setAlignment(Qt::AlignCenter);

    mottoLabel = new QLabel("AI智慧课堂", leftPanel);
    mottoLabel->setObjectName("brandLabel");
    mottoLabel->setAlignment(Qt::AlignCenter);

    quoteLabel = new QLabel("智慧赋能思政课堂", leftPanel);
    quoteLabel->setObjectName("brandHeadline");
    quoteLabel->setWordWrap(true);
    quoteLabel->setAlignment(Qt::AlignCenter);

    authorLabel = new QLabel("以科技重构教学体验，点亮思政新可能。", leftPanel);
    authorLabel->setObjectName("brandSubline");
    authorLabel->setWordWrap(true);
    authorLabel->setAlignment(Qt::AlignCenter);

    translationLabel = new QLabel("Smart Civic Education · Powered by AI", leftPanel);
    translationLabel->setObjectName("brandFooter");
    translationLabel->setAlignment(Qt::AlignCenter);
    translationLabel->setWordWrap(true);

    leftLayout->addStretch();
    leftLayout->addWidget(mottoLabel);
    leftLayout->addWidget(quoteLabel);
    leftLayout->addWidget(authorLabel);
    leftLayout->addStretch();
    leftLayout->addWidget(translationLabel);
    leftLayout->addStretch(2);

    rightPanel = new QFrame(this);
    rightPanel->setObjectName("rightPanel");
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(72, 40, 72, 40);
    rightLayout->setSpacing(24);

    QFrame *formContainer = new QFrame(rightPanel);
    formContainer->setObjectName("formContainer");
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(40, 48, 40, 36);
    formLayout->setSpacing(18);
    formLayout->setAlignment(Qt::AlignTop);

    titleLabel = new QLabel("开启智慧思政新篇章", formContainer);
    titleLabel->setObjectName("mainTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);

    registerLabel = new QLabel("创建新账户", formContainer);
    registerLabel->setObjectName("accentSubtitle");
    registerLabel->setAlignment(Qt::AlignCenter);
    registerLabel->setWordWrap(true);

    subtitleLabel = new QLabel("AI智慧课堂 · 智慧思政生态", formContainer);
    subtitleLabel->setObjectName("supportSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);

    descLabel = new QLabel("加入我们，与智能教研助手一起开启沉浸式思政教学。", formContainer);
    descLabel->setObjectName("description");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);

    formLayout->addWidget(titleLabel);
    formLayout->addWidget(registerLabel);
    formLayout->addWidget(subtitleLabel);
    formLayout->addWidget(descLabel);
    formLayout->addSpacing(12);

    usernameLabel = new QLabel("用户名", formContainer);
    usernameLabel->setProperty("role", "fieldLabel");
    usernameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFrame *usernameGroup = new QFrame(formContainer);
    usernameGroup->setProperty("component", "inputGroup");
    QHBoxLayout *usernameLayout = new QHBoxLayout(usernameGroup);
    usernameLayout->setContentsMargins(16, 12, 16, 12);
    usernameLayout->setSpacing(12);

    QLabel *usernameIcon = new QLabel(usernameGroup);
    usernameIcon->setProperty("role", "inputIcon");
    usernameIcon->setFixedSize(36, 36);
    usernameIcon->setAlignment(Qt::AlignCenter);
    usernameIcon->setPixmap(createIconPixmap("person").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    usernameLayout->addWidget(usernameIcon);

    usernameEdit = new QLineEdit(usernameGroup);
    usernameEdit->setPlaceholderText("请输入您的用户名");
    usernameEdit->setClearButtonEnabled(true);
    usernameEdit->setProperty("role", "textField");
    usernameEdit->setMinimumHeight(44);
    usernameLayout->addWidget(usernameEdit, 1);

    formLayout->addWidget(usernameLabel);
    formLayout->addWidget(usernameGroup);

    emailLabel = new QLabel("电子邮件地址", formContainer);
    emailLabel->setProperty("role", "fieldLabel");
    emailLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFrame *emailGroup = new QFrame(formContainer);
    emailGroup->setProperty("component", "inputGroup");
    QHBoxLayout *emailLayout = new QHBoxLayout(emailGroup);
    emailLayout->setContentsMargins(16, 12, 16, 12);
    emailLayout->setSpacing(12);

    QLabel *emailIcon = new QLabel(emailGroup);
    emailIcon->setProperty("role", "inputIcon");
    emailIcon->setFixedSize(36, 36);
    emailIcon->setAlignment(Qt::AlignCenter);
    emailIcon->setPixmap(createIconPixmap("mail").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    emailLayout->addWidget(emailIcon);

    emailEdit = new QLineEdit(emailGroup);
    emailEdit->setPlaceholderText("请输入您的电子邮件地址");
    emailEdit->setClearButtonEnabled(true);
    emailEdit->setProperty("role", "textField");
    emailEdit->setMinimumHeight(44);
    emailLayout->addWidget(emailEdit, 1);

    formLayout->addWidget(emailLabel);
    formLayout->addWidget(emailGroup);

    passwordLabel1 = new QLabel("密码", formContainer);
    passwordLabel1->setProperty("role", "fieldLabel");
    passwordLabel1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFrame *passwordGroup1 = new QFrame(formContainer);
    passwordGroup1->setProperty("component", "inputGroup");
    QHBoxLayout *passwordLayout1 = new QHBoxLayout(passwordGroup1);
    passwordLayout1->setContentsMargins(16, 12, 16, 12);
    passwordLayout1->setSpacing(12);

    QLabel *passwordIcon1 = new QLabel(passwordGroup1);
    passwordIcon1->setProperty("role", "inputIcon");
    passwordIcon1->setFixedSize(36, 36);
    passwordIcon1->setAlignment(Qt::AlignCenter);
    passwordIcon1->setPixmap(createIconPixmap("lock").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    passwordLayout1->addWidget(passwordIcon1);

    passwordEdit1 = new QLineEdit(passwordGroup1);
    passwordEdit1->setPlaceholderText("请输入至少8位密码");
    passwordEdit1->setEchoMode(QLineEdit::Password);
    passwordEdit1->setProperty("role", "textField");
    passwordEdit1->setMinimumHeight(44);
    passwordLayout1->addWidget(passwordEdit1, 1);

    togglePassword1Btn = new QPushButton("👁", passwordGroup1);
    togglePassword1Btn->setObjectName("passwordToggle");
    togglePassword1Btn->setFixedSize(36, 36);
    togglePassword1Btn->setCursor(Qt::PointingHandCursor);
    passwordLayout1->addWidget(togglePassword1Btn);

    formLayout->addWidget(passwordLabel1);
    formLayout->addWidget(passwordGroup1);

    passwordLabel2 = new QLabel("确认密码", formContainer);
    passwordLabel2->setProperty("role", "fieldLabel");
    passwordLabel2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFrame *passwordGroup2 = new QFrame(formContainer);
    passwordGroup2->setProperty("component", "inputGroup");
    QHBoxLayout *passwordLayout2 = new QHBoxLayout(passwordGroup2);
    passwordLayout2->setContentsMargins(16, 12, 16, 12);
    passwordLayout2->setSpacing(12);

    QLabel *passwordIcon2 = new QLabel(passwordGroup2);
    passwordIcon2->setProperty("role", "inputIcon");
    passwordIcon2->setFixedSize(36, 36);
    passwordIcon2->setAlignment(Qt::AlignCenter);
    passwordIcon2->setPixmap(createIconPixmap("lock").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    passwordLayout2->addWidget(passwordIcon2);

    passwordEdit2 = new QLineEdit(passwordGroup2);
    passwordEdit2->setPlaceholderText("请再次输入您的密码");
    passwordEdit2->setEchoMode(QLineEdit::Password);
    passwordEdit2->setProperty("role", "textField");
    passwordEdit2->setMinimumHeight(44);
    passwordLayout2->addWidget(passwordEdit2, 1);

    togglePassword2Btn = new QPushButton("👁", passwordGroup2);
    togglePassword2Btn->setObjectName("passwordToggle");
    togglePassword2Btn->setFixedSize(36, 36);
    togglePassword2Btn->setCursor(Qt::PointingHandCursor);
    passwordLayout2->addWidget(togglePassword2Btn);

    formLayout->addWidget(passwordLabel2);
    formLayout->addWidget(passwordGroup2);

    registerButton = new QPushButton("立即注册", formContainer);
    registerButton->setObjectName("primaryButton");
    registerButton->setMinimumHeight(54);
    registerButton->setCursor(Qt::PointingHandCursor);
    formLayout->addSpacing(6);
    formLayout->addWidget(registerButton);

    QHBoxLayout *loginLinkLayout = new QHBoxLayout();
    loginLinkLayout->setSpacing(6);
    loginLinkLayout->setAlignment(Qt::AlignCenter);

    loginLabel = new QLabel("已有账号？", formContainer);
    loginLabel->setObjectName("helperText");

    loginBtn = new QPushButton("去登录", formContainer);
    loginBtn->setObjectName("linkButton");
    loginBtn->setCursor(Qt::PointingHandCursor);

    loginLinkLayout->addWidget(loginLabel);
    loginLinkLayout->addWidget(loginBtn);

    formLayout->addSpacing(4);
    formLayout->addLayout(loginLinkLayout);

    auto *shadowEffect = new QGraphicsDropShadowEffect(formContainer);
    shadowEffect->setBlurRadius(42);
    shadowEffect->setOffset(0, 20);
    shadowEffect->setColor(QColor(15, 23, 42, 45));
    formContainer->setGraphicsEffect(shadowEffect);

    rightLayout->addStretch(1);
    rightLayout->addWidget(formContainer);
    rightLayout->addStretch(1);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);

    connect(registerButton, &QPushButton::clicked, this, &SignUpWindow::onSignupClicked);
    connect(loginBtn, &QPushButton::clicked, this, &SignUpWindow::onBackToLoginClicked);
    connect(togglePassword1Btn, &QPushButton::clicked, this, &SignUpWindow::onTogglePassword1Clicked);
    connect(togglePassword2Btn, &QPushButton::clicked, this, &SignUpWindow::onTogglePassword2Clicked);

    emailEdit->clear();
    usernameEdit->clear();

    qDebug() << "注册窗口UI设置完成！";
}

void SignUpWindow::setupStyle()
{
    qDebug() << "设置注册窗口样式...";

    const QString styleSheet = R"(
SignUpWindow#signupWindow {
    background-color: #EEF2F8;
}
QFrame#leftPanel {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #C92B2B,
                                stop:0.55 #D95749,
                                stop:1 #81202D);
}
QFrame#leftPanel QLabel {
    color: rgba(255,255,255,0.92);
}
QLabel#brandLabel {
    font-size: 32px;
    font-weight: 700;
    letter-spacing: 2px;
}
QLabel#brandHeadline {
    font-size: 24px;
    font-weight: 600;
    line-height: 1.3;
}
QLabel#brandSubline {
    font-size: 15px;
    line-height: 1.6;
    color: rgba(255,255,255,0.85);
}
QLabel#brandFooter {
    font-size: 12px;
    letter-spacing: 0.18em;
    color: rgba(255,255,255,0.72);
}

QFrame#rightPanel {
    background: transparent;
}
QFrame#formContainer {
    background-color: rgba(255,255,255,0.78);
    border-radius: 28px;
    border: 1px solid rgba(255,255,255,0.55);
}
QLabel#mainTitle {
    color: #101828;
    font-size: 30px;
    font-weight: 700;
    letter-spacing: 0.5px;
}
QLabel#accentSubtitle {
    color: #4A90E2;
    font-size: 22px;
    font-weight: 700;
    margin-bottom: 4px;
}
QLabel#supportSubtitle {
    color: rgba(15,23,42,0.65);
    font-size: 14px;
    font-weight: 600;
}
QLabel#description {
    color: rgba(71,85,105,0.85);
    font-size: 13px;
    line-height: 1.6;
}
QLabel[role="fieldLabel"] {
    color: #1E293B;
    font-size: 14px;
    font-weight: 600;
    margin-bottom: 2px;
}
QFrame[component="inputGroup"] {
    background-color: rgba(255,255,255,0.9);
    border: 1px solid rgba(148,163,184,0.4);
    border-radius: 16px;
}
QFrame[component="inputGroup"]:hover {
    border-color: rgba(74,144,226,0.7);
}
QFrame[component="inputGroup"] QLabel[role="inputIcon"] {
    background-color: rgba(74,144,226,0.14);
    border-radius: 12px;
    padding: 4px;
}
QLineEdit[role="textField"] {
    border: none;
    background: transparent;
    color: #0f172a;
    font-size: 14px;
    padding: 4px 0;
}
QLineEdit[role="textField"]::placeholder {
    color: rgba(100,116,139,0.75);
}
QLineEdit[role="textField"]:focus {
    border: none;
    color: #0b1220;
}
QPushButton#primaryButton {
    background-color: #C92B2B;
    border-radius: 16px;
    border: none;
    color: white;
    font-size: 16px;
    font-weight: 700;
    padding: 16px 0;
    margin-top: 12px;
}
QPushButton#primaryButton:hover {
    background-color: #b32626;
}
QPushButton#primaryButton:pressed {
    background-color: #a01f1f;
}
QPushButton#primaryButton:disabled {
    background-color: rgba(201,43,43,0.35);
    color: rgba(255,255,255,0.78);
}
QPushButton#passwordToggle {
    border: none;
    background: transparent;
    color: rgba(71,85,105,0.85);
    font-size: 18px;
}
QPushButton#passwordToggle:hover {
    color: #4A90E2;
}
QLabel#helperText {
    color: rgba(71,85,105,0.85);
    font-size: 13px;
}
QPushButton#linkButton {
    color: #4A90E2;
    font-size: 13px;
    font-weight: 600;
    background: transparent;
    border: none;
    text-decoration: underline;
}
QPushButton#linkButton:hover {
    color: #C92B2B;
}
    )";

    setStyleSheet(styleSheet);

    qDebug() << "注册窗口样式设置完成！";
}

void SignUpWindow::onSignupClicked()
{
    if (!validateInput()) {
        return;
    }

    QString email = emailEdit->text().trimmed();
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit1->text();

    // 防止重复处理
    if (m_signupProcessed) {
        qDebug() << "注册已处理，跳过重复调用";
        return;
    }
    m_signupProcessed = true;

    qDebug() << "尝试注册:" << email;

    registerButton->setEnabled(false);
    registerButton->setText("注册中...");

    // 调用Supabase注册
    m_supabaseClient->signup(email, password, username);
}

void SignUpWindow::onBackToLoginClicked()
{
    qDebug() << "返回登录页面";
    openLoginWindow();
}

void SignUpWindow::onTogglePassword1Clicked()
{
    if (passwordEdit1->echoMode() == QLineEdit::Password) {
        passwordEdit1->setEchoMode(QLineEdit::Normal);
        togglePassword1Btn->setText("👁‍🗨");
    } else {
        passwordEdit1->setEchoMode(QLineEdit::Password);
        togglePassword1Btn->setText("👁");
    }
}

void SignUpWindow::onTogglePassword2Clicked()
{
    if (passwordEdit2->echoMode() == QLineEdit::Password) {
        passwordEdit2->setEchoMode(QLineEdit::Normal);
        togglePassword2Btn->setText("👁‍🗨");
    } else {
        passwordEdit2->setEchoMode(QLineEdit::Password);
        togglePassword2Btn->setText("👁");
    }
}

bool SignUpWindow::validateInput()
{
    QString email = emailEdit->text().trimmed();
    QString password1 = passwordEdit1->text();
    QString password2 = passwordEdit2->text();

    // 验证邮箱
    if (email.isEmpty()) {
        showMessage("输入错误", "请输入邮箱地址！", QMessageBox::Warning);
        emailEdit->setFocus();
        return false;
    }

    // 简单邮箱格式验证
    if (!email.contains("@") || !email.contains(".")) {
        showMessage("输入错误", "请输入有效的邮箱地址！", QMessageBox::Warning);
        emailEdit->setFocus();
        return false;
    }

    // 验证密码
    if (password1.isEmpty()) {
        showMessage("输入错误", "请输入密码！", QMessageBox::Warning);
        passwordEdit1->setFocus();
        return false;
    }

    if (password1.length() < 8) {
        showMessage("输入错误", "密码至少需要8位字符！", QMessageBox::Warning);
        passwordEdit1->setFocus();
        return false;
    }

    // 验证确认密码
    if (password2.isEmpty()) {
        showMessage("输入错误", "请确认密码！", QMessageBox::Warning);
        passwordEdit2->setFocus();
        return false;
    }

    if (password1 != password2) {
        showMessage("输入错误", "两次输入的密码不一致！", QMessageBox::Warning);
        passwordEdit1->clear();
        passwordEdit2->clear();
        passwordEdit1->setFocus();
        return false;
    }

    return true;
}

void SignUpWindow::onSignupSuccess(const QString &message)
{
    qDebug() << "Supabase注册成功! 消息:" << message;

    QString email = emailEdit->text().trimmed();
    showMessage("注册成功",
                QString("账户创建成功！\n\n"
                       "邮箱: %1\n"
                       "请检查您的邮箱并点击验证链接以激活账户。\n\n"
                       "即将跳转到登录页面...")
                    .arg(email),
                QMessageBox::Information);

    // 2秒后跳转到登录页面
    QTimer::singleShot(2000, this, &SignUpWindow::openLoginWindow);
}

void SignUpWindow::onSignupFailed(const QString &errorMessage)
{
    qDebug() << "Supabase注册失败:" << errorMessage;

    showMessage("注册失败", errorMessage, QMessageBox::Warning);

    registerButton->setEnabled(true);
    registerButton->setText("注册账户");

    m_signupProcessed = false;
}

void SignUpWindow::openLoginWindow()
{
    qDebug() << "准备打开登录窗口...";

    // 关闭注册窗口
    this->close();

    // 创建并显示登录窗口
    QWidget *loginWindow = new QWidget();
    loginWindow->setWindowTitle("登录 - AI智慧课堂");
    loginWindow->setFixedSize(1000, 600);
    loginWindow->show();
    qDebug() << "已打开登录窗口";

    Q_UNUSED(loginWindow);
}

void SignUpWindow::showMessage(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.setStyleSheet(
        "QMessageBox {"
        "  background-color: white;"
        "}"
        "QMessageBox QLabel {"
        "  color: #0F172A;"
        "  font-size: 14px;"
        "}"
        "QMessageBox QPushButton {"
        "  background-color: #C62828;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px 24px;"
        "  border-radius: 6px;"
        "  font-size: 14px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "  background-color: #B71C1C;"
        "}"
    );
    msgBox.exec();
}
