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

// 柔和的左侧色条颜色，不刺眼
static const QStringList CARD_ACCENT_COLORS = {
    "#C62828",  // 暗红
    "#4338CA",  // 靛蓝
    "#B45309",  // 琥珀
    "#047857",  // 翠绿
    "#1565C0",  // 蓝
    "#7B1FA2",  // 紫
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
    mainLayout->setContentsMargins(36, 32, 36, 32);
    mainLayout->setSpacing(0);

    // ── 标题行 ──
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    // 标题图标
    QLabel *titleIcon = new QLabel();
    QIcon bookIcon(":/icons/resources/icons/book.svg");
    titleIcon->setPixmap(bookIcon.pixmap(22, 22));
    titleIcon->setFixedSize(22, 22);
    titleIcon->setStyleSheet("background: transparent;");

    QLabel *titleLabel = new QLabel("我的班级");
    titleLabel->setStyleSheet(QString(
        "font-size: 22px; font-weight: 700; color: %1; letter-spacing: 0.5px;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    // 统计标签（班级数量）
    m_countLabel = new QLabel();
    m_countLabel->setStyleSheet("font-size: 13px; color: #9CA3AF; font-weight: 400;");

    headerLayout->addWidget(titleIcon);
    headerLayout->addWidget(titleLabel);
    headerLayout->addSpacing(4);
    headerLayout->addWidget(m_countLabel);
    headerLayout->addStretch();

    // 学生端签到按钮
    if (!m_isTeacher) {
        QPushButton *signBtn = new QPushButton("考勤签到");
        signBtn->setCursor(Qt::PointingHandCursor);
        signBtn->setStyleSheet(
            "QPushButton { background: transparent; border: 1px solid #D1D5DB;"
            "  color: #374151; font-size: 13px; font-weight: 500;"
            "  padding: 6px 16px; border-radius: 6px; }"
            "QPushButton:hover { background: #F9FAFB; border-color: #9CA3AF; }"
        );
        connect(signBtn, &QPushButton::clicked, this, &MyClassWidget::onSignAttendance);
        headerLayout->addWidget(signBtn);
    }

    // 操作按钮
    QPushButton *actionBtn = new QPushButton(m_isTeacher ? "＋ 创建班级" : "＋ 加入班级");
    actionBtn->setCursor(Qt::PointingHandCursor);
    actionBtn->setStyleSheet(QString(
        "QPushButton { background: %1; border: none; color: white;"
        "  font-size: 13px; font-weight: 600; padding: 7px 18px; border-radius: 6px; }"
        "QPushButton:hover { background: %2; }"
    ).arg(StyleConfig::PATRIOTIC_RED, StyleConfig::PATRIOTIC_RED_DARK));

    headerLayout->addWidget(actionBtn);
    mainLayout->addLayout(headerLayout);

    // 间距
    mainLayout->addSpacing(16);

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px; border: none;").arg(StyleConfig::BORDER_LIGHT));
    mainLayout->addWidget(line);

    mainLayout->addSpacing(20);

    // ── 滚动区域 ──
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #D1D5DB; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #9CA3AF; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    m_gridContainer = new QWidget();
    m_gridContainer->setStyleSheet("background: transparent;");
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setSpacing(16);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
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
    // 外层容器，用来承载左侧色条 + 右侧内容
    QFrame *card = new QFrame();
    card->setObjectName("classCard");
    card->setCursor(Qt::PointingHandCursor);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    int colorIdx = info.colorIndex % CARD_ACCENT_COLORS.size();
    QString accentColor = CARD_ACCENT_COLORS[colorIdx];

    // 左侧 4px 色条作为视觉标识，简洁不花哨
    card->setStyleSheet(QString(
        "#classCard {"
        "  background-color: %1; border: 1px solid %2; border-radius: 10px;"
        "  border-left: 4px solid %3;"
        "}"
        "#classCard:hover { background-color: #FAFBFC; border-color: #C9CDD4; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT, accentColor));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(12);
    shadow->setColor(QColor(0, 0, 0, 12));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(18, 16, 16, 14);

    // 第一行：班级名 + 状态
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(10);

    QLabel *nameLabel = new QLabel(info.name);
    nameLabel->setStyleSheet(QString(
        "font-size: 15px; font-weight: 600; color: %1;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    QLabel *statusLabel = new QLabel(info.status == "active" ? "进行中" : "已结束");
    statusLabel->setStyleSheet(QString(
        "font-size: 11px; padding: 2px 8px; border-radius: 4px;"
        "background: %1; color: %2; font-weight: 500;"
    ).arg(info.status == "active" ? "#ECFDF5" : "#F3F4F6",
          info.status == "active" ? "#059669" : "#6B7280"));

    topRow->addWidget(nameLabel);
    topRow->addStretch();
    topRow->addWidget(statusLabel);
    cardLayout->addLayout(topRow);

    // 第二行：班级码 / 教师名 — 用稍低对比度的颜色
    if (m_isTeacher) {
        QHBoxLayout *infoRow = new QHBoxLayout();
        infoRow->setSpacing(16);

        QLabel *codeLabel = new QLabel(QString("加课码  %1").arg(info.code));
        codeLabel->setStyleSheet(
            "font-size: 13px; color: #6B7280; font-weight: 500; letter-spacing: 0.5px;"
        );

        QLabel *countLabel = new QLabel(QString("%1 名学生").arg(info.studentCount));
        countLabel->setStyleSheet("font-size: 13px; color: #9CA3AF; font-weight: 400;");

        infoRow->addWidget(codeLabel);
        infoRow->addWidget(countLabel);
        infoRow->addStretch();
        cardLayout->addLayout(infoRow);
    } else {
        QLabel *teacherLabel = new QLabel(info.teacher);
        teacherLabel->setStyleSheet("font-size: 13px; color: #6B7280; font-weight: 400;");
        cardLayout->addWidget(teacherLabel);
    }

    // 第三行：描述（如果有的话），最多一行
    if (!info.description.isEmpty()) {
        QLabel *descLabel = new QLabel(
            info.description.length() > 40
                ? info.description.left(40) + "..."
                : info.description);
        descLabel->setStyleSheet("font-size: 12px; color: #B0B6C0; font-weight: 400;");
        cardLayout->addWidget(descLabel);
    }

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
    layout->setSpacing(12);

    // 用 SVG 图标代替 emoji
    QLabel *icon = new QLabel();
    QIcon svgIcon(":/icons/resources/icons/folder.svg");
    icon->setPixmap(svgIcon.pixmap(48, 48));
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: transparent;");

    QString hint = m_isTeacher
        ? "还没有班级"
        : "还没有加入班级";
    QLabel *text = new QLabel(hint);
    text->setStyleSheet(QString(
        "font-size: 15px; color: %1; font-weight: 500;"
    ).arg(StyleConfig::TEXT_SECONDARY));
    text->setAlignment(Qt::AlignCenter);

    QString subHint = m_isTeacher
        ? "点击右上角「创建班级」来新建你的第一个班级"
        : "点击右上角「加入班级」输入加课码";
    QLabel *subText = new QLabel(subHint);
    subText->setStyleSheet("font-size: 13px; color: #B0B6C0;");
    subText->setAlignment(Qt::AlignCenter);

    layout->addWidget(icon);
    layout->addWidget(text);
    layout->addWidget(subText);
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

    // 更新统计
    if (m_classes.isEmpty()) {
        m_countLabel->setText("");
        m_gridLayout->addWidget(createEmptyState(), 0, 0, 1, 2);
        return;
    }

    int totalStudents = 0;
    for (const auto &cls : m_classes) totalStudents += cls.studentCount;

    if (m_isTeacher) {
        m_countLabel->setText(QString("%1 个班级 · %2 名学生")
            .arg(m_classes.size()).arg(totalStudents));
    } else {
        m_countLabel->setText(QString("%1 个班级").arg(m_classes.size()));
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
