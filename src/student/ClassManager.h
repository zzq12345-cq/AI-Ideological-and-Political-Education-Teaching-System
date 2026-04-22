#ifndef CLASSMANAGER_H
#define CLASSMANAGER_H

#include "ClassInfo.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QPair>

class ClassManager : public QObject
{
    Q_OBJECT

public:
    static ClassManager* instance();

    // 异步操作 —— 通过信号返回结果
    void loadTeacherClasses(const QString &email);
    void loadStudentClasses(const QString &email);
    void createClass(const QString &name, const QString &teacherName, const QString &teacherEmail);
    void refreshCode(const QString &classId);
    void joinClass(const QString &code, const QString &studentEmail, const QString &studentName, const QString &studentNumber);
    void deleteClass(const QString &classId);
    void loadClassMembers(const QString &classId);
    void removeMember(const QString &classId, const QString &studentEmail);
    void updateClass(const QString &classId, const QString &name, const QString &description, bool isPublic);

    // 同步缓存
    QList<ClassInfo> cachedClasses() const { return m_cached; }

    struct MemberInfo { QString email; QString name; QString number; };

signals:
    void classesLoaded(const QList<ClassInfo> &classes);
    void classCreated(const ClassInfo &info);
    void classUpdated(const ClassInfo &info);
    void classDeleted(const QString &classId);
    void codeRefreshed(const QString &classId, const QString &newCode);
    void joinResult(bool success, const QString &message, const QString &className);
    void membersLoaded(const QString &classId, const QList<MemberInfo> &members);
    void memberRemoved(const QString &classId, const QString &studentEmail);
    void error(const QString &message);

private:
    ClassManager(QObject *parent = nullptr);
    static ClassManager *s_instance;
    QNetworkAccessManager *m_networkManager;
    QList<ClassInfo> m_cached;
};

#endif
