#include "AttendanceSignInDialog.h"
#include "../shared/StyleConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AttendanceSignInDialog::AttendanceSignInDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("签到");
    setFixedSize(380, 240);
    setStyleSheet(QString("QDialog { background: %1; }").arg(StyleConfig::BG_APP));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 28, 32, 24);
    layout->setSpacing(16);

    QLabel *titleLabel = new QLabel("考勤签到");
    titleLabel->setStyleSheet(QString(
        "font-size: 20px; font-weight: 700; color: %1;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    QLabel *descLabel = new QLabel("输入教师提供的签到码完成签到");
    descLabel->setStyleSheet("font-size: 13px; color: #9CA3AF;");

    m_codeEdit = new QLineEdit();
    m_codeEdit->setPlaceholderText("请输入6位签到码");
    m_codeEdit->setMaxLength(6);
    m_codeEdit->setStyleSheet(
        "padding: 12px 14px; border: 1px solid #E5E7EB; border-radius: 8px;"
        "font-size: 16px; letter-spacing: 4px;"
    );

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("取消");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { padding: 10px 24px; border: 1px solid #E5E7EB; border-radius: 8px;"
        "  background: white; color: #6B7280; font-size: 14px; }"
        "QPushButton:hover { background: #F9FAFB; }"
    );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *signBtn = new QPushButton("签到");
    signBtn->setCursor(Qt::PointingHandCursor);
    signBtn->setStyleSheet(QString(
        "QPushButton { padding: 10px 36px; border: none; border-radius: 8px;"
        "  background: %1; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(signBtn, &QPushButton::clicked, this, [this]() {
        if (!m_codeEdit->text().trimmed().isEmpty()) {
            accept();
        }
    });

    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(signBtn);

    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addWidget(m_codeEdit);
    layout->addLayout(btnRow);
}

QString AttendanceSignInDialog::signCode() const
{
    return m_codeEdit->text().trimmed().toUpper();
}
