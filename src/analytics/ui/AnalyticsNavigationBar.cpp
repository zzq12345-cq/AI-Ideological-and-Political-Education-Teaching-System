#include "AnalyticsNavigationBar.h"
#include "../../shared/StyleConfig.h"

#include <QDebug>

AnalyticsNavigationBar::AnalyticsNavigationBar(QWidget *parent)
    : QWidget(parent)
    , m_currentView(Overview)
{
    setupUI();
    setupStyles();
}

AnalyticsNavigationBar::~AnalyticsNavigationBar()
{
}

void AnalyticsNavigationBar::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(8);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    // 创建三个导航按钮
    QPushButton *overviewBtn = createNavButton("数据概览", "overview");
    QPushButton *personalBtn = createNavButton("个人分析", "personal");
    QPushButton *classBtn = createNavButton("班级分析", "class");

    m_buttons.append(overviewBtn);
    m_buttons.append(personalBtn);
    m_buttons.append(classBtn);

    m_buttonGroup->addButton(overviewBtn, Overview);
    m_buttonGroup->addButton(personalBtn, Personal);
    m_buttonGroup->addButton(classBtn, ClassWide);

    m_layout->addWidget(overviewBtn);
    m_layout->addWidget(personalBtn);
    m_layout->addWidget(classBtn);
    m_layout->addStretch();

    // 默认选中概览
    overviewBtn->setChecked(true);

    connect(m_buttonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &AnalyticsNavigationBar::onButtonClicked);
}

void AnalyticsNavigationBar::setupStyles()
{
    setFixedHeight(48);
    setStyleSheet("background: transparent;");
    updateButtonStyles();
}

QPushButton* AnalyticsNavigationBar::createNavButton(const QString &text, const QString &icon)
{
    Q_UNUSED(icon)

    QPushButton *btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(36);
    btn->setMinimumWidth(120);

    return btn;
}

void AnalyticsNavigationBar::updateButtonStyles()
{
    // 通用样式
    QString baseStyle = QString(
        "QPushButton {"
        "    background: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    border-radius: 18px;"
        "    padding: 8px 20px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background: %4;"
        "    border-color: %5;"
        "}"
        "QPushButton:checked {"
        "    background: %6;"
        "    color: white;"
        "    border-color: %6;"
        "}"
    ).arg(StyleConfig::BG_CARD)
     .arg(StyleConfig::TEXT_PRIMARY)
     .arg(StyleConfig::BORDER_LIGHT)
     .arg("#F0F4F8")
     .arg(StyleConfig::PATRIOTIC_RED)
     .arg(StyleConfig::PATRIOTIC_RED);

    for (QPushButton *btn : m_buttons) {
        btn->setStyleSheet(baseStyle);
    }
}

void AnalyticsNavigationBar::setCurrentView(ViewType view)
{
    if (m_currentView != view) {
        m_currentView = view;
        if (view >= 0 && view < m_buttons.size()) {
            m_buttons[view]->setChecked(true);
        }
    }
}

AnalyticsNavigationBar::ViewType AnalyticsNavigationBar::currentView() const
{
    return m_currentView;
}

void AnalyticsNavigationBar::onButtonClicked(int id)
{
    ViewType newView = static_cast<ViewType>(id);
    if (m_currentView != newView) {
        m_currentView = newView;
        emit viewChanged(newView);
        qDebug() << "[AnalyticsNavigationBar] View changed to:" << id;
    }
}
