#include "SidebarManager.h"
#include "../shared/StyleConfig.h"

#include <QGraphicsDropShadowEffect>
#include <QFile>
#include <QDebug>

SidebarManager::SidebarManager(QWidget *parentWidget, QObject *parent)
    : QObject(parent)
    , m_parentWidget(parentWidget)
    , m_sidebar(nullptr)
    , m_sidebarLayout(nullptr)
    , m_profileWidget(nullptr)
    , m_avatarLabel(nullptr)
    , m_userNameLabel(nullptr)
    , m_teacherCenterBtn(nullptr)
    , m_newsTrackingBtn(nullptr)
    , m_aiPreparationBtn(nullptr)
    , m_resourceManagementBtn(nullptr)
    , m_learningAnalysisBtn(nullptr)
    , m_dataAnalysisBtn(nullptr)
    , m_settingsBtn(nullptr)
    , m_helpBtn(nullptr)
    , m_activeButton(nullptr)
{
    setupSidebar();
    setupNavButtons();
    setupBottomButtons();
    createSidebarProfile();
    applySidebarIcons();

    qDebug() << "[SidebarManager] Sidebar initialized";
}

SidebarManager::~SidebarManager()
{
    // 组件随父对象自动销毁
}

void SidebarManager::setupSidebar()
{
    m_sidebar = new QFrame(m_parentWidget);
    m_sidebar->setObjectName("sidebar");
    m_sidebar->setFixedWidth(240);

    m_sidebarLayout = new QVBoxLayout(m_sidebar);
    m_sidebarLayout->setContentsMargins(16, 20, 16, 20);
    m_sidebarLayout->setSpacing(8);

    // 侧边栏样式
    m_sidebar->setStyleSheet(QString(
        "QFrame#sidebar {"
        "   background-color: %1;"
        "   border-right: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));
}

void SidebarManager::setupNavButtons()
{
    // 创建导航按钮的辅助 lambda
    auto createNavButton = [this](const QString &text, const QString &iconName) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, m_sidebar);
        btn->setObjectName(iconName + "Btn");
        btn->setFixedHeight(44);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: transparent;"
            "   border: none;"
            "   border-radius: %1px;"
            "   padding: 10px 16px;"
            "   text-align: left;"
            "   font-size: 14px;"
            "   color: %2;"
            "}"
            "QPushButton:hover {"
            "   background-color: %3;"
            "}"
            "QPushButton:checked, QPushButton[active=\"true\"] {"
            "   background-color: %4;"
            "   color: %5;"
            "   font-weight: 600;"
            "}"
        ).arg(StyleConfig::RADIUS_S)
         .arg(StyleConfig::TEXT_PRIMARY)
         .arg(StyleConfig::PATRIOTIC_RED_LIGHT)
         .arg(StyleConfig::PATRIOTIC_RED_LIGHT)
         .arg(StyleConfig::PATRIOTIC_RED));
        return btn;
    };

    // 创建导航按钮
    m_teacherCenterBtn = createNavButton("教师中心", "teacherCenter");
    m_newsTrackingBtn = createNavButton("时政新闻", "newsTracking");
    m_aiPreparationBtn = createNavButton("AI 智能备课", "aiPreparation");
    m_resourceManagementBtn = createNavButton("资源库管理", "resourceManagement");
    m_learningAnalysisBtn = createNavButton("学情与教评", "learningAnalysis");
    m_dataAnalysisBtn = createNavButton("数据分析报告", "dataAnalysis");

    // 添加到布局
    m_sidebarLayout->addWidget(m_teacherCenterBtn);
    m_sidebarLayout->addWidget(m_newsTrackingBtn);
    m_sidebarLayout->addWidget(m_aiPreparationBtn);
    m_sidebarLayout->addWidget(m_resourceManagementBtn);
    m_sidebarLayout->addWidget(m_learningAnalysisBtn);
    m_sidebarLayout->addWidget(m_dataAnalysisBtn);
    m_sidebarLayout->addStretch();

    // 连接信号
    connect(m_teacherCenterBtn, &QPushButton::clicked, this, &SidebarManager::onTeacherCenterClicked);
    connect(m_newsTrackingBtn, &QPushButton::clicked, this, &SidebarManager::onNewsTrackingClicked);
    connect(m_aiPreparationBtn, &QPushButton::clicked, this, &SidebarManager::onAIPreparationClicked);
    connect(m_resourceManagementBtn, &QPushButton::clicked, this, &SidebarManager::onResourceManagementClicked);
    connect(m_learningAnalysisBtn, &QPushButton::clicked, this, &SidebarManager::onLearningAnalysisClicked);
    connect(m_dataAnalysisBtn, &QPushButton::clicked, this, &SidebarManager::onDataAnalysisClicked);

    // 默认选中教师中心
    setActiveNavigation(Dashboard);
}

void SidebarManager::setupBottomButtons()
{
    m_settingsBtn = new QPushButton("系统设置", m_sidebar);
    m_settingsBtn->setObjectName("settingsBtn");
    m_settingsBtn->setFixedHeight(40);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);

    m_helpBtn = new QPushButton("帮助中心", m_sidebar);
    m_helpBtn->setObjectName("helpBtn");
    m_helpBtn->setFixedHeight(40);
    m_helpBtn->setCursor(Qt::PointingHandCursor);

    // 底部按钮样式
    QString bottomBtnStyle = QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   padding: 8px 16px;"
        "   text-align: left;"
        "   font-size: 13px;"
        "   color: %1;"
        "}"
        "QPushButton:hover {"
        "   color: %2;"
        "}"
    ).arg(StyleConfig::TEXT_SECONDARY, StyleConfig::PATRIOTIC_RED);

    m_settingsBtn->setStyleSheet(bottomBtnStyle);
    m_helpBtn->setStyleSheet(bottomBtnStyle);

    m_sidebarLayout->addWidget(m_settingsBtn);
    m_sidebarLayout->addWidget(m_helpBtn);

    connect(m_settingsBtn, &QPushButton::clicked, this, &SidebarManager::settingsClicked);
    connect(m_helpBtn, &QPushButton::clicked, this, &SidebarManager::helpClicked);
}

void SidebarManager::createSidebarProfile()
{
    m_profileWidget = new QFrame(m_sidebar);
    m_profileWidget->setObjectName("profileWidget");
    m_profileWidget->setFixedHeight(80);

    QHBoxLayout *profileLayout = new QHBoxLayout(m_profileWidget);
    profileLayout->setContentsMargins(8, 8, 8, 8);
    profileLayout->setSpacing(12);

    // 头像
    m_avatarLabel = new QLabel(m_profileWidget);
    m_avatarLabel->setFixedSize(48, 48);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet(QString(
        "QLabel {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2);"
        "   border-radius: 24px;"
        "   color: white;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED, StyleConfig::PATRIOTIC_RED_DARK));

    // 用户名
    m_userNameLabel = new QLabel(m_profileWidget);
    m_userNameLabel->setStyleSheet(QString(
        "QLabel {"
        "   color: %1;"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "}"
    ).arg(StyleConfig::TEXT_PRIMARY));

    profileLayout->addWidget(m_avatarLabel);
    profileLayout->addWidget(m_userNameLabel);
    profileLayout->addStretch();

    // 插入到布局顶部
    m_sidebarLayout->insertWidget(0, m_profileWidget);
    m_sidebarLayout->insertSpacing(1, 16);
}

void SidebarManager::applySidebarIcons()
{
    // TODO: 从资源文件加载图标
    // 暂时使用文字，后续可替换为 SVG 图标
}

QIcon SidebarManager::loadSidebarIcon(const QString &themeName, QStyle::StandardPixmap fallback) const
{
    QString iconPath = QString(":/icons/resources/icons/%1.svg").arg(themeName);
    if (QFile::exists(iconPath)) {
        return QIcon(iconPath);
    }
    return m_parentWidget->style()->standardIcon(fallback);
}

void SidebarManager::setUserInfo(const QString &username, const QString &role)
{
    m_username = username;
    m_userRole = role;

    if (m_avatarLabel && !username.isEmpty()) {
        m_avatarLabel->setText(username.left(1));  // 显示姓氏首字
    }
    if (m_userNameLabel) {
        m_userNameLabel->setText(username);
    }

    qDebug() << "[SidebarManager] User info set:" << username << role;
}

void SidebarManager::setActiveNavigation(PageIndex index)
{
    QPushButton *targetBtn = nullptr;

    switch (index) {
        case Dashboard: targetBtn = m_teacherCenterBtn; break;
        case NewsTracking: targetBtn = m_newsTrackingBtn; break;
        case AIPreparation: targetBtn = m_aiPreparationBtn; break;
        case ResourceManagement: targetBtn = m_resourceManagementBtn; break;
        case LearningAnalysis: targetBtn = m_learningAnalysisBtn; break;
        case DataAnalysis: targetBtn = m_dataAnalysisBtn; break;
        default: return;
    }

    if (targetBtn) {
        updateButtonStyles(targetBtn);
    }
}

void SidebarManager::updateButtonStyles(QPushButton *activeButton)
{
    // 清除之前的选中状态
    if (m_activeButton) {
        m_activeButton->setProperty("active", false);
        m_activeButton->style()->unpolish(m_activeButton);
        m_activeButton->style()->polish(m_activeButton);
    }

    // 设置新的选中状态
    m_activeButton = activeButton;
    if (m_activeButton) {
        m_activeButton->setProperty("active", true);
        m_activeButton->style()->unpolish(m_activeButton);
        m_activeButton->style()->polish(m_activeButton);
    }
}

void SidebarManager::onTeacherCenterClicked()
{
    updateButtonStyles(m_teacherCenterBtn);
    emit navigationRequested(Dashboard);
}

void SidebarManager::onNewsTrackingClicked()
{
    updateButtonStyles(m_newsTrackingBtn);
    emit navigationRequested(NewsTracking);
}

void SidebarManager::onAIPreparationClicked()
{
    updateButtonStyles(m_aiPreparationBtn);
    emit navigationRequested(AIPreparation);
}

void SidebarManager::onResourceManagementClicked()
{
    updateButtonStyles(m_resourceManagementBtn);
    emit navigationRequested(ResourceManagement);
}

void SidebarManager::onLearningAnalysisClicked()
{
    updateButtonStyles(m_learningAnalysisBtn);
    emit navigationRequested(LearningAnalysis);
}

void SidebarManager::onDataAnalysisClicked()
{
    updateButtonStyles(m_dataAnalysisBtn);
    emit navigationRequested(DataAnalysis);
}
