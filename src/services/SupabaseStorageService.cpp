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
    , m_bucketName("question-images")
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
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString ext = originalName.section('.', -1);
    return QString("%1_%2.%3").arg(timestamp, uuid, ext);
}

QString SupabaseStorageService::getPublicUrl(const QString &filePath)
{
    return QString("%1/storage/v1/object/public/%2/%3")
            .arg(SupabaseConfig::supabaseUrl(), m_bucketName, filePath);
}

// ===== 异步 API =====

void SupabaseStorageService::uploadImageAsync(const QByteArray &imageData, const QString &fileName,
                                               const QString &mimeType, const QString &requestId)
{
    m_lastError.clear();

    if (imageData.isEmpty()) {
        m_lastError = "图片数据为空";
        emit imageUploadError(requestId, m_lastError);
        return;
    }

    QString uniqueFileName = generateUniqueFileName(fileName);
    QString uploadUrl = QString("%1/storage/v1/object/%2/%3")
                            .arg(SupabaseConfig::supabaseUrl(), m_bucketName, uniqueFileName);

    qDebug() << "[SupabaseStorageService] 异步上传图片到:" << uploadUrl;

    QNetworkRequest request = NetworkRequestFactory::createGeneralRequest(QUrl(uploadUrl), 30000);
    request.setRawHeader("apikey", SupabaseConfig::supabaseAnonKey().toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(SupabaseConfig::supabaseAnonKey()).toUtf8());
    request.setRawHeader("Content-Type", mimeType.toUtf8());
    request.setRawHeader("x-upsert", "true");

    QNetworkReply *reply = m_networkManager->post(request, imageData);
    // 通过 property 传递上下文
    reply->setProperty("requestId", requestId);
    reply->setProperty("uniqueFileName", uniqueFileName);

    connect(reply, &QNetworkReply::finished, this, &SupabaseStorageService::onAsyncUploadFinished);
}

void SupabaseStorageService::onAsyncUploadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QString requestId = reply->property("requestId").toString();
    QString uniqueFileName = reply->property("uniqueFileName").toString();

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = QString("上传失败: %1").arg(reply->errorString());
        qDebug() << "[SupabaseStorageService]" << m_lastError;
        emit imageUploadError(requestId, m_lastError);
    } else {
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpStatus != 200 && httpStatus != 201) {
            m_lastError = QString("上传失败，HTTP 状态码: %1").arg(httpStatus);
            qDebug() << "[SupabaseStorageService]" << m_lastError;
            emit imageUploadError(requestId, m_lastError);
        } else {
            QString publicUrl = getPublicUrl(uniqueFileName);
            qDebug() << "[SupabaseStorageService] 异步上传成功:" << publicUrl;
            emit imageUploaded(requestId, publicUrl);
        }
    }

    reply->deleteLater();

    // 批量模式：更新进度并处理下一个
    if (m_isBatchMode) {
        m_batchCompleted++;

        // 记录结果
        if (reply->error() == QNetworkReply::NoError) {
            int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (httpStatus == 200 || httpStatus == 201) {
                m_batchResults[requestId] = getPublicUrl(uniqueFileName);
            }
        }

        emit uploadProgress(m_batchCompleted, m_batchTotal);

        if (m_uploadQueue.isEmpty()) {
            // 批量上传全部完成
            m_isBatchMode = false;
            emit batchUploadFinished(m_batchResults);
        } else {
            processNextUpload();
        }
    }
}

void SupabaseStorageService::uploadImagesAsync(const QMap<QString, QPair<QByteArray, QString>> &images)
{
    m_uploadQueue.clear();
    m_batchResults.clear();
    m_batchTotal = images.size();
    m_batchCompleted = 0;
    m_isBatchMode = true;

    for (auto it = images.begin(); it != images.end(); ++it) {
        UploadTask task;
        task.requestId = it.key();
        task.imageData = it.value().first;
        task.mimeType = it.value().second;

        // 从 mimeType 推断扩展名
        QString ext = "png";
        if (task.mimeType.contains("jpeg") || task.mimeType.contains("jpg")) {
            ext = "jpg";
        } else if (task.mimeType.contains("gif")) {
            ext = "gif";
        } else if (task.mimeType.contains("bmp")) {
            ext = "bmp";
        }
        task.fileName = QString("%1.%2").arg(task.requestId, ext);

        m_uploadQueue.enqueue(task);
    }

    if (m_uploadQueue.isEmpty()) {
        m_isBatchMode = false;
        emit batchUploadFinished(m_batchResults);
        return;
    }

    processNextUpload();
}

void SupabaseStorageService::processNextUpload()
{
    if (m_uploadQueue.isEmpty()) return;

    UploadTask task = m_uploadQueue.dequeue();
    uploadImageAsync(task.imageData, task.fileName, task.mimeType, task.requestId);
}

// ===== 同步 API（保持向后兼容）=====

QString SupabaseStorageService::uploadImage(const QByteArray &imageData,
                                            const QString &fileName,
                                            const QString &mimeType)
{
    m_lastError.clear();

    if (imageData.isEmpty()) {
        m_lastError = "图片数据为空";
        return QString();
    }

    QString uniqueFileName = generateUniqueFileName(fileName);
    QString uploadUrl = QString("%1/storage/v1/object/%2/%3")
                            .arg(SupabaseConfig::supabaseUrl(), m_bucketName, uniqueFileName);

    qDebug() << "[SupabaseStorageService] 上传图片到:" << uploadUrl;

    QNetworkRequest request = NetworkRequestFactory::createGeneralRequest(QUrl(uploadUrl), 30000);
    request.setRawHeader("apikey", SupabaseConfig::supabaseAnonKey().toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(SupabaseConfig::supabaseAnonKey()).toUtf8());
    request.setRawHeader("Content-Type", mimeType.toUtf8());
    request.setRawHeader("x-upsert", "true");

    // 同步等待（使用事件循环，保持 UI 响应）
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    QNetworkReply *reply = m_networkManager->post(request, imageData);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    timeout.start(30000);
    loop.exec();

    if (timeout.isActive()) {
        timeout.stop();
    } else {
        m_lastError = "上传超时";
        reply->abort();
        reply->deleteLater();
        return QString();
    }

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = QString("上传失败: %1").arg(reply->errorString());
        qDebug() << "[SupabaseStorageService]" << m_lastError;
        reply->deleteLater();
        return QString();
    }

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpStatus != 200 && httpStatus != 201) {
        m_lastError = QString("上传失败，HTTP 状态码: %1").arg(httpStatus);
        qDebug() << "[SupabaseStorageService]" << m_lastError;
        reply->deleteLater();
        return QString();
    }

    reply->deleteLater();

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
