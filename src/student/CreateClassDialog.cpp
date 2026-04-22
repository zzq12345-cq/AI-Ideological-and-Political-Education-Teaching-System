#include "CreateClassDialog.h"
#include "../shared/StyleConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

CreateClassDialog::CreateClassDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("创建班级");
    setFixedSize(420, 220);
    setStyleSheet(QString("QDialog { background: white; }"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 28, 28, 24);
    layout->setSpacing(20);

    // 标题
    QLabel *titleLabel = new QLabel("创建新班级");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 700; color: %1;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    QLabel *descLabel = new QLabel("输入课程名称，系统将自动生成班级码");
    descLabel->setStyleSheet("font-size: 13px; color: #6B7280;");

    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);

    // 输入框
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("例如：思想道德与法治");
    m_nameEdit->setFixedHeight(42);
    m_nameEdit->setStyleSheet(
        "QLineEdit {"
        "  padding: 0 14px; border: 1px solid #E5E7EB; border-radius: 8px;"
        "  font-size: 14px; color: #1A1A1A; background: #F9FAFB;"
        "}"
        "QLineEdit:focus { border: 2px solid #E53935; background: white; }"
    );
    layout->addWidget(m_nameEdit);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton("取消");
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(
        "QPushButton { padding: 10px 24px; border: 1px solid #D1D5DB; border-radius: 8px;"
        "  background: white; color: #6B7280; font-size: 14px; }"
        "QPushButton:hover { background: #F3F4F6; }"
    );
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_createBtn = new QPushButton("创建");
    m_createBtn->setCursor(Qt::PointingHandCursor);
    m_createBtn->setStyleSheet(
        "QPushButton { padding: 10px 32px; border: none; border-radius: 8px;"
        "  background: #E53935; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #D32F2F; }"
    );
    connect(m_createBtn, &QPushButton::clicked, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) return;
        accept();
    });

    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_createBtn);
    layout->addLayout(btnLayout);
}

QString CreateClassDialog::className() const
{
    return m_nameEdit->text().trimmed();
}
