#include "signupwindow.h"
#include "../login/simpleloginwindow.h"

#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QStringList>
#include <QSizePolicy>

#if defined(_MSC_VER)
#  pragma execution_character_set("utf-8")
#endif

namespace {
inline QString cn(const char *text)
{
    if (!text) {
        return QString();
    }
    return QString::fromUtf8(text);
}

QFont chooseChineseFont(int pointSize, QFont::Weight weight = QFont::Normal)
{
    QFont baseFont = QApplication::font();
    const QString fallbackFamily = baseFont.family();
    const QStringList candidates = {
#if defined(Q_OS_MAC)
        QStringLiteral("PingFang SC"),
#endif
        QStringLiteral("Noto Sans SC"),
        QStringLiteral("WenQuanYi Micro Hei"),
        fallbackFamily
    };

    for (const auto &family : candidates) {
        if (family.isEmpty()) {
            continue;
        }
        if (QFontDatabase::hasFamily(family)) {
            baseFont.setFamily(family);
            break;
        }
    }

    if (pointSize > 0) {
        baseFont.setPointSize(pointSize);
    }
    baseFont.setWeight(weight);
    return baseFont;
}

inline void enforceChineseFont(QWidget *widget, int pointSize, QFont::Weight weight = QFont::Normal)
{
    if (!widget) {
        return;
    }
    widget->setFont(chooseChineseFont(pointSize, weight));
}
} // namespace

SignUpWindow::SignUpWindow(QWidget *parent)
    : QWidget(parent)
{
    const QStringList availableFamilies = QFontDatabase::families();
    qDebug() << cn("[FontDebug] 可用字体数量:") << availableFamilies.size();
    qDebug() << cn("[FontDebug] 可用字体列表:") << availableFamilies;

    const QString targetFamily = QStringLiteral("PingFang SC");
    const bool hasTargetFamily = QFontDatabase::hasFamily(targetFamily);
    qDebug() << cn("[FontDebug] QFontDatabase::hasFamily('PingFang SC') ->") << hasTargetFamily;

    // 设置窗口字体，确保中文在窗口内渲染正常
    QFont windowFont = chooseChineseFont(12, QFont::Normal);
    if (hasTargetFamily) {
        windowFont.setFamily(targetFamily);
        qDebug() << cn("[FontDebug] 已强制使用字体:") << targetFamily;
    } else {
        qWarning() << cn("[FontDebug] 系统缺少 PingFang SC，保留自动选择字体:") << windowFont.family();
    }
    setFont(windowFont);

    setWindowTitle(cn("注册 - AI智慧课堂"));
    resize(1180, 760);
    setAttribute(Qt::WA_DeleteOnClose);

    // 初始化Supabase客户端
    m_supabaseClient = new SupabaseClient(this);

    // 连接Supabase信号
    connect(m_supabaseClient, &SupabaseClient::signupSuccess,
            this, &SignUpWindow::onSignupSuccess);
    connect(m_supabaseClient, &SupabaseClient::signupFailed,
            this, &SignUpWindow::onSignupFailed);

    qDebug() << cn("SignUpWindow 构造函数");
    setupUI();
    setupStyle();
}

SignUpWindow::~SignUpWindow()
{
    qDebug() << cn("SignUpWindow 析构函数");
}

void SignUpWindow::setupUI()
{
    qDebug() << cn("开始设置注册窗口UI...");

    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("signupWindow");
    setMinimumSize(1100, 720);
    resize(1200, 760);

    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignCenter);

    QFrame *contentCard = new QFrame(this);
    enforceChineseFont(contentCard, 12, QFont::Normal);
    contentCard->setObjectName("contentCard");
    contentCard->setMinimumSize(1050, 700);

    auto *shadowEffect = new QGraphicsDropShadowEffect(contentCard);
    shadowEffect->setBlurRadius(48);
    shadowEffect->setOffset(0, 24);
    shadowEffect->setColor(QColor(15, 23, 42, 60));
    contentCard->setGraphicsEffect(shadowEffect);

    QHBoxLayout *contentLayout = new QHBoxLayout(contentCard);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    leftPanel = new QFrame(contentCard);
    enforceChineseFont(leftPanel, 12, QFont::Normal);
    leftPanel->setObjectName("heroPanel");
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(48, 72, 48, 72);
    leftLayout->setSpacing(12);
    buildHeroPanel();

    rightPanel = new QFrame(contentCard);
    enforceChineseFont(rightPanel, 12, QFont::Normal);
    rightPanel->setObjectName("formPanel");
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(64, 48, 64, 48);
    rightLayout->setSpacing(0);
    buildFormPanel();

    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(rightPanel);
    contentLayout->setStretch(0, 5);
    contentLayout->setStretch(1, 7);

    mainLayout->addWidget(contentCard);

    connect(registerButton, &QPushButton::clicked, this, &SignUpWindow::onSignupClicked);
    connect(loginBtn, &QPushButton::clicked, this, &SignUpWindow::onBackToLoginClicked);
    connect(togglePassword1Btn, &QPushButton::clicked, this, &SignUpWindow::onTogglePassword1Clicked);
    connect(togglePassword2Btn, &QPushButton::clicked, this, &SignUpWindow::onTogglePassword2Clicked);

    emailEdit->clear();
    usernameEdit->clear();

    qDebug() << cn("注册窗口UI设置完成！");
}

void SignUpWindow::buildHeroPanel()
{
    if (!leftLayout) {
        return;
    }

    leftLayout->addStretch(1);

    mottoLabel = new QLabel(cn("AI智慧课堂"), leftPanel);
    enforceChineseFont(mottoLabel, 38, QFont::Bold);
    mottoLabel->setObjectName("heroTitle");
    mottoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mottoLabel->setWordWrap(true);
    leftLayout->addWidget(mottoLabel);

    quoteLabel = new QLabel(cn("智慧赋能思政课堂"), leftPanel);
    enforceChineseFont(quoteLabel, 30, QFont::DemiBold);
    quoteLabel->setObjectName("heroSubtitle");
    quoteLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    quoteLabel->setWordWrap(true);
    leftLayout->addWidget(quoteLabel);

    leftLayout->addSpacing(12);

    authorLabel = new QLabel(cn("以科技重构教学体验，点亮思政新可能。"), leftPanel);
    enforceChineseFont(authorLabel, 16, QFont::Medium);
    authorLabel->setObjectName("heroDescription");
    authorLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    authorLabel->setWordWrap(true);
    leftLayout->addWidget(authorLabel);

    translationLabel = new QLabel(QStringLiteral("Smart Civic Education · Powered by AI"), leftPanel);
    enforceChineseFont(translationLabel, 13, QFont::Normal);
    translationLabel->setObjectName("heroFootnote");
    translationLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    translationLabel->setWordWrap(true);
    leftLayout->addSpacing(16);
    leftLayout->addWidget(translationLabel);

    leftLayout->addStretch(2);
}

void SignUpWindow::buildFormPanel()
{
    if (!rightLayout) {
        return;
    }

    QWidget *formWrapper = new QWidget(rightPanel);
    enforceChineseFont(formWrapper, 12, QFont::Normal);
    formWrapper->setObjectName("formWrapper");
    formWrapper->setMinimumWidth(420);
    formWrapper->setMaximumWidth(460);

    QVBoxLayout *formLayout = new QVBoxLayout(formWrapper);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(12);
    formLayout->setAlignment(Qt::AlignTop);

    titleLabel = new QLabel(cn("开启智慧思政新篇章"), formWrapper);
    enforceChineseFont(titleLabel, 26, QFont::Bold);
    titleLabel->setObjectName("formTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);
    formLayout->addWidget(titleLabel);

    subtitleLabel = new QLabel(cn("创建新账户"), formWrapper);
    enforceChineseFont(subtitleLabel, 16, QFont::Medium);
    subtitleLabel->setObjectName("formSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);
    formLayout->addWidget(subtitleLabel);

    registerLabel = new QLabel(cn("AI智慧课堂 · 智慧思政生态"), formWrapper);
    enforceChineseFont(registerLabel, 13, QFont::Medium);
    registerLabel->setObjectName("formTagline");
    registerLabel->setAlignment(Qt::AlignCenter);
    registerLabel->setWordWrap(true);
    formLayout->addWidget(registerLabel);

    descLabel = new QLabel(cn("加入我们，与智能教研助手一起开启沉浸式思政教学。"), formWrapper);
    enforceChineseFont(descLabel, 12, QFont::Normal);
    descLabel->setObjectName("formDescription");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    formLayout->addWidget(descLabel);

    formLayout->addSpacing(16);

    usernameLabel = new QLabel(cn("用户名"), formWrapper);
    enforceChineseFont(usernameLabel, 14, QFont::DemiBold);
    usernameLabel->setProperty("role", "fieldLabel");
    formLayout->addWidget(usernameLabel);

    QFrame *usernameGroup = createInputGroup(formWrapper, cn("请输入用户名"), &usernameEdit);
    formLayout->addWidget(usernameGroup);

    emailLabel = new QLabel(cn("电子邮件"), formWrapper);
    enforceChineseFont(emailLabel, 14, QFont::DemiBold);
    emailLabel->setProperty("role", "fieldLabel");
    formLayout->addWidget(emailLabel);

    QFrame *emailGroup = createInputGroup(formWrapper, cn("请输入您的电子邮箱"), &emailEdit);
    formLayout->addWidget(emailGroup);

    passwordLabel1 = new QLabel(cn("密码"), formWrapper);
    enforceChineseFont(passwordLabel1, 14, QFont::DemiBold);
    passwordLabel1->setProperty("role", "fieldLabel");
    formLayout->addWidget(passwordLabel1);

    QFrame *passwordGroup1 = createInputGroup(formWrapper, cn("请输入至少8位密码"), &passwordEdit1, true, &togglePassword1Btn);
    formLayout->addWidget(passwordGroup1);

    passwordLabel2 = new QLabel(cn("确认密码"), formWrapper);
    enforceChineseFont(passwordLabel2, 14, QFont::DemiBold);
    passwordLabel2->setProperty("role", "fieldLabel");
    formLayout->addWidget(passwordLabel2);

    QFrame *passwordGroup2 = createInputGroup(formWrapper, cn("请再次输入密码"), &passwordEdit2, true, &togglePassword2Btn);
    formLayout->addWidget(passwordGroup2);

    formLayout->addSpacing(8);

    registerButton = new QPushButton(cn("立即注册"), formWrapper);
    enforceChineseFont(registerButton, 16, QFont::DemiBold);
    registerButton->setObjectName("primaryButton");
    registerButton->setCursor(Qt::PointingHandCursor);
    registerButton->setMinimumHeight(54);
    registerButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addWidget(registerButton);

    QWidget *loginLinkWidget = new QWidget(formWrapper);
    QHBoxLayout *loginLinkLayout = new QHBoxLayout(loginLinkWidget);
    loginLinkLayout->setContentsMargins(0, 4, 0, 0);
    loginLinkLayout->setSpacing(6);
    loginLinkLayout->setAlignment(Qt::AlignCenter);

    loginLabel = new QLabel(cn("已有账号？"), loginLinkWidget);
    enforceChineseFont(loginLabel, 13, QFont::Normal);
    loginLabel->setObjectName("helperText");

    loginBtn = new QPushButton(cn("立即登录"), loginLinkWidget);
    enforceChineseFont(loginBtn, 13, QFont::Medium);
    loginBtn->setObjectName("linkButton");
    loginBtn->setCursor(Qt::PointingHandCursor);

    loginLinkLayout->addWidget(loginLabel);
    loginLinkLayout->addWidget(loginBtn);

    formLayout->addWidget(loginLinkWidget);
    formLayout->addStretch(1);

    rightLayout->addStretch(1);
    rightLayout->addWidget(formWrapper);
    rightLayout->addStretch(1);
}

QFrame *SignUpWindow::createInputGroup(QWidget *parent,
                                       const QString &placeholderText,
                                       QLineEdit **lineEdit,
                                       bool isPassword,
                                       QPushButton **toggleButton)
{
    QFrame *group = new QFrame(parent);
    group->setObjectName("inputGroup");
    group->setProperty("role", "inputGroup");

    QHBoxLayout *layout = new QHBoxLayout(group);
    layout->setContentsMargins(20, 6, 20, 6);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignVCenter);

    QLineEdit *edit = new QLineEdit(group);
    enforceChineseFont(edit, 14, QFont::Normal);
    edit->setPlaceholderText(placeholderText);
    edit->setClearButtonEnabled(!isPassword);
    edit->setProperty("role", "inputField");
    edit->setMinimumHeight(50);
    if (isPassword) {
        edit->setEchoMode(QLineEdit::Password);
    }

    layout->addWidget(edit, 1);

    if (isPassword && toggleButton) {
        QPushButton *toggle = new QPushButton(cn("👁"), group);
        enforceChineseFont(toggle, 14, QFont::Normal);
        toggle->setObjectName("passwordToggle");
        toggle->setCursor(Qt::PointingHandCursor);
        toggle->setFixedSize(40, 40);
        layout->addWidget(toggle, 0, Qt::AlignRight | Qt::AlignVCenter);
        *toggleButton = toggle;
    } else if (toggleButton) {
        *toggleButton = nullptr;
    }

    *lineEdit = edit;
    return group;
}

QString SignUpWindow::resolveStyleSheetPath() const
{
    const QString baseDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QStringLiteral("resources/styles/auth.qss"),
        QStringLiteral("../resources/styles/auth.qss"),
        QStringLiteral("../../resources/styles/auth.qss"),
        QStringLiteral("../../../resources/styles/auth.qss"),
        baseDir + QStringLiteral("/resources/styles/auth.qss"),
        baseDir + QStringLiteral("/../resources/styles/auth.qss"),
        baseDir + QStringLiteral("/../../resources/styles/auth.qss"),
        baseDir + QStringLiteral("/../../../resources/styles/auth.qss"),
        baseDir + QStringLiteral("/../../../../resources/styles/auth.qss")
    };

    for (const QString &path : candidates) {
        QFileInfo info(path);
        if (info.exists() && info.isFile() && info.isReadable()) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

void SignUpWindow::setupStyle()
{
    qDebug() << cn("设置注册窗口样式...");

    auto applySheet = [this](QFile &file, const QString &source) {
        const QString styleSheet = QString::fromUtf8(file.readAll());
        setStyleSheet(styleSheet);
        qDebug() << cn("注册窗口样式设置完成！样式来源:") << source;
    };

    QFile embeddedFile(QStringLiteral(":/styles/auth.qss"));
    if (embeddedFile.exists()) {
        if (embeddedFile.open(QFile::ReadOnly | QFile::Text)) {
            applySheet(embeddedFile, QStringLiteral(":/styles/auth.qss"));
            return;
        }
        qWarning() << cn("无法读取内置样式文件 :/styles/auth.qss");
    }

    const QString stylePath = resolveStyleSheetPath();
    if (stylePath.isEmpty()) {
        qWarning() << cn("未找到 auth.qss 样式文件，跳过自定义样式");
        return;
    }

    QFile styleFile(stylePath);
    if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << cn("无法打开样式文件:") << stylePath;
        return;
    }

    applySheet(styleFile, stylePath);
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
        qDebug() << cn("注册已处理，跳过重复调用");
        return;
    }
    m_signupProcessed = true;

    qDebug() << cn("尝试注册:") << email;

    registerButton->setEnabled(false);
    registerButton->setText(cn("注册中..."));

    // 调用Supabase注册
    m_supabaseClient->signup(email, password, username);
}

void SignUpWindow::onBackToLoginClicked()
{
    qDebug() << cn("返回登录页面");
    openLoginWindow();
}

void SignUpWindow::onTogglePassword1Clicked()
{
    if (passwordEdit1->echoMode() == QLineEdit::Password) {
        passwordEdit1->setEchoMode(QLineEdit::Normal);
        togglePassword1Btn->setText(cn("👁‍🗨"));
    } else {
        passwordEdit1->setEchoMode(QLineEdit::Password);
        togglePassword1Btn->setText(cn("👁"));
    }
}

void SignUpWindow::onTogglePassword2Clicked()
{
    if (passwordEdit2->echoMode() == QLineEdit::Password) {
        passwordEdit2->setEchoMode(QLineEdit::Normal);
        togglePassword2Btn->setText(cn("👁‍🗨"));
    } else {
        passwordEdit2->setEchoMode(QLineEdit::Password);
        togglePassword2Btn->setText(cn("👁"));
    }
}

bool SignUpWindow::validateInput()
{
    QString email = emailEdit->text().trimmed();
    QString password1 = passwordEdit1->text();
    QString password2 = passwordEdit2->text();

    // 验证邮箱
    if (email.isEmpty()) {
        showMessage(cn("输入错误"), cn("请输入邮箱地址！"), QMessageBox::Warning);
        emailEdit->setFocus();
        return false;
    }

    // 简单邮箱格式验证
    if (!email.contains("@") || !email.contains(".")) {
        showMessage(cn("输入错误"), cn("请输入有效的邮箱地址！"), QMessageBox::Warning);
        emailEdit->setFocus();
        return false;
    }

    // 验证密码
    if (password1.isEmpty()) {
        showMessage(cn("输入错误"), cn("请输入密码！"), QMessageBox::Warning);
        passwordEdit1->setFocus();
        return false;
    }

    if (password1.length() < 8) {
        showMessage(cn("输入错误"), cn("密码至少需要8位字符！"), QMessageBox::Warning);
        passwordEdit1->setFocus();
        return false;
    }

    // 验证确认密码
    if (password2.isEmpty()) {
        showMessage(cn("输入错误"), cn("请确认密码！"), QMessageBox::Warning);
        passwordEdit2->setFocus();
        return false;
    }

    if (password1 != password2) {
        showMessage(cn("输入错误"), cn("两次输入的密码不一致！"), QMessageBox::Warning);
        passwordEdit1->clear();
        passwordEdit2->clear();
        passwordEdit1->setFocus();
        return false;
    }

    return true;
}

void SignUpWindow::onSignupSuccess(const QString &message)
{
    qDebug() << cn("Supabase注册成功! 消息:") << message;

    QString email = emailEdit->text().trimmed();
    showMessage(cn("注册成功"),
                cn("账户创建成功！\n\n邮箱: %1\n请检查您的邮箱并点击验证链接以激活账户。\n\n即将跳转到登录页面...")
                    .arg(email),
                QMessageBox::Information);

    // 2秒后跳转到登录页面
    QTimer::singleShot(2000, this, &SignUpWindow::openLoginWindow);
}

void SignUpWindow::onSignupFailed(const QString &errorMessage)
{
    qDebug() << cn("Supabase注册失败:") << errorMessage;

    showMessage(cn("注册失败"), errorMessage, QMessageBox::Warning);

    registerButton->setEnabled(true);
    registerButton->setText(cn("立即注册"));

    m_signupProcessed = false;
}

void SignUpWindow::openLoginWindow()
{
    qDebug() << cn("准备打开登录窗口...");

    // 关闭注册窗口
    this->close();

    // 创建并显示真正的登录窗口
    SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
    loginWindow->show();
    loginWindow->raise();
    loginWindow->activateWindow();
    qDebug() << cn("已打开登录窗口");
}

void SignUpWindow::showMessage(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    enforceChineseFont(&msgBox, 14, QFont::Normal);
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
