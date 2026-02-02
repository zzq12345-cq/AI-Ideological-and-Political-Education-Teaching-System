#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "models/Notification.h"

/**
 * @brief 通知服务层
 * 老王说：参考HotspotService模式，Supabase REST API一把梭
 */
class NotificationService : public QObject
{
    Q_OBJECT

public:
    explicit NotificationService(QObject *parent = nullptr);
    ~NotificationService();

    // 业务方法
    void fetchNotifications();
    void fetchUnreadCount();
    void markAsRead(const QString &notificationId);
    void markAllAsRead();
    void deleteNotification(const QString &notificationId);

    // 设置当前用户ID
    void setCurrentUserId(const QString &userId);
    QString currentUserId() const { return m_currentUserId; }

    // 获取缓存的通知列表
    QList<Notification> notifications() const { return m_notifications; }
    int unreadCount() const { return m_unreadCount; }

signals:
    void notificationsReceived(const QList<Notification> &notifications);
    void unreadCountChanged(int count);
    void notificationMarkedRead(const QString &id);
    void errorOccurred(const QString &error);
    void loadingStateChanged(bool isLoading);

private slots:
    void onFetchNotificationsFinished();
    void onFetchUnreadCountFinished();
    void onMarkAsReadFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QNetworkRequest createRequest(const QString &endpoint) const;
    void loadSampleNotifications();  // 加载示例通知数据

    QNetworkAccessManager *m_networkManager;
    QString m_currentUserId;
    QList<Notification> m_notifications;
    int m_unreadCount = 0;
    bool m_isLoading = false;
};

#endif // NOTIFICATIONSERVICE_H
