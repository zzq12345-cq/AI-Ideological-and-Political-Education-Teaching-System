#ifndef SUPABASESTORAGESERVICE_H
#define SUPABASESTORAGESERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QQueue>

/**
 * @brief Supabase Storage 服务
 *
 * 提供图片上传到 Supabase Storage 的功能。
 * 推荐使用异步 API（uploadImageAsync / uploadImagesAsync），避免阻塞主线程。
 * 同步 API（uploadImage / uploadImages）仅供无法改造为异步的调用方使用。
 */
class SupabaseStorageService : public QObject
{
    Q_OBJECT

public:
    explicit SupabaseStorageService(QObject *parent = nullptr);

    // ===== 异步 API（推荐）=====

    /**
     * @brief 异步上传单张图片
     * @param imageData 图片二进制数据
     * @param fileName 文件名
     * @param mimeType MIME 类型
     * @param requestId 调用方自定义请求 ID，用于关联回调
     */
    void uploadImageAsync(const QByteArray &imageData, const QString &fileName,
                          const QString &mimeType, const QString &requestId = QString());

    /**
     * @brief 异步批量上传图片（内部队列串行，不阻塞主线程）
     * @param images rId -> {imageData, mimeType} 的映射
     */
    void uploadImagesAsync(const QMap<QString, QPair<QByteArray, QString>> &images);

    // ===== 同步 API（兼容旧调用方）=====

    /**
     * @brief 同步上传图片（内部使用 QEventLoop，阻塞调用线程）
     * @deprecated 推荐使用 uploadImageAsync
     */
    QString uploadImage(const QByteArray &imageData, const QString &fileName, const QString &mimeType);

    /**
     * @brief 同步批量上传图片
     * @deprecated 推荐使用 uploadImagesAsync
     */
    QMap<QString, QString> uploadImages(const QMap<QString, QPair<QByteArray, QString>> &images);

    QString lastError() const;
    void setBucketName(const QString &bucketName);

signals:
    // 异步上传信号
    void imageUploaded(const QString &requestId, const QString &url);
    void imageUploadError(const QString &requestId, const QString &error);
    void batchUploadFinished(const QMap<QString, QString> &results);

    // 通用进度信号
    void uploadProgress(int current, int total);
    void uploadCompleted(const QString &url);
    void uploadFailed(const QString &error);

private slots:
    void onAsyncUploadFinished();

private:
    struct UploadTask {
        QString requestId;
        QByteArray imageData;
        QString fileName;
        QString mimeType;
    };

    QString generateUniqueFileName(const QString &originalName);
    QString getPublicUrl(const QString &filePath);
    void processNextUpload();

    QNetworkAccessManager *m_networkManager;
    QString m_bucketName;
    QString m_lastError;

    // 异步批量上传队列
    QQueue<UploadTask> m_uploadQueue;
    QMap<QString, QString> m_batchResults;
    int m_batchTotal = 0;
    int m_batchCompleted = 0;
    bool m_isBatchMode = false;
};

#endif // SUPABASESTORAGESERVICE_H
