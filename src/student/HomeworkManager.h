#ifndef HOMEWORKMANAGER_H
#define HOMEWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

class HomeworkManager : public QObject
{
    Q_OBJECT

public:
    static HomeworkManager* instance();

    struct AssignmentInfo {
        QString id, classId, teacherEmail, title, description;
        int totalScore = 100, status = 2;
        QDateTime endTime, createdAt;
        static AssignmentInfo fromJson(const QJsonObject &json);
    };

    struct SubmissionInfo {
        QString id, assignmentId, studentEmail, studentName;
        QString content, feedback, fileUrl;
        int score = -1, status = 1;
        bool allowResubmit = false;
        QDateTime submitTime, gradeTime;
        static SubmissionInfo fromJson(const QJsonObject &json);
    };

    // 教师操作
    void createAssignment(const QString &classId, const QString &teacherEmail,
                          const QString &title, const QString &description,
                          int totalScore, const QDateTime &endTime);
    void loadAssignments(const QString &classId);
    void loadSubmissions(const QString &assignmentId);
    void deleteAssignment(const QString &assignmentId);
    void gradeSubmission(const QString &submissionId, int score, const QString &feedback,
                         bool allowResubmit = false);
    void submitHomework(const QString &assignmentId, const QString &studentEmail,
                        const QString &studentName, const QString &content,
                        const QString &fileUrl = QString());
    void resubmitHomework(const QString &submissionId, const QString &newContent,
                          const QString &newFileUrl);

signals:
    void assignmentCreated(const AssignmentInfo &info);
    void assignmentDeleted(const QString &assignmentId);
    void assignmentsLoaded(const QList<AssignmentInfo> &list);
    void submissionsLoaded(const QString &assignmentId, const QList<SubmissionInfo> &list);
    void submissionGraded(const QString &submissionId);
    void homeworkSubmitted(const QString &assignmentId);
    void error(const QString &msg);

private:
    HomeworkManager(QObject *parent = nullptr);
    static HomeworkManager *s_instance;
    QNetworkAccessManager *m_networkManager;
};

#endif
