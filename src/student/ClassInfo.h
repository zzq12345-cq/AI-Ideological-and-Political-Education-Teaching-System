#ifndef CLASSINFO_H
#define CLASSINFO_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>

struct ClassInfo {
    QString id;
    QString name;
    QString teacher;
    QString teacherEmail;
    QString code;
    QString status;  // "active" / "inactive"
    QString description;
    bool isPublic = false;
    int colorIndex = 0;
    int studentCount = 0;

    QJsonObject toJson() const {
        return QJsonObject{
            {"id", id},
            {"name", name},
            {"teacher", teacher},
            {"teacherEmail", teacherEmail},
            {"code", code},
            {"status", status},
            {"description", description},
            {"is_public", isPublic},
            {"colorIndex", colorIndex},
            {"studentCount", studentCount}
        };
    }

    static ClassInfo fromJson(const QJsonObject &json) {
        ClassInfo info;
        info.id = json["id"].toString();
        info.name = json["name"].toString();
        info.teacher = json["teacher"].toString();
        info.teacherEmail = json["teacherEmail"].toString();
        info.code = json["code"].toString();
        info.status = json["status"].toString("active");
        info.description = json["description"].toString();
        info.isPublic = json["is_public"].toBool(false);
        info.colorIndex = json["colorIndex"].toInt(0);
        info.studentCount = json["studentCount"].toInt(0);
        return info;
    }
};

#endif
