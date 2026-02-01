#include "Notification.h"
#include <QJsonDocument>

Notification Notification::fromJson(const QJsonObject &json)
{
    Notification notification;
    notification.m_id = json["id"].toString();
    notification.m_type = stringToType(json["type"].toString());
    notification.m_title = json["title"].toString();
    notification.m_content = json["content"].toString();
    notification.m_senderId = json["sender_id"].toString();
    notification.m_receiverId = json["receiver_id"].toString();
    notification.m_createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    notification.m_isRead = json["is_read"].toBool(false);
    return notification;
}

QJsonObject Notification::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["type"] = typeToString(m_type);
    json["title"] = m_title;
    json["content"] = m_content;
    json["sender_id"] = m_senderId;
    json["receiver_id"] = m_receiverId;
    json["created_at"] = m_createdAt.toString(Qt::ISODate);
    json["is_read"] = m_isRead;
    return json;
}

bool Notification::isValid() const
{
    return !m_id.isEmpty() && !m_title.isEmpty();
}

QString Notification::typeToString(NotificationType type)
{
    switch (type) {
        case NotificationType::HomeworkSubmission: return "homework_submission";
        case NotificationType::LeaveApproval: return "leave_approval";
        case NotificationType::GradeRelease: return "grade_release";
        case NotificationType::SystemAnnouncement: return "system_announcement";
    }
    return "system_announcement";
}

NotificationType Notification::stringToType(const QString &typeStr)
{
    if (typeStr == "homework_submission") return NotificationType::HomeworkSubmission;
    if (typeStr == "leave_approval") return NotificationType::LeaveApproval;
    if (typeStr == "grade_release") return NotificationType::GradeRelease;
    return NotificationType::SystemAnnouncement;
}

QString Notification::typeDisplayName() const
{
    switch (m_type) {
        case NotificationType::HomeworkSubmission: return "作业提交";
        case NotificationType::LeaveApproval: return "请假审批";
        case NotificationType::GradeRelease: return "成绩发布";
        case NotificationType::SystemAnnouncement: return "系统公告";
    }
    return "系统公告";
}
