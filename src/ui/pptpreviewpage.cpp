#include "pptpreviewpage.h"

#include "macosquicklookpreview.h"

#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

PPTPreviewPage::PPTPreviewPage(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 24, 32, 24);
    mainLayout->setSpacing(16);

    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    m_backButton = new QPushButton(QStringLiteral("返回"), this);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setFixedHeight(40);
    m_backButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; color: #C00000; border: 1px solid #C00000; border-radius: 8px; padding: 0 16px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: rgba(192,0,0,0.06); }"
    ));

    m_downloadButton = new QPushButton(QStringLiteral("下载PPT"), this);
    m_downloadButton->setCursor(Qt::PointingHandCursor);
    m_downloadButton->setFixedHeight(40);
    m_downloadButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #C00000; color: white; border: none; border-radius: 8px; padding: 0 18px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #A00000; }"
        "QPushButton:disabled { background: #D1D5DB; color: #9CA3AF; }"
    ));

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(4);

    m_titleLabel = new QLabel(QStringLiteral("PPT 预览"), this);
    m_titleLabel->setStyleSheet(QStringLiteral("color: #111827; font-size: 24px; font-weight: 700;"));

    m_hintLabel = new QLabel(QStringLiteral("生成完成后会在这里直接预览当前 PPT。"), this);
    m_hintLabel->setStyleSheet(QStringLiteral("color: #6B7280; font-size: 14px;"));

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(m_hintLabel);

    headerLayout->addWidget(m_backButton, 0, Qt::AlignTop);
    headerLayout->addLayout(titleLayout, 1);
    headerLayout->addWidget(m_downloadButton, 0, Qt::AlignTop);

    auto *previewCard = new QFrame(this);
    previewCard->setStyleSheet(QStringLiteral(
        "QFrame { background: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 16px; }"
    ));

    auto *previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(20, 20, 20, 20);
    previewLayout->setSpacing(12);

    m_fileLabel = new QLabel(this);
    m_fileLabel->setStyleSheet(QStringLiteral("color: #6B7280; font-size: 13px;"));
    m_fileLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_previewWidget = new MacOSQuickLookPreview(previewCard);

    previewLayout->addWidget(m_fileLabel);
    previewLayout->addWidget(m_previewWidget, 1);

    // 修改建议输入区
    auto *suggestionBar = new QFrame(this);
    suggestionBar->setFixedHeight(56);
    suggestionBar->setStyleSheet(QStringLiteral(
        "QFrame { background: #F8F9FA; border: 1px solid #E5E7EB; border-radius: 12px; }"
    ));

    auto *suggestionLayout = new QHBoxLayout(suggestionBar);
    suggestionLayout->setContentsMargins(12, 8, 8, 8);
    suggestionLayout->setSpacing(8);

    auto *suggestionIcon = new QLabel(QStringLiteral("\xF0\x9F\x92\xA1"), this); // 💡
    suggestionIcon->setStyleSheet(QStringLiteral("font-size: 18px; border: none; background: transparent;"));
    suggestionIcon->setFixedWidth(24);

    m_suggestionInput = new QLineEdit(this);
    m_suggestionInput->setPlaceholderText(QStringLiteral("输入修改建议，如「第3页补充案例」「配色换蓝色系」..."));
    m_suggestionInput->setStyleSheet(QStringLiteral(
        "QLineEdit { border: none; background: transparent; font-size: 14px; color: #111827; padding: 4px 0; }"
        "QLineEdit::placeholder { color: #9CA3AF; }"
    ));

    m_sendSuggestionBtn = new QPushButton(QStringLiteral("提交修改"), this);
    m_sendSuggestionBtn->setCursor(Qt::PointingHandCursor);
    m_sendSuggestionBtn->setFixedSize(90, 36);
    m_sendSuggestionBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #C00000; color: white; border: none; border-radius: 8px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #A00000; }"
        "QPushButton:disabled { background: #D1D5DB; color: #9CA3AF; }"
    ));

    suggestionLayout->addWidget(suggestionIcon);
    suggestionLayout->addWidget(m_suggestionInput, 1);
    suggestionLayout->addWidget(m_sendSuggestionBtn);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(previewCard, 1);
    mainLayout->addWidget(suggestionBar);

    connect(m_backButton, &QPushButton::clicked, this, &PPTPreviewPage::backRequested);
    connect(m_downloadButton, &QPushButton::clicked, this, &PPTPreviewPage::downloadRequested);
    connect(m_sendSuggestionBtn, &QPushButton::clicked, this, &PPTPreviewPage::onSendSuggestion);
    connect(m_suggestionInput, &QLineEdit::returnPressed, this, &PPTPreviewPage::onSendSuggestion);

    updateState();
}

void PPTPreviewPage::setPresentation(const QString &title, const QString &filePath)
{
    m_title = title;
    m_filePath = filePath;

    if (m_previewWidget) {
        m_previewWidget->setFilePath(filePath);
    }

    // 清空之前的建议输入
    if (m_suggestionInput) {
        m_suggestionInput->clear();
    }

    updateState();
}

QString PPTPreviewPage::presentationPath() const
{
    return m_filePath;
}

void PPTPreviewPage::updateState()
{
    const QString displayTitle = m_title.isEmpty()
        ? QStringLiteral("PPT 预览")
        : QStringLiteral("PPT生成：%1").arg(m_title);
    const QFileInfo fileInfo(m_filePath);
    const bool hasFile = fileInfo.exists() && fileInfo.isFile();

    m_titleLabel->setText(displayTitle);
    m_hintLabel->setText(hasFile
        ? QStringLiteral("当前展示的是已生成的 PPT 文件，可先预览再下载。")
        : QStringLiteral("当前还没有可预览的 PPT 文件。"));
    m_fileLabel->setText(hasFile
        ? QStringLiteral("文件位置：%1").arg(QDir::toNativeSeparators(fileInfo.absoluteFilePath()))
        : QStringLiteral("文件位置：未生成"));
    m_downloadButton->setEnabled(hasFile);
    m_sendSuggestionBtn->setEnabled(hasFile);
}

void PPTPreviewPage::onSendSuggestion()
{
    QString suggestion = m_suggestionInput ? m_suggestionInput->text().trimmed() : QString();
    if (suggestion.isEmpty()) return;

    m_suggestionInput->clear();
    emit modifySuggestionSubmitted(suggestion);
}
