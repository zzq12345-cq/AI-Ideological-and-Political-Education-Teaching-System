#include "UserSettingsDialog.h"
#include "UserSettingsManager.h"
#include "../shared/StyleConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QScrollArea>
#include <QCoreApplication>
#include <QtSvgWidgets/QSvgWidget>
#include <QFile>
#include <QPalette>
#include <QStyleFactory>

UserSettingsDialog::UserSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("个人设置");
    // 艹，之前用 setFixedSize 把窗口锁死了，改成可调整大小
    setMinimumSize(540, 680);
    resize(560, 720);  // 默认大小稍微大一点
    setAttribute(Qt::WA_TranslucentBackground, false);
    setStyleSheet(QString("QDialog { background-color: %1; }").arg(StyleConfig::BG_APP));
    setupUI();
    loadSettings();
}

UserSettingsDialog::~UserSettingsDialog()
{
}

QString UserSettingsDialog::getIconPath(const QString &name) const
{
    // 获取图标路径
    QString basePath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MAC
    basePath += "/../../../../resources/icons/";
#else
    basePath += "/../resources/icons/";
#endif
    return basePath + name + ".svg";
}

QString UserSettingsDialog::getInputStyle() const
{
    // 艹，老王用最直接的方式写样式
    return QString(
        "padding: 12px 14px;"
        "border: 1px solid #E5E7EB;"
        "border-radius: 8px;"
        "font-size: 14px;"
        "min-height: 20px;"
    );
}

// 老王专门搞个函数配置输入框，用 Fusion 风格 + QPalette 强制设置颜色
static void configureInputWidget(QWidget *widget)
{
    // 艹，macOS 原生样式会覆盖 QSS，必须用 Fusion 风格
    widget->setStyle(QStyleFactory::create("Fusion"));
    widget->setAutoFillBackground(true);

    QPalette pal = widget->palette();
    pal.setColor(QPalette::Base, QColor("#FFFFFF"));
    pal.setColor(QPalette::Text, QColor("#1A1A1A"));
    pal.setColor(QPalette::PlaceholderText, QColor("#9CA3AF"));
    pal.setColor(QPalette::Window, QColor("#FFFFFF"));
    pal.setColor(QPalette::WindowText, QColor("#1A1A1A"));
    pal.setColor(QPalette::Button, QColor("#FFFFFF"));
    pal.setColor(QPalette::ButtonText, QColor("#1A1A1A"));
    widget->setPalette(pal);
}

void UserSettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 32, 32, 28);
    mainLayout->setSpacing(24);

    // 大标题区域
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    // 设置图标
    QSvgWidget *headerIcon = new QSvgWidget(getIconPath("settings"));
    headerIcon->setFixedSize(28, 28);

    QLabel *titleLabel = new QLabel("个人信息设置");
    titleLabel->setStyleSheet(QString(
        "font-size: 24px;"
        "font-weight: 700;"
        "color: %1;"
        "letter-spacing: 0.5px;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    headerLayout->addWidget(headerIcon);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // 用户卡片
    mainLayout->addWidget(createUserCard());

    // 基本信息区
    mainLayout->addWidget(createBasicInfoSection());

    mainLayout->addStretch();

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    // 恢复默认按钮
    m_resetBtn = new QPushButton("恢复默认");
    m_resetBtn->setCursor(Qt::PointingHandCursor);
    m_resetBtn->setStyleSheet(QString(
        "QPushButton {"
        "  padding: 12px 24px;"
        "  border: 1px solid %1;"
        "  border-radius: %2px;"
        "  background: white;"
        "  color: %3;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "  background: %4;"
        "  border-color: #CBD5E1;"
        "}"
        "QPushButton:pressed {"
        "  background: %1;"
        "}"
    ).arg(StyleConfig::BORDER_LIGHT)
     .arg(StyleConfig::RADIUS_S)
     .arg(StyleConfig::TEXT_SECONDARY)
     .arg(StyleConfig::SEPARATOR));
    connect(m_resetBtn, &QPushButton::clicked, this, &UserSettingsDialog::onResetClicked);
    buttonLayout->addWidget(m_resetBtn);

    buttonLayout->addStretch();

    // 取消按钮
    m_cancelBtn = new QPushButton("取消");
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(m_resetBtn->styleSheet());
    connect(m_cancelBtn, &QPushButton::clicked, this, &UserSettingsDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelBtn);

    // 保存按钮
    m_saveBtn = new QPushButton("保存设置");
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(QString(
        "QPushButton {"
        "  padding: 12px 36px;"
        "  border: none;"
        "  border-radius: %1px;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %2, stop:1 #EF5350);"
        "  color: white;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  letter-spacing: 0.5px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #EF5350, stop:1 %2);"
        "}"
        "QPushButton:pressed {"
        "  background: %3;"
        "}"
    ).arg(StyleConfig::RADIUS_S)
     .arg(StyleConfig::PATRIOTIC_RED)
     .arg(StyleConfig::PATRIOTIC_RED_DARK));
    connect(m_saveBtn, &QPushButton::clicked, this, &UserSettingsDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(buttonLayout);
}

QWidget* UserSettingsDialog::createUserCard()
{
    QFrame *card = new QFrame();
    card->setObjectName("userCard");
    card->setStyleSheet(QString(
        "#userCard {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:0.5 #EF5350, stop:1 #FF8A80);"
        "  border-radius: %2px;"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED).arg(StyleConfig::RADIUS_L));
    card->setFixedHeight(100);

    // 阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(24);
    shadow->setColor(QColor(229, 57, 53, 60));
    shadow->setOffset(0, 6);
    card->setGraphicsEffect(shadow);

    QHBoxLayout *layout = new QHBoxLayout(card);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(18);

    // 头像
    m_avatarPreview = new QLabel("U");
    m_avatarPreview->setFixedSize(64, 64);
    m_avatarPreview->setAlignment(Qt::AlignCenter);
    m_avatarPreview->setStyleSheet(QString(
        "background: white;"
        "color: %1;"
        "font-size: 28px;"
        "font-weight: 700;"
        "border-radius: 32px;"
        "border: 3px solid rgba(255, 255, 255, 0.3);"
    ).arg(StyleConfig::PATRIOTIC_RED));
    layout->addWidget(m_avatarPreview);

    // 用户信息
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(3);

    m_previewLabel = new QLabel("用户名老师");
    m_previewLabel->setStyleSheet(
        "color: white;"
        "font-size: 20px;"
        "font-weight: 600;"
        "letter-spacing: 0.3px;"
    );

    m_emailLabel = new QLabel("user@example.com");
    m_emailLabel->setStyleSheet(
        "color: rgba(255, 255, 255, 0.9);"
        "font-size: 13px;"
    );

    QLabel *deptLabel = new QLabel("思政教研组");
    deptLabel->setObjectName("deptLabel");
    deptLabel->setStyleSheet(
        "color: rgba(255, 255, 255, 0.75);"
        "font-size: 12px;"
    );

    infoLayout->addWidget(m_previewLabel);
    infoLayout->addWidget(m_emailLabel);
    infoLayout->addWidget(deptLabel);

    layout->addLayout(infoLayout);
    layout->addStretch();

    // 右侧装饰图标
    QSvgWidget *decorIcon = new QSvgWidget(getIconPath("user"));
    decorIcon->setFixedSize(48, 48);
    decorIcon->setStyleSheet("opacity: 0.2;");
    layout->addWidget(decorIcon);

    return card;
}

QWidget* UserSettingsDialog::createSectionHeader(const QString &iconPath, const QString &title)
{
    QWidget *header = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 8);
    layout->setSpacing(10);

    // 图标
    QSvgWidget *icon = new QSvgWidget(iconPath);
    icon->setFixedSize(20, 20);

    // 标题
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QString(
        "font-size: 15px;"
        "font-weight: 600;"
        "color: %1;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    layout->addWidget(icon);
    layout->addWidget(titleLabel);
    layout->addStretch();

    return header;
}

QWidget* UserSettingsDialog::createFormRow(const QString &iconPath, const QString &label, QWidget *input)
{
    QWidget *row = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // 标签行
    QHBoxLayout *labelRow = new QHBoxLayout();
    labelRow->setSpacing(8);

    // SVG 图标
    QSvgWidget *icon = new QSvgWidget(iconPath);
    icon->setFixedSize(16, 16);

    QLabel *labelWidget = new QLabel(label);
    labelWidget->setStyleSheet(QString(
        "font-size: 13px;"
        "font-weight: 500;"
        "color: %1;"
    ).arg(StyleConfig::TEXT_SECONDARY));

    labelRow->addWidget(icon);
    labelRow->addWidget(labelWidget);
    labelRow->addStretch();

    layout->addLayout(labelRow);
    layout->addWidget(input);

    return row;
}

QWidget* UserSettingsDialog::createBasicInfoSection()
{
    QFrame *section = new QFrame();
    section->setObjectName("basicSection");
    section->setStyleSheet(QString(
        "#basicSection {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: %3px;"
        "}"
    ).arg(StyleConfig::BG_CARD)
     .arg(StyleConfig::BORDER_LIGHT)
     .arg(StyleConfig::RADIUS_M));

    QVBoxLayout *layout = new QVBoxLayout(section);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(16);

    // 区域标题
    layout->addWidget(createSectionHeader(getIconPath("info-card"), "基本信息"));

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::SEPARATOR));
    layout->addWidget(line);

    // 姓名 - 艹，用 QPalette 强制设置颜色
    m_nicknameEdit = new QLineEdit();
    m_nicknameEdit->setPlaceholderText("请输入您的姓名或昵称");
    configureInputWidget(m_nicknameEdit);
    m_nicknameEdit->setStyleSheet(getInputStyle());
    connect(m_nicknameEdit, &QLineEdit::textChanged, this, &UserSettingsDialog::updatePreview);
    layout->addWidget(createFormRow(getIconPath("user"), "姓名 / 昵称", m_nicknameEdit));

    // 部门
    m_departmentEdit = new QLineEdit();
    m_departmentEdit->setPlaceholderText("请输入所属部门或教研组");
    configureInputWidget(m_departmentEdit);
    m_departmentEdit->setStyleSheet(getInputStyle());
    connect(m_departmentEdit, &QLineEdit::textChanged, this, &UserSettingsDialog::updatePreview);
    layout->addWidget(createFormRow(getIconPath("building"), "部门", m_departmentEdit));

    // 职称
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("请输入职称（可选）");
    configureInputWidget(m_titleEdit);
    m_titleEdit->setStyleSheet(getInputStyle());
    layout->addWidget(createFormRow(getIconPath("award"), "职称", m_titleEdit));

    return section;
}

void UserSettingsDialog::loadSettings()
{
    UserSettingsManager *settings = UserSettingsManager::instance();
    m_nicknameEdit->setText(settings->nickname());
    m_departmentEdit->setText(settings->department());
    m_titleEdit->setText(settings->title());

    // 加载邮箱
    QString email = settings->email();
    if (email.isEmpty()) {
        email = "未设置邮箱";
    }
    m_emailLabel->setText(email);

    updatePreview();
}

void UserSettingsDialog::updatePreview()
{
    QString nickname = m_nicknameEdit->text().trimmed();
    QString department = m_departmentEdit->text().trimmed();

    // 更新头像
    if (!nickname.isEmpty()) {
        m_avatarPreview->setText(nickname.left(1));
    } else {
        m_avatarPreview->setText("U");
    }

    // 更新预览文本 - 统一显示"姓名老师"
    QString displayName = nickname.isEmpty() ? "未设置" : (nickname + "老师");
    m_previewLabel->setText(displayName);

    // 更新部门标签
    QLabel *deptLabel = findChild<QLabel*>("deptLabel");
    if (deptLabel) {
        deptLabel->setText(department.isEmpty() ? "未设置部门" : department);
    }
}

void UserSettingsDialog::onSaveClicked()
{
    QString nickname = m_nicknameEdit->text().trimmed();
    if (nickname.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("提示");
        msgBox.setText("请输入您的姓名或昵称！");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStyleSheet(QString(
            "QMessageBox { background: white; }"
            "QLabel { color: %1; }"
            "QPushButton { padding: 8px 24px; border-radius: 6px; background: %2; color: white; font-weight: 500; }"
        ).arg(StyleConfig::TEXT_PRIMARY).arg(StyleConfig::PATRIOTIC_RED));
        msgBox.exec();
        m_nicknameEdit->setFocus();
        return;
    }

    UserSettingsManager *settings = UserSettingsManager::instance();
    settings->setNickname(nickname);
    settings->setDepartment(m_departmentEdit->text().trimmed());
    settings->setTitle(m_titleEdit->text().trimmed());
    settings->setHonorific("老师");  // 统一称呼为老师
    settings->save();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("成功");
    msgBox.setText("设置已保存！");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet(QString(
        "QMessageBox { background: white; }"
        "QLabel { color: %1; }"
        "QPushButton { padding: 8px 24px; border-radius: 6px; background: %2; color: white; font-weight: 500; }"
    ).arg(StyleConfig::TEXT_PRIMARY).arg(StyleConfig::SUCCESS_GREEN));
    msgBox.exec();
    accept();
}

void UserSettingsDialog::onCancelClicked()
{
    reject();
}

void UserSettingsDialog::onResetClicked()
{
    m_nicknameEdit->clear();
    m_departmentEdit->setText("思政教研组");
    m_titleEdit->clear();
    updatePreview();
}
