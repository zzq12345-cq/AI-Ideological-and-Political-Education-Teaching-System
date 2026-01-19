#ifndef SUPABASESTORAGESERVICE_H
#define SUPABASESTORAGESERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>

/**
 * @brief Supabase Storage 服务
 *
 * 提供图片上传到 Supabase Storage 的功能
 */
class SupabaseStorageService : public QObject
{
    Q_OBJECT

public:
    explicit SupabaseStorageService(QObject *parent = nullptr);

    /**
     * @brief 上传图片到 Supabase Storage
     * @param imageData 图片二进制数据
     * @param fileName 文件名（如 image1.png）
     * @param mimeType MIME 类型（如 image/png）
     * @return 上传后的公开 URL，失败返回空字符串
     */
    QString uploadImage(const QByteArray &imageData, const QString &fileName, const QString &mimeType);

    /**
     * @brief 批量上传图片
     * @param images rId -> {imageData, mimeType} 的映射
     * @return rId -> 公开 URL 的映射
     */
    QMap<QString, QString> uploadImages(const QMap<QString, QPair<QByteArray, QString>> &images);

    /**
     * @brief 获取最后一次错误信息
     */
    QString lastError() const;

    /**
     * @brief 设置存储桶名称
     */
    void setBucketName(const QString &bucketName);

signals:
    void uploadProgress(int current, int total);
    void uploadCompleted(const QString &url);
    void uploadFailed(const QString &error);

private:
    QString generateUniqueFileName(const QString &originalName);
    QString getPublicUrl(const QString &filePath);

    QNetworkAccessManager *m_networkManager;
    QString m_bucketName;
    QString m_lastError;
};

#endif // SUPABASESTORAGESERVICE_H
