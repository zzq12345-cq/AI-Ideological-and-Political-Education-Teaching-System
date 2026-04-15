#include "pptpreviewpage.h"

#include "macosquicklookpreview.h"

#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
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

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(previewCard, 1);

    connect(m_backButton, &QPushButton::clicked, this, &PPTPreviewPage::backRequested);
    connect(m_downloadButton, &QPushButton::clicked, this, &PPTPreviewPage::downloadRequested);

    updateState();
}

void PPTPreviewPage::setPresentation(const QString &title, const QString &filePath)
{
    m_title = title;
    m_filePath = filePath;

    if (m_previewWidget) {
        m_previewWidget->setFilePath(filePath);
    }

    updateState();
}

QString PPTPreviewPage::presentationPath() const
{
    return m_filePath;
}

void PPTPreviewPage::updateState()
{
    const QString displayTitle = m_title.isEmpty() ? QStringLiteral("PPT 预览") : m_title;
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
}
