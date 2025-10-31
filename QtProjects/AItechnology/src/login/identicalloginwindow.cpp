#include "identicalloginwindow.h"
#include "modernmainwindow.h"
#include <QMessageBox>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QRegularExpression>
#include <QDebug>

IdenticalLoginWindow::IdenticalLoginWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("AI思政智慧课堂系统 - 登录");
    setFixedSize(1200, 750); // 增加高度以容纳新功能
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    // 确保不使用任何透明效果
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_NoSystemBackground, false);

    settings = new QSettings("AIPoliticsClassroom", "ModernLoginSettings", this);
    loginTimer = new QTimer(this);

    initUI();
    setupStyles();
    setupAnimations();
    connectSignals();
    loadSettings();

    // 居中显示
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move((screenGeometry.width() - width()) / 2,
         (screenGeometry.height() - height()) / 2);

    connect(loginTimer, &QTimer::timeout, this, &IdenticalLoginWindow::onLoginAnimationFinished);
}

IdenticalLoginWindow::~IdenticalLoginWindow()
{
    saveSettings();
}

void IdenticalLoginWindow::initUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();

    contentFrame = new QFrame(this);
    contentFrame->setObjectName("contentFrame");
    contentFrame->setFixedSize(1100, 670);
    contentFrame->setStyleSheet(
        "QFrame#contentFrame {"
        "   background-color: rgb(255, 255, 255);"
        "   border-radius: 18px;"
        "   border: 2px solid rgb(200, 200, 200);"
        "}"
    );

    // 移除阴影效果，避免透明问题
    // auto *shadow = new QGraphicsDropShadowEffect(contentFrame);
    // shadow->setBlurRadius(42);
    // shadow->setOffset(0, 24);
    // shadow->setColor(QColor(13, 18, 27, 60));
    // contentFrame->setGraphicsEffect(shadow);

    contentLayout = new QHBoxLayout(contentFrame);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    leftPanel = new QFrame(contentFrame);
    leftPanel->setObjectName("leftPanel");
    leftPanel->setMinimumWidth(540);
    leftPanel->setMinimumHeight(670);
    leftPanel->setStyleSheet(
        "QFrame#leftPanel {"
        "   background-color: rgb(198, 40, 40);"
        "   border-top-left-radius: 18px;"
        "   border-bottom-left-radius: 18px;"
        "   color: #FFFFFF;"
        "}"
    );

    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(48, 56, 48, 56);
    leftLayout->setSpacing(0);
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->addStretch();

    leftPanelTitle = new QLabel("\"不忘初心，牢记使命\"", leftPanel);
    leftPanelTitle->setAlignment(Qt::AlignCenter);
    leftPanelTitle->setWordWrap(true);
    leftPanelTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 44px;"
        "   font-weight: 900;"
        "   color: #FFE08A;"
        "   letter-spacing: -0.025em;"
        "}"
    );

    leftPanelSubtitle = new QLabel("\"Remain true to our original aspiration and keep our mission firmly in mind.\"", leftPanel);
    leftPanelSubtitle->setAlignment(Qt::AlignCenter);
    leftPanelSubtitle->setWordWrap(true);
    leftPanelSubtitle->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: 500;"
        "   color: #FFE08A;"
        "   margin-top: 12px;"
        "   line-height: 1.4;"
        "}"
    );

    QWidget *separatorContainer = new QWidget(leftPanel);
    QVBoxLayout *separatorLayout = new QVBoxLayout(separatorContainer);
    separatorLayout->setContentsMargins(0, 36, 0, 36);
    separatorLayout->setSpacing(0);

    QFrame *separatorLine = new QFrame(separatorContainer);
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setStyleSheet("QFrame { background-color: rgba(255, 255, 255, 0.35); height: 1px; }");
    separatorLayout->addWidget(separatorLine);
    separatorLayout->setAlignment(separatorLine, Qt::AlignCenter);

    QWidget *quoteContainer = new QWidget(leftPanel);
    QVBoxLayout *quoteLayout = new QVBoxLayout(quoteContainer);
    quoteLayout->setContentsMargins(0, 0, 0, 0);
    quoteLayout->setSpacing(0);

    quoteLabel = new QLabel("\"为中华之崛起而读书\"", quoteContainer);
    quoteLabel->setAlignment(Qt::AlignCenter);
    quoteLabel->setWordWrap(true);
    quoteLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 32px;"
        "   font-weight: 700;"
        "   color: #FFFFFF;"
        "   line-height: 1.25;"
        "}"
    );

    authorLabel = new QLabel("—— 周恩来 (Zhou Enlai)\n\"Study for the rise of China.\"", quoteContainer);
    authorLabel->setAlignment(Qt::AlignCenter);
    authorLabel->setWordWrap(true);
    authorLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: 500;"
        "   color: rgba(255, 249, 196, 0.9);"
        "   margin-top: 18px;"
        "}"
    );

    quoteLayout->addWidget(quoteLabel);
    quoteLayout->addWidget(authorLabel);
    quoteLayout->setAlignment(quoteLabel, Qt::AlignCenter);
    quoteLayout->setAlignment(authorLabel, Qt::AlignCenter);

    leftLayout->addWidget(leftPanelTitle);
    leftLayout->addWidget(leftPanelSubtitle);
    leftLayout->addWidget(separatorContainer);
    leftLayout->addWidget(quoteContainer);
    leftLayout->addStretch();

    rightPanel = new QFrame(contentFrame);
    rightPanel->setObjectName("rightPanel");
    rightPanel->setMinimumWidth(560);
    rightPanel->setMinimumHeight(670);
    rightPanel->setStyleSheet(
        "QFrame#rightPanel {"
        "   background-color: rgb(255, 255, 255);"
        "   border-top-right-radius: 18px;"
        "   border-bottom-right-radius: 18px;"
        "}"
    );

    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(36, 48, 36, 48);
    rightLayout->setSpacing(0);
    rightLayout->setAlignment(Qt::AlignCenter);
    rightLayout->addStretch();

    QWidget *mainContainer = new QWidget(rightPanel);
    mainContainer->setFixedWidth(456);
    QVBoxLayout *containerLayout = new QVBoxLayout(mainContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(32);

    QWidget *titleSection = new QWidget(mainContainer);
    QVBoxLayout *titleLayout = new QVBoxLayout(titleSection);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(6);
    titleLayout->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel("思想政治智慧课堂", titleSection);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 36px;"
        "   font-weight: 700;"
        "   color: #C62828;"
        "   letter-spacing: -0.02em;"
        "}"
    );

    englishTitle = new QLabel("Ideological & Political Smart Classroom", titleSection);
    englishTitle->setAlignment(Qt::AlignCenter);
    englishTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   color: #6B7280;"
        "}"
    );

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(englishTitle);
    containerLayout->addWidget(titleSection);

    QWidget *welcomeSection = new QWidget(mainContainer);
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeSection);
    welcomeLayout->setContentsMargins(0, 0, 0, 0);
    welcomeLayout->setSpacing(8);
    welcomeLayout->setAlignment(Qt::AlignCenter);

    welcomeLabel = new QLabel("欢迎回来", welcomeSection);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 44px;"
        "   font-weight: 900;"
        "   color: #0D121B;"
        "   letter-spacing: -0.033em;"
        "}"
    );

    descriptionLabel = new QLabel("请登录您的账户以继续", welcomeSection);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   color: #9CA3AF;"
        "}"
    );

    welcomeLayout->addWidget(welcomeLabel);
    welcomeLayout->addWidget(descriptionLabel);
    containerLayout->addWidget(welcomeSection);

    QWidget *formSection = new QWidget(mainContainer);
    QVBoxLayout *formLayout = new QVBoxLayout(formSection);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(20);

    // 角色选择区域
    roleSelectionLabel = new QLabel("选择身份", formSection);
    roleSelectionLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: 500;"
        "   color: #0D121B;"
        "   margin-bottom: 8px;"
        "}"
    );

    roleSelectionFrame = new QFrame(formSection);
    roleSelectionFrame->setObjectName("roleSelectionFrame");
    roleSelectionFrame->setFixedHeight(56);
    roleSelectionFrame->setStyleSheet(
        "QFrame#roleSelectionFrame {"
        "   background-color: #F6F6F8;"
        "   border: 1px solid #CFD7E7;"
        "   border-radius: 8px;"
        "}"
    );

    QHBoxLayout *roleSelectionLayout = new QHBoxLayout(roleSelectionFrame);
    roleSelectionLayout->setContentsMargins(16, 0, 16, 0);
    roleSelectionLayout->setSpacing(20);

    roleGroup = new QButtonGroup(this);

    teacherRadio = new QRadioButton("👨‍🏫 教师", roleSelectionFrame);
    studentRadio = new QRadioButton("👨‍🎓 学生", roleSelectionFrame);
    adminRadio = new QRadioButton("👨‍💼 管理员", roleSelectionFrame);

    teacherRadio->setChecked(true);

    QString radioStyle = R"(
        QRadioButton {
            font-size: 15px;
            color: #0D121B;
            spacing: 8px;
        }
        QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #C62828;
            border-radius: 9px;
            background-color: transparent;
        }
        QRadioButton::indicator::unchecked {
            background-color: transparent;
        }
        QRadioButton::indicator::checked {
            background-color: #C62828;
            border: 2px solid #C62828;
        }
        QRadioButton::indicator::checked::disabled {
            background-color: #CCCCCC;
            border-color: #CCCCCC;
        }
    )";

    teacherRadio->setStyleSheet(radioStyle);
    studentRadio->setStyleSheet(radioStyle);
    adminRadio->setStyleSheet(radioStyle);

    roleSelectionLayout->addWidget(teacherRadio);
    roleSelectionLayout->addWidget(studentRadio);
    roleSelectionLayout->addWidget(adminRadio);
    roleSelectionLayout->addStretch();

    roleGroup->addButton(teacherRadio, 1);
    roleGroup->addButton(studentRadio, 2);
    roleGroup->addButton(adminRadio, 3);

    formLayout->addWidget(roleSelectionLabel);
    formLayout->addWidget(roleSelectionFrame);

    usernameLabel = new QLabel("用户名或邮箱", formSection);
    usernameLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: 500;"
        "   color: #0D121B;"
        "   margin-bottom: 8px;"
        "}"
    );

    usernameFieldFrame = new QFrame(formSection);
    usernameFieldFrame->setObjectName("usernameFieldFrame");
    usernameFieldFrame->setFixedHeight(56);
    usernameFieldFrame->setStyleSheet(
        "QFrame#usernameFieldFrame {"
        "   background-color: #F6F6F8;"
        "   border: 1px solid #CFD7E7;"
        "   border-radius: 8px;"
        "}"
    );

    QHBoxLayout *usernameFieldLayout = new QHBoxLayout(usernameFieldFrame);
    usernameFieldLayout->setContentsMargins(16, 0, 16, 0);
    usernameFieldLayout->setSpacing(12);

    QLabel *usernameIconLabel = new QLabel(usernameFieldFrame);
    usernameIconLabel->setFixedSize(24, 24);
    usernameIconLabel->setAlignment(Qt::AlignCenter);
    usernameIconLabel->setText("👤");
    usernameIconLabel->setStyleSheet("font-size: 18px;");

    usernameEdit = new QLineEdit(usernameFieldFrame);
    usernameEdit->setPlaceholderText("请输入您的用户名或邮箱");
    usernameEdit->setClearButtonEnabled(true);
    usernameEdit->setFrame(false);
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "   font-size: 16px;"
        "   color: #0D121B;"
        "   background: transparent;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #9CA3AF;"
        "}"
    );

    usernameFieldLayout->addWidget(usernameIconLabel);
    usernameFieldLayout->addWidget(usernameEdit);

    formLayout->addWidget(usernameLabel);
    formLayout->addWidget(usernameFieldFrame);

    passwordLabel = new QLabel("密码", formSection);
    passwordLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: 500;"
        "   color: #0D121B;"
        "   margin-top: 8px;"
        "   margin-bottom: 8px;"
        "}"
    );

    passwordFieldFrame = new QFrame(formSection);
    passwordFieldFrame->setObjectName("passwordFieldFrame");
    passwordFieldFrame->setFixedHeight(56);
    passwordFieldFrame->setStyleSheet(
        "QFrame#passwordFieldFrame {"
        "   background-color: #F6F6F8;"
        "   border: 1px solid #CFD7E7;"
        "   border-radius: 8px;"
        "}"
    );

    QHBoxLayout *passwordFieldLayout = new QHBoxLayout(passwordFieldFrame);
    passwordFieldLayout->setContentsMargins(16, 0, 16, 0);
    passwordFieldLayout->setSpacing(12);

    QLabel *passwordIconLabel = new QLabel(passwordFieldFrame);
    passwordIconLabel->setFixedSize(24, 24);
    passwordIconLabel->setAlignment(Qt::AlignCenter);
    passwordIconLabel->setText("🔒");
    passwordIconLabel->setStyleSheet("font-size: 18px;");

    passwordEdit = new QLineEdit(passwordFieldFrame);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("请输入您的密码");
    passwordEdit->setFrame(false);
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "   font-size: 16px;"
        "   color: #0D121B;"
        "   background: transparent;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #9CA3AF;"
        "}"
    );

    togglePasswordButton = new QPushButton(passwordFieldFrame);
    togglePasswordButton->setText("👁");
    togglePasswordButton->setFixedSize(32, 32);
    togglePasswordButton->setCursor(Qt::PointingHandCursor);
    togglePasswordButton->setFocusPolicy(Qt::NoFocus);
    togglePasswordButton->setToolTip("显示密码");
    togglePasswordButton->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   background: transparent;"
        "   font-size: 18px;"
        "}"
        "QPushButton:hover {"
        "   background: rgba(198, 40, 40, 0.1);"
        "   border-radius: 6px;"
        "}"
    );

    passwordFieldLayout->addWidget(passwordIconLabel);
    passwordFieldLayout->addWidget(passwordEdit);
    passwordFieldLayout->addWidget(togglePasswordButton);
    passwordFieldLayout->setStretch(1, 1);

    usernameEdit->installEventFilter(this);
    passwordEdit->installEventFilter(this);

    formLayout->addWidget(passwordLabel);
    formLayout->addWidget(passwordFieldFrame);

    // 密码强度指示器
    passwordStrengthLabel = new QLabel("密码强度", formSection);
    passwordStrengthLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 12px;"
        "   color: #9CA3AF;"
        "   margin-top: 4px;"
        "   margin-bottom: 4px;"
        "}"
    );

    passwordStrengthBar = new QProgressBar(formSection);
    passwordStrengthBar->setRange(0, 100);
    passwordStrengthBar->setValue(0);
    passwordStrengthBar->setTextVisible(false);
    passwordStrengthBar->setFixedHeight(4);
    passwordStrengthBar->setStyleSheet(
        "QProgressBar {"
        "   border: none;"
        "   border-radius: 2px;"
        "   background-color: #F3F4F6;"
        "}"
        "QProgressBar::chunk {"
        "   border-radius: 2px;"
        "   background-color: #10B981;"
        "}"
    );

    formLayout->addWidget(passwordStrengthLabel);
    formLayout->addWidget(passwordStrengthBar);

    QWidget *optionsContainer = new QWidget(formSection);
    QHBoxLayout *optionsLayout = new QHBoxLayout(optionsContainer);
    optionsLayout->setContentsMargins(0, 4, 0, 0);
    optionsLayout->setSpacing(12);

    rememberMeCheck = new QCheckBox("记住我", optionsContainer);
    rememberMeCheck->setCursor(Qt::PointingHandCursor);
    rememberMeCheck->setStyleSheet(
        "QCheckBox {"
        "   font-size: 14px;"
        "   color: #0D121B;"
        "   spacing: 12px;"
        "}"
        "QCheckBox::indicator {"
        "   width: 20px;"
        "   height: 20px;"
        "   border: 1px solid #D1D5DB;"
        "   border-radius: 4px;"
        "   background-color: #FFFFFF;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background-color: #C62828;"
        "   border-color: #C62828;"
        "   image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTQiIGhlaWdodD0iMTAiIHZpZXdCb3g9IjAgMCAxNCAxMCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTEzIDAuNUw0LjUgOUwxIDUuNSIgc3Ryb2tlPSJ3aGl0ZSIgc3Ryb2tlLXdpZHRoPSIyIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KPC9zdmc+);"
        "}"
    );

    autoLoginCheck = new QCheckBox("自动登录", optionsContainer);
    autoLoginCheck->setCursor(Qt::PointingHandCursor);
    autoLoginCheck->setStyleSheet(rememberMeCheck->styleSheet());

    optionsLayout->addWidget(rememberMeCheck);
    optionsLayout->addWidget(autoLoginCheck);
    optionsLayout->addStretch();

    forgotPasswordButton = new QPushButton("忘记密码?", optionsContainer);
    forgotPasswordButton->setCursor(Qt::PointingHandCursor);
    forgotPasswordButton->setStyleSheet(
        "QPushButton {"
        "   color: #C62828;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   border: none;"
        "   background: transparent;"
        "   padding: 4px 8px;"
        "}"
        "QPushButton:hover {"
        "   color: #B91C1C;"
        "   text-decoration: underline;"
        "}"
    );

    optionsLayout->addWidget(forgotPasswordButton);
    formLayout->addWidget(optionsContainer);

    loginButton = new QPushButton("登 录", formSection);
    loginButton->setAutoDefault(true);
    loginButton->setDefault(true);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setFixedHeight(56);
    loginButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #C62828;"
        "   color: white;"
        "   border: none;"
        "   font-size: 16px;"
        "   font-weight: 700;"
        "   border-radius: 8px;"
        "   letter-spacing: 2px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #DC2626;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #B91C1C;"
        "}"
    );
    formLayout->addWidget(loginButton);

    QWidget *registerContainer = new QWidget(formSection);
    QHBoxLayout *registerLayout = new QHBoxLayout(registerContainer);
    registerLayout->setContentsMargins(0, 0, 0, 0);
    registerLayout->setSpacing(4);
    registerLayout->setAlignment(Qt::AlignCenter);

    QLabel *noAccountLabel = new QLabel("没有账户? ", registerContainer);
    noAccountLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 14px;"
        "   color: #9CA3AF;"
        "}"
    );

    registerButton = new QPushButton("立即注册", registerContainer);
    registerButton->setCursor(Qt::PointingHandCursor);
    registerButton->setStyleSheet(
        "QPushButton {"
        "   color: #C62828;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   border: none;"
        "   background: transparent;"
        "   padding: 4px 8px;"
        "}"
        "QPushButton:hover {"
        "   color: #B91C1C;"
        "   text-decoration: underline;"
        "}"
    );

    registerLayout->addWidget(noAccountLabel);
    registerLayout->addWidget(registerButton);
    formLayout->addWidget(registerContainer);

    containerLayout->addWidget(formSection);
    rightLayout->addWidget(mainContainer);
    rightLayout->setAlignment(mainContainer, Qt::AlignCenter);
    rightLayout->addStretch();

    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(rightPanel);
    contentLayout->setStretch(0, 1);
    contentLayout->setStretch(1, 1);

    QHBoxLayout *cardWrapper = new QHBoxLayout();
    cardWrapper->setContentsMargins(0, 0, 0, 0);
    cardWrapper->setSpacing(0);
    cardWrapper->addStretch();
    cardWrapper->addWidget(contentFrame);
    cardWrapper->addStretch();
    cardWrapper->setAlignment(contentFrame, Qt::AlignCenter);

    mainLayout->addLayout(cardWrapper);
    mainLayout->addStretch();

}

void IdenticalLoginWindow::setupStyles()
{
    // 设置窗口整体样式，使用纯色背景
    this->setStyleSheet(
        "QDialog {"
        "   background-color: #f0f0f0;"
        "}"
    );
}

void IdenticalLoginWindow::setupAnimations()
{
    // 完全移除透明动画，避免显示问题
    // 窗口直接显示，不使用任何透明效果
}

void IdenticalLoginWindow::connectSignals()
{
    connect(loginButton, &QPushButton::clicked, this, &IdenticalLoginWindow::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &IdenticalLoginWindow::onRegisterClicked);
    connect(forgotPasswordButton, &QPushButton::clicked, this, &IdenticalLoginWindow::onForgotPasswordClicked);
    connect(togglePasswordButton, &QPushButton::clicked, this, &IdenticalLoginWindow::onTogglePasswordVisibility);
    connect(passwordEdit, &QLineEdit::textChanged, this, &IdenticalLoginWindow::onPasswordChanged);
    connect(roleGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &IdenticalLoginWindow::onRoleSelectionChanged);
}

bool IdenticalLoginWindow::eventFilter(QObject *watched, QEvent *event)
{
    auto updateFrame = [](QFrame *frame, bool focused) {
        if (!frame) {
            return;
        }
        frame->setStyleSheet(
            QStringLiteral(
                "QFrame#%1 {"
                "   background-color: #F6F6F8;"
                "   border: %2;"
                "   border-radius: 8px;"
                "}"
            ).arg(frame->objectName(),
                  focused ? QStringLiteral("2px solid rgba(198, 40, 40, 0.5)")
                          : QStringLiteral("1px solid #CFD7E7"))
        );
    };

    if (watched == usernameEdit) {
        if (event->type() == QEvent::FocusIn) {
            updateFrame(usernameFieldFrame, true);
        } else if (event->type() == QEvent::FocusOut) {
            updateFrame(usernameFieldFrame, false);
        }
    } else if (watched == passwordEdit) {
        if (event->type() == QEvent::FocusIn) {
            updateFrame(passwordFieldFrame, true);
        } else if (event->type() == QEvent::FocusOut) {
            updateFrame(passwordFieldFrame, false);
        }
    }

    return QDialog::eventFilter(watched, event);
}

void IdenticalLoginWindow::paintEvent(QPaintEvent *event)
{
    // 简化绘制，避免自定义绘制导致的显示问题
    QDialog::paintEvent(event);
}

void IdenticalLoginWindow::onLoginClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();

    validateInput();

    if (username.isEmpty()) {
        showError("请输入用户名！");
        usernameEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        showError("请输入密码！");
        passwordEdit->setFocus();
        return;
    }

    if (password.length() < 6) {
        showError("密码长度至少为6位！");
        passwordEdit->setFocus();
        return;
    }

    // 获取选择的角色
    int roleId = roleGroup->checkedId();
    QString roleName;
    if (roleId == 1) roleName = "教师";
    else if (roleId == 2) roleName = "学生";
    else roleName = "管理员";

    // 开始登录动画
    startLoginAnimation();

    // 模拟异步登录验证
    QTimer::singleShot(2000, [this, username, password, roleName, roleId]() {
        bool loginSuccess = false;
        QString welcomeMessage = "登录成功！欢迎，" + username;

        // 根据角色验证不同的账户
        if (roleId == 1) { // 教师
            if ((username == "teacher" && password == "teacher123") ||
                (username == "admin" && password == "123456")) {
                loginSuccess = true;
            }
        } else if (roleId == 2) { // 学生
            if ((username == "student" && password == "student123") ||
                (username == "admin" && password == "123456")) {
                loginSuccess = true;
            }
        } else { // 管理员
            if ((username == "admin" && password == "123456")) {
                loginSuccess = true;
            }
        }

        if (loginSuccess) {
            showSuccess(welcomeMessage);

            // 保存登录设置
            saveSettings();

            // 延迟关闭并打开主窗口
            QTimer::singleShot(1000, [this, roleName, username]() {
                ModernMainWindow *mainWin = new ModernMainWindow(roleName, username);
                mainWin->show();
                this->close();
            });
        } else {
            QString errorMessage = QString("用户名或密码错误！\n\n%1账户示例：\n")
                                .arg(roleName);

            if (roleId == 1) {
                errorMessage += "教师: teacher/teacher123";
            } else if (roleId == 2) {
                errorMessage += "学生: student/student123";
            } else {
                errorMessage += "管理员: admin/123456";
            }

            showError(errorMessage);
            onLoginAnimationFinished();
        }
    });
}

void IdenticalLoginWindow::onRegisterClicked()
{
    QMessageBox::information(this, "注册", "注册功能正在开发中...\n\n请联系管理员获取账户信息。");
}

void IdenticalLoginWindow::onForgotPasswordClicked()
{
    QMessageBox::information(this, "忘记密码", "密码重置功能正在开发中...\n\n请联系管理员重置密码。");
}

void IdenticalLoginWindow::onTogglePasswordVisibility()
{
    if (passwordEdit->echoMode() == QLineEdit::Password) {
        passwordEdit->setEchoMode(QLineEdit::Normal);
        togglePasswordButton->setText("👁‍🗨");
        togglePasswordButton->setToolTip("隐藏密码");
    } else {
        passwordEdit->setEchoMode(QLineEdit::Password);
        togglePasswordButton->setText("👁");
        togglePasswordButton->setToolTip("显示密码");
    }
}

void IdenticalLoginWindow::performLogin(const QString &username, const QString &password)
{
    Q_UNUSED(username)
    Q_UNUSED(password)
    // 实现登录逻辑
}

void IdenticalLoginWindow::showError(const QString &message)
{
    QMessageBox::warning(this, "登录失败", message);
}

void IdenticalLoginWindow::showSuccess(const QString &message)
{
    QMessageBox::information(this, "登录成功", message);
}

void IdenticalLoginWindow::loadSettings()
{
    QString savedUsername = settings->value("username", "").toString();
    QString savedPassword = settings->value("password", "").toString();
    int savedRole = settings->value("role", 1).toInt();
    bool rememberMe = settings->value("rememberMe", false).toBool();
    bool autoLogin = settings->value("autoLogin", false).toBool();

    if (!savedUsername.isEmpty()) {
        usernameEdit->setText(savedUsername);
    }

    if (rememberMe && !savedPassword.isEmpty()) {
        passwordEdit->setText(savedPassword);
        rememberMeCheck->setChecked(true);
    }

    autoLoginCheck->setChecked(autoLogin);

    QAbstractButton *savedRadio = roleGroup->button(savedRole);
    if (savedRadio) {
        savedRadio->setChecked(true);
    }

    // 自动登录
    if (autoLogin && !savedUsername.isEmpty() && !savedPassword.isEmpty()) {
        QTimer::singleShot(1500, this, &IdenticalLoginWindow::onLoginClicked);
    }
}

void IdenticalLoginWindow::saveSettings()
{
    settings->setValue("username", usernameEdit->text());
    settings->setValue("role", roleGroup->checkedId());

    if (rememberMeCheck->isChecked()) {
        settings->setValue("password", passwordEdit->text());
        settings->setValue("rememberMe", true);
    } else {
        settings->remove("password");
        settings->setValue("rememberMe", false);
    }

    settings->setValue("autoLogin", autoLoginCheck->isChecked());
}

void IdenticalLoginWindow::validateInput()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (username.isEmpty()) {
        usernameFieldFrame->setStyleSheet(
            "QFrame#usernameFieldFrame {"
            "   background-color: #FEE2E2;"
            "   border: 2px solid #EF4444;"
            "   border-radius: 8px;"
            "}"
        );
        return;
    } else {
        usernameFieldFrame->setStyleSheet(
            "QFrame#usernameFieldFrame {"
            "   background-color: #F6F6F8;"
            "   border: 1px solid #CFD7E7;"
            "   border-radius: 8px;"
            "}"
        );
    }

    if (password.length() < 6) {
        passwordFieldFrame->setStyleSheet(
            "QFrame#passwordFieldFrame {"
            "   background-color: #FEE2E2;"
            "   border: 2px solid #EF4444;"
            "   border-radius: 8px;"
            "}"
        );
        return;
    } else {
        passwordFieldFrame->setStyleSheet(
            "QFrame#passwordFieldFrame {"
            "   background-color: #F6F6F8;"
            "   border: 1px solid #CFD7E7;"
            "   border-radius: 8px;"
            "}"
        );
    }
}

void IdenticalLoginWindow::updatePasswordStrength(const QString &password)
{
    int strength = 0;
    QString strengthText;
    QString strengthColor;

    if (password.length() >= 8) strength += 25;
    if (password.length() >= 12) strength += 25;
    if (password.contains(QRegularExpression("[A-Z]"))) strength += 20;
    if (password.contains(QRegularExpression("[a-z]"))) strength += 15;
    if (password.contains(QRegularExpression("[0-9]"))) strength += 10;
    if (password.contains(QRegularExpression("[!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?]"))) strength += 5;

    strength = qMin(strength, 100);

    if (strength <= 30) {
        strengthText = "弱";
        strengthColor = "#EF4444";
    } else if (strength <= 60) {
        strengthText = "中等";
        strengthColor = "#F59E0B";
    } else if (strength <= 80) {
        strengthText = "强";
        strengthColor = "#10B981";
    } else {
        strengthText = "非常强";
        strengthColor = "#3B82F6";
    }

    passwordStrengthBar->setValue(strength);
    passwordStrengthLabel->setText(QString("密码强度: %1").arg(strengthText));
    passwordStrengthLabel->setStyleSheet(QString(
        "QLabel {"
        "   font-size: 12px;"
        "   color: %1;"
        "   margin-top: 4px;"
        "   margin-bottom: 4px;"
        "}"
    ).arg(strengthColor));

    QString barStyle = QString(
        "QProgressBar {"
        "   border: none;"
        "   border-radius: 2px;"
        "   background-color: #F3F4F6;"
        "}"
        "QProgressBar::chunk {"
        "   border-radius: 2px;"
        "   background-color: %1;"
        "}"
    ).arg(strengthColor);

    passwordStrengthBar->setStyleSheet(barStyle);
}

void IdenticalLoginWindow::startLoginAnimation()
{
    loginButton->setEnabled(false);
    loginButton->setText("🔄 登录中...");

    // 简单的样式变化，避免透明效果
    loginButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #999999;"
        "   color: white;"
        "   border: none;"
        "   font-size: 16px;"
        "   font-weight: 700;"
        "   border-radius: 8px;"
        "   letter-spacing: 2px;"
        "}"
    );

    loginTimer->start(2000);
}

void IdenticalLoginWindow::onLoginAnimationFinished()
{
    loginTimer->stop();

    loginButton->setEnabled(true);
    loginButton->setText("登 录");

    // 恢复原始样式
    loginButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #C62828;"
        "   color: white;"
        "   border: none;"
        "   font-size: 16px;"
        "   font-weight: 700;"
        "   border-radius: 8px;"
        "   letter-spacing: 2px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #DC2626;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #B91C1C;"
        "}"
    );
}

void IdenticalLoginWindow::onPasswordChanged(const QString &password)
{
    updatePasswordStrength(password);
}

void IdenticalLoginWindow::onRoleSelectionChanged()
{
    // 角色选择改变时的视觉反馈
    QFrame *selectedFrame = roleSelectionFrame;
    if (selectedFrame) {
        selectedFrame->setStyleSheet(
            "QFrame#roleSelectionFrame {"
            "   background-color: #FEF3C7;"
            "   border: 2px solid #F59E0B;"
            "   border-radius: 8px;"
            "}"
        );

        // 恢复原始样式
        QTimer::singleShot(300, [selectedFrame]() {
            selectedFrame->setStyleSheet(
                "QFrame#roleSelectionFrame {"
                "   background-color: #F6F6F8;"
                "   border: 1px solid #CFD7E7;"
                "   border-radius: 8px;"
                "}"
            );
        });
    }
}
