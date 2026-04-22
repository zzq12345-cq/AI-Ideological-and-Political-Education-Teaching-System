#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class MaterialManager : public QObject
{
    Q_OBJECT

public:
    static MaterialManager* instance();

    struct MaterialInfo {
        QString id, classId, folderId, name, type; // type: "file" or "folder"
        QString fileUrl, mimeType, uploaderEmail;
        qint64 fileSize = 0;
        QDateTime createdAt;
        static MaterialInfo fromJson(const QJsonObject &json);
    };

    void createFolder(const QString &classId, const QString &parentId,
                      const QString &name, const QString &uploaderEmail);
    void uploadFile(const QString &classId, const QString &parentId,
                    const QString &localPath, const QString &mimeType,
                    const QString &uploaderEmail);
    void loadMaterials(const QString &classId, const QString &folderId);
    void deleteMaterial(const QString &materialId);

signals:
    void folderCreated(const MaterialInfo &info);
    void fileUploaded(const MaterialInfo &info);
    void materialsLoaded(const QString &folderId, const QList<MaterialInfo> &list);
    void materialDeleted(const QString &materialId);
    void error(const QString &msg);

private:
    MaterialManager(QObject *parent = nullptr);
    static MaterialManager *s_instance;
    QNetworkAccessManager *m_networkManager;
};

#endif
