#ifndef ATTENDANCERECORD_H
#define ATTENDANCERECORD_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QJsonObject>
#include "AttendanceStatus.h"

/**
 * @brief 考勤记录模型
 * 老王说：一条考勤记录就是一个学生在某节课的出勤情况，简单明了
 */
class AttendanceRecord
{
public:
    AttendanceRecord();
    AttendanceRecord(int studentId, int classId, const QDate &date, int lessonNumber);

    // 从JSON构造（Supabase返回的数据）
    static AttendanceRecord fromJson(const QJsonObject &json);

    // 转换为JSON（提交给Supabase）
    QJsonObject toJson() const;

    // 是否有效
    bool isValid() const;

    // Getters
    int id() const { return m_id; }
    int studentId() const { return m_studentId; }
    int classId() const { return m_classId; }
    QString courseId() const { return m_courseId; }
    int lessonNumber() const { return m_lessonNumber; }
    QDate date() const { return m_date; }
    QTime checkTime() const { return m_checkTime; }
    AttendanceStatus status() const { return m_status; }
    QString remark() const { return m_remark; }
    QString recorderId() const { return m_recorderId; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }

    // Setters
    void setId(int id) { m_id = id; }
    void setStudentId(int studentId) { m_studentId = studentId; }
    void setClassId(int classId) { m_classId = classId; }
    void setCourseId(const QString &courseId) { m_courseId = courseId; }
    void setLessonNumber(int lessonNumber) { m_lessonNumber = lessonNumber; }
    void setDate(const QDate &date) { m_date = date; }
    void setCheckTime(const QTime &checkTime) { m_checkTime = checkTime; }
    void setStatus(AttendanceStatus status) { m_status = status; }
    void setRemark(const QString &remark) { m_remark = remark; }
    void setRecorderId(const QString &recorderId) { m_recorderId = recorderId; }
    void setCreatedAt(const QDateTime &createdAt) { m_createdAt = createdAt; }
    void setUpdatedAt(const QDateTime &updatedAt) { m_updatedAt = updatedAt; }

    // 辅助方法
    QString statusDisplayName() const;
    QString statusColorHex() const;

private:
    int m_id = 0;                      // 记录ID
    int m_studentId = 0;               // 学生ID
    int m_classId = 0;                 // 班级ID
    QString m_courseId;                // 课程ID（可选）
    int m_lessonNumber = 1;            // 课次（第几节课）
    QDate m_date;                      // 考勤日期
    QTime m_checkTime;                 // 签到时间
    AttendanceStatus m_status = AttendanceStatus::Present;  // 考勤状态
    QString m_remark;                  // 备注
    QString m_recorderId;              // 记录人ID（教师）
    QDateTime m_createdAt;             // 创建时间
    QDateTime m_updatedAt;             // 更新时间
};

#endif // ATTENDANCERECORD_H
