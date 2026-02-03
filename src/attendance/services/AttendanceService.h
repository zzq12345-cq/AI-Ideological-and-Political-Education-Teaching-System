#ifndef ATTENDANCESERVICE_H
#define ATTENDANCESERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDate>
#include "../models/AttendanceRecord.h"
#include "../models/AttendanceSummary.h"
#include "../../analytics/models/Student.h"

/**
 * @brief 考勤服务层
 * 老王说：参考NotificationService模式，Supabase REST API一把梭
 */
class AttendanceService : public QObject
{
    Q_OBJECT

public:
    explicit AttendanceService(QObject *parent = nullptr);
    ~AttendanceService();

    // 设置当前用户ID（教师）
    void setCurrentUserId(const QString &userId);
    QString currentUserId() const { return m_currentUserId; }

    // 获取班级学生列表
    void fetchStudentsByClass(int classId);

    // 获取某课次的考勤记录
    void fetchAttendanceByLesson(int classId, const QDate &date, int lessonNumber);

    // 获取班级某日期范围的考勤记录
    void fetchAttendanceByDateRange(int classId, const QDate &startDate, const QDate &endDate);

    // 批量提交考勤记录
    void submitAttendance(const QList<AttendanceRecord> &records);

    // 更新单条考勤记录
    void updateAttendance(const AttendanceRecord &record);

    // 删除考勤记录
    void deleteAttendance(int recordId);

    // 获取班级考勤统计
    void fetchClassStatistics(int classId, const QDate &startDate, const QDate &endDate);

    // 获取学生考勤统计
    void fetchStudentStatistics(int studentId, const QDate &startDate, const QDate &endDate);

    // 获取缓存的数据
    QList<Student> students() const { return m_students; }
    QList<AttendanceRecord> attendanceRecords() const { return m_attendanceRecords; }
    AttendanceSummary currentSummary() const { return m_currentSummary; }

    // 加载状态
    bool isLoading() const { return m_isLoading; }

signals:
    // 数据接收信号
    void studentsReceived(const QList<Student> &students);
    void attendanceReceived(const QList<AttendanceRecord> &records);
    void attendanceSubmitted(bool success, const QString &message);
    void attendanceUpdated(bool success);
    void attendanceDeleted(bool success);
    void statisticsReceived(const AttendanceSummary &summary);

    // 状态信号
    void loadingStateChanged(bool isLoading);
    void errorOccurred(const QString &error);

private slots:
    void onFetchStudentsFinished();
    void onFetchAttendanceFinished();
    void onSubmitAttendanceFinished();
    void onUpdateAttendanceFinished();
    void onDeleteAttendanceFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QNetworkRequest createRequest(const QString &endpoint) const;
    void setLoading(bool loading);

    // 加载示例数据（开发测试用）
    void loadSampleStudents(int classId);
    void loadSampleAttendance(int classId, const QDate &date, int lessonNumber);

    QNetworkAccessManager *m_networkManager;
    QString m_currentUserId;

    // 缓存数据
    QList<Student> m_students;
    QList<AttendanceRecord> m_attendanceRecords;
    AttendanceSummary m_currentSummary;

    bool m_isLoading = false;
};

#endif // ATTENDANCESERVICE_H
