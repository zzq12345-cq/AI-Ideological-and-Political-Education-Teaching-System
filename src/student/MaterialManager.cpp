#include "MaterialManager.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QDebug>

MaterialManager* MaterialManager::s_instance = nullptr;

MaterialManager* MaterialManager::instance()
{
    if (!s_instance) s_instance = new MaterialManager();
    return s_instance;
}

MaterialManager::MaterialManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

MaterialManager::MaterialInfo MaterialManager::MaterialInfo::fromJson(const QJsonObject &json)
{
    MaterialInfo m;
    m.id = json["id"].toString();
    m.classId = json["class_id"].toString();
    m.folderId = json["folder_id"].isNull() ? QString() : json["folder_id"].toString();
    m.name = json["name"].toString();
    m.type = json["type"].toString();
    m.fileUrl = json["file_url"].toString();
    m.mimeType = json["mime_type"].toString();
    m.uploaderEmail = json["uploader_email"].toString();
    m.fileSize = json["file_size"].toVariant().toLongLong();
    m.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODateWithMs);
    return m;
}

// ── 创建文件夹 ──
void MaterialManager::createFolder(const QString &classId, const QString &parentId,
                                    const QString &name, const QString &uploaderEmail)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/materials");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "return=representation");

    QJsonObject body;
    body["class_id"] = classId;
    body["folder_id"] = parentId.isEmpty() ? QJsonValue() : parentId;
    body["name"] = name;
    body["type"] = "folder";
    body["uploader_email"] = uploaderEmail;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Material] create folder failed:" << reply->errorString();
            emit error("创建文件夹失败");
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        if (!arr.isEmpty()) {
            MaterialInfo info = MaterialInfo::fromJson(arr[0].toObject());
            qDebug() << "[Material] 文件夹已创建:" << info.name;
            emit folderCreated(info);
        }
    });
}

// ── 上传文件 ──
void MaterialManager::uploadFile(const QString &classId, const QString &parentId,
                                  const QString &localPath, const QString &mimeType,
                                  const QString &uploaderEmail)
{
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("无法打开文件");
        return;
    }

    qint64 fileSize = file.size();
    QByteArray fileData = file.readAll();
    file.close();

    QString fileName = QFileInfo(localPath).fileName();
    // 存储路径用 hash 避免中文/特殊字符导致 URL 编码问题
    QString storageName = QDateTime::currentDateTime().toString("yyyyMMddHHmmss")
                          + "_" + QString::number(qHash(fileName), 16);
    QString storagePath = classId + "/" + storageName;

    // 1. 上传到 Supabase Storage（multipart/form-data）
    QUrl storageUrl(SupabaseConfig::supabaseUrl() + "/storage/v1/object/materials/" + storagePath);
    QNetworkRequest request(storageUrl);
    request.setRawHeader("apikey", SupabaseConfig::supabaseAnonKey().toUtf8());
    request.setRawHeader("Authorization",
        QString("Bearer %1").arg(SupabaseConfig::supabaseAnonKey()).toUtf8());
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    // 不设 Content-Type，由 QHttpMultiPart 自动设置 multipart/form-data; boundary=...

    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName)));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mimeType));
    filePart.setBody(fileData);
    multiPart->append(filePart);

    QNetworkReply *reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray responseBody = reply->readAll();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Material] upload failed:" << reply->errorString();
            qWarning() << "[Material] response:" << responseBody;
            emit error("文件上传失败: " + QString::fromUtf8(responseBody));
            return;
        }

        QString fileUrl = SupabaseConfig::supabaseUrl() + "/storage/v1/object/public/materials/" + storagePath;

        // 2. 创建 material 记录
        QUrl dbUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/materials");
        QNetworkRequest dbRequest = NetworkRequestFactory::createAuthRequest(dbUrl);
        dbRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        dbRequest.setRawHeader("Prefer", "return=representation");

        QJsonObject body;
        body["class_id"] = classId;
        body["folder_id"] = parentId.isEmpty() ? QJsonValue() : parentId;
        body["name"] = fileName;
        body["type"] = "file";
        body["file_url"] = fileUrl;
        body["file_size"] = fileSize;
        body["mime_type"] = mimeType;
        body["uploader_email"] = uploaderEmail;

        QNetworkReply *dbReply = m_networkManager->post(dbRequest, QJsonDocument(body).toJson());
        connect(dbReply, &QNetworkReply::finished, this, [this, dbReply]() {
            dbReply->deleteLater();
            if (dbReply->error() != QNetworkReply::NoError) {
                qWarning() << "[Material] create record failed:" << dbReply->errorString();
                emit error("创建文件记录失败");
                return;
            }
            QJsonArray arr = QJsonDocument::fromJson(dbReply->readAll()).array();
            if (!arr.isEmpty()) {
                MaterialInfo info = MaterialInfo::fromJson(arr[0].toObject());
                qDebug() << "[Material] 文件已上传:" << info.name;
                emit fileUploaded(info);
            }
        });
    });
}

// ── 加载目录内容 ──
void MaterialManager::loadMaterials(const QString &classId, const QString &folderId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/materials");
    QUrlQuery query;
    query.addQueryItem("select", "id,class_id,folder_id,name,type,file_url,file_size,mime_type,uploader_email,created_at");
    query.addQueryItem("class_id", "eq." + classId);
    if (folderId.isEmpty()) {
        query.addQueryItem("folder_id", "is.null");
    } else {
        query.addQueryItem("folder_id", "eq." + folderId);
    }
    query.addQueryItem("order", "type.desc,created_at.asc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, folderId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Material] load failed:" << reply->errorString();
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<MaterialInfo> list;
        for (const auto &val : arr) {
            list.append(MaterialInfo::fromJson(val.toObject()));
        }
        qDebug() << "[Material] 目录内容:" << list.size();
        emit materialsLoaded(folderId, list);
    });
}

// ── 删除 ──
void MaterialManager::deleteMaterial(const QString &materialId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/materials");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + materialId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, materialId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Material] delete failed:" << reply->errorString();
            emit error("删除失败");
            return;
        }
        qDebug() << "[Material] 已删除:" << materialId;
        emit materialDeleted(materialId);
    });
}
