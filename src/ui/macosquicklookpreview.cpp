#include "macosquicklookpreview.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QImage>
#include <QBuffer>
#include <QtEndian>
#include <algorithm>

// ====== 轻量级 ZIP 读取（只读取文件列表和解压） ======
namespace {

struct ZipEntry {
    QString fileName;
    quint32 compressedSize;
    quint32 uncompressedSize;
    quint32 localHeaderOffset;
    quint16 method;
};

// 读取小端 16 位
static quint16 readLE16(const QByteArray &data, int offset)
{
    if (offset + 2 > data.size()) return 0;
    return qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data.constData() + offset));
}

// 读取小端 32 位
static quint32 readLE32(const QByteArray &data, int offset)
{
    if (offset + 4 > data.size()) return 0;
    return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(data.constData() + offset));
}

// 从 ZIP 文件中提取指定路径的文件内容（只支持 STORE 方式）
static QByteArray extractFileFromZip(const QByteArray &zipData, const ZipEntry &entry)
{
    // 读取 Local File Header
    int offset = static_cast<int>(entry.localHeaderOffset);
    if (offset + 30 > zipData.size()) return {};

    quint32 sig = readLE32(zipData, offset);
    if (sig != 0x04034b50) return {};

    quint16 fnLen = readLE16(zipData, offset + 26);
    quint16 extraLen = readLE16(zipData, offset + 28);
    int dataOffset = offset + 30 + fnLen + extraLen;

    if (entry.method == 0) {
        // STORE
        return zipData.mid(dataOffset, entry.uncompressedSize);
    } else if (entry.method == 8) {
        // DEFLATE - 使用 Qt 自带的 qUncompress (需要添加 zlib 头)
        // qUncompress 期望 4 字节大端长度 + zlib 数据
        // 但 ZIP 中是 raw deflate，我们需要手动添加 zlib 头
        QByteArray compressed = zipData.mid(dataOffset, entry.compressedSize);

        // 构造 zlib 格式：header(78 01) + raw deflate + adler32
        QByteArray zlibData;
        zlibData.append('\x78');
        zlibData.append('\x01');
        zlibData.append(compressed);

        // 计算 Adler-32 校验和（qUncompress 需要）
        // 简单起见，先尝试不加校验和
        quint32 expectedSize = entry.uncompressedSize;

        // qUncompress 需要 4 字节大端长度前缀
        QByteArray withHeader;
        withHeader.resize(4);
        qToBigEndian(expectedSize, withHeader.data());
        withHeader.append(zlibData);

        QByteArray result = qUncompress(withHeader);
        if (!result.isEmpty()) {
            return result;
        }

        // 如果失败，尝试原始数据加前缀
        QByteArray withHeader2;
        withHeader2.resize(4);
        qToBigEndian(expectedSize, withHeader2.data());
        // 添加 zlib 完整头
        withHeader2.append('\x78');
        withHeader2.append('\x9C');
        withHeader2.append(compressed);
        result = qUncompress(withHeader2);
        return result;
    }
    return {};
}

// 解析 ZIP Central Directory，返回所有文件条目
static QVector<ZipEntry> parseZipDirectory(const QByteArray &zipData)
{
    QVector<ZipEntry> entries;

    // 从末尾找 End of Central Directory
    int eocdPos = -1;
    for (int i = zipData.size() - 22; i >= 0 && i >= zipData.size() - 65536; --i) {
        if (readLE32(zipData, i) == 0x06054b50) {
            eocdPos = i;
            break;
        }
    }
    if (eocdPos < 0) return entries;

    quint16 totalEntries = readLE16(zipData, eocdPos + 10);
    quint32 cdOffset = readLE32(zipData, eocdPos + 16);

    int pos = static_cast<int>(cdOffset);
    for (int i = 0; i < totalEntries && pos + 46 <= zipData.size(); ++i) {
        if (readLE32(zipData, pos) != 0x02014b50) break;

        ZipEntry entry;
        entry.method = readLE16(zipData, pos + 10);
        entry.compressedSize = readLE32(zipData, pos + 20);
        entry.uncompressedSize = readLE32(zipData, pos + 24);
        quint16 fnLen = readLE16(zipData, pos + 28);
        quint16 extraLen = readLE16(zipData, pos + 30);
        quint16 commentLen = readLE16(zipData, pos + 32);
        entry.localHeaderOffset = readLE32(zipData, pos + 42);

        entry.fileName = QString::fromUtf8(zipData.mid(pos + 46, fnLen));
        entries.append(entry);

        pos += 46 + fnLen + extraLen + commentLen;
    }

    return entries;
}

// 从 PPTX 中提取幻灯片图片
static QVector<QImage> extractSlideImages(const QString &pptxPath)
{
    QVector<QImage> images;

    QFile file(pptxPath);
    if (!file.open(QIODevice::ReadOnly)) return images;
    QByteArray zipData = file.readAll();
    file.close();

    auto entries = parseZipDirectory(zipData);
    if (entries.isEmpty()) return images;

    // 收集 ppt/media/ 下的图片文件
    QVector<ZipEntry> mediaEntries;
    for (const auto &entry : entries) {
        if (entry.fileName.startsWith("ppt/media/") &&
            (entry.fileName.endsWith(".png", Qt::CaseInsensitive) ||
             entry.fileName.endsWith(".jpg", Qt::CaseInsensitive) ||
             entry.fileName.endsWith(".jpeg", Qt::CaseInsensitive))) {
            mediaEntries.append(entry);
        }
    }

    // 按文件名排序
    std::sort(mediaEntries.begin(), mediaEntries.end(),
              [](const ZipEntry &a, const ZipEntry &b) {
        return a.fileName < b.fileName;
    });

    for (const auto &entry : mediaEntries) {
        QByteArray imageData = extractFileFromZip(zipData, entry);
        if (imageData.isEmpty()) continue;

        QImage img;
        if (img.loadFromData(imageData)) {
            images.append(img);
        }
    }

    return images;
}

} // anonymous namespace

// ====== MacOSQuickLookPreview 实现（Windows 版本：幻灯片图片预览） ======

MacOSQuickLookPreview::MacOSQuickLookPreview(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // 占位标签（无文件时显示）
    m_placeholderLabel = new QLabel(QStringLiteral("选择课时并生成 PPT 后，将在此处预览幻灯片。"), this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setWordWrap(true);
    m_placeholderLabel->setStyleSheet(QStringLiteral(
        "color: #9CA3AF; font-size: 15px; padding: 40px;"
    ));

    // 滚动区域（展示幻灯片缩略图）
    m_container = new QWidget(this);
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_container);
    scrollArea->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; }"
        "QWidget#slideContainer { background: transparent; }"
    ));
    m_container->setObjectName("slideContainer");

    m_layout->addWidget(m_placeholderLabel);
    m_layout->addWidget(scrollArea);

    scrollArea->hide();
}

MacOSQuickLookPreview::~MacOSQuickLookPreview() = default;

void MacOSQuickLookPreview::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
    m_previewAvailable = false;

    // 清理旧的缩略图
    if (m_container->layout()) {
        QLayoutItem *item;
        while ((item = m_container->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete m_container->layout();
    }

    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) {
        m_placeholderLabel->setText(QStringLiteral("当前还没有可预览的 PPT 文件。"));
        m_placeholderLabel->show();
        // 找到 scrollArea 并隐藏
        for (int i = 0; i < m_layout->count(); ++i) {
            if (auto *sa = qobject_cast<QScrollArea*>(m_layout->itemAt(i)->widget())) {
                sa->hide();
                break;
            }
        }
        return;
    }

    // 从 PPTX 中解压图片
    QVector<QImage> slideImages = extractSlideImages(filePath);

    if (slideImages.isEmpty()) {
        m_placeholderLabel->setText(QStringLiteral(
            "无法提取幻灯片预览图片。\n\n"
            "请点击「下载PPT」按钮保存后，使用 PowerPoint 或 WPS 打开查看。"
        ));
        m_placeholderLabel->show();
        for (int i = 0; i < m_layout->count(); ++i) {
            if (auto *sa = qobject_cast<QScrollArea*>(m_layout->itemAt(i)->widget())) {
                sa->hide();
                break;
            }
        }
        return;
    }

    // 构建幻灯片缩略图列表
    m_previewAvailable = true;
    m_placeholderLabel->hide();

    auto *slideLayout = new QVBoxLayout(m_container);
    slideLayout->setContentsMargins(16, 16, 16, 16);
    slideLayout->setSpacing(20);

    for (int i = 0; i < slideImages.size(); ++i) {
        // 每张幻灯片的卡片
        auto *slideCard = new QFrame(m_container);
        slideCard->setStyleSheet(QStringLiteral(
            "QFrame { background: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 12px; }"
        ));

        auto *cardLayout = new QVBoxLayout(slideCard);
        cardLayout->setContentsMargins(16, 12, 16, 12);
        cardLayout->setSpacing(8);

        // 幻灯片编号
        auto *slideNumber = new QLabel(QStringLiteral("第 %1 页").arg(i + 1), slideCard);
        slideNumber->setStyleSheet(QStringLiteral(
            "color: #6B7280; font-size: 13px; font-weight: 600; background: transparent; border: none;"
        ));

        // 幻灯片图片
        auto *slideLabel = new QLabel(slideCard);
        slideLabel->setAlignment(Qt::AlignCenter);
        slideLabel->setStyleSheet(QStringLiteral("background: transparent; border: none;"));

        QPixmap pix = QPixmap::fromImage(slideImages[i]);
        // 限制宽度最大 720px，保持比例
        if (pix.width() > 720) {
            pix = pix.scaledToWidth(720, Qt::SmoothTransformation);
        }
        slideLabel->setPixmap(pix);

        cardLayout->addWidget(slideNumber);
        cardLayout->addWidget(slideLabel);

        slideLayout->addWidget(slideCard);
    }

    slideLayout->addStretch();

    // 显示滚动区域
    for (int i = 0; i < m_layout->count(); ++i) {
        if (auto *sa = qobject_cast<QScrollArea*>(m_layout->itemAt(i)->widget())) {
            sa->show();
            break;
        }
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
