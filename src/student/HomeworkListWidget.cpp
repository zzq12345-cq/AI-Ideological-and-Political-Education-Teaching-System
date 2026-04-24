#include "HomeworkListWidget.h"
#include "../shared/StyleConfig.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

HomeworkListWidget::HomeworkListWidget(const QString &classId, QWidget *parent)
    : QWidget(parent)
    , m_classId(classId)
{
    setupUI();
    refreshList();
}

void HomeworkListWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    // 标题行
    auto *headerRow = new QHBoxLayout();
    auto *title = new QLabel("作业列表");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));

    auto *createBtn = new QPushButton("+ 发布新作业");
    createBtn->setCursor(Qt::PointingHandCursor);
    createBtn->setFixedHeight(36);
    createBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px;"
        "  padding: 0 20px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(createBtn, &QPushButton::clicked, this, &HomeworkListWidget::createRequested);

    headerRow->addWidget(title);
    headerRow->addStretch();
    headerRow->addWidget(createBtn);
    mainLayout->addLayout(headerRow);

    // 滚动列表
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *container = new QWidget();
    m_listLayout = new QVBoxLayout(container);
    m_listLayout->setSpacing(12);
    m_listLayout->setContentsMargins(0, 0, 0, 0);

    scroll->setWidget(container);
    mainLayout->addWidget(scroll, 1);
}

void HomeworkListWidget::refreshList()
{
    // 清空
    while (m_listLayout->count()) {
        auto *item = m_listLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    connect(HomeworkManager::instance(), &HomeworkManager::assignmentsLoaded, this,
            [this](const QList<HomeworkManager::AssignmentInfo> &list) {
        // 清空旧内容
        while (m_listLayout->count()) {
            auto *item = m_listLayout->takeAt(0);
            delete item->widget();
            delete item;
        }

        if (list.isEmpty()) {
            auto *empty = new QLabel("暂无作业，点击右上角发布新作业");
            empty->setAlignment(Qt::AlignCenter);
            empty->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 40px;");
            m_listLayout->addWidget(empty);
        } else {
            for (const auto &a : list) {
                m_listLayout->addWidget(createAssignmentCard(a));
            }
        }
        m_listLayout->addStretch();
    }, Qt::SingleShotConnection);

    HomeworkManager::instance()->loadAssignments(m_classId);
}

QWidget* HomeworkListWidget::createAssignmentCard(const HomeworkManager::AssignmentInfo &info)
{
    auto *card = new QFrame();
    card->setObjectName("hwCard");
    card->setStyleSheet(QString(
        "#hwCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
        "#hwCard:hover { border-color: %3; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT, StyleConfig::PATRIOTIC_RED));

    // 不使用 QGraphicsDropShadowEffect，避免阴影渲染到子控件文字上

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(8);

    // 标题行
    auto *titleRow = new QHBoxLayout();
    auto *titleLabel = new QLabel(info.title);
    titleLabel->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1; background: transparent;")
        .arg(StyleConfig::TEXT_PRIMARY));

    auto *scoreLabel = new QLabel(QString("满分 %1").arg(info.totalScore));
    scoreLabel->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent;");

    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(scoreLabel);
    layout->addLayout(titleRow);

    // 描述
    if (!info.description.isEmpty()) {
        auto *descLabel = new QLabel(info.description.length() > 80
            ? info.description.left(80) + "..." : info.description);
        descLabel->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent;");
        descLabel->setWordWrap(true);
        layout->addWidget(descLabel);
    }

    // 底部行：截止时间 + 查看提交按钮
    auto *bottomRow = new QHBoxLayout();

    QString timeText = info.endTime.isValid()
        ? "截止: " + info.endTime.toString("MM-dd HH:mm") : "无截止时间";
    auto *timeLabel = new QLabel(timeText);
    timeLabel->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent;");

    auto *viewBtn = new QPushButton("查看提交");
    viewBtn->setCursor(Qt::PointingHandCursor);
    viewBtn->setFixedSize(80, 28);
    viewBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: 1px solid %2;"
        "  border-radius: 6px; font-size: 12px; font-weight: 600; }"
        "QPushButton:hover { background: #FFF5F5; }"
    ).arg(StyleConfig::PATRIOTIC_RED, StyleConfig::PATRIOTIC_RED));

    connect(viewBtn, &QPushButton::clicked, this, [this, info]() {
        emit viewSubmissions(info);
    });

    auto *deleteBtn = new QPushButton("删除");
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setFixedSize(60, 28);
    deleteBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #9CA3AF; border: 1px solid #E5E7EB;"
        "  border-radius: 6px; font-size: 12px; }"
        "QPushButton:hover { color: #EF4444; border-color: #FCA5A5; background: #FEF2F2; }"
    );
    connect(deleteBtn, &QPushButton::clicked, this, [this, info]() {
        auto ret = QMessageBox::question(this, "确认删除",
            QString("确定删除作业 \"%1\" 吗？").arg(info.title),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            HomeworkManager::instance()->deleteAssignment(info.id);
            connect(HomeworkManager::instance(), &HomeworkManager::assignmentDeleted, this,
                    [this](const QString &) { refreshList(); }, Qt::SingleShotConnection);
        }
    });

    bottomRow->addWidget(timeLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(deleteBtn);
    bottomRow->addWidget(viewBtn);
    layout->addLayout(bottomRow);

    return card;
}
