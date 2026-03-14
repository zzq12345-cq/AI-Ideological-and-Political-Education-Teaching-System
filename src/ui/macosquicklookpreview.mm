#include "macosquicklookpreview.h"

#include <QFileInfo>
#include <QLabel>
#include <QVBoxLayout>
#include <QWindow>

#import <AppKit/AppKit.h>
#import <QuickLookUI/QLPreviewView.h>

namespace {
QString defaultPlaceholderText()
{
    return QStringLiteral("正在准备 PPT 预览...");
}
}

MacOSQuickLookPreview::MacOSQuickLookPreview(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_placeholderLabel = new QLabel(defaultPlaceholderText(), this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setWordWrap(true);
    m_placeholderLabel->setStyleSheet(QStringLiteral("color: #6B7280; font-size: 14px; padding: 24px;"));
    m_layout->addWidget(m_placeholderLabel);

    QLPreviewView *previewView = [[QLPreviewView alloc] initWithFrame:NSMakeRect(0, 0, 960, 640)
                                                                 style:QLPreviewViewStyleNormal];
    previewView.autostarts = YES;
    previewView.shouldCloseWithWindow = NO;

    m_nativePreviewView = previewView;
    m_foreignWindow = QWindow::fromWinId(reinterpret_cast<WId>(previewView));
    if (!m_foreignWindow) {
        setPlaceholderText(QStringLiteral("应用内 PPT 预览初始化失败，请下载后查看。"));
        return;
    }

    m_container = QWidget::createWindowContainer(m_foreignWindow, this);
    m_container->setFocusPolicy(Qt::StrongFocus);
    m_container->setMinimumSize(640, 480);
    m_layout->addWidget(m_container);
    m_previewAvailable = true;
    m_placeholderLabel->hide();
}

MacOSQuickLookPreview::~MacOSQuickLookPreview()
{
    if (m_container) {
        delete m_container;
        m_container = nullptr;
    }

    if (m_nativePreviewView) {
        QLPreviewView *previewView = static_cast<QLPreviewView *>(m_nativePreviewView);
        [previewView close];
        [previewView release];
        m_nativePreviewView = nullptr;
    }
}

void MacOSQuickLookPreview::setFilePath(const QString &filePath)
{
    m_filePath = filePath;

    if (!m_previewAvailable || !m_nativePreviewView) {
        setPlaceholderText(QStringLiteral("应用内 PPT 预览不可用，请下载后查看。"));
        return;
    }

    QLPreviewView *previewView = static_cast<QLPreviewView *>(m_nativePreviewView);
    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) {
        previewView.previewItem = nil;
        setPlaceholderText(QStringLiteral("未找到可预览的 PPT 文件。"));
        if (m_placeholderLabel) {
            m_placeholderLabel->show();
        }
        return;
    }

    const QByteArray encodedPath = QFile::encodeName(filePath);
    NSString *nativePath = [NSString stringWithUTF8String:encodedPath.constData()];
    NSURL *fileUrl = [NSURL fileURLWithPath:nativePath];
    previewView.previewItem = (id<QLPreviewItem>)fileUrl;
    [previewView refreshPreviewItem];

    if (m_placeholderLabel) {
        m_placeholderLabel->hide();
    }
}

QString MacOSQuickLookPreview::filePath() const
{
    return m_filePath;
}

bool MacOSQuickLookPreview::isPreviewAvailable() const
{
    return m_previewAvailable;
}

void MacOSQuickLookPreview::setPlaceholderText(const QString &text)
{
    if (m_placeholderLabel) {
        m_placeholderLabel->setText(text);
    }
}
