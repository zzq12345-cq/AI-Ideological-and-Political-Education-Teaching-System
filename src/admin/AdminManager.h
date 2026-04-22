#ifndef ADMINMANAGER_H
#define ADMINMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class AdminManager : public QObject
{
    Q_OBJECT

public:
    static AdminManager* instance();

    // ── 数据结构 ──
    struct SchoolInfo {
        QString id, name;
        int classQuota = 5;
        QDateTime createdAt;
        static SchoolInfo fromJson(const QJsonObject &json);
    };

    struct InviteCodeInfo {
        QString id, schoolId, code;
        bool used = false;
        QString usedByEmail;
        QDateTime createdAt;
        static InviteCodeInfo fromJson(const QJsonObject &json);
    };

    struct TeacherInfo {
        QString email, name, schoolId;
        static TeacherInfo fromJson(const QJsonObject &json);
    };

    struct ClassBrief {
        QString id, name, teacherName, teacherEmail, schoolId, code, description, status;
        int studentCount = 0;
        QDateTime createdAt;
        static ClassBrief fromJson(const QJsonObject &json);
    };

    // ── 学校管理 ──
    void loadSchools();
    void createSchool(const QString &name, int classQuota = 5);
    void updateSchool(const QString &schoolId, const QString &name, int classQuota);
    void deleteSchool(const QString &schoolId);

    // ── 邀请码 ──
    void loadInviteCodes(const QString &schoolId);
    void generateInviteCode(const QString &schoolId, int count = 1);
    void deleteInviteCode(const QString &codeId);

    // ── 邀请码验证（设置页用）──
    void validateAndUseInviteCode(const QString &code, const QString &userEmail);

    // ── 教师 ──
    void loadTeachers(const QString &schoolId = QString());
    void assignTeacherToSchool(const QString &email, const QString &schoolId);
    void updateTeacherRole(const QString &email, const QString &role);
    void deleteTeacher(const QString &email);
    void resetTeacherPassword(const QString &email, const QString &newPassword);

    // ── 班级 ──
    void loadAllClasses();
    void loadSchoolClasses(const QString &schoolId);
    void updateClass(const QString &classId, const QString &name, const QString &description, const QString &status);
    void deleteClass(const QString &classId);

    // ── 统计 ──
    void loadOverviewStats();

signals:
    void schoolsLoaded(const QList<SchoolInfo> &schools);
    void schoolCreated(const SchoolInfo &school);
    void schoolUpdated();
    void schoolDeleted(const QString &schoolId);
    void inviteCodesLoaded(const QList<InviteCodeInfo> &codes);
    void inviteCodeGenerated(const QStringList &codes);
    void inviteCodeDeleted(const QString &codeId);
    // 邀请码验证结果
    void inviteCodeValid(const QString &schoolId, const QString &schoolName);
    void inviteCodeInvalid(const QString &msg);
    void teachersLoaded(const QList<TeacherInfo> &teachers);
    void teacherUpdated();
    void teacherDeleted(const QString &email);
    void passwordReset(const QString &email);
    void allClassesLoaded(const QList<ClassBrief> &classes);
    void schoolClassesLoaded(const QString &schoolId, const QList<ClassBrief> &classes);
    void classUpdated();
    void classDeleted(const QString &classId);
    void overviewStatsLoaded(int schools, int teachers, int classes, int students);
    void error(const QString &msg);

private:
    AdminManager(QObject *parent = nullptr);
    static AdminManager *s_instance;
    QNetworkAccessManager *m_networkManager;
};

#endif // ADMINMANAGER_H
