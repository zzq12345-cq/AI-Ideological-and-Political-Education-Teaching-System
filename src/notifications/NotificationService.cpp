#include "NotificationService.h"
#include "../auth/supabase/supabaseconfig.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

NotificationService::NotificationService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_unreadCount(0)
    , m_isLoading(false)
{
}

NotificationService::~NotificationService()
{
}

void NotificationService::setCurrentUserId(const QString &userId)
{
    m_currentUserId = userId;
}

QNetworkRequest NotificationService::createRequest(const QString &endpoint) const
{
    QUrl url(SupabaseConfig::SUPABASE_URL + endpoint);
    QNetworkRequest request(url);
    request.setRawHeader("apikey", SupabaseConfig::SUPABASE_ANON_KEY.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + SupabaseConfig::SUPABASE_ANON_KEY).toUtf8());
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Prefer", "return=representation");

    // 禁用HTTP/2，避免macOS上的网络问题
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    return request;
}

void NotificationService::fetchNotifications()
{
    if (m_currentUserId.isEmpty()) {
        qWarning() << "[NotificationService] 用户ID未设置，无法获取通知";
        return;
    }

    m_isLoading = true;
    emit loadingStateChanged(true);

    QString endpoint = QString("/rest/v1/notifications?receiver_id=eq.%1&order=created_at.desc&limit=50")
                           .arg(m_currentUserId);
    QNetworkRequest request = createRequest(endpoint);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &NotificationService::onFetchNotificationsFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &NotificationService::onNetworkError);
}

void NotificationService::onFetchNotificationsFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    m_isLoading = false;
    emit loadingStateChanged(false);

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[NotificationService] 获取通知失败:" << reply->errorString();
        // 网络错误时使用示例数据
        loadSampleNotifications();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();

    m_notifications.clear();
    m_unreadCount = 0;

    for (const QJsonValue &val : array) {
        Notification notification = Notification::fromJson(val.toObject());
        m_notifications.append(notification);
        if (!notification.isRead()) {
            m_unreadCount++;
        }
    }

    // 如果没有获取到数据，使用示例数据
    if (m_notifications.isEmpty()) {
        loadSampleNotifications();
    } else {
        qDebug() << "[NotificationService] 获取通知成功，共" << m_notifications.size() << "条，未读" << m_unreadCount << "条";
        emit notificationsReceived(m_notifications);
        emit unreadCountChanged(m_unreadCount);
    }

    reply->deleteLater();
}

void NotificationService::loadSampleNotifications()
{
    m_notifications.clear();
    m_unreadCount = 0;

    // 示例通知数据
    Notification n1;
    n1.setId("sample-1");
    n1.setType(NotificationType::HomeworkSubmission);
    n1.setTitle("作业提交提醒");
    n1.setContent("七年级(3)班 张小明 同学提交了《道德与法治》第一单元作业，请及时批改。");
    n1.setCreatedAt(QDateTime::currentDateTime().addSecs(-1800));  // 30分钟前
    n1.setIsRead(false);
    m_notifications.append(n1);
    m_unreadCount++;

    Notification n2;
    n2.setId("sample-2");
    n2.setType(NotificationType::SystemAnnouncement);
    n2.setTitle("系统更新通知");
    n2.setContent("AI智慧课堂系统已升级至v2.0版本，新增教案编辑器功能，支持AI一键生成教案。");
    n2.setCreatedAt(QDateTime::currentDateTime().addSecs(-7200));  // 2小时前
    n2.setIsRead(false);
    m_notifications.append(n2);
    m_unreadCount++;

    Notification n3;
    n3.setId("sample-3");
    n3.setType(NotificationType::GradeRelease);
    n3.setTitle("期中考试成绩已发布");
    n3.setContent("2024-2025学年第一学期期中考试成绩已发布，请登录系统查看班级学情分析报告。");
    n3.setCreatedAt(QDateTime::currentDateTime().addSecs(-86400));  // 1天前
    n3.setIsRead(false);
    m_notifications.append(n3);
    m_unreadCount++;

    Notification n4;
    n4.setId("sample-4");
    n4.setType(NotificationType::LeaveApproval);
    n4.setTitle("请假申请待审批");
    n4.setContent("八年级(1)班 李小红 同学申请病假2天（1月15日-1月16日），请审批。");
    n4.setCreatedAt(QDateTime::currentDateTime().addSecs(-172800));  // 2天前
    n4.setIsRead(true);
    m_notifications.append(n4);

    Notification n5;
    n5.setId("sample-5");
    n5.setType(NotificationType::HomeworkSubmission);
    n5.setTitle("批量作业提交");
    n5.setContent("七年级(3)班有15名同学提交了《法律在我们身边》课后作业，提交率达到88%。");
    n5.setCreatedAt(QDateTime::currentDateTime().addSecs(-259200));  // 3天前
    n5.setIsRead(true);
    m_notifications.append(n5);

    qDebug() << "[NotificationService] 加载示例通知，共" << m_notifications.size() << "条，未读" << m_unreadCount << "条";
    emit notificationsReceived(m_notifications);
    emit unreadCountChanged(m_unreadCount);
}

void NotificationService::fetchUnreadCount()
{
    if (m_currentUserId.isEmpty()) return;

    QString endpoint = QString("/rest/v1/notifications?receiver_id=eq.%1&is_read=eq.false&select=count")
                           .arg(m_currentUserId);
    QNetworkRequest request = createRequest(endpoint);
    request.setRawHeader("Prefer", "count=exact");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &NotificationService::onFetchUnreadCountFinished);
}

void NotificationService::onFetchUnreadCountFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        // Supabase返回的count在header里
        QString countHeader = reply->rawHeader("content-range");
        if (!countHeader.isEmpty()) {
            // 格式: "0-9/42" 或 "*/42"
            int slashIndex = countHeader.lastIndexOf('/');
            if (slashIndex >= 0) {
                m_unreadCount = countHeader.mid(slashIndex + 1).toInt();
                emit unreadCountChanged(m_unreadCount);
            }
        }
    }

    reply->deleteLater();
}

void NotificationService::markAsRead(const QString &notificationId)
{
    QString endpoint = QString("/rest/v1/notifications?id=eq.%1").arg(notificationId);
    QNetworkRequest request = createRequest(endpoint);

    QJsonObject body;
    body["is_read"] = true;
    QByteArray data = QJsonDocument(body).toJson();

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", data);
    // 设置property以便在回调中获取notificationId
    reply->setProperty("notificationId", notificationId);
    connect(reply, &QNetworkReply::finished, this, &NotificationService::onMarkAsReadFinished);
}

void NotificationService::onMarkAsReadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        // 更新本地缓存
        for (int i = 0; i < m_notifications.size(); ++i) {
            if (m_notifications[i].id() == reply->property("notificationId").toString()) {
                m_notifications[i].setIsRead(true);
                break;
            }
        }
        if (m_unreadCount > 0) {
            m_unreadCount--;
            emit unreadCountChanged(m_unreadCount);
        }
        qDebug() << "[NotificationService] 标记已读成功";
    }

    reply->deleteLater();
}

void NotificationService::markAllAsRead()
{
    if (m_currentUserId.isEmpty()) return;

    QString endpoint = QString("/rest/v1/notifications?receiver_id=eq.%1&is_read=eq.false")
                           .arg(m_currentUserId);
    QNetworkRequest request = createRequest(endpoint);

    QJsonObject body;
    body["is_read"] = true;
    QByteArray data = QJsonDocument(body).toJson();

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", data);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 更新本地缓存
            for (int i = 0; i < m_notifications.size(); ++i) {
                m_notifications[i].setIsRead(true);
            }
            m_unreadCount = 0;
            emit unreadCountChanged(0);
            emit notificationsReceived(m_notifications);
            qDebug() << "[NotificationService] 全部标记已读成功";
        }
        reply->deleteLater();
    });
}

void NotificationService::deleteNotification(const QString &notificationId)
{
    QString endpoint = QString("/rest/v1/notifications?id=eq.%1").arg(notificationId);
    QNetworkRequest request = createRequest(endpoint);

    QNetworkReply *reply = m_networkManager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, notificationId]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 从本地缓存移除
            for (int i = 0; i < m_notifications.size(); ++i) {
                if (m_notifications[i].id() == notificationId) {
                    if (!m_notifications[i].isRead()) {
                        m_unreadCount--;
                        emit unreadCountChanged(m_unreadCount);
                    }
                    m_notifications.removeAt(i);
                    break;
                }
            }
            emit notificationsReceived(m_notifications);
        }
        reply->deleteLater();
    });
}

void NotificationService::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        QString errorMsg = reply->errorString();
        qWarning() << "[NotificationService] 网络错误:" << errorMsg;
        emit errorOccurred(errorMsg);
    }
    m_isLoading = false;
    emit loadingStateChanged(false);
}
