#include "JoinClassDialog.h"
#include "../shared/StyleConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

JoinClassDialog::JoinClassDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("加入新班级");
    setFixedSize(400, 300);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 32, 32, 24);
    mainLayout->setSpacing(20);

    // 标题
    QLabel *titleLabel = new QLabel("加入新班级");
    titleLabel->setStyleSheet(QString(
        "font-size: 22px; font-weight: 700; color: %1;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *descLabel = new QLabel("输入老师提供的班级码");
    descLabel->setStyleSheet(QString(
        "font-size: 14px; color: %1;"
    ).arg(StyleConfig::TEXT_SECONDARY));
    descLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descLabel);

    // 输入框
    m_codeEdit = new QLineEdit();
    m_codeEdit->setPlaceholderText("输入班级码（如 SZ2024）");
    m_codeEdit->setFixedHeight(44);
    m_codeEdit->setStyleSheet(QString(
        "QLineEdit {"
        "  border: 1px solid %1;"
        "  border-radius: %2px;"
        "  padding: 0 16px;"
        "  font-size: 14px;"
        "  color: %3;"
        "}"
        "QLineEdit:focus {"
        "  border-color: %4;"
        "  border-width: 2px;"
        "}"
    ).arg(StyleConfig::BORDER_LIGHT)
     .arg(StyleConfig::RADIUS_S)
     .arg(StyleConfig::TEXT_PRIMARY)
     .arg(StyleConfig::PATRIOTIC_RED));

    mainLayout->addWidget(m_codeEdit);

    mainLayout->addStretch();

    // 按钮区
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_joinBtn = new QPushButton("加入");
    m_joinBtn->setFixedHeight(44);
    m_joinBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: %2px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: %3; }"
    ).arg(StyleConfig::PATRIOTIC_RED)
     .arg(StyleConfig::RADIUS_S)
     .arg(StyleConfig::PATRIOTIC_RED_DARK));

    m_cancelBtn = new QPushButton("取消");
    m_cancelBtn->setFixedHeight(44);
    m_cancelBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: %4px;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: %3; }"
    ).arg(StyleConfig::PATRIOTIC_RED_LIGHT)
     .arg(StyleConfig::PATRIOTIC_RED)
     .arg(StyleConfig::BORDER_LIGHT)
     .arg(StyleConfig::RADIUS_S));

    btnLayout->addWidget(m_joinBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_joinBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_codeEdit, &QLineEdit::returnPressed, this, &QDialog::accept);
}

QString JoinClassDialog::classCode() const
{
    return m_codeEdit->text().trimmed().toUpper();
}
