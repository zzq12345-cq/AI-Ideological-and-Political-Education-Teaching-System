#ifndef ATTENDANCEMANAGER_H
#define ATTENDANCEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class AttendanceManager : public QObject
{
    Q_OBJECT

public:
    static AttendanceManager* instance();

    struct SessionInfo {
        QString id;
        QString classId;
        QString code;
        QString status;
        QString name;
        QDateTime createdAt;
        QDateTime endedAt;
        static SessionInfo fromJson(const QJsonObject &json);
    };

    struct RecordInfo {
        QString id;
        QString sessionId;
        QString studentEmail;
        QString studentName;
        QString status;
        QDateTime signedAt;
        static RecordInfo fromJson(const QJsonObject &json);
    };

    // 教师操作
    void startAttendance(const QString &classId, const QString &name = "");
    void endAttendance(const QString &sessionId, const QString &classId = QString());
    void loadSessionRecords(const QString &sessionId);
    void loadSessionsByClassAndDate(const QString &classId, const QDate &date);
    void updateRecordStatus(const QString &recordId, const QString &status);

    // 学生操作
    void signAttendance(const QString &code, const QString &studentEmail, const QString &studentName);
    void loadStudentAttendance(const QString &classId, const QString &studentEmail);

signals:
    void attendanceStarted(const SessionInfo &session);
    void attendanceEnded(const QString &sessionId, const QString &classId);
    void recordsLoaded(const QString &sessionId, const QList<RecordInfo> &records);
    void sessionsLoaded(const QList<SessionInfo> &sessions);
    void recordStatusUpdated(const QString &recordId, const QString &status);
    void signResult(bool success, const QString &message);
    void studentAttendanceLoaded(const QString &classId,
        const QList<QPair<SessionInfo, QString>> &records);
    void error(const QString &message);

private:
    AttendanceManager(QObject *parent = nullptr);
    static AttendanceManager *s_instance;
    QNetworkAccessManager *m_networkManager;
};

#endif
