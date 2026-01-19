#include "NetworkImageTextBrowser.h"
#include <QNetworkRequest>
#include <QImage>
#include <QTextDocument>
#include <QRegularExpression>
#include <QDebug>

NetworkImageTextBrowser::NetworkImageTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &NetworkImageTextBrowser::onImageDownloaded);
}

void NetworkImageTextBrowser::setHtml(const QString &html)
{
    // 先设置 HTML
    QTextBrowser::setHtml(html);

    // 扫描 HTML 中的图片 URL 并预下载
    QRegularExpression imgRe(R"(<img[^>]+src\s*=\s*[\"']([^\"']+)[\"'])");
    QRegularExpressionMatchIterator it = imgRe.globalMatch(html);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString src = match.captured(1);
        QUrl url(src);

        if (url.scheme() == "http" || url.scheme() == "https") {
            if (!m_imageCache.contains(url) && !m_pendingDownloads.contains(url)) {
                downloadImage(url);
            }
        }
    }
}

void NetworkImageTextBrowser::downloadImage(const QUrl &url)
{
    m_pendingDownloads.insert(url);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (compatible; Qt NetworkImageTextBrowser)");

    m_networkManager->get(request);
    qDebug() << "[NetworkImageTextBrowser] 开始下载图片:" << url.toString();
}

void NetworkImageTextBrowser::onImageDownloaded(QNetworkReply *reply)
{
    QUrl url = reply->url();
    m_pendingDownloads.remove(url);

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        m_imageCache[url] = data;

        qDebug() << "[NetworkImageTextBrowser] 图片下载成功:" << url.toString()
                 << "大小:" << data.size() << "bytes";

        // 重新加载文档以显示图片
        QString html = toHtml();
        QTextBrowser::setHtml(html);

        // 调整高度
        document()->setDocumentMargin(0);
        QSize docSize = document()->size().toSize();
        setMinimumHeight(docSize.height() + 20);
    } else {
        qDebug() << "[NetworkImageTextBrowser] 图片下载失败:" << url.toString()
                 << "错误:" << reply->errorString();
    }

    reply->deleteLater();
}

QVariant NetworkImageTextBrowser::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource) {
        // 检查是否是网络 URL
        if (name.scheme() == "http" || name.scheme() == "https") {
            // 检查缓存
            if (m_imageCache.contains(name)) {
                QImage image;
                if (image.loadFromData(m_imageCache[name])) {
                    return image;
                }
            }

            // 如果还没下载，开始下载
            if (!m_pendingDownloads.contains(name)) {
                const_cast<NetworkImageTextBrowser*>(this)->downloadImage(name);
            }

            // 返回空，等待下载完成后刷新
            return QVariant();
        }
    }

    // 其他资源使用默认处理
    return QTextBrowser::loadResource(type, name);
}
