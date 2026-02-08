#include "SupabaseStorageService.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QUuid>
#include <QDateTime>

SupabaseStorageService::SupabaseStorageService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_bucketName("question-images")  // 默认存储桶名称
{
}

QString SupabaseStorageService::lastError() const
{
    return m_lastError;
}

void SupabaseStorageService::setBucketName(const QString &bucketName)
{
    m_bucketName = bucketName;
}

QString SupabaseStorageService::generateUniqueFileName(const QString &originalName)
{
    // 生成唯一文件名: timestamp_uuid_originalname
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString ext = originalName.section('.', -1);
    return QString("%1_%2.%3").arg(timestamp, uuid, ext);
}

QString SupabaseStorageService::getPublicUrl(const QString &filePath)
{
    // Supabase Storage 公开 URL 格式
    return QString("%1/storage/v1/object/public/%2/%3")
            .arg(SupabaseConfig::SUPABASE_URL, m_bucketName, filePath);
}

QString SupabaseStorageService::uploadImage(const QByteArray &imageData,
                                            const QString &fileName,
                                            const QString &mimeType)
{
    m_lastError.clear();

    if (imageData.isEmpty()) {
        m_lastError = "图片数据为空";
        return QString();
    }

    // 生成唯一文件名
    QString uniqueFileName = generateUniqueFileName(fileName);

    // 构建上传 URL
    QString uploadUrl = QString("%1/storage/v1/object/%2/%3")
                            .arg(SupabaseConfig::SUPABASE_URL, m_bucketName, uniqueFileName);

    qDebug() << "[SupabaseStorageService] 上传图片到:" << uploadUrl;

    // 构建请求（使用通用请求工厂，手动添加 Storage 特有的头）
    QNetworkRequest request = NetworkRequestFactory::createGeneralRequest(QUrl(uploadUrl), 30000);
    request.setRawHeader("apikey", SupabaseConfig::SUPABASE_ANON_KEY.toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(SupabaseConfig::SUPABASE_ANON_KEY).toUtf8());
    request.setRawHeader("Content-Type", mimeType.toUtf8());
    request.setRawHeader("x-upsert", "true");  // 允许覆盖

    // 同步上传（使用事件循环等待）
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    QNetworkReply *reply = m_networkManager->post(request, imageData);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    timeout.start(30000);  // 30秒超时
    loop.exec();

    if (timeout.isActive()) {
        timeout.stop();
    } else {
        m_lastError = "上传超时";
        reply->abort();
        reply->deleteLater();
        return QString();
    }

    // 检查响应
    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = QString("上传失败: %1").arg(reply->errorString());
        qDebug() << "[SupabaseStorageService]" << m_lastError;
        qDebug() << "[SupabaseStorageService] 响应:" << reply->readAll();
        reply->deleteLater();
        return QString();
    }

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpStatus != 200 && httpStatus != 201) {
        m_lastError = QString("上传失败，HTTP 状态码: %1").arg(httpStatus);
        qDebug() << "[SupabaseStorageService]" << m_lastError;
        qDebug() << "[SupabaseStorageService] 响应:" << reply->readAll();
        reply->deleteLater();
        return QString();
    }

    reply->deleteLater();

    // 返回公开 URL
    QString publicUrl = getPublicUrl(uniqueFileName);
    qDebug() << "[SupabaseStorageService] 上传成功:" << publicUrl;

    return publicUrl;
}

QMap<QString, QString> SupabaseStorageService::uploadImages(
    const QMap<QString, QPair<QByteArray, QString>> &images)
{
    QMap<QString, QString> result;

    int current = 0;
    int total = images.size();

    for (auto it = images.begin(); it != images.end(); ++it) {
        QString rId = it.key();
        QByteArray imageData = it.value().first;
        QString mimeType = it.value().second;

        // 从 mimeType 推断扩展名
        QString ext = "png";
        if (mimeType.contains("jpeg") || mimeType.contains("jpg")) {
            ext = "jpg";
        } else if (mimeType.contains("gif")) {
            ext = "gif";
        } else if (mimeType.contains("bmp")) {
            ext = "bmp";
        }

        QString fileName = QString("%1.%2").arg(rId, ext);
        QString url = uploadImage(imageData, fileName, mimeType);

        if (!url.isEmpty()) {
            result[rId] = url;
        }

        current++;
        emit uploadProgress(current, total);
    }

    return result;
}
