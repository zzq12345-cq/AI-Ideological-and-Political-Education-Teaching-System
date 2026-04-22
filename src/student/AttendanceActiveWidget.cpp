#include "AttendanceActiveWidget.h"
#include "../shared/StyleConfig.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QScrollArea>

AttendanceActiveWidget::AttendanceActiveWidget(
    const AttendanceManager::SessionInfo &session, int totalMembers, QWidget *parent)
    : QWidget(parent)
    , m_session(session)
    , m_totalMembers(totalMembers)
{
    setupUI();

    // 监听记录更新
    auto *mgr = AttendanceManager::instance();
    connect(mgr, &AttendanceManager::recordsLoaded, this,
            [this](const QString &sessionId, const QList<AttendanceManager::RecordInfo> &records) {
        if (sessionId != m_session.id) return;
        updateSignedList(records);

        // 统计已签到人数
        int signedCount = 0;
        for (const auto &r : records) {
            if (r.status == "present") signedCount++;
        }
        m_progressLabel->setText(QString("已签到 %1 / %2 人").arg(signedCount).arg(m_totalMembers));

        // 进度条
        int pct = m_totalMembers > 0 ? signedCount * 100 / m_totalMembers : 0;
        int filled = pct / 5;  // 20格
        QString bar = QString(pct > 0 ? "" : "").repeated(filled) +
                      QString("").repeated(20 - filled);
        m_progressBar->setText(QString("[%1] %2%").arg(bar).arg(pct));

        // 全部签到完成 → 自动结束
        if (signedCount >= m_totalMembers && m_totalMembers > 0) {
            m_pollTimer->stop();
            AttendanceManager::instance()->endAttendance(m_session.id, m_session.classId);
            emit attendanceFinished(m_session.id);
        }
    });

    connect(mgr, &AttendanceManager::attendanceEnded, this,
            [this](const QString &sessionId, const QString &) {
        if (sessionId == m_session.id) {
            m_pollTimer->stop();
            emit attendanceFinished(sessionId);
        }
    });

    // 首次加载
    mgr->loadSessionRecords(m_session.id);

    // 每3秒轮询
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        AttendanceManager::instance()->loadSessionRecords(m_session.id);
    });
    m_pollTimer->start(3000);
}

void AttendanceActiveWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    // 签到码卡片（红色渐变）
    QFrame *codeCard = new QFrame();
    codeCard->setObjectName("attCodeCard");
    codeCard->setStyleSheet(QString(
        "#attCodeCard {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 #EF5350);"
        "  border-radius: 16px;"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(codeCard);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(229, 57, 53, 50));
    shadow->setOffset(0, 4);
    codeCard->setGraphicsEffect(shadow);

    QVBoxLayout *codeLayout = new QVBoxLayout(codeCard);
    codeLayout->setContentsMargins(28, 24, 28, 24);
    codeLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel("考勤签到码");
    titleLabel->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 14px; background: transparent;");

    m_codeLabel = new QLabel(m_session.code);
    m_codeLabel->setStyleSheet(
        "color: white; font-size: 42px; font-weight: 700; letter-spacing: 10px; background: transparent;");

    QLabel *hintLabel = new QLabel("请告知学生此签到码完成签到");
    hintLabel->setStyleSheet("color: rgba(255,255,255,0.6); font-size: 12px; background: transparent;");

    codeLayout->addWidget(titleLabel);
    codeLayout->addWidget(m_codeLabel);
    codeLayout->addWidget(hintLabel);

    mainLayout->addWidget(codeCard);

    // 进度区
    QFrame *progressCard = new QFrame();
    progressCard->setObjectName("attProgressCard");
    progressCard->setStyleSheet(QString(
        "#attProgressCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *progressLayout = new QVBoxLayout(progressCard);
    progressLayout->setContentsMargins(20, 16, 20, 16);
    progressLayout->setSpacing(8);

    m_progressLabel = new QLabel("已签到 0 / 0 人");
    m_progressLabel->setStyleSheet(QString(
        "font-size: 15px; font-weight: 600; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    m_progressBar = new QLabel("[                    ] 0%");
    m_progressBar->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent;");

    progressLayout->addWidget(m_progressLabel);
    progressLayout->addWidget(m_progressBar);

    mainLayout->addWidget(progressCard);

    // 已签到列表
    QFrame *listCard = new QFrame();
    listCard->setObjectName("attListCard");
    listCard->setStyleSheet(QString(
        "#attListCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    m_signedLayout = new QVBoxLayout(listCard);
    m_signedLayout->setContentsMargins(20, 16, 20, 16);
    m_signedLayout->setSpacing(6);

    QLabel *listTitle = new QLabel("签到详情");
    listTitle->setStyleSheet(QString(
        "font-size: 14px; font-weight: 600; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    m_signedLayout->addWidget(listTitle);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::SEPARATOR));
    m_signedLayout->addWidget(line);

    QLabel *placeholder = new QLabel("等待学生签到...");
    placeholder->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 16px; background: transparent;");
    m_signedLayout->addWidget(placeholder);
    m_signedLayout->addStretch();

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setWidget(listCard);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    mainLayout->addWidget(scroll, 1);

    // 结束考勤按钮
    QPushButton *endBtn = new QPushButton("结束考勤");
    endBtn->setCursor(Qt::PointingHandCursor);
    endBtn->setFixedHeight(44);
    endBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: white; border: none; border-radius: 10px;"
        "  font-size: 15px; font-weight: 600;"
        "}"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(endBtn, &QPushButton::clicked, this, [this]() {
        m_pollTimer->stop();
        AttendanceManager::instance()->endAttendance(m_session.id, m_session.classId);
    });

    mainLayout->addWidget(endBtn);
}

void AttendanceActiveWidget::updateSignedList(const QList<AttendanceManager::RecordInfo> &records)
{
    // 清除标题和分隔线之后的内容
    while (m_signedLayout->count() > 2) {
        QLayoutItem *item = m_signedLayout->takeAt(m_signedLayout->count() - 1);
        delete item->widget();
        delete item;
    }

    for (const auto &r : records) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(8);

        QString displayName = r.studentName.isEmpty()
            ? r.studentEmail.split('@')[0] : r.studentName;

        QLabel *nameLabel = new QLabel(displayName);
        nameLabel->setStyleSheet(QString(
            "font-size: 13px; font-weight: 500; color: %1; background: transparent;"
        ).arg(StyleConfig::TEXT_PRIMARY));

        QString statusText;
        QString statusColor;
        QString statusBg;
        if (r.status == "present") {
            statusText = "已签到";
            statusColor = "#16A34A";
            statusBg = "#DCFCE7";
        } else {
            statusText = "未签到";
            statusColor = "#9CA3AF";
            statusBg = "#F3F4F6";
        }

        QLabel *statusLabel = new QLabel(statusText);
        statusLabel->setStyleSheet(QString(
            "font-size: 11px; padding: 2px 10px; border-radius: 8px;"
            "background: %1; color: %2;"
        ).arg(statusBg, statusColor));

        row->addWidget(nameLabel);
        row->addStretch();
        row->addWidget(statusLabel);
        m_signedLayout->addLayout(row);
    }

    m_signedLayout->addStretch();
}
