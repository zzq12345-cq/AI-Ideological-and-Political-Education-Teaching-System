#include "AttendanceResultWidget.h"
#include "../shared/StyleConfig.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QComboBox>
#include <QScrollArea>

AttendanceResultWidget::AttendanceResultWidget(const QString &sessionId, QWidget *parent)
    : QWidget(parent)
    , m_sessionId(sessionId)
{
    setupUI();

    connect(AttendanceManager::instance(), &AttendanceManager::recordsLoaded, this,
            [this](const QString &sessionId, const QList<AttendanceManager::RecordInfo> &records) {
        if (sessionId != m_sessionId) return;
        m_records = records;
        updateResultList(records);
    });

    connect(AttendanceManager::instance(), &AttendanceManager::recordStatusUpdated, this,
            [this](const QString &, const QString &) {
        AttendanceManager::instance()->loadSessionRecords(m_sessionId);
    });

    AttendanceManager::instance()->loadSessionRecords(m_sessionId);
}

void AttendanceResultWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    // 顶部：返回 + 标题
    QHBoxLayout *headerRow = new QHBoxLayout();

    QPushButton *backBtn = new QPushButton("< 返回班级详情");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #6B7280;"
        "  font-size: 14px; padding: 4px 8px; }"
        "QPushButton:hover { color: #E53935; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &AttendanceResultWidget::backToDetail);

    QLabel *titleLabel = new QLabel("考勤结果");
    titleLabel->setStyleSheet(QString(
        "font-size: 20px; font-weight: 700; color: %1;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    m_summaryLabel = new QLabel("加载中...");
    m_summaryLabel->setStyleSheet("font-size: 13px; color: #9CA3AF;");

    headerRow->addWidget(backBtn);
    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(m_summaryLabel);
    mainLayout->addLayout(headerRow);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::BORDER_LIGHT));
    mainLayout->addWidget(line);

    // 列表区
    QFrame *listCard = new QFrame();
    listCard->setObjectName("attResultCard");
    listCard->setStyleSheet(QString(
        "#attResultCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    m_listLayout = new QVBoxLayout(listCard);
    m_listLayout->setContentsMargins(20, 16, 20, 16);
    m_listLayout->setSpacing(8);

    // 列头
    QHBoxLayout *colHeader = new QHBoxLayout();
    QLabel *colName = new QLabel("学生");
    colName->setStyleSheet(QString("font-size: 12px; font-weight: 600; color: %1; background: transparent;")
        .arg(StyleConfig::TEXT_SECONDARY));
    QLabel *colStatus = new QLabel("状态");
    colStatus->setStyleSheet(colName->styleSheet());
    colStatus->setFixedWidth(100);
    colHeader->addWidget(colName);
    colHeader->addStretch();
    colHeader->addWidget(colStatus);
    m_listLayout->addLayout(colHeader);

    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::SEPARATOR));
    m_listLayout->addWidget(line2);

    QLabel *placeholder = new QLabel("加载中...");
    placeholder->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 24px; background: transparent;");
    placeholder->setAlignment(Qt::AlignCenter);
    m_listLayout->addWidget(placeholder);
    m_listLayout->addStretch();

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setWidget(listCard);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    mainLayout->addWidget(scroll, 1);
}

void AttendanceResultWidget::updateResultList(const QList<AttendanceManager::RecordInfo> &records)
{
    // 清除 colHeader + line 之后的内容
    while (m_listLayout->count() > 2) {
        QLayoutItem *item = m_listLayout->takeAt(m_listLayout->count() - 1);
        delete item->widget();
        delete item;
    }

    int present = 0, absent = 0, late = 0, excused = 0;
    for (const auto &r : records) {
        if (r.status == "present") present++;
        else if (r.status == "late") late++;
        else if (r.status == "excused") excused++;
        else absent++;
    }
    m_summaryLabel->setText(
        QString("出勤 %1 | 迟到 %2 | 请假 %3 | 缺勤 %4").arg(present).arg(late).arg(excused).arg(absent));

    for (const auto &r : records) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(8);

        QString displayName = r.studentName.isEmpty()
            ? r.studentEmail.split('@')[0] : r.studentName;

        QLabel *nameLabel = new QLabel(displayName);
        nameLabel->setStyleSheet(QString(
            "font-size: 13px; font-weight: 500; color: %1; background: transparent;"
        ).arg(StyleConfig::TEXT_PRIMARY));

        // 状态下拉框
        QComboBox *statusCombo = new QComboBox();
        statusCombo->addItems({"present", "late", "excused", "absent"});
        statusCombo->setCurrentText(r.status);
        statusCombo->setFixedWidth(100);
        statusCombo->setStyleSheet(
            "QComboBox { padding: 4px 8px; border: 1px solid #E5E7EB; border-radius: 6px;"
            "  font-size: 12px; background: white; }"
            "QComboBox::drop-down { border: none; }"
            "QComboBox QAbstractItemView { selection-background-color: #E5E7EB; selection-color: #111827; }"
        );

        // 中文映射
        int idx = 0;
        if (r.status == "present") idx = 0;
        else if (r.status == "late") idx = 1;
        else if (r.status == "excused") idx = 2;
        else idx = 3;
        statusCombo->setCurrentIndex(idx);

        QString recordId = r.id;
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, recordId, statusCombo](int index) {
            QStringList statuses = {"present", "late", "excused", "absent"};
            if (index >= 0 && index < statuses.size()) {
                AttendanceManager::instance()->updateRecordStatus(recordId, statuses[index]);
            }
        });

        row->addWidget(nameLabel);
        row->addStretch();
        row->addWidget(statusCombo);
        m_listLayout->addLayout(row);
    }

    m_listLayout->addStretch();
}
