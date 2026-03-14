#include "macosquicklookpreview.h"

#include <QLabel>
#include <QVBoxLayout>

MacOSQuickLookPreview::MacOSQuickLookPreview(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_placeholderLabel = new QLabel(QStringLiteral("当前平台暂不支持应用内直接预览 PPT，请先下载后使用 PowerPoint 或 WPS 打开。"), this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setWordWrap(true);
    m_placeholderLabel->setStyleSheet(QStringLiteral("color: #6B7280; font-size: 14px; padding: 24px;"));
    m_layout->addWidget(m_placeholderLabel);
}

MacOSQuickLookPreview::~MacOSQuickLookPreview() = default;

void MacOSQuickLookPreview::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

QString MacOSQuickLookPreview::filePath() const
{
    return m_filePath;
}

bool MacOSQuickLookPreview::isPreviewAvailable() const
{
    return false;
}

void MacOSQuickLookPreview::setPlaceholderText(const QString &text)
{
    if (m_placeholderLabel) {
        m_placeholderLabel->setText(text);
    }
}
