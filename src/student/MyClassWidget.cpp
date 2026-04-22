#include "MyClassWidget.h"
#include "JoinClassDialog.h"
#include "CreateClassDialog.h"
#include "ClassManager.h"
#include "ClassDetailWidget.h"
#include "StudentClassDetailWidget.h"
#include "AttendanceManager.h"
#include "AttendanceSignInDialog.h"
#include "../settings/UserSettingsManager.h"
#include "../shared/StyleConfig.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>

static const QStringList CARD_COLORS = {
    "#E53935, #EF5350",
    "#6366F1, #818CF8",
    "#F57C00, #FFA726",
    "#10B981, #34D399",
    "#1976D2, #42A5F5",
    "#9C27B0, #BA68C8",
};

MyClassWidget::MyClassWidget(bool isTeacher, const QString &userEmail, QWidget *parent)
    : QWidget(parent)
    , m_isTeacher(isTeacher)
    , m_userEmail(userEmail)
{
    setupUI();

    // 连接 ClassManager 信号
    auto *mgr = ClassManager::instance();
    connect(mgr, &ClassManager::classesLoaded, this, &MyClassWidget::refreshCards);
    connect(mgr, &ClassManager::classCreated, this, [this](const ClassInfo &) {
        reloadClasses();
    });
    connect(mgr, &ClassManager::joinResult, this, [this](bool ok, const QString &msg, const QString &) {
        if (ok) {
            QMessageBox::information(this, "加入成功", msg);
            reloadClasses();
        } else {
            QMessageBox::warning(this, "加入失败", msg);
        }
    });

    // 学生端签到结果
    if (!m_isTeacher) {
        connect(AttendanceManager::instance(), &AttendanceManager::signResult,
                this, &MyClassWidget::onSignResult);
    }

    // 首次加载
    reloadClasses();
}

void MyClassWidget::setupUI()
{
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    m_stack = new QStackedWidget();

    // === 列表页 ===
    m_listPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_listPage);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(20);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("我的班级");
    titleLabel->setStyleSheet(QString("font-size: 24px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));

    QPushButton *actionBtn = new QPushButton(m_isTeacher ? "+ 创建班级" : "+ 加入班级");
    actionBtn->setCursor(Qt::PointingHandCursor);
    actionBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; border: none; color: %1;"
        "  font-size: 14px; font-weight: 600; padding: 6px 12px; }"
        "QPushButton:hover { background-color: %2; border-radius: 6px; }"
    ).arg(StyleConfig::PATRIOTIC_RED, StyleConfig::PATRIOTIC_RED_LIGHT));

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // 学生端多一个签到按钮
    if (!m_isTeacher) {
        QPushButton *signBtn = new QPushButton("考勤签到");
        signBtn->setCursor(Qt::PointingHandCursor);
        signBtn->setStyleSheet(actionBtn->styleSheet());
        connect(signBtn, &QPushButton::clicked, this, &MyClassWidget::onSignAttendance);
        headerLayout->addWidget(signBtn);
    }

    headerLayout->addWidget(actionBtn);
    mainLayout->addLayout(headerLayout);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::BORDER_LIGHT));
    mainLayout->addWidget(line);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_gridContainer = new QWidget();
    m_gridContainer->setStyleSheet("background: transparent;");
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setSpacing(16);
    m_gridLayout->setContentsMargins(0, 8, 0, 0);
    m_gridLayout->setColumnStretch(0, 1);
    m_gridLayout->setColumnStretch(1, 1);

    scrollArea->setWidget(m_gridContainer);
    mainLayout->addWidget(scrollArea);

    if (m_isTeacher) {
        connect(actionBtn, &QPushButton::clicked, this, &MyClassWidget::onCreateClicked);
    } else {
        connect(actionBtn, &QPushButton::clicked, this, &MyClassWidget::onJoinClicked);
    }

    m_stack->addWidget(m_listPage);
    outerLayout->addWidget(m_stack);
}

QWidget* MyClassWidget::createClassCard(const ClassInfo &info)
{
    QFrame *card = new QFrame();
    card->setObjectName("classCard");
    card->setCursor(Qt::PointingHandCursor);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setStyleSheet(QString(
        "#classCard { background-color: %1; border: 1px solid %2; border-radius: 12px; padding: 16px; }"
        "#classCard:hover { background-color: #FAFAFA; border-color: #D1D5DB; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 1);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(6);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    int colorIdx = info.colorIndex % CARD_COLORS.size();
    QStringList colors = CARD_COLORS[colorIdx].split(", ");

    QLabel *iconLabel = new QLabel(info.name.isEmpty() ? QString() : info.name.left(1));
    iconLabel->setFixedSize(40, 40);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(QString(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);"
        "color: white; font-size: 16px; font-weight: 700; border-radius: 10px;"
    ).arg(colors[0], colors[1]));

    QLabel *statusLabel = new QLabel(info.status == "active" ? "进行中" : "已结束");
    statusLabel->setStyleSheet(QString(
        "font-size: 10px; padding: 2px 8px; border-radius: 8px;"
        "background: %1; color: %2;"
    ).arg(info.status == "active" ? "#DCFCE7" : StyleConfig::SEPARATOR,
          info.status == "active" ? "#16A34A" : StyleConfig::TEXT_SECONDARY));

    topRow->addWidget(iconLabel);
    topRow->addStretch();
    topRow->addWidget(statusLabel);
    cardLayout->addLayout(topRow);

    QLabel *nameLabel = new QLabel(info.name);
    nameLabel->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1;").arg(StyleConfig::TEXT_PRIMARY));

    QString subtitle;
    if (m_isTeacher) {
        subtitle = QString("加课码: %1  |  %2 名学生").arg(info.code).arg(info.studentCount);
    } else {
        subtitle = info.teacher;
    }
    QLabel *subLabel = new QLabel(subtitle);
    subLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(StyleConfig::TEXT_SECONDARY));

    cardLayout->addWidget(nameLabel);
    cardLayout->addWidget(subLabel);

    card->installEventFilter(this);
    card->setProperty("classInfo", info.id);

    return card;
}

QWidget* MyClassWidget::createEmptyState()
{
    QWidget *empty = new QWidget();
    empty->setStyleSheet("background: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(empty);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(8);

    QLabel *icon = new QLabel("📚");
    icon->setStyleSheet("font-size: 48px;");
    icon->setAlignment(Qt::AlignCenter);

    QString hint = m_isTeacher ? "暂无班级，点击\"创建班级\"开始教学" : "暂无班级，点击\"加入班级\"开始学习";
    QLabel *text = new QLabel(hint);
    text->setStyleSheet(QString("font-size: 14px; color: %1;").arg(StyleConfig::TEXT_SECONDARY));
    text->setAlignment(Qt::AlignCenter);

    layout->addWidget(icon);
    layout->addWidget(text);
    return empty;
}

void MyClassWidget::refreshCards(const QList<ClassInfo> &classes)
{
    m_classes = classes;

    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_classes.isEmpty()) {
        m_gridLayout->addWidget(createEmptyState(), 0, 0, 1, 2);
        return;
    }

    for (int i = 0; i < m_classes.size(); ++i) {
        int row = i / 2;
        int col = i % 2;
        m_gridLayout->addWidget(createClassCard(m_classes[i]), row, col);
    }
    int lastRow = (m_classes.size() - 1) / 2 + 1;
    m_gridLayout->setRowStretch(lastRow, 1);
}

void MyClassWidget::reloadClasses()
{
    auto *mgr = ClassManager::instance();
    if (m_isTeacher) {
        mgr->loadTeacherClasses(m_userEmail);
    } else {
        mgr->loadStudentClasses(m_userEmail);
    }
}

void MyClassWidget::onCreateClicked()
{
    CreateClassDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted && !dialog.className().isEmpty()) {
        QString teacherName = UserSettingsManager::instance()->displayName();
        ClassManager::instance()->createClass(dialog.className(), teacherName, m_userEmail);
    }
}

void MyClassWidget::openClassDetail(const ClassInfo &info)
{
    if (m_detailWidget) {
        m_detailWidget->deleteLater();
    }
    m_detailWidget = new ClassDetailWidget(info);
    connect(m_detailWidget, &ClassDetailWidget::backRequested, this, [this]() {
        closeClassDetail();
    });
    m_stack->addWidget(m_detailWidget);
    m_stack->setCurrentWidget(m_detailWidget);
}

void MyClassWidget::closeClassDetail()
{
    if (m_detailWidget) {
        m_stack->removeWidget(m_detailWidget);
        m_detailWidget->deleteLater();
        m_detailWidget = nullptr;
    }
    if (m_studentDetailWidget) {
        m_stack->removeWidget(m_studentDetailWidget);
        m_studentDetailWidget->deleteLater();
        m_studentDetailWidget = nullptr;
    }
    m_stack->setCurrentWidget(m_listPage);
    reloadClasses();
}

void MyClassWidget::openStudentClassDetail(const ClassInfo &info)
{
    if (m_studentDetailWidget) m_studentDetailWidget->deleteLater();
    m_studentDetailWidget = new StudentClassDetailWidget(info);
    connect(m_studentDetailWidget, &StudentClassDetailWidget::backRequested, this, [this]() {
        m_stack->removeWidget(m_studentDetailWidget);
        m_studentDetailWidget->deleteLater();
        m_studentDetailWidget = nullptr;
        m_stack->setCurrentWidget(m_listPage);
        reloadClasses();
    });
    m_stack->addWidget(m_studentDetailWidget);
    m_stack->setCurrentWidget(m_studentDetailWidget);
}

void MyClassWidget::onJoinClicked()
{
    JoinClassDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString code = dialog.classCode();
        if (code.isEmpty()) return;
        QString studentName = UserSettingsManager::instance()->nickname();
        QString studentNumber = UserSettingsManager::instance()->title();
        ClassManager::instance()->joinClass(code, m_userEmail, studentName, studentNumber);
    }
}

void MyClassWidget::onSignAttendance()
{
    AttendanceSignInDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString code = dialog.signCode();
        if (code.isEmpty()) return;
        QString studentName = UserSettingsManager::instance()->nickname();
        AttendanceManager::instance()->signAttendance(code, m_userEmail, studentName);
    }
}

void MyClassWidget::onSignResult(bool success, const QString &message)
{
    if (success) {
        QMessageBox::information(this, "签到成功", message);
    } else {
        QMessageBox::warning(this, "签到失败", message);
    }
}

bool MyClassWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QString classId = watched->property("classInfo").toString();
        if (!classId.isEmpty()) {
            for (const auto &cls : m_classes) {
                if (cls.id == classId) {
                    if (m_isTeacher) openClassDetail(cls);
                    else openStudentClassDetail(cls);
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
