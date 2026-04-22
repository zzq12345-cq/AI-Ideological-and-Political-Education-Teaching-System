#include "HomeworkSubmissionsWidget.h"
#include "HomeworkManager.h"
#include "../shared/StyleConfig.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>
#include <QInputDialog>
#include <QSpinBox>
#include <QTextEdit>
#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>

HomeworkSubmissionsWidget::HomeworkSubmissionsWidget(
    const HomeworkManager::AssignmentInfo &assignment,
    const QList<ClassManager::MemberInfo> &members,
    QWidget *parent)
    : QWidget(parent)
    , m_assignment(assignment)
    , m_members(members)
{
    setupUI();
    loadSubmissions();
}

void HomeworkSubmissionsWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    // 标题
    auto *headerRow = new QHBoxLayout();
    auto *backBtn = new QPushButton("< 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #6B7280;"
        "  font-size: 14px; padding: 4px 8px; }"
        "QPushButton:hover { color: #E53935; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &HomeworkSubmissionsWidget::backRequested);

    auto *title = new QLabel(m_assignment.title + " - 提交情况");
    title->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));

    headerRow->addWidget(backBtn);
    headerRow->addWidget(title);
    headerRow->addStretch();
    mainLayout->addLayout(headerRow);

    // 列表头
    auto *listCard = new QFrame();
    listCard->setObjectName("subListCard");
    listCard->setStyleSheet(QString(
        "#subListCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    auto *shadow = new QGraphicsDropShadowEffect(listCard);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 12));
    shadow->setOffset(0, 2);
    listCard->setGraphicsEffect(shadow);

    auto *cardLayout = new QVBoxLayout(listCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // 表头
    auto *colHeader = new QFrame();
    colHeader->setFixedHeight(44);
    colHeader->setStyleSheet(QString("background: %1; border-top-left-radius: 12px; border-top-right-radius: 12px; border-bottom: 1px solid %2;")
        .arg("#F8FAFC", StyleConfig::BORDER_LIGHT));
    auto *colLayout = new QHBoxLayout(colHeader);
    colLayout->setContentsMargins(20, 0, 20, 0);

    auto mkLbl = [](const QString &t, int w = -1) {
        auto *l = new QLabel(t);
        l->setStyleSheet("font-size: 12px; font-weight: 600; color: #6B7280;");
        if (w > 0) l->setFixedWidth(w);
        return l;
    };
    colLayout->addWidget(mkLbl("学生", 140));
    colLayout->addWidget(mkLbl("状态", 80));
    colLayout->addWidget(mkLbl("提交时间", 130));
    colLayout->addWidget(mkLbl("得分", 60));
    colLayout->addStretch();
    colLayout->addWidget(mkLbl("操作", 80));
    cardLayout->addWidget(colHeader);

    // 滚动区
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *container = new QWidget();
    m_listLayout = new QVBoxLayout(container);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);

    scroll->setWidget(container);
    cardLayout->addWidget(scroll);

    mainLayout->addWidget(listCard, 1);
}

void HomeworkSubmissionsWidget::loadSubmissions()
{
    connect(HomeworkManager::instance(), &HomeworkManager::submissionsLoaded, this,
            [this](const QString &assignmentId, const QList<HomeworkManager::SubmissionInfo> &submissions) {
        if (assignmentId != m_assignment.id) return;

        // 清空
        while (m_listLayout->count()) {
            auto *item = m_listLayout->takeAt(0);
            delete item->widget();
            delete item;
        }

        // 建立邮箱→提交的映射
        QMap<QString, HomeworkManager::SubmissionInfo> subMap;
        for (const auto &s : submissions) {
            subMap[s.studentEmail] = s;
        }

        // 遍历所有成员
        for (const auto &m : m_members) {
            const auto *sub = subMap.contains(m.email) ? &subMap[m.email] : nullptr;
            m_listLayout->addWidget(createStudentRow(
                m.name.isEmpty() ? m.email.split('@')[0] : m.name, m.email, sub));
        }
        m_listLayout->addStretch();
    }, Qt::SingleShotConnection);

    HomeworkManager::instance()->loadSubmissions(m_assignment.id);
}

QWidget* HomeworkSubmissionsWidget::createStudentRow(const QString &name, const QString &email,
                                                      const HomeworkManager::SubmissionInfo *submission)
{
    Q_UNUSED(email)
    auto *row = new QFrame();
    row->setFixedHeight(52);
    row->setStyleSheet(QString("QFrame { border-bottom: 1px solid %1; } QFrame:hover { background: #F8FAFC; }")
        .arg(StyleConfig::BORDER_LIGHT));

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(20, 0, 20, 0);

    // 学生名
    auto *nameLabel = new QLabel(name);
    nameLabel->setFixedWidth(140);
    nameLabel->setStyleSheet(QString("font-size: 14px; font-weight: 500; color: %1; background: transparent;")
        .arg(StyleConfig::TEXT_PRIMARY));

    // 状态
    auto *statusLabel = new QLabel();
    statusLabel->setFixedWidth(80);
    statusLabel->setAlignment(Qt::AlignCenter);
    if (!submission) {
        statusLabel->setText("未提交");
        statusLabel->setStyleSheet("font-size: 12px; color: #9CA3AF; background: #F3F4F6; border-radius: 4px; padding: 2px 8px;");
    } else if (submission->status == 1) {
        statusLabel->setText("待批改");
        statusLabel->setStyleSheet("font-size: 12px; color: #D97706; background: #FEF3C7; border-radius: 4px; padding: 2px 8px;");
    } else {
        statusLabel->setText("已批改");
        statusLabel->setStyleSheet("font-size: 12px; color: #059669; background: #D1FAE5; border-radius: 4px; padding: 2px 8px;");
    }

    // 提交时间
    auto *timeLabel = new QLabel(submission ? submission->submitTime.toString("MM-dd HH:mm") : "-");
    timeLabel->setFixedWidth(130);
    timeLabel->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent;");

    // 得分
    auto *scoreLabel = new QLabel(submission && submission->score >= 0
        ? QString::number(submission->score) : "-");
    scoreLabel->setFixedWidth(60);
    scoreLabel->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1; background: transparent;")
        .arg(StyleConfig::TEXT_PRIMARY));

    layout->addWidget(nameLabel);
    layout->addWidget(statusLabel);
    layout->addWidget(timeLabel);
    layout->addWidget(scoreLabel);
    layout->addStretch();

    // 操作按钮
    if (submission) {
        auto *gradeBtn = new QPushButton(submission->status == 2 ? "查看" : "批改");
        gradeBtn->setCursor(Qt::PointingHandCursor);
        gradeBtn->setFixedSize(60, 28);
        gradeBtn->setStyleSheet(QString(
            "QPushButton { background: transparent; color: %1; border: 1px solid %1;"
            "  border-radius: 6px; font-size: 12px; font-weight: 600; }"
            "QPushButton:hover { background: #FFF5F5; }"
        ).arg(StyleConfig::PATRIOTIC_RED));
        connect(gradeBtn, &QPushButton::clicked, this, [this, sub = *submission]() {
            showGradeDialog(sub);
        });
        layout->addWidget(gradeBtn);
    } else {
        layout->addWidget(new QLabel(""));
    }

    return row;
}

void HomeworkSubmissionsWidget::showGradeDialog(const HomeworkManager::SubmissionInfo &submission)
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("批改作业 - " + submission.studentName);
    dialog->setMinimumWidth(400);
    dialog->setStyleSheet("QDialog { background: white; }");

    auto *layout = new QVBoxLayout(dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 20, 24, 24);

    // 学生名
    auto *nameLabel = new QLabel("学生: " + submission.studentName);
    nameLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #1A1A1A;");
    layout->addWidget(nameLabel);

    // 提交内容
    auto *contentTitle = new QLabel("提交内容:");
    contentTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #6B7280;");
    layout->addWidget(contentTitle);

    auto *contentLabel = new QLabel(submission.content.isEmpty() ? "(无文字内容)" : submission.content);
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet("font-size: 14px; color: #1A1A1A; padding: 12px; background: #F9FAFB; border-radius: 8px;");
    layout->addWidget(contentLabel);

    // 附件文件列表
    if (!submission.fileUrl.isEmpty()) {
        QStringList urls = submission.fileUrl.split(",", Qt::SkipEmptyParts);
        if (!urls.isEmpty()) {
            auto *fileTitle = new QLabel("附件文件:");
            fileTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #6B7280;");
            layout->addWidget(fileTitle);

            for (const QString &url : urls) {
                // 从 URL 提取文件名
                QString fileName = url.split("/").last();
                // 尝试还原可读名称（去掉时间戳前缀）
                int underscoreIdx = fileName.indexOf("_");
                if (underscoreIdx > 0 && underscoreIdx < 20) {
                    QString possibleName = fileName.mid(underscoreIdx + 1);
                    if (!possibleName.isEmpty()) fileName = possibleName;
                }

                auto *fileRow = new QHBoxLayout();
                fileRow->setSpacing(8);
                auto *fileIcon = new QLabel("📄");
                fileIcon->setStyleSheet("font-size: 16px; background: transparent; border: none;");
                auto *fileNameLabel = new QLabel(fileName);
                fileNameLabel->setStyleSheet("font-size: 13px; color: #374151; background: transparent; border: none;");
                fileNameLabel->setToolTip(url);
                auto *dlBtn = new QPushButton("下载");
                dlBtn->setCursor(Qt::PointingHandCursor);
                dlBtn->setFixedHeight(24);
                dlBtn->setStyleSheet(
                    "QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px;"
                    "  color: #6B7280; font-size: 12px; padding: 2px 10px; }"
                    "QPushButton:hover { border-color: #E53935; color: #E53935; }");
                QString dlUrl = url;
                connect(dlBtn, &QPushButton::clicked, dlBtn, [dlUrl]() {
                    QDesktopServices::openUrl(QUrl(dlUrl));
                });
                fileRow->addWidget(fileIcon);
                fileRow->addWidget(fileNameLabel, 1);
                fileRow->addWidget(dlBtn);
                layout->addLayout(fileRow);
            }
        }
    }

    // 分数
    auto *scoreRow = new QHBoxLayout();
    auto *scoreLbl = new QLabel("得分:");
    scoreLbl->setStyleSheet("font-size: 13px; font-weight: 600;");
    scoreRow->addWidget(scoreLbl);
    auto *scoreSpin = new QSpinBox();
    scoreSpin->setRange(0, m_assignment.totalScore);
    scoreSpin->setSuffix(" 分");
    if (submission.score >= 0) scoreSpin->setValue(submission.score);
    scoreSpin->setFixedHeight(32);
    scoreRow->addWidget(scoreSpin);
    auto *totalLbl = new QLabel(QString("/ %1").arg(m_assignment.totalScore));
    totalLbl->setStyleSheet("font-size: 13px; color: #9CA3AF;");
    scoreRow->addWidget(totalLbl);
    scoreRow->addStretch();
    layout->addLayout(scoreRow);

    // 反馈
    auto *fbLabel = new QLabel("反馈:");
    fbLabel->setStyleSheet("font-size: 13px; font-weight: 600;");
    layout->addWidget(fbLabel);
    auto *feedbackEdit = new QTextEdit();
    feedbackEdit->setPlaceholderText("输入批改反馈（选填）");
    feedbackEdit->setText(submission.feedback);
    feedbackEdit->setFixedHeight(80);
    feedbackEdit->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 8px; font-size: 13px;");
    layout->addWidget(feedbackEdit);

    // 按钮
    auto *btnRow = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setFixedHeight(36);
    cancelBtn->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; color: #6B7280; padding: 0 20px;");
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    auto *submitBtn = new QPushButton("提交批改");
    submitBtn->setFixedHeight(36);
    submitBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px;"
        "  padding: 0 20px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(submitBtn, &QPushButton::clicked, this, [this, dialog, submission, scoreSpin, feedbackEdit]() {
        connect(HomeworkManager::instance(), &HomeworkManager::submissionGraded, this,
            [this](const QString &submissionId) {
            Q_UNUSED(submissionId)
            loadSubmissions();
        }, Qt::SingleShotConnection);
        HomeworkManager::instance()->gradeSubmission(
            submission.id, scoreSpin->value(), feedbackEdit->toPlainText().trimmed(), false);
        dialog->accept();
    });

    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(submitBtn);
    layout->addLayout(btnRow);

    dialog->exec();
    dialog->deleteLater();
}
