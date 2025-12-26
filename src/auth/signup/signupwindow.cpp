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
    , m_eyeShowIcon(QIcon(QStringLiteral(":/images/眼睛_显示.png")))
    , m_eyeHideIcon(QIcon(QStringLiteral(":/images/眼睛_隐藏.png")))
{
    const QStringList availableFamilies = QFontDatabase::families();
    qDebug() << cn("[FontDebug] 可用字体数量:") << availableFamilies.size();

    const QString targetFamily = QStringLiteral("PingFang SC");
    const bool hasTargetFamily = QFontDatabase::hasFamily(targetFamily);
    qDebug() << cn("[FontDebug] QFontDatabase::hasFamily('PingFang SC') ->") << hasTargetFamily;

    // 设置窗口字体，确保中文在窗口内渲染正常
    QFont windowFont = chooseChineseFont(12, QFont::Normal);
    if (hasTargetFamily) {
        windowFont.setFamily(targetFamily);
    }
    setFont(windowFont);

    // 初始化Supabase客户端
    m_supabaseClient = new SupabaseClient(this);

    // 连接Supabase信号
    connect(m_supabaseClient, &SupabaseClient::signupSuccess,
            this, &SignUpWindow::onSignupSuccess);
    connect(m_supabaseClient, &SupabaseClient::signupFailed,
            this, &SignUpWindow::onSignupFailed);

    setupUI();
    setupStyle();
}

SignUpWindow::~SignUpWindow()
{
}

void SignUpWindow::setupUI()
{
    qDebug() << cn("开始设置注册窗口UI...");

    // 设置基本窗口属性，保持与登录窗口一致
    setWindowTitle(cn("注册 - 思想政治智慧课堂"));
    resize(1400, 900);
    setMinimumSize(1000, 800);

    // 创建主布局
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧面板 - 保持与登录窗口一致的风格
    leftPanel = new QFrame(this);
    leftPanel->setFixedWidth(720); // 宽度的60%
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    buildHeroPanel();

    // 右侧面板 - 注册表单
    rightPanel = new QFrame(this);
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(50, 50, 50, 50);
    buildFormPanel();

    // 添加到主布局
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);

    // 连接信号
    connect(registerButton, &QPushButton::clicked, this, &SignUpWindow::onSignupClicked);
    connect(loginBtn, &QPushButton::clicked, this, &SignUpWindow::onBackToLoginClicked);
    
    // 连接眼睛按钮信号
    if (togglePassword1Btn) {
        connect(togglePassword1Btn, &QPushButton::clicked, this, &SignUpWindow::onTogglePassword1Clicked);
    }
    if (togglePassword2Btn) {
        connect(togglePassword2Btn, &QPushButton::clicked, this, &SignUpWindow::onTogglePassword2Clicked);
    }

    emailEdit->clear();
    usernameEdit->clear();

    qDebug() << cn("注册窗口UI设置完成！");
}

void SignUpWindow::buildHeroPanel()
{
    if (!leftLayout) return;

    // 添加小logo图片到左上方
    QLabel *imageLabel = new QLabel();
    imageLabel->setFixedSize(160, 160);
    imageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    imageLabel->setScaledContents(true);
    imageLabel->setStyleSheet("background-color: transparent; border: none;");

    // 加载Logo
    QPixmap pixmap(":/images/download.png");
    if (pixmap.isNull()) {
        pixmap = QPixmap("/Users/zhouzhiqi/QtProjects/AItechnology/src/shared/resources/download.png");
    }
    if (!pixmap.isNull()) {
        imageLabel->setPixmap(pixmap);
    } else {
        imageLabel->setText("Logo");
    }

    leftLayout->addWidget(imageLabel, 0, Qt::AlignLeft | Qt::AlignTop);
    leftLayout->addStretch(); 

    // 中间区域放置文字内容
    // 口号标签
    mottoLabel = new QLabel(cn("\"智慧思政，立德树人\""));
    enforceChineseFont(mottoLabel, 32, QFont::Black);
    mottoLabel->setStyleSheet("color: #C9A64E; font-size: 32px; font-weight: 900; text-align: center; background-color: transparent;");
    
    QLabel *mottoEnglish = new QLabel("\"Smart Civic Education, Cultivating Virtue\"");
    enforceChineseFont(mottoEnglish, 18, QFont::Medium);
    mottoEnglish->setStyleSheet("color: #E8D5B5; font-size: 18px; font-weight: 500; text-align: center; background-color: transparent;");

    // 分隔线
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedSize(400, 1);
    separator->setStyleSheet("background-color: #C9A64E; border: none;");

    // 引言
    quoteLabel = new QLabel(cn("\"开启沉浸式思政教学新篇章\""));
    enforceChineseFont(quoteLabel, 24, QFont::Bold);
    quoteLabel->setStyleSheet("color: #C9A64E; font-size: 24px; font-weight: bold; text-align: center; background-color: transparent;");

    // 描述
    authorLabel = new QLabel(cn("AI赋能 · 资源共享 · 智慧教学"));
    enforceChineseFont(authorLabel, 16, QFont::Medium);
    authorLabel->setStyleSheet("color: #E8D5B5; font-size: 16px; font-weight: 500; font-style: italic; background-color: transparent;");

    leftLayout->addWidget(mottoLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(mottoEnglish, 0, Qt::AlignCenter);
    leftLayout->addSpacing(30);
    leftLayout->addWidget(separator, 0, Qt::AlignCenter);
    leftLayout->addSpacing(30);
    leftLayout->addWidget(quoteLabel, 0, Qt::AlignCenter);
    leftLayout->addWidget(authorLabel, 0, Qt::AlignCenter);
    leftLayout->addStretch(); 

    // 添加天安门图片到底部
    QLabel *tiananmenLabel = new QLabel();
    tiananmenLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tiananmenLabel->setAlignment(Qt::AlignCenter);
    tiananmenLabel->setScaledContents(true);
    tiananmenLabel->setContentsMargins(0, 0, 0, 0);

    QPixmap tiananmenPixmap("/Users/zhouzhiqi/QtProjects/AItechnology/images/摄图网_401630316_天安门(非营利使用).png");
    if (!tiananmenPixmap.isNull()) {
        tiananmenLabel->setPixmap(tiananmenPixmap);
        tiananmenLabel->setStyleSheet("margin: 0px; padding: 0px; border: none; background: transparent;");
    }

    leftLayout->addWidget(tiananmenLabel, 6);
}

void SignUpWindow::buildFormPanel()
{
    if (!rightLayout) return;

    rightLayout->addStretch();

    // 品牌标题区域
    titleLabel = new QLabel(cn("思想政治智慧课堂"));
    enforceChineseFont(titleLabel, 42, QFont::Black);
    titleLabel->setStyleSheet("color: #C62828; font-size: 42px; font-weight: 900; text-align: center; margin: 10px 0;");
    
    subtitleLabel = new QLabel(cn("创建新账户"));
    enforceChineseFont(subtitleLabel, 32, QFont::Bold);
    subtitleLabel->setStyleSheet("color: #0F172A; font-size: 32px; font-weight: 900; text-align: center; margin-top: 20px;");

    descLabel = new QLabel(cn("请填写以下信息完成注册"));
    enforceChineseFont(descLabel, 14, QFont::Normal);
    descLabel->setStyleSheet("color: #6B7280; font-size: 14px; text-align: center;");

    rightLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(subtitleLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(descLabel, 0, Qt::AlignCenter);
    rightLayout->addSpacing(30);

    // 用户名
    usernameLabel = new QLabel(cn("用户名"));
    enforceChineseFont(usernameLabel, 16, QFont::Medium);
    usernameLabel->setStyleSheet("color: #0F172A; font-size: 16px; font-weight: 500;");
    
    usernameEdit = new QLineEdit();
    enforceChineseFont(usernameEdit, 16, QFont::Normal);
    usernameEdit->setPlaceholderText(cn("请输入用户名"));
    usernameEdit->setFixedHeight(50);
    usernameEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #CFD7E7; border-radius: 8px; padding: 0 16px; font-size: 16px; background-color: #F6F6F8; }"
        "QLineEdit:focus { border: 2px solid #C62828; outline: none; }"
    );

    rightLayout->addWidget(usernameLabel);
    rightLayout->addWidget(usernameEdit);
    rightLayout->addSpacing(16);

    // 邮箱
    emailLabel = new QLabel(cn("电子邮箱"));
    enforceChineseFont(emailLabel, 16, QFont::Medium);
    emailLabel->setStyleSheet("color: #0F172A; font-size: 16px; font-weight: 500;");
    
    emailEdit = new QLineEdit();
    enforceChineseFont(emailEdit, 16, QFont::Normal);
    emailEdit->setPlaceholderText(cn("请输入电子邮箱"));
    emailEdit->setFixedHeight(50);
    emailEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #CFD7E7; border-radius: 8px; padding: 0 16px; font-size: 16px; background-color: #F6F6F8; }"
        "QLineEdit:focus { border: 2px solid #C62828; outline: none; }"
    );

    rightLayout->addWidget(emailLabel);
    rightLayout->addWidget(emailEdit);
    rightLayout->addSpacing(16);

    // 准备图标
    QIcon showIcon, hideIcon;
    if (!m_eyeShowIcon.isNull()) {
        showIcon = m_eyeShowIcon;
    } else {
        showIcon = QIcon("/Users/zhouzhiqi/QtProjects/AItechnology/images/眼睛_显示.png");
    }
    if (!m_eyeHideIcon.isNull()) {
        hideIcon = m_eyeHideIcon;
    } else {
        hideIcon = QIcon("/Users/zhouzhiqi/QtProjects/AItechnology/images/眼睛_隐藏.png");
    }
    // 初始状态是密码模式，显示“闭眼”或者“睁眼”？通常：
    // 密码模式(***) -> 显示"显示图标"(睁眼)
    // 正常模式(abc) -> 显示"隐藏图标"(闭眼/斜杠眼)
    // 之前LoginWindow逻辑：
    // if password -> setIcon(showIcon/hideIcon?)
    // togglePasswordBtn->setIcon(passwordEdit->echoMode() == QLineEdit::Password ? hideIcon : showIcon);
    // 等等，LoginWindow里：
    // m_eyeShowIcon 是 "眼睛_显示.png" (通常是睁眼)
    // m_eyeHideIcon 是 "眼睛_隐藏.png" (通常是闭眼)
    // 默认是Password模式，应该显示“显示”按钮（闭眼图标，点击后变睁眼显示密码？或者睁眼图标，点击查看？）
    // LoginWindow代码：
    // togglePasswordBtn->setIcon(passwordEdit->echoMode() == QLineEdit::Password ? hideIcon : showIcon);
    // 如果是Password模式，显示 HideIcon？这就反了。通常Password模式显示ShowIcon（点击显示）。
    // 让我们照搬LoginWindow的逻辑：
    // `togglePasswordBtn->setIcon(passwordEdit->echoMode() == QLineEdit::Password ? hideIcon : showIcon);`
    // 这意味着：密码模式下显示 HideIcon。这可能意味着 "当前是隐藏状态" 或者 "点击隐藏"？
    // 但下面的toggle逻辑：
    // if (Password) { setNormal; setIcon(showIcon); }
    // else { setPassword; setIcon(hideIcon); }
    // 这意味着 Normal(明文) 模式下显示 ShowIcon。Password(密文) 模式下显示 HideIcon。
    // 这有点奇怪，通常明文时显示"隐藏"(HideIcon)，密文时显示"显示"(ShowIcon)。
    // 无论如何，我将保持与 LoginWindow 完全一致的逻辑。
    
    QIcon initialIcon = hideIcon; // 因为默认是Password模式

    // 密码1
    passwordLabel1 = new QLabel(cn("密码"));
    enforceChineseFont(passwordLabel1, 16, QFont::Medium);
    passwordLabel1->setStyleSheet("color: #0F172A; font-size: 16px; font-weight: 500;");
    
    passwordEdit1 = new QLineEdit();
    enforceChineseFont(passwordEdit1, 16, QFont::Normal);
    passwordEdit1->setEchoMode(QLineEdit::Password);
    passwordEdit1->setPlaceholderText(cn("请输入密码 (至少8位)"));
    passwordEdit1->setFixedHeight(50);
    passwordEdit1->setStyleSheet(
        "QLineEdit { border: 1px solid #CFD7E7; border-radius: 8px; padding: 0 50px 0 16px; font-size: 16px; background-color: #F6F6F8; }"
        "QLineEdit:focus { border: 2px solid #C62828; outline: none; }"
    );

    // 眼睛按钮1
    togglePassword1Btn = new QPushButton(passwordEdit1);
    togglePassword1Btn->setFixedSize(30, 30);
    togglePassword1Btn->setCursor(Qt::PointingHandCursor);
    togglePassword1Btn->setStyleSheet("border: none; background: transparent;");
    togglePassword1Btn->setIcon(initialIcon);
    togglePassword1Btn->setIconSize(QSize(28, 28));
    togglePassword1Btn->move(passwordEdit1->width() - 40, (passwordEdit1->height() - 30) / 2);
    passwordEdit1->installEventFilter(this);
    
    rightLayout->addWidget(passwordLabel1);
    rightLayout->addWidget(passwordEdit1);
    rightLayout->addSpacing(16);

    // 密码2
    passwordLabel2 = new QLabel(cn("确认密码"));
    enforceChineseFont(passwordLabel2, 16, QFont::Medium);
    passwordLabel2->setStyleSheet("color: #0F172A; font-size: 16px; font-weight: 500;");
    
    passwordEdit2 = new QLineEdit();
    enforceChineseFont(passwordEdit2, 16, QFont::Normal);
    passwordEdit2->setEchoMode(QLineEdit::Password);
    passwordEdit2->setPlaceholderText(cn("请再次输入密码"));
    passwordEdit2->setFixedHeight(50);
    passwordEdit2->setStyleSheet(
        "QLineEdit { border: 1px solid #CFD7E7; border-radius: 8px; padding: 0 50px 0 16px; font-size: 16px; background-color: #F6F6F8; }"
        "QLineEdit:focus { border: 2px solid #C62828; outline: none; }"
    );

    // 眼睛按钮2
    togglePassword2Btn = new QPushButton(passwordEdit2);
    togglePassword2Btn->setFixedSize(30, 30);
    togglePassword2Btn->setCursor(Qt::PointingHandCursor);
    togglePassword2Btn->setStyleSheet("border: none; background: transparent;");
    togglePassword2Btn->setIcon(initialIcon);
    togglePassword2Btn->setIconSize(QSize(28, 28));
    togglePassword2Btn->move(passwordEdit2->width() - 40, (passwordEdit2->height() - 30) / 2);
    passwordEdit2->installEventFilter(this);

    rightLayout->addWidget(passwordLabel2);
    rightLayout->addWidget(passwordEdit2);
    rightLayout->addSpacing(24);

    // 注册按钮
    registerButton = new QPushButton(cn("立即注册"));
    enforceChineseFont(registerButton, 16, QFont::Bold);
    registerButton->setFixedHeight(56);
    registerButton->setStyleSheet(
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
        "}"
        "QPushButton:pressed {"
        "  background-color: #B71C1C;"
        "  border-color: #B71C1C;"
        "}"
    );
    rightLayout->addWidget(registerButton);

    // 返回登录
    QHBoxLayout *loginLinkLayout = new QHBoxLayout();
    loginLinkLayout->setContentsMargins(0, 15, 0, 0);

    loginLabel = new QLabel(cn("已有账号?"));
    enforceChineseFont(loginLabel, 14, QFont::Normal);
    loginLabel->setStyleSheet("color: #6B7280; font-size: 14px;");

    loginBtn = new QPushButton(cn("立即登录"));
    enforceChineseFont(loginBtn, 14, QFont::Medium);
    loginBtn->setStyleSheet(
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

    loginLinkLayout->addStretch();
    loginLinkLayout->addWidget(loginLabel);
    loginLinkLayout->addWidget(loginBtn);
    loginLinkLayout->addStretch();

    rightLayout->addLayout(loginLinkLayout);
    rightLayout->addStretch();
}

void SignUpWindow::setupStyle()
{
    setStyleSheet(
        "QWidget {"
        "  background-color: white;"
        "}"
        "QFrame#leftPanel {"
        "  background-color: #B71C1C;"
        "}"
        "QFrame#rightPanel {"
        "  background-color: white;"
        "}"
    );

    if (leftPanel) leftPanel->setObjectName("leftPanel");
    if (rightPanel) rightPanel->setObjectName("rightPanel");
}

QFrame *SignUpWindow::createInputGroup(QWidget *parent,
                                       const QString &placeholderText,
                                       QLineEdit **lineEdit,
                                       bool isPassword,
                                       QPushButton **toggleButton)
{
    return nullptr; 
}

QString SignUpWindow::resolveStyleSheetPath() const
{
    return QString(); 
}

void SignUpWindow::onSignupClicked()
{
    if (!validateInput()) {
        return;
    }

    QString email = emailEdit->text().trimmed();
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit1->text();

    if (m_signupProcessed) {
        return;
    }
    m_signupProcessed = true;

    registerButton->setEnabled(false);
    registerButton->setText(cn("注册中..."));

    m_supabaseClient->signup(email, password, username);
}

void SignUpWindow::onBackToLoginClicked()
{
    openLoginWindow();
}

void SignUpWindow::onTogglePassword1Clicked()
{
    QIcon showIcon = !m_eyeShowIcon.isNull() ? m_eyeShowIcon : QIcon("/Users/zhouzhiqi/QtProjects/AItechnology/images/眼睛_显示.png");
    QIcon hideIcon = !m_eyeHideIcon.isNull() ? m_eyeHideIcon : QIcon("/Users/zhouzhiqi/QtProjects/AItechnology/images/眼睛_隐藏.png");

    if (passwordEdit1->echoMode() == QLineEdit::Password) {
        passwordEdit1->setEchoMode(QLineEdit::Normal);
        togglePassword1Btn->setIcon(showIcon);
    } else {
        passwordEdit1->setEchoMode(QLineEdit::Password);
        togglePassword1Btn->setIcon(hideIcon);
    }
}

void SignUpWindow::onTogglePassword2Clicked()
{
    QIcon showIcon = !m_eyeShowIcon.isNull() ? m_eyeShowIcon : QIcon("/Users/zhouzhiqi/QtProjects/AItechnology/images/眼睛_显示.png");
    QIcon hideIcon = !m_eyeHideIcon.isNull() ? m_eyeHideIcon : QIcon("/Users/zhouzhiqi/QtProjects/AItechnology/images/眼睛_隐藏.png");

    if (passwordEdit2->echoMode() == QLineEdit::Password) {
        passwordEdit2->setEchoMode(QLineEdit::Normal);
        togglePassword2Btn->setIcon(showIcon);
    } else {
        passwordEdit2->setEchoMode(QLineEdit::Password);
        togglePassword2Btn->setIcon(hideIcon);
    }
}

bool SignUpWindow::validateInput()
{
    QString email = emailEdit->text().trimmed();
    QString password1Text = passwordEdit1->text();
    QString password2Text = passwordEdit2->text();

    if (email.isEmpty()) {
        showMessage(cn("输入错误"), cn("请输入邮箱地址！"), QMessageBox::Warning);
        emailEdit->setFocus();
        return false;
    }

    if (!email.contains("@") || !email.contains(".")) {
        showMessage(cn("输入错误"), cn("请输入有效的邮箱地址！"), QMessageBox::Warning);
        emailEdit->setFocus();
        return false;
    }

    if (password1Text.isEmpty()) {
        showMessage(cn("输入错误"), cn("请输入密码！"), QMessageBox::Warning);
        passwordEdit1->setFocus();
        return false;
    }

    if (password1Text.length() < 8) {
        showMessage(cn("输入错误"), cn("密码至少需要8位字符！"), QMessageBox::Warning);
        passwordEdit1->setFocus();
        return false;
    }

    if (password2Text.isEmpty()) {
        showMessage(cn("输入错误"), cn("请确认密码！"), QMessageBox::Warning);
        passwordEdit2->setFocus();
        return false;
    }

    if (password1Text != password2Text) {
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
    QString email = emailEdit->text().trimmed();
    showMessage(cn("注册成功"),
                cn("账户创建成功！\n\n邮箱: %1\n请检查您的邮箱并点击验证链接以激活账户。\n\n即将跳转到登录页面...")
                    .arg(email),
                QMessageBox::Information);

    QTimer::singleShot(2000, this, &SignUpWindow::openLoginWindow);
}

void SignUpWindow::onSignupFailed(const QString &errorMessage)
{
    showMessage(cn("注册失败"), errorMessage, QMessageBox::Warning);
    registerButton->setEnabled(true);
    registerButton->setText(cn("立即注册"));
    m_signupProcessed = false;
}

void SignUpWindow::openLoginWindow()
{
    this->close();
    SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
    loginWindow->show();
    loginWindow->raise();
    loginWindow->activateWindow();
}

void SignUpWindow::showMessage(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    enforceChineseFont(&msgBox, 14, QFont::Normal);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QMessageBox QLabel { color: #0F172A; font-size: 14px; }"
        "QMessageBox QPushButton { background-color: #C62828; color: white; border: none; padding: 8px 24px; border-radius: 6px; font-size: 14px; }"
        "QMessageBox QPushButton:hover { background-color: #B71C1C; }"
    );
    msgBox.exec();
}

bool SignUpWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        if (watched == passwordEdit1) {
            if (togglePassword1Btn) togglePassword1Btn->move(passwordEdit1->width() - 40, (passwordEdit1->height() - 30) / 2);
        } else if (watched == passwordEdit2) {
            if (togglePassword2Btn) togglePassword2Btn->move(passwordEdit2->width() - 40, (passwordEdit2->height() - 30) / 2);
        }
    }
    return QWidget::eventFilter(watched, event);
}
