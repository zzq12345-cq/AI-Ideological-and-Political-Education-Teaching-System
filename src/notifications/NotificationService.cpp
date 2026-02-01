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
        emit errorOccurred(reply->errorString());
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

    qDebug() << "[NotificationService] 获取通知成功，共" << m_notifications.size() << "条，未读" << m_unreadCount << "条";
    emit notificationsReceived(m_notifications);
    emit unreadCountChanged(m_unreadCount);

    reply->deleteLater();
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
