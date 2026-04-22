#include "AttendanceWidget.h"
#include "../../student/ClassManager.h"
#include "../../student/AttendanceManager.h"
#include "../../utils/LayoutUtils.h"
#include "../models/AttendanceSummary.h"
#include "../../analytics/models/Student.h"
#include "../../settings/UserSettingsManager.h"
#include "../../auth/supabase/supabaseconfig.h"
#include "../../utils/NetworkRequestFactory.h"
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

// ============ 高级 UI 设计规范 ============
static const QString COL_BG = "#F3F4F6";
static const QString COL_CARD = "#FFFFFF";
static const QString COL_PRIMARY = "#10B981";
static const QString COL_DANGER = "#EF4444";
static const QString COL_WARNING = "#F59E0B";
static const QString COL_INFO = "#3B82F6";
static const QString COL_PURPLE = "#8B5CF6";

static const QString COL_TEXT_MAIN = "#111827";
static const QString COL_TEXT_SUB = "#6B7280";
static const QString COL_BORDER = "#E5E7EB";
static const QString COL_SEGMENT_BG = "#F3F4F6";

AttendanceWidget::AttendanceWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();

    // 连接 ClassManager 加载真实班级
    connect(ClassManager::instance(), &ClassManager::classesLoaded, this,
            [this](const QList<ClassInfo> &classes) {
        m_classCombo->blockSignals(true);
        m_classCombo->clear();
        for (const auto &cls : classes) {
            m_classCombo->addItem(cls.name, cls.id);
        }
        m_classCombo->blockSignals(false);
        if (!classes.isEmpty()) {
            loadAttendanceForCurrentSelection();
        }
    });

    // 连接 AttendanceManager 加载 sessions 列表
    connect(AttendanceManager::instance(), &AttendanceManager::sessionsLoaded, this,
            [this](const QList<AttendanceManager::SessionInfo> &sessions) {
        m_sessionCombo->blockSignals(true);
        m_sessionCombo->clear();
        if (sessions.isEmpty()) {
            m_sessionCombo->addItem("当日无考勤记录");
        } else {
            int targetIdx = -1;
            for (int i = 0; i < sessions.size(); ++i) {
                const auto &s = sessions[i];
                QString label = s.name.isEmpty()
                    ? QString("签到码 %1").arg(s.code)
                    : s.name;
                if (s.status == "active") label += " · 进行中";
                m_sessionCombo->addItem(label, s.id);
                if (s.id == m_targetSessionId) targetIdx = i;
            }
            // 优先选中目标 session，否则选中第一个
            int selectIdx = (targetIdx >= 0) ? targetIdx : 0;
            m_sessionCombo->setCurrentIndex(selectIdx);
            m_currentSessionId = sessions[selectIdx].id;
            AttendanceManager::instance()->loadSessionRecords(m_currentSessionId);
            m_targetSessionId.clear();
        }
        m_sessionCombo->blockSignals(false);
    });

    // 连接 ClassManager 获取学号
    connect(ClassManager::instance(), &ClassManager::membersLoaded, this,
            [this](const QString &classId, const QList<ClassManager::MemberInfo> &members) {
        if (classId != m_currentClassId) return;
        // 构建 email → 学号映射
        m_emailToStudentNo.clear();
        for (const auto &m : members) {
            if (!m.number.isEmpty()) {
                m_emailToStudentNo[m.email] = m.number;
            }
        }
        // 更新学生学号并重建列表
        bool changed = false;
        for (int i = 0; i < m_students.size() && i < m_studentEmails.size(); ++i) {
            QString no = m_emailToStudentNo.value(m_studentEmails[i], "无学号信息");
            if (m_students[i].studentNo() != no) {
                m_students[i].setStudentNo(no);
                changed = true;
            }
        }
        if (changed && !m_records.isEmpty()) {
            // 重建列表 UI
            qDeleteAll(m_statusButtonGroups);
            m_statusButtonGroups.clear();
            LayoutUtils::clearLayout(m_listLayout);
            for (int i = 0; i < m_students.size(); ++i) {
                m_listLayout->addWidget(createStudentItem(i, m_students[i].name(),
                    m_students[i].studentNo(), m_records[i].status()));
            }
            m_listLayout->addStretch();
        }
    });

    // 连接 AttendanceManager 加载记录
    connect(AttendanceManager::instance(), &AttendanceManager::recordsLoaded, this,
            [this](const QString &sessionId, const QList<AttendanceManager::RecordInfo> &records) {
        if (sessionId != m_currentSessionId) return;
        m_records.clear();
        m_studentEmails.clear();
        qDeleteAll(m_statusButtonGroups);
        m_statusButtonGroups.clear();
        LayoutUtils::clearLayout(m_listLayout);

        // 构建 Student + AttendanceRecord 列表
        m_students.clear();
        m_originalStatuses.clear();
        int idx = 0;
        for (const auto &r : records) {
            Student s;
            s.setId(idx);
            s.setName(r.studentName.isEmpty() ? r.studentEmail.split('@')[0] : r.studentName);
            s.setStudentNo("无学号信息");
            m_students.append(s);
            m_studentEmails.append(r.studentEmail);

            AttendanceRecord rec;
            rec.setId(idx);
            rec.setStudentId(idx);
            AttendanceStatus status = AttendanceStatusHelper::fromString(r.status);
            rec.setStatus(status);
            m_records.append(rec);
            m_originalStatuses.append(status);

            m_listLayout->addWidget(createStudentItem(idx, s.name(), s.studentNo(), rec.status()));
            idx++;
        }
        m_listLayout->addStretch();

        // 存储 record id 用于更新
        m_recordIds.clear();
        for (const auto &r : records) {
            m_recordIds.append(r.id);
        }

        updateStatistics();
        updateSaveButtonState();

        // 加载班级成员获取学号
        QString classId = m_classCombo->currentData().toString();
        if (!classId.isEmpty()) {
            m_currentClassId = classId;
            ClassManager::instance()->loadClassMembers(classId);
        }
    });

    // 监听状态更新 — 批量保存后更新 originalStatuses
    connect(AttendanceManager::instance(), &AttendanceManager::recordStatusUpdated, this,
            [this](const QString &recordId, const QString &status) {
        Q_UNUSED(status)
        // 找到对应索引，更新 originalStatuses
        int idx = m_recordIds.indexOf(recordId);
        if (idx >= 0 && idx < m_originalStatuses.size()) {
            m_originalStatuses[idx] = m_records[idx].status();
        }
        updateSaveButtonState();
    });

    // 首次加载：如果班级已缓存，直接填充；否则用邮箱触发加载
    if (!ClassManager::instance()->cachedClasses().isEmpty()) {
        m_classCombo->blockSignals(true);
        m_classCombo->clear();
        for (const auto &cls : ClassManager::instance()->cachedClasses()) {
            m_classCombo->addItem(cls.name, cls.id);
        }
        m_classCombo->blockSignals(false);
        loadAttendanceForCurrentSelection();
    } else {
        QString email = UserSettingsManager::instance()->email();
        if (!email.isEmpty()) {
            ClassManager::instance()->loadTeacherClasses(email);
        }
    }
}

AttendanceWidget::~AttendanceWidget()
{
    qDeleteAll(m_statusButtonGroups);
}

void AttendanceWidget::setAttendanceService(AttendanceService *service)
{
    Q_UNUSED(service)
    // 不再使用旧的 mock service，数据由 AttendanceManager 直接提供
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
    setupConnections();
}

void AttendanceWidget::setupStyles()
{
    setStyleSheet(QString("QWidget#AttendanceWidget { background-color: %1; }").arg(COL_BG));
}

void AttendanceWidget::setupConnections()
{
    connect(m_classCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AttendanceWidget::onClassChanged);
    connect(m_dateEdit, &QDateEdit::dateChanged, this, &AttendanceWidget::onDateChanged);
    connect(m_sessionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AttendanceWidget::onSessionChanged);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AttendanceWidget::onRefreshClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &AttendanceWidget::onResetChanges);
    connect(m_saveChangesBtn, &QPushButton::clicked, this, &AttendanceWidget::onSaveChanges);
}

void AttendanceWidget::applyCardShadow(QWidget *widget, qreal blur, qreal offset)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blur);
    shadow->setOffset(0, offset);
    shadow->setColor(QColor(0, 0, 0, 20));
    widget->setGraphicsEffect(shadow);
}

// ============ 1. 头部筛选 ============
void AttendanceWidget::createHeaderCard()
{
    auto *header = new QWidget();
    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *titleSection = new QVBoxLayout();
    titleSection->setSpacing(2);
    auto *title = new QLabel("考勤管理");
    title->setStyleSheet(QString("font-size: 26px; font-weight: 800; color: %1;").arg(COL_TEXT_MAIN));
    auto *sub = new QLabel("班级考勤记录与数据统计");
    sub->setStyleSheet(QString("font-size: 13px; color: %1;").arg(COL_TEXT_SUB));
    titleSection->addWidget(title);
    titleSection->addWidget(sub);

    auto *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    auto *toolbar = new QFrame();
    toolbar->setStyleSheet(QString("QFrame { background-color: white; border: 1px solid %1; border-radius: 10px; }").arg(COL_BORDER));
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(12, 6, 12, 6);
    tbLayout->setSpacing(12);

    QString comboStyle = QString("QComboBox { border: none; background: transparent; font-size: 13px; font-weight: 600; color: %1; min-width: 100px; } QComboBox::drop-down { border: none; }").arg(COL_TEXT_MAIN);

    m_classCombo = new QComboBox();
    m_classCombo->addItem("请选择班级");
    m_classCombo->setStyleSheet(comboStyle);

    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setFixedWidth(110);
    m_dateEdit->setStyleSheet(QString("QDateEdit { border: none; background: transparent; font-size: 13px; font-weight: 600; color: %1; }").arg(COL_TEXT_MAIN));

    m_sessionCombo = new QComboBox();
    m_sessionCombo->addItem("请先选择班级和日期");
    m_sessionCombo->setStyleSheet(comboStyle);
    m_sessionCombo->setMinimumWidth(280);

    m_refreshBtn = new QPushButton();
    m_refreshBtn->setFixedSize(28, 28);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setText("↻");
    m_refreshBtn->setStyleSheet(
        "QPushButton { border: none; border-radius: 6px; font-size: 16px; color: #6B7280; }"
        "QPushButton:hover { background: #E5E7EB; }"
    );

    tbLayout->addWidget(m_classCombo);
    tbLayout->addWidget(m_dateEdit);
    tbLayout->addWidget(m_sessionCombo);
    tbLayout->addWidget(m_refreshBtn);

    actionLayout->addWidget(toolbar);

    layout->addLayout(titleSection);
    layout->addStretch();
    layout->addLayout(actionLayout);

    m_mainLayout->addWidget(header);
}

// ============ 2. 统计卡片 ============
void AttendanceWidget::createSummaryCard()
{
    auto *dashboard = new QWidget();
    auto *layout = new QHBoxLayout(dashboard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    layout->addWidget(createStatCard("已出勤", m_presentCountLabel, "#059669", ""));
    layout->addWidget(createStatCard("缺勤", m_absentCountLabel, "#EF4444", ""));
    layout->addWidget(createStatCard("迟到", m_lateCountLabel, "#F59E0B", ""));
    layout->addWidget(createStatCard("请假", m_leaveCountLabel, "#3B82F6", ""));
    layout->addWidget(createStatCard("早退", m_earlyCountLabel, "#8B5CF6", ""));

    layout->addStretch();

    m_mainLayout->addWidget(dashboard);
}

QWidget* AttendanceWidget::createStatCard(const QString &label, QLabel* &countLabel, const QString &color, const QString &iconPath)
{
    Q_UNUSED(iconPath)

    auto *card = new QFrame();
    card->setFixedWidth(140);
    card->setFixedHeight(56);
    card->setStyleSheet(QString("QFrame { background: white; border-radius: 8px; border: 1px solid %1; }").arg(COL_BORDER));

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(16, 0, 16, 0);

    auto *statusLabel = new QLabel(label);
    statusLabel->setStyleSheet(QString(
        "font-size: 14px; font-weight: 600; color: %1; border: none; background: transparent;"
    ).arg(color));

    countLabel = new QLabel("0");
    countLabel->setStyleSheet(
        "font-size: 22px; font-weight: 800; color: #000000; border: none; background: transparent;"
    );

    layout->addWidget(statusLabel);
    layout->addStretch();
    layout->addWidget(countLabel);

    return card;
}

// ============ 3. 学生列表 ============
void AttendanceWidget::createStudentListCard()
{
    m_listCard = new QFrame();
    m_listCard->setStyleSheet(QString("QFrame { background: white; border-radius: 20px; border: 1px solid %1; }").arg(COL_BORDER));
    applyCardShadow(m_listCard, 12, 4);

    auto *layout = new QVBoxLayout(m_listCard);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

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
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #D1D5DB; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #9CA3AF; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    m_listContainer = new QWidget();
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);

    m_scrollArea->setWidget(m_listContainer);
    layout->addWidget(m_scrollArea);

    // 底部操作栏：重置 + 保存修改
    auto *bottomBar = new QFrame();
    bottomBar->setFixedHeight(60);
    bottomBar->setStyleSheet(QString(
        "background: %1; border-bottom-left-radius: 20px; border-bottom-right-radius: 20px; "
        "border-top: 1px solid %2;"
    ).arg(COL_SEGMENT_BG, COL_BORDER));
    auto *bLayout = new QHBoxLayout(bottomBar);
    bLayout->setContentsMargins(32, 0, 32, 0);

    bLayout->addStretch();

    m_resetBtn = new QPushButton("重置");
    m_resetBtn->setFixedSize(100, 36);
    m_resetBtn->setCursor(Qt::PointingHandCursor);
    m_resetBtn->setStyleSheet(QString(
        "QPushButton { background: white; color: %1; border: 1px solid %2; border-radius: 8px;"
        "  font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #F9FAFB; border-color: #9CA3AF; }"
    ).arg(COL_TEXT_MAIN, COL_BORDER));

    m_saveChangesBtn = new QPushButton("保存修改");
    m_saveChangesBtn->setFixedSize(120, 36);
    m_saveChangesBtn->setCursor(Qt::PointingHandCursor);
    m_saveChangesBtn->setEnabled(false);
    m_saveChangesBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px;"
        "  font-size: 13px; font-weight: 700; }"
        "QPushButton:hover { background: #374151; }"
        "QPushButton:disabled { background: #D1D5DB; color: #9CA3AF; }"
    ).arg(COL_TEXT_MAIN));

    bLayout->addWidget(m_resetBtn);
    bLayout->addSpacing(12);
    bLayout->addWidget(m_saveChangesBtn);

    layout->addWidget(bottomBar);

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

    auto *avatar = new QLabel(name.left(1));
    avatar->setFixedSize(40, 40);
    avatar->setAlignment(Qt::AlignCenter);
    QString avatarCol = getMorandiColor(index);
    avatar->setStyleSheet(QString("background: %1; color: white; border-radius: 20px; font-size: 15px; font-weight: 700;").arg(avatarCol));

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

    auto *segment = createStatusButtonGroup(index, status);

    auto *remark = new QPushButton();
    remark->setFixedSize(32, 32);
    remark->setCursor(Qt::PointingHandCursor);
    remark->setText("✎");
    remark->setStyleSheet(
        "QPushButton { border: 1px solid #E5E7EB; border-radius: 8px; background: white;"
        "  font-size: 14px; color: #6B7280; }"
        "QPushButton:hover { background: #F3F4F6; }"
    );
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

    for(int i = 0; i < 5; ++i) {
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

// ============ 数据加载 ============
void AttendanceWidget::loadAttendanceForCurrentSelection()
{
    QString classId = m_classCombo->currentData().toString();
    if (classId.isEmpty()) return;

    m_currentClassId = classId;
    QDate selectedDate = m_dateEdit->date();
    AttendanceManager::instance()->loadSessionsByClassAndDate(classId, selectedDate);
}

void AttendanceWidget::loadSessionResults(const QString &sessionId, const QString &classId)
{
    m_currentSessionId = sessionId;

    // 先直接加载记录
    AttendanceManager::instance()->loadSessionRecords(sessionId);

    if (!classId.isEmpty()) {
        m_currentClassId = classId;

        // 设置班级选择器到目标班级
        m_classCombo->blockSignals(true);
        for (int i = 0; i < m_classCombo->count(); ++i) {
            if (m_classCombo->itemData(i).toString() == classId) {
                m_classCombo->setCurrentIndex(i);
                break;
            }
        }
        m_classCombo->blockSignals(false);

        // 设置日期为今天
        m_dateEdit->blockSignals(true);
        m_dateEdit->setDate(QDate::currentDate());
        m_dateEdit->blockSignals(false);

        // 记住目标 session
        m_targetSessionId = sessionId;

        // 后台加载 session 列表填充选择器
        AttendanceManager::instance()->loadSessionsByClassAndDate(classId, QDate::currentDate());
    }
}

// ============ 事件处理 ============
void AttendanceWidget::onClassChanged(int index)
{
    Q_UNUSED(index)
    m_currentSessionId.clear();
    m_sessionCombo->clear();
    m_sessionCombo->addItem("加载中...");
    loadAttendanceForCurrentSelection();
}

void AttendanceWidget::onDateChanged(const QDate &)
{
    m_currentSessionId.clear();
    m_sessionCombo->clear();
    m_sessionCombo->addItem("加载中...");
    loadAttendanceForCurrentSelection();
}

void AttendanceWidget::onSessionChanged(int index)
{
    if (index < 0) return;
    QString sessionId = m_sessionCombo->currentData().toString();
    if (sessionId.isEmpty()) return;
    m_currentSessionId = sessionId;
    AttendanceManager::instance()->loadSessionRecords(sessionId);
}

void AttendanceWidget::onRefreshClicked()
{
    if (!m_currentSessionId.isEmpty()) {
        AttendanceManager::instance()->loadSessionRecords(m_currentSessionId);
    } else {
        loadAttendanceForCurrentSelection();
    }
}

void AttendanceWidget::updateSaveButtonState()
{
    if (!m_saveChangesBtn) return;
    bool hasChanges = false;
    for (int i = 0; i < m_records.size() && i < m_originalStatuses.size(); ++i) {
        if (m_records[i].status() != m_originalStatuses[i]) {
            hasChanges = true;
            break;
        }
    }
    m_saveChangesBtn->setEnabled(hasChanges);
}

void AttendanceWidget::onSaveChanges()
{
    int changedCount = 0;
    for (int i = 0; i < m_records.size() && i < m_recordIds.size() && i < m_originalStatuses.size(); ++i) {
        if (m_records[i].status() != m_originalStatuses[i]) {
            AttendanceManager::instance()->updateRecordStatus(
                m_recordIds[i],
                AttendanceStatusHelper::toString(m_records[i].status()));
            changedCount++;
        }
    }
    if (changedCount == 0) return;

    m_saveChangesBtn->setText("保存中...");
    m_saveChangesBtn->setEnabled(false);

    // 延迟显示保存成功（等网络请求完成）
    QTimer::singleShot(1000, this, [this]() {
        // 更新 originalStatuses 与当前状态同步
        m_originalStatuses.clear();
        for (const auto &r : m_records) {
            m_originalStatuses.append(r.status());
        }
        m_saveChangesBtn->setText("✓ 已保存");
        QTimer::singleShot(1500, this, [this]() {
            m_saveChangesBtn->setText("保存修改");
            updateSaveButtonState();
        });
    });
}

void AttendanceWidget::onResetChanges()
{
    // 重置为原始状态，重新加载记录
    if (!m_currentSessionId.isEmpty()) {
        AttendanceManager::instance()->loadSessionRecords(m_currentSessionId);
    }
}

void AttendanceWidget::onStatusButtonClicked(int studentIndex, AttendanceStatus status)
{
    if (studentIndex >= 0 && studentIndex < m_records.size()) {
        m_records[studentIndex].setStatus(status);
        updateStatistics();
        updateSaveButtonState();
    }
}

void AttendanceWidget::onRemarkClicked(int studentIndex)
{
    if (studentIndex < 0 || studentIndex >= m_records.size()) return;

    bool ok;
    QString currentRemark = m_records[studentIndex].remark();
    QString text = QInputDialog::getText(this,
                                         "添加备注",
                                         QString("为学生 %1 添加考勤备注:").arg(m_students[studentIndex].name()),
                                         QLineEdit::Normal,
                                         currentRemark, &ok);
    if (ok) {
        m_records[studentIndex].setRemark(text);
    }
}

void AttendanceWidget::updateStatistics()
{
    int p = 0, a = 0, l = 0, lv = 0, e = 0;
    for (const auto &r : m_records) {
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

QIcon AttendanceWidget::loadSvgIcon(const QString &path, const QString &color) {
    QFile file(path); if(!file.open(QIODevice::ReadOnly)) return QIcon();
    QString svg = QString::fromUtf8(file.readAll()); file.close();
    if(!color.isEmpty()) { svg.replace("currentColor", color); svg.replace("stroke=\"#000\"", QString("stroke=\"%1\"").arg(color)); }
    QSvgRenderer renderer(svg.toUtf8()); QPixmap pix(24, 24); pix.fill(Qt::transparent);
    QPainter p(&pix); renderer.render(&p); p.end(); return QIcon(pix);
}

void AttendanceWidget::setButtonIcon(QPushButton *btn, const QString &path, const QString &col) {
    btn->setIcon(loadSvgIcon(path, col)); btn->setIconSize(QSize(18, 18));
}
