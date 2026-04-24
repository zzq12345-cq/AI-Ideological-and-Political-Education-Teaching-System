#include "HomeworkCreateWidget.h"
#include "HomeworkManager.h"
#include "../shared/StyleConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

HomeworkCreateWidget::HomeworkCreateWidget(const QString &classId, const QString &teacherEmail, QWidget *parent)
    : QWidget(parent)
    , m_classId(classId)
    , m_teacherEmail(teacherEmail)
{
    setupUI();
}

void HomeworkCreateWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    // 标题
    auto *headerRow = new QHBoxLayout();
    auto *title = new QLabel("发布新作业");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));
    headerRow->addWidget(title);
    headerRow->addStretch();
    mainLayout->addLayout(headerRow);

    // 表单卡片
    auto *card = new QFrame();
    card->setObjectName("hwCreateCard");
    card->setStyleSheet(QString(
        "#hwCreateCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    // 不使用 QGraphicsDropShadowEffect，避免阴影渲染到子控件文字上

    auto *formLayout = new QVBoxLayout(card);
    formLayout->setContentsMargins(24, 20, 24, 24);
    formLayout->setSpacing(20);

    QString labelStyle = QString("font-size: 13px; font-weight: 600; color: %1;").arg(StyleConfig::TEXT_PRIMARY);
    QString inputStyle = QString(
        "QLineEdit, QTextEdit {"
        "  background: %1; border: 1px solid %2; border-radius: 8px;"
        "  padding: 10px 14px; font-size: 14px; color: %3;"
        "}"
        "QLineEdit:focus, QTextEdit:focus { border-color: %4; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT, StyleConfig::TEXT_PRIMARY, StyleConfig::PATRIOTIC_RED);

    // 标题
    auto *titleLabel = new QLabel("作业标题");
    titleLabel->setStyleSheet(labelStyle);
    formLayout->addWidget(titleLabel);
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("请输入作业标题");
    m_titleEdit->setStyleSheet(inputStyle);
    m_titleEdit->setFixedHeight(42);
    formLayout->addWidget(m_titleEdit);

    // 描述
    auto *descLabel = new QLabel("作业要求");
    descLabel->setStyleSheet(labelStyle);
    formLayout->addWidget(descLabel);
    m_descEdit = new QTextEdit();
    m_descEdit->setPlaceholderText("描述作业要求（选填）");
    m_descEdit->setStyleSheet(inputStyle);
    m_descEdit->setFixedHeight(100);
    formLayout->addWidget(m_descEdit);

    // 总分 + 截止时间
    auto *settingsRow = new QHBoxLayout();
    settingsRow->setSpacing(20);

    auto *scoreCol = new QVBoxLayout();
    auto *scoreLabel = new QLabel("总分");
    scoreLabel->setStyleSheet(labelStyle);
    scoreCol->addWidget(scoreLabel);
    m_scoreSpin = new QSpinBox();
    m_scoreSpin->setRange(1, 1000);
    m_scoreSpin->setValue(100);
    m_scoreSpin->setSuffix(" 分");
    m_scoreSpin->setFixedHeight(36);
    m_scoreSpin->setStyleSheet(inputStyle + "QSpinBox { padding: 0 10px; }");
    scoreCol->addWidget(m_scoreSpin);
    settingsRow->addLayout(scoreCol);

    auto *timeCol = new QVBoxLayout();
    auto *timeLabel = new QLabel("截止时间");
    timeLabel->setStyleSheet(labelStyle);
    timeCol->addWidget(timeLabel);
    m_endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(7));
    m_endTimeEdit->setCalendarPopup(true);
    m_endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_endTimeEdit->setFixedHeight(36);
    m_endTimeEdit->setStyleSheet(inputStyle + "QDateTimeEdit { padding: 0 10px; }");
    timeCol->addWidget(m_endTimeEdit);
    settingsRow->addLayout(timeCol, 1);

    formLayout->addLayout(settingsRow);

    // 分隔线
    auto *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::BORDER_LIGHT));
    formLayout->addWidget(line);

    // 按钮
    auto *btnRow = new QHBoxLayout();
    auto *backBtn = new QPushButton("返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFixedSize(80, 40);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #6B7280; border: 1px solid #E5E7EB;"
        "  border-radius: 8px; font-size: 14px; }"
        "QPushButton:hover { background: #F9FAFB; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &HomeworkCreateWidget::backRequested);

    m_publishBtn = new QPushButton("发布作业");
    m_publishBtn->setCursor(Qt::PointingHandCursor);
    m_publishBtn->setFixedSize(120, 40);
    m_publishBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px;"
        "  font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(m_publishBtn, &QPushButton::clicked, this, &HomeworkCreateWidget::onPublishClicked);

    btnRow->addWidget(backBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_publishBtn);
    formLayout->addLayout(btnRow);

    mainLayout->addWidget(card);
    mainLayout->addStretch();
}

void HomeworkCreateWidget::onPublishClicked()
{
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        m_titleEdit->setStyleSheet(m_titleEdit->styleSheet() + "QLineEdit { border-color: #EF4444; }");
        return;
    }

    m_publishBtn->setEnabled(false);
    m_publishBtn->setText("发布中...");

    HomeworkManager::instance()->createAssignment(
        m_classId, m_teacherEmail, title,
        m_descEdit->toPlainText().trimmed(),
        m_scoreSpin->value(), m_endTimeEdit->dateTime()
    );

    connect(HomeworkManager::instance(), &HomeworkManager::assignmentCreated, this,
            [this](const HomeworkManager::AssignmentInfo &) {
        QMessageBox::information(this, "成功", "作业已发布！");
        emit created();
    }, Qt::SingleShotConnection);

    connect(HomeworkManager::instance(), &HomeworkManager::error, this,
            [this](const QString &msg) {
        m_publishBtn->setEnabled(true);
        m_publishBtn->setText("发布作业");
        QMessageBox::warning(this, "失败", msg);
    }, Qt::SingleShotConnection);
}
