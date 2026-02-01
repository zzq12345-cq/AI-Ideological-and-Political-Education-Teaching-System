#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

/**
 * @brief 通知类型枚举
 * 老王说：4种通知类型，简单明了
 */
enum class NotificationType {
    HomeworkSubmission = 0,  // 作业提交
    LeaveApproval = 1,       // 请假审批
    GradeRelease = 2,        // 成绩发布
    SystemAnnouncement = 3   // 系统公告
};

/**
 * @brief 通知数据模型
 * 老王说：参考Student模式，fromJson/toJson一把梭
 */
class Notification
{
public:
    Notification() = default;
    ~Notification() = default;

    // 序列化方法
    static Notification fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
    bool isValid() const;

    // Getters
    QString id() const { return m_id; }
    NotificationType type() const { return m_type; }
    QString title() const { return m_title; }
    QString content() const { return m_content; }
    QString senderId() const { return m_senderId; }
    QString receiverId() const { return m_receiverId; }
    QDateTime createdAt() const { return m_createdAt; }
    bool isRead() const { return m_isRead; }

    // Setters
    void setId(const QString &id) { m_id = id; }
    void setType(NotificationType type) { m_type = type; }
    void setTitle(const QString &title) { m_title = title; }
    void setContent(const QString &content) { m_content = content; }
    void setSenderId(const QString &senderId) { m_senderId = senderId; }
    void setReceiverId(const QString &receiverId) { m_receiverId = receiverId; }
    void setCreatedAt(const QDateTime &createdAt) { m_createdAt = createdAt; }
    void setIsRead(bool isRead) { m_isRead = isRead; }

    // 辅助方法
    static QString typeToString(NotificationType type);
    static NotificationType stringToType(const QString &typeStr);
    QString typeDisplayName() const;

private:
    QString m_id;
    NotificationType m_type = NotificationType::SystemAnnouncement;
    QString m_title;
    QString m_content;
    QString m_senderId;
    QString m_receiverId;
    QDateTime m_createdAt;
    bool m_isRead = false;
};

#endif // NOTIFICATION_H
