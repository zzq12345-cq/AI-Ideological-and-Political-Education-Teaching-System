#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include "../models/Notification.h"

class NotificationService;

/**
 * @brief 通知中心弹窗组件
 * 显示通知列表，支持标记已读、删除等操作
 * 老王说：不要搞AI风格那些花里胡哨的，简洁专业
 */
class NotificationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit NotificationWidget(NotificationService *service, QWidget *parent = nullptr);

    // 刷新通知列表
    void refresh();

    // 显示/隐藏弹窗
    void showPopup();
    void hidePopup();
    bool isPopupVisible() const { return isVisible(); }

signals:
    // 点击通知时发出（可用于跳转到相关页面）
    void notificationClicked(const Notification &notification);
    // 弹窗关闭时发出
    void popupClosed();

private slots:
    void onNotificationsReceived(const QList<Notification> &notifications);
    void onMarkAllAsRead();
    void onNotificationItemClicked(const QString &notificationId);
    void onDeleteNotification(const QString &notificationId);

private:
    void setupUI();
    void setupConnections();
    QWidget *createNotificationItem(const Notification &notification);
    void updateNotificationList(const QList<Notification> &notifications);
    QString getTypeIconPath(NotificationType type) const;
    QString getTypeColor(NotificationType type) const;
    QString formatTime(const QDateTime &dateTime) const;

    // UI 组件
    QLabel *m_titleLabel = nullptr;
    QLabel *m_countLabel = nullptr;
    QPushButton *m_markAllReadButton = nullptr;
    QPushButton *m_closeButton = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QLabel *m_emptyLabel = nullptr;

    // 服务
    NotificationService *m_service = nullptr;

    // 缓存的通知列表
    QList<Notification> m_notifications;

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // NOTIFICATIONWIDGET_H
