#include "AttendanceRecord.h"
#include <QJsonDocument>

AttendanceRecord::AttendanceRecord()
    : m_date(QDate::currentDate())
    , m_checkTime(QTime::currentTime())
{
}

AttendanceRecord::AttendanceRecord(int studentId, int classId, const QDate &date, int lessonNumber)
    : m_studentId(studentId)
    , m_classId(classId)
    , m_date(date)
    , m_lessonNumber(lessonNumber)
    , m_checkTime(QTime::currentTime())
    , m_status(AttendanceStatus::Present)
{
}

AttendanceRecord AttendanceRecord::fromJson(const QJsonObject &json)
{
    AttendanceRecord record;

    record.m_id = json["id"].toInt();
    record.m_studentId = json["student_id"].toInt();
    record.m_classId = json["class_id"].toInt();
    record.m_courseId = json["course_id"].toString();
    record.m_lessonNumber = json["lesson_number"].toInt(1);

    // 解析日期
    QString dateStr = json["attendance_date"].toString();
    if (!dateStr.isEmpty()) {
        record.m_date = QDate::fromString(dateStr, Qt::ISODate);
    }

    // 解析签到时间
    QString timeStr = json["check_time"].toString();
    if (!timeStr.isEmpty()) {
        record.m_checkTime = QTime::fromString(timeStr, "HH:mm:ss");
    }

    // 解析状态
    record.m_status = AttendanceStatusHelper::fromInt(json["status"].toInt());

    record.m_remark = json["remark"].toString();
    record.m_recorderId = json["recorder_id"].toString();

    // 解析时间戳
    QString createdAtStr = json["created_at"].toString();
    if (!createdAtStr.isEmpty()) {
        record.m_createdAt = QDateTime::fromString(createdAtStr, Qt::ISODate);
    }

    QString updatedAtStr = json["updated_at"].toString();
    if (!updatedAtStr.isEmpty()) {
        record.m_updatedAt = QDateTime::fromString(updatedAtStr, Qt::ISODate);
    }

    return record;
}

QJsonObject AttendanceRecord::toJson() const
{
    QJsonObject json;

    if (m_id > 0) {
        json["id"] = m_id;
    }

    json["student_id"] = m_studentId;
    json["class_id"] = m_classId;

    if (!m_courseId.isEmpty()) {
        json["course_id"] = m_courseId;
    }

    json["lesson_number"] = m_lessonNumber;
    json["attendance_date"] = m_date.toString(Qt::ISODate);

    if (m_checkTime.isValid()) {
        json["check_time"] = m_checkTime.toString("HH:mm:ss");
    }

    json["status"] = AttendanceStatusHelper::toInt(m_status);

    if (!m_remark.isEmpty()) {
        json["remark"] = m_remark;
    }

    if (!m_recorderId.isEmpty()) {
        json["recorder_id"] = m_recorderId;
    }

    return json;
}

bool AttendanceRecord::isValid() const
{
    return m_studentId > 0 && m_classId > 0 && m_date.isValid();
}

QString AttendanceRecord::statusDisplayName() const
{
    return AttendanceStatusHelper::displayName(m_status);
}

QString AttendanceRecord::statusColorHex() const
{
    return AttendanceStatusHelper::colorHex(m_status);
}
