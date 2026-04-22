#include "ClassSettingsWidget.h"
#include "ClassManager.h"
#include "../shared/StyleConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

ClassSettingsWidget::ClassSettingsWidget(const ClassInfo &info, QWidget *parent)
    : QWidget(parent)
    , m_classInfo(info)
    , m_isPublic(info.isPublic)
{
    setupUI();
}

void ClassSettingsWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    // 标题
    auto *headerRow = new QHBoxLayout();
    auto *title = new QLabel("课程设置");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));
    headerRow->addWidget(title);
    headerRow->addStretch();
    mainLayout->addLayout(headerRow);

    // 表单卡片
    auto *card = new QFrame();
    card->setObjectName("settingsCard");
    card->setStyleSheet(QString(
        "#settingsCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(12);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

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

    // 班级名称
    auto *nameLabel = new QLabel("班级名称");
    nameLabel->setStyleSheet(labelStyle);
    m_nameEdit = new QLineEdit(m_classInfo.name);
    m_nameEdit->setPlaceholderText("请输入班级名称");
    m_nameEdit->setStyleSheet(inputStyle);
    m_nameEdit->setFixedHeight(42);

    formLayout->addWidget(nameLabel);
    formLayout->addWidget(m_nameEdit);

    // 班级描述
    auto *descLabel = new QLabel("班级描述");
    descLabel->setStyleSheet(labelStyle);
    m_descEdit = new QTextEdit();
    m_descEdit->setPlaceholderText("添加班级描述（选填）");
    m_descEdit->setStyleSheet(inputStyle);
    m_descEdit->setFixedHeight(80);
    m_descEdit->setText(m_classInfo.description);

    formLayout->addWidget(descLabel);
    formLayout->addWidget(m_descEdit);

    // 公开设置
    auto *publicRow = new QHBoxLayout();
    auto *publicLabel = new QLabel("公开班级");
    publicLabel->setStyleSheet(labelStyle);
    auto *publicHint = new QLabel("公开后学生可通过搜索找到并加入");
    publicHint->setStyleSheet("font-size: 12px; color: #9CA3AF;");
    publicRow->addWidget(publicLabel);
    publicRow->addSpacing(8);
    publicRow->addWidget(publicHint);
    publicRow->addStretch();
    publicRow->addWidget(createToggle(m_isPublic));

    formLayout->addLayout(publicRow);

    // 分隔线
    auto *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::BORDER_LIGHT));
    formLayout->addWidget(line);

    // 底部按钮
    auto *btnRow = new QHBoxLayout();

    auto *saveBtn = new QPushButton("保存修改");
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setFixedSize(120, 40);
    saveBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px;"
        "  font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(saveBtn, &QPushButton::clicked, this, &ClassSettingsWidget::onSaveClicked);

    auto *deleteBtn = new QPushButton("删除班级");
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setFixedSize(100, 40);
    deleteBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #EF4444; border: 1px solid #FCA5A5;"
        "  border-radius: 8px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #FEF2F2; }"
    );
    connect(deleteBtn, &QPushButton::clicked, this, &ClassSettingsWidget::onDeleteClicked);

    btnRow->addWidget(saveBtn);
    btnRow->addStretch();
    btnRow->addWidget(deleteBtn);

    formLayout->addLayout(btnRow);

    mainLayout->addWidget(card);
    mainLayout->addStretch();
}

QWidget* ClassSettingsWidget::createToggle(bool isPublic)
{
    m_publicToggle = new QPushButton();
    m_publicToggle->setFixedSize(48, 26);
    m_publicToggle->setCursor(Qt::PointingHandCursor);

    auto updateStyle = [this]() {
        if (m_isPublic) {
            m_publicToggle->setStyleSheet(
                "QPushButton { background: #10B981; border: none; border-radius: 13px; }"
                "QPushButton::after { background: white; }"
            );
            m_publicToggle->setText("开");
            m_publicToggle->setStyleSheet(
                "QPushButton { background: #10B981; border: none; border-radius: 13px;"
                "  color: white; font-size: 11px; font-weight: 700; }"
            );
        } else {
            m_publicToggle->setText("关");
            m_publicToggle->setStyleSheet(
                "QPushButton { background: #D1D5DB; border: none; border-radius: 13px;"
                "  color: white; font-size: 11px; font-weight: 700; }"
            );
        }
    };

    updateStyle();

    connect(m_publicToggle, &QPushButton::clicked, this, [this, updateStyle]() {
        m_isPublic = !m_isPublic;
        updateStyle();
    });

    return m_publicToggle;
}

void ClassSettingsWidget::onSaveClicked()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        m_nameEdit->setStyleSheet(m_nameEdit->styleSheet() + "QLineEdit { border-color: #EF4444; }");
        return;
    }

    ClassManager::instance()->updateClass(
        m_classInfo.id, name, m_descEdit->toPlainText().trimmed(), m_isPublic);
}

void ClassSettingsWidget::onDeleteClicked()
{
    auto ret = QMessageBox::warning(this, "确认删除",
        QString("确定要删除班级 \"%1\" 吗？\n删除后所有成员和考勤记录将被清除，此操作不可撤销。")
            .arg(m_classInfo.name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        emit deleteRequested(m_classInfo.id);
    }
}
