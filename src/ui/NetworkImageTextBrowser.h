#ifndef NETWORKIMAGETEXTBROWSER_H
#define NETWORKIMAGETEXTBROWSER_H

#include <QTextBrowser>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QMap>
#include <QUrl>

/**
 * @brief 支持网络图片加载的 QTextBrowser
 *
 * 继承 QTextBrowser，重写 loadResource 方法以支持从网络 URL 加载图片
 */
class NetworkImageTextBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    explicit NetworkImageTextBrowser(QWidget *parent = nullptr);

    void setHtml(const QString &html);

protected:
    QVariant loadResource(int type, const QUrl &name) override;

private slots:
    void onImageDownloaded(QNetworkReply *reply);

private:
    void downloadImage(const QUrl &url);

    QNetworkAccessManager *m_networkManager;
    QNetworkDiskCache *m_diskCache = nullptr;   // L2 磁盘缓存
    QMap<QUrl, QByteArray> m_imageCache;         // L1 内存缓存
    QSet<QUrl> m_pendingDownloads;
};

#endif // NETWORKIMAGETEXTBROWSER_H
