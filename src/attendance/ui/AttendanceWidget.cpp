#include "AttendanceWidget.h"
#include "../services/AttendanceService.h"
#include "../models/AttendanceSummary.h"
#include "../../analytics/models/Student.h"
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QDate>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QInputDialog>

// ============ 高级 UI 设计规范 (iOS/macOS 风格) ============
static const QString COL_BG = "#F3F4F6";          // 页面底色
static const QString COL_CARD = "#FFFFFF";        // 卡片底色
static const QString COL_PRIMARY = "#10B981";     // 品牌主色 (Emerald-500)
static const QString COL_DANGER = "#EF4444";      // 危险/缺勤 (Rose-500)
static const QString COL_WARNING = "#F59E0B";     // 警告/迟到 (Amber-500)
static const QString COL_INFO = "#3B82F6";        // 信息/请假 (Blue-500)
static const QString COL_PURPLE = "#8B5CF6";      // 紫色/早退 (Violet-500)

static const QString COL_TEXT_MAIN = "#111827";   // 标题 (Gray-900)
static const QString COL_TEXT_SUB = "#6B7280";    // 副标题 (Gray-500)
static const QString COL_BORDER = "#E5E7EB";      // 边框 (Gray-200)
static const QString COL_SEGMENT_BG = "#F3F4F6";  // 分段控件背景

AttendanceWidget::AttendanceWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
    setupConnections();
}

AttendanceWidget::~AttendanceWidget()
{
    qDeleteAll(m_statusButtonGroups);
}

void AttendanceWidget::setAttendanceService(AttendanceService *service)
{
    m_service = service;
    if (m_service) {
        connect(m_service, &AttendanceService::studentsReceived, this, &AttendanceWidget::onStudentsLoaded);
        connect(m_service, &AttendanceService::attendanceReceived, this, &AttendanceWidget::onAttendanceLoaded);
        connect(m_service, &AttendanceService::attendanceSubmitted, this, [this](bool success, const QString&) {
            if (success) {
                m_saveBtn->setText("✓ 已保存");
                QTimer::singleShot(2000, this, [this]() { m_saveBtn->setText("保存考勤记录"); });
            }
        });
    }
}

void AttendanceWidget::setupUI()
{
    setObjectName("AttendanceWidget");
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(32, 24, 32, 24);
    m_mainLayout->setSpacing(20);

    createHeaderCard();
    createSummaryCard();
    createStudentListCard();
}

void AttendanceWidget::setupStyles()
{
    setStyleSheet(QString("QWidget#AttendanceWidget { background-color: %1; }").arg(COL_BG));
}

void AttendanceWidget::setupConnections()
{
    connect(m_classCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AttendanceWidget::onClassChanged);
    connect(m_dateEdit, &QDateEdit::dateChanged, this, &AttendanceWidget::onDateChanged);
    connect(m_lessonCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AttendanceWidget::onLessonChanged);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AttendanceWidget::onRefreshClicked);
    connect(m_allPresentBtn, &QPushButton::clicked, this, &AttendanceWidget::onAllPresentClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &AttendanceWidget::onSaveClicked);
}

void AttendanceWidget::applyCardShadow(QWidget *widget, qreal blur, qreal offset)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blur);
    shadow->setOffset(0, offset);
    shadow->setColor(QColor(0, 0, 0, 20));
    widget->setGraphicsEffect(shadow);
}

// ============ 1. 纯净头部与极简筛选 ============
void AttendanceWidget::createHeaderCard()
{
    auto *header = new QWidget();
    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);

    // Title Section
    auto *titleSection = new QVBoxLayout();
    titleSection->setSpacing(2);
    auto *title = new QLabel("考勤管理");
    title->setStyleSheet(QString("font-size: 26px; font-weight: 800; color: %1;").arg(COL_TEXT_MAIN));
    auto *sub = new QLabel("班级日常考勤与数据实时统计");
    sub->setStyleSheet(QString("font-size: 13px; color: %1;").arg(COL_TEXT_SUB));
    titleSection->addWidget(title);
    titleSection->addWidget(sub);

    // Action Section (Filter + Save)
    auto *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    // Filter Toolbar
    auto *toolbar = new QFrame();
    toolbar->setStyleSheet(QString("QFrame { background-color: white; border: 1px solid %1; border-radius: 10px; }").arg(COL_BORDER));
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(12, 6, 12, 6);
    tbLayout->setSpacing(12);

    QString comboStyle = QString("QComboBox { border: none; background: transparent; font-size: 13px; font-weight: 600; color: %1; min-width: 80px; } QComboBox::drop-down { border: none; }").arg(COL_TEXT_MAIN);

    m_classCombo = new QComboBox();
    m_classCombo->addItems({"初二1班", "初二2班", "初三1班"});
    m_classCombo->setStyleSheet(comboStyle);

    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setFixedWidth(110);
    m_dateEdit->setStyleSheet(QString("QDateEdit { border: none; background: transparent; font-size: 13px; font-weight: 600; color: %1; }").arg(COL_TEXT_MAIN));

    m_lessonCombo = new QComboBox();
    for(int i=1; i<=8; ++i) m_lessonCombo->addItem(QString("第%1节").arg(i));
    m_lessonCombo->setStyleSheet(comboStyle);

    m_refreshBtn = new QPushButton();
    m_refreshBtn->setFixedSize(28, 28);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet("QPushButton { border: none; border-radius: 6px; } QPushButton:hover { background: #E5E7EB; }");
    setButtonIcon(m_refreshBtn, ":/icons/resources/icons/refresh.svg", COL_TEXT_SUB);

    tbLayout->addWidget(m_classCombo);
    tbLayout->addWidget(m_dateEdit);
    tbLayout->addWidget(m_lessonCombo);
    tbLayout->addWidget(m_refreshBtn);

    // Header Save Button
    m_saveBtn = new QPushButton("保存记录");
    m_saveBtn->setFixedSize(100, 40);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border-radius: 10px; font-size: 13px; font-weight: 700; } QPushButton:hover { background: #374151; } QPushButton:disabled { background: #D1D5DB; }").arg(COL_TEXT_MAIN));
    setButtonIcon(m_saveBtn, ":/icons/resources/icons/save.svg", "#FFFFFF");

    actionLayout->addWidget(toolbar);
    actionLayout->addWidget(m_saveBtn);

    layout->addLayout(titleSection);
    layout->addStretch();
    layout->addLayout(actionLayout);

    m_mainLayout->addWidget(header);
}

// ============ 2. 统计仪表盘 Stat Cards ============
void AttendanceWidget::createSummaryCard()
{
    auto *dashboard = new QWidget();
    auto *layout = new QHBoxLayout(dashboard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    // 翡翠绿/玫瑰红/琥珀色/蓝色
    layout->addWidget(createStatCard("已出勤", m_presentCountLabel, "#059669", ""));
    layout->addWidget(createStatCard("缺勤", m_absentCountLabel, "#EF4444", ""));
    layout->addWidget(createStatCard("迟到", m_lateCountLabel, "#F59E0B", ""));
    layout->addWidget(createStatCard("请假", m_leaveCountLabel, "#3B82F6", ""));
    layout->addWidget(createStatCard("早退", m_earlyCountLabel, "#8B5CF6", ""));

    layout->addStretch();

    // 一键全员签到 - 变得更加克制
    m_allPresentBtn = new QPushButton("一键全员签到");
    m_allPresentBtn->setCursor(Qt::PointingHandCursor);
    m_allPresentBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px; "
        "padding: 0 20px; height: 40px; font-size: 13px; font-weight: 700; }"
        "QPushButton:hover { background: #059669; }"
    ).arg(COL_PRIMARY));
    layout->addWidget(m_allPresentBtn, 0, Qt::AlignVCenter);

    m_mainLayout->addWidget(dashboard);
}

QWidget* AttendanceWidget::createStatCard(const QString &label, QLabel* &countLabel, const QString &color, const QString &iconPath)
{
    Q_UNUSED(iconPath) // 彻底丢弃图标

    auto *card = new QFrame();
    card->setFixedWidth(140);
    card->setFixedHeight(56); // 更加扁平精致
    card->setStyleSheet(QString(
        "QFrame { background: white; border-radius: 8px; border: 1px solid %1; }"
    ).arg(COL_BORDER));

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(16, 0, 16, 0);

    // 左侧：状态名称 (中等字号，对应颜色)
    auto *statusLabel = new QLabel(label);
    statusLabel->setStyleSheet(QString(
        "font-size: 14px; font-weight: 600; color: %1; border: none; background: transparent;"
    ).arg(color));

    // 右侧：大号数字 (加粗黑色)
    countLabel = new QLabel("0");
    countLabel->setStyleSheet(QString(
        "font-size: 22px; font-weight: 800; color: #000000; border: none; background: transparent;"
    ));

    layout->addWidget(statusLabel);
    layout->addStretch();
    layout->addWidget(countLabel);

    return card;
}

// ============ 3. 学生列表与分段控制 ============
void AttendanceWidget::createStudentListCard()
{
    m_listCard = new QFrame();
    m_listCard->setStyleSheet(QString("QFrame { background: white; border-radius: 20px; border: 1px solid %1; }").arg(COL_BORDER));
    applyCardShadow(m_listCard, 12, 4);

    auto *layout = new QVBoxLayout(m_listCard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // List Header
    auto *header = new QFrame();
    header->setFixedHeight(50);
    header->setStyleSheet(QString("background: %1; border-top-left-radius: 20px; border-top-right-radius: 20px; border-bottom: 1px solid %2;")
                         .arg(COL_SEGMENT_BG, COL_BORDER));
    auto *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(32, 0, 32, 0);

    auto createHLbl = [&](const QString &txt, int w = -1) {
        auto *l = new QLabel(txt);
        l->setStyleSheet(QString("font-size: 12px; font-weight: 700; color: %1; text-transform: uppercase; letter-spacing: 0.5px;").arg(COL_TEXT_SUB));
        if(w > 0) l->setFixedWidth(w);
        return l;
    };

    hLayout->addWidget(createHLbl("学生信息", 200));
    hLayout->addWidget(createHLbl("出勤状态选项", -1), 1, Qt::AlignCenter);
    hLayout->addWidget(createHLbl("备注", 80), 0, Qt::AlignRight);
    layout->addWidget(header);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; } QScrollBar:vertical { width: 4px; background: transparent; } QScrollBar::handle:vertical { background: #D1D5DB; border-radius: 2px; }");

    m_listContainer = new QWidget();
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);

    m_scrollArea->setWidget(m_listContainer);
    layout->addWidget(m_scrollArea);

    m_mainLayout->addWidget(m_listCard, 1);
}

QWidget* AttendanceWidget::createStudentItem(int index, const QString &name, const QString &studentNo, AttendanceStatus status)
{
    auto *row = new QFrame();
    row->setFixedHeight(68);
    row->setStyleSheet(QString("QFrame { border-bottom: 1px solid %1; } QFrame:hover { background: %2; }").arg(COL_BORDER, "#F8FAFC"));

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(32, 0, 32, 0);
    layout->setSpacing(24);

    // 头像 (莫兰迪色)
    auto *avatar = new QLabel(name.left(1));
    avatar->setFixedSize(40, 40);
    avatar->setAlignment(Qt::AlignCenter);
    QString avatarCol = getMorandiColor(index);
    avatar->setStyleSheet(QString("background: %1; color: white; border-radius: 20px; font-size: 15px; font-weight: 700;").arg(avatarCol));

    // 信息 - 纯文字排版，移除所有边框背景
    auto *info = new QVBoxLayout();
    info->setSpacing(0);
    auto *nameL = new QLabel(name);
    nameL->setStyleSheet(QString("font-size: 15px; font-weight: 700; color: %1; background: transparent; border: none;").arg(COL_TEXT_MAIN));
    auto *noL = new QLabel(studentNo);
    noL->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent; border: none;").arg(COL_TEXT_SUB));
    info->addWidget(nameL);
    info->addWidget(noL);
    auto *infoW = new QWidget(); infoW->setLayout(info); infoW->setFixedWidth(140);
    infoW->setStyleSheet("background: transparent; border: none;");

    // Segmented Control (胶囊切换器)
    auto *segment = createStatusButtonGroup(index, status);

    // 备注按钮 - 变得更低调
    auto *remark = new QPushButton();
    remark->setFixedSize(32, 32);
    remark->setCursor(Qt::PointingHandCursor);
    remark->setStyleSheet("QPushButton { border: 1px solid #E5E7EB; border-radius: 8px; background: white; } QPushButton:hover { background: #F3F4F6; }");
    setButtonIcon(remark, ":/icons/resources/icons/edit.svg", COL_TEXT_SUB);

    connect(remark, &QPushButton::clicked, this, [=](){ onRemarkClicked(index); });

    layout->addWidget(avatar);
    layout->addWidget(infoW);
    layout->addWidget(segment, 1, Qt::AlignCenter);
    layout->addWidget(remark);

    return row;
}

QWidget* AttendanceWidget::createStatusButtonGroup(int studentIndex, AttendanceStatus currentStatus)
{
    auto *bg = new QFrame();
    bg->setFixedHeight(40);
    bg->setStyleSheet(QString("background: %1; border-radius: 10px; padding: 2px;").arg(COL_SEGMENT_BG));

    auto *layout = new QHBoxLayout(bg);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    auto *group = new QButtonGroup(this);
    QStringList lbls = {"出勤", "缺勤", "迟到", "请假", "早退"};
    QStringList cols = {COL_PRIMARY, COL_DANGER, COL_WARNING, COL_INFO, COL_PURPLE};

    for(int i=0; i<5; ++i) {
        auto *btn = new QPushButton(lbls[i]);
        btn->setCheckable(true);
        btn->setFixedHeight(36);
        btn->setCursor(Qt::PointingHandCursor);

        QString activeStyle = QString(
            "QPushButton:checked { background: %1; color: white; border-radius: 8px; font-weight: 700; font-size: 13px; }"
            "QPushButton { background: transparent; color: %2; border: none; padding: 0 16px; font-size: 13px; font-weight: 500; }"
        ).arg(cols[i], COL_TEXT_SUB);

        btn->setStyleSheet(activeStyle);
        if(i == (int)currentStatus) btn->setChecked(true);

        group->addButton(btn, i);
        layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, this, [=](){ onStatusButtonClicked(studentIndex, (AttendanceStatus)i); });
    }

    m_statusButtonGroups.append(group);
    return bg;
}

QString AttendanceWidget::getMorandiColor(int index)
{
    static const QStringList colors = {"#8E9775", "#E28E8E", "#9FB8AD", "#C38D9E", "#776B5D", "#B4A397", "#916BBF"};
    return colors[index % colors.size()];
}

// ============ 事件处理复用之前的逻辑 ============
void AttendanceWidget::onClassChanged(int index) { if(m_service) m_service->fetchStudentsByClass(index+1); }
void AttendanceWidget::onDateChanged(const QDate&) { onRefreshClicked(); }
void AttendanceWidget::onLessonChanged(int) { onRefreshClicked(); }
void AttendanceWidget::onRefreshClicked() { if(m_classCombo->currentIndex() >= 0 && m_service) m_service->fetchAttendanceByLesson(m_classCombo->currentIndex()+1, m_dateEdit->date(), m_lessonCombo->currentIndex()+1); }
void AttendanceWidget::onAllPresentClicked() { for(int i=0; i<m_records.size(); ++i){ m_records[i].setStatus(AttendanceStatus::Present); if(i < m_statusButtonGroups.size()){ auto *b = m_statusButtonGroups[i]->button(0); if(b) b->setChecked(true); } } updateStatistics(); }
void AttendanceWidget::onSaveClicked() { if(m_service && !m_records.isEmpty()) m_service->submitAttendance(m_records); }

void AttendanceWidget::onStudentsLoaded()
{
    if(!m_service) return;
    m_records.clear();
    qDeleteAll(m_statusButtonGroups); m_statusButtonGroups.clear();
    QLayoutItem *item;
    while((item = m_listLayout->takeAt(0))) { if(item->widget()) item->widget()->deleteLater(); delete item; }

    const auto &students = m_service->students();
    for(const Student &s : students) {
        AttendanceRecord r(s.id(), m_classCombo->currentIndex()+1, m_dateEdit->date(), m_lessonCombo->currentIndex()+1);
        r.setStatus(AttendanceStatus::Present); m_records.append(r);
    }
    loadStudentList(); updateStatistics(); m_saveBtn->setEnabled(true);
}

void AttendanceWidget::onAttendanceLoaded()
{
    if(!m_service) return;
    m_records = m_service->attendanceRecords();
    loadStudentList(); updateStatistics(); m_saveBtn->setEnabled(true);
}

void AttendanceWidget::loadStudentList()
{
    QLayoutItem *item;
    while((item = m_listLayout->takeAt(0))) { if(item->widget()) item->widget()->deleteLater(); delete item; }
    qDeleteAll(m_statusButtonGroups); m_statusButtonGroups.clear();

    const auto &students = m_service->students();
    for(int i=0; i<students.size() && i<m_records.size(); ++i) {
        m_listLayout->addWidget(createStudentItem(i, students[i].name(), students[i].studentNo(), m_records[i].status()));
    }
    m_listLayout->addStretch();
}

void AttendanceWidget::onStatusButtonClicked(int studentIndex, AttendanceStatus status) { if(studentIndex>=0 && studentIndex<m_records.size()){ m_records[studentIndex].setStatus(status); updateStatistics(); } }

void AttendanceWidget::onRemarkClicked(int studentIndex)
{
    if (studentIndex < 0 || studentIndex >= m_records.size()) return;

    bool ok;
    QString currentRemark = m_records[studentIndex].remark();
    QString text = QInputDialog::getText(this, "添加备注",
                                         QString("为学生 %1 添加考勤备注:").arg(m_service->students()[studentIndex].name()),
                                         QLineEdit::Normal,
                                         currentRemark, &ok);
    if (ok) {
        m_records[studentIndex].setRemark(text);
        qDebug() << "备注已更新:" << text;
    }
}

void AttendanceWidget::updateStatistics()
{
    int p=0, a=0, l=0, lv=0, e=0;
    for(const auto &r : m_records) {
        switch(r.status()){
            case AttendanceStatus::Present: p++; break;
            case AttendanceStatus::Absent: a++; break;
            case AttendanceStatus::Late: l++; break;
            case AttendanceStatus::Leave: lv++; break;
            case AttendanceStatus::EarlyLeave: e++; break;
        }
    }
    if(m_presentCountLabel) m_presentCountLabel->setText(QString::number(p));
    if(m_absentCountLabel) m_absentCountLabel->setText(QString::number(a));
    if(m_lateCountLabel) m_lateCountLabel->setText(QString::number(l));
    if(m_leaveCountLabel) m_leaveCountLabel->setText(QString::number(lv));
    if(m_earlyCountLabel) m_earlyCountLabel->setText(QString::number(e));
}

// ============ SVG 辅助 ============
QIcon AttendanceWidget::loadSvgIcon(const QString &path, const QString &color) {
    QFile file(path); if(!file.open(QIODevice::ReadOnly)) return QIcon();
    QString svg = QString::fromUtf8(file.readAll()); file.close();
    if(!color.isEmpty()) { svg.replace("currentColor", color); svg.replace("stroke=\"#000\"", QString("stroke=\"%1\"").arg(color)); }
    QSvgRenderer renderer(svg.toUtf8()); QPixmap pix(24, 24); pix.fill(Qt::transparent);
    QPainter p(&pix); renderer.render(&p); p.end(); return QIcon(pix);
}
void AttendanceWidget::setButtonIcon(QPushButton *btn, const QString &path, const QString &col) { btn->setIcon(loadSvgIcon(path, col)); btn->setIconSize(QSize(18, 18)); }
