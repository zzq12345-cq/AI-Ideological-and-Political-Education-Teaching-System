#include "ModernDialogHelper.h"
#include <QDialog>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QGraphicsDropShadowEffect>

// ── 内部工具：创建带阴影的无边框弹窗骨架 ──
static QDialog* createShell(QWidget *parent, int w, int h, QFrame **outCard)
{
    auto *dlg = new QDialog(parent);
    dlg->setFixedSize(w, h);
    dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg->setAttribute(Qt::WA_TranslucentBackground);

    auto *card = new QFrame(dlg);
    card->setObjectName("mdCard");
    card->setStyleSheet("#mdCard { background: #FFFFFF; border-radius: 20px; }");
    card->setGeometry(16, 16, w - 32, h - 32);

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(15, 23, 42, 50));
    card->setGraphicsEffect(shadow);

    if (outCard) *outCard = card;
    return dlg;
}

// ── 确认弹窗 ──
bool ModernDialogHelper::confirm(QWidget *parent, const QString &title, const QString &message)
{
    QFrame *card = nullptr;
    auto *dlg = createShell(parent, 480, 260, &card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(14);

    auto *titleRow = new QHBoxLayout();
    auto *icon = new QLabel("⚠️");
    icon->setStyleSheet("font-size: 22px; background: transparent;");
    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");
    titleRow->addWidget(icon);
    titleRow->addSpacing(6);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();

    auto *msgLabel = new QLabel(message);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.5;");

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #F1F5F9; color: #64748B; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #E2E8F0; color: #475569; }");
    QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    auto *okBtn = new QPushButton("确定");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(100, 40);
    okBtn->setStyleSheet(
        "QPushButton { background: #EF4444; color: white; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #DC2626; }"
        "QPushButton:pressed { background: #B91C1C; }");
    QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);

    layout->addLayout(titleRow);
    layout->addWidget(msgLabel);
    layout->addStretch();
    layout->addLayout(btnRow);

    bool result = (dlg->exec() == QDialog::Accepted);
    dlg->deleteLater();
    return result;
}

// ── 信息弹窗 ──
void ModernDialogHelper::info(QWidget *parent, const QString &title, const QString &message)
{
    QFrame *card = nullptr;
    auto *dlg = createShell(parent, 440, 230, &card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(14);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");

    auto *msgLabel = new QLabel(message);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.5;");

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    auto *okBtn = new QPushButton("知道了");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(100, 40);
    okBtn->setStyleSheet(
        "QPushButton { background: #E53935; color: white; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #EF5350; }"
        "QPushButton:pressed { background: #C62828; }");
    QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    btnRow->addWidget(okBtn);

    layout->addWidget(titleLabel);
    layout->addWidget(msgLabel);
    layout->addStretch();
    layout->addLayout(btnRow);

    dlg->exec();
    dlg->deleteLater();
}

// ── 警告弹窗 ──
void ModernDialogHelper::warning(QWidget *parent, const QString &title, const QString &message)
{
    QFrame *card = nullptr;
    auto *dlg = createShell(parent, 460, 240, &card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(14);

    auto *titleRow = new QHBoxLayout();
    auto *icon = new QLabel("⚠️");
    icon->setStyleSheet("font-size: 22px; background: transparent;");
    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");
    titleRow->addWidget(icon);
    titleRow->addSpacing(6);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();

    auto *msgLabel = new QLabel(message);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.5;");

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    auto *okBtn = new QPushButton("知道了");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(100, 40);
    okBtn->setStyleSheet(
        "QPushButton { background: #F59E0B; color: white; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #D97706; }"
        "QPushButton:pressed { background: #B45309; }");
    QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    btnRow->addWidget(okBtn);

    layout->addLayout(titleRow);
    layout->addWidget(msgLabel);
    layout->addStretch();
    layout->addLayout(btnRow);

    dlg->exec();
    dlg->deleteLater();
}

// ── 输入弹窗 ──
QString ModernDialogHelper::input(QWidget *parent, const QString &title, const QString &hint,
                                   const QString &defaultText)
{
    QFrame *card = nullptr;
    auto *dlg = createShell(parent, 520, 280, &card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(16);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");

    auto *hintLabel = new QLabel(hint);
    hintLabel->setStyleSheet("font-size: 14px; color: #64748B; font-weight: 400;");

    auto *inputEdit = new QLineEdit();
    if (!defaultText.isEmpty()) {
        inputEdit->setText(defaultText);
        inputEdit->selectAll();
    }
    inputEdit->setStyleSheet(
        "QLineEdit { padding: 10px 14px; border: 1.5px solid #E2E8F0;"
        "  border-radius: 12px; font-size: 14px; color: #1E293B;"
        "  background: #F8FAFC; }"
        "QLineEdit:focus { border-color: #6366F1; background: #FFFFFF; }");

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #F1F5F9; color: #64748B; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #E2E8F0; color: #475569; }");
    QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    auto *okBtn = new QPushButton("确定");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(100, 40);
    okBtn->setStyleSheet(
        "QPushButton { background: #E53935; color: white; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #EF5350; }"
        "QPushButton:pressed { background: #C62828; }");
    QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QObject::connect(inputEdit, &QLineEdit::returnPressed, dlg, &QDialog::accept);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);

    layout->addWidget(titleLabel);
    layout->addWidget(hintLabel);
    layout->addWidget(inputEdit);
    layout->addStretch();
    layout->addLayout(btnRow);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        result = inputEdit->text().trimmed();
    }
    dlg->deleteLater();
    return result;
}

// ── 整数输入弹窗 ──
int ModernDialogHelper::getInt(QWidget *parent, const QString &title, const QString &label,
                                int value, int min, int max, int step, bool *ok)
{
    QFrame *card = nullptr;
    auto *dlg = createShell(parent, 420, 260, &card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(16);

    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");

    auto *labelRow = new QHBoxLayout();
    auto *lbl = new QLabel(label);
    lbl->setStyleSheet("font-size: 14px; color: #475569;");

    auto *spinBox = new QSpinBox();
    spinBox->setValue(value);
    spinBox->setMinimum(min);
    spinBox->setMaximum(max);
    spinBox->setSingleStep(step);
    spinBox->setFixedWidth(120);
    spinBox->setStyleSheet(
        "QSpinBox { padding: 8px 12px; border: 1.5px solid #E2E8F0;"
        "  border-radius: 10px; font-size: 15px; color: #1E293B;"
        "  background: #F8FAFC; }"
        "QSpinBox:focus { border-color: #E53935; background: #FFFFFF; }"
        "QSpinBox::up-button, QSpinBox::down-button { background: #F1F5F9; border-radius: 4px; width: 24px; }"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover { background: #E2E8F0; }"
    );

    labelRow->addWidget(lbl);
    labelRow->addStretch();
    labelRow->addWidget(spinBox);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #F1F5F9; color: #64748B; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #E2E8F0; color: #475569; }");
    QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    auto *okBtn = new QPushButton("确定");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(100, 40);
    okBtn->setStyleSheet(
        "QPushButton { background: #E53935; color: white; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #EF5350; }"
        "QPushButton:pressed { background: #C62828; }");
    QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);

    layout->addWidget(titleLabel);
    layout->addLayout(labelRow);
    layout->addStretch();
    layout->addLayout(btnRow);

    int result = value;
    if (dlg->exec() == QDialog::Accepted) {
        result = spinBox->value();
        if (ok) *ok = true;
    } else {
        if (ok) *ok = false;
    }
    dlg->deleteLater();
    return result;
}
