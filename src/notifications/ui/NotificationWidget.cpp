#include "NotificationWidget.h"
#include "../NotificationService.h"
#include "../../shared/StyleConfig.h"
#include <QApplication>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>

namespace {
constexpr int kWidgetWidth = 380;
constexpr int kWidgetHeight = 500;
constexpr int kItemMinHeight = 72;

void applyShadow(QWidget *target, qreal blurRadius, const QPointF &offset, const QColor &color)
{
    if (!target) return;
    auto *shadow = new QGraphicsDropShadowEffect(target);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(offset);
    shadow->setColor(color);
    target->setGraphicsEffect(shadow);
}
}

NotificationWidget::NotificationWidget(NotificationService *service, QWidget *parent)
    : QFrame(parent)
    , m_service(service)
{
    setObjectName("notificationWidget");
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kWidgetWidth, kWidgetHeight);

    setupUI();
    setupConnections();
}

void NotificationWidget::setupUI()
{
    // 主容器
    auto *container = new QFrame(this);
    container->setObjectName("notificationContainer");
    container->setStyleSheet(
        "QFrame#notificationContainer {"
        "  background: #FFFFFF;"
        "  border-radius: 16px;"
        "  border: 1px solid #E5E7EB;"
        "}"
    );
    applyShadow(container, 30, QPointF(0, 10), QColor(0, 0, 0, 40));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // ========== 头部区域 ==========
    auto *headerFrame = new QFrame(container);
    headerFrame->setObjectName("notificationHeader");
    headerFrame->setFixedHeight(56);
    headerFrame->setStyleSheet(
        "QFrame#notificationHeader {"
        "  background: #FAFAFA;"
        "  border-top-left-radius: 16px;"
        "  border-top-right-radius: 16px;"
        "  border-bottom: 1px solid #E5E7EB;"
        "}"
    );

    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 0, 16, 0);
    headerLayout->setSpacing(12);

    // 标题
    m_titleLabel = new QLabel("通知中心", headerFrame);
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  color: #1A1A1A;"
        "  background: transparent;"
        "}"
    );

    // 未读数量标签
    m_countLabel = new QLabel("", headerFrame);
    m_countLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 13px;"
        "  color: #6B7280;"
        "  background: transparent;"
        "}"
    );

    // 全部已读按钮
    m_markAllReadButton = new QPushButton("全部已读", headerFrame);
    m_markAllReadButton->setObjectName("markAllReadBtn");
    m_markAllReadButton->setCursor(Qt::PointingHandCursor);
    m_markAllReadButton->setStyleSheet(
        "QPushButton#markAllReadBtn {"
        "  background: transparent;"
        "  color: " + QString(StyleConfig::PATRIOTIC_RED) + ";"
        "  border: none;"
        "  font-size: 13px;"
        "  padding: 4px 8px;"
        "}"
        "QPushButton#markAllReadBtn:hover {"
        "  background: " + QString(StyleConfig::PATRIOTIC_RED_LIGHT) + ";"
        "  border-radius: 4px;"
        "}"
        "QPushButton#markAllReadBtn:disabled {"
        "  color: #9CA3AF;"
        "}"
    );

    // 关闭按钮
    m_closeButton = new QPushButton("×", headerFrame);
    m_closeButton->setObjectName("closeNotificationBtn");
    m_closeButton->setFixedSize(28, 28);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setStyleSheet(
        "QPushButton#closeNotificationBtn {"
        "  background: transparent;"
        "  color: #9CA3AF;"
        "  border: none;"
        "  font-size: 20px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#closeNotificationBtn:hover {"
        "  color: #1A1A1A;"
        "  background: #F3F4F6;"
        "  border-radius: 14px;"
        "}"
    );

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_countLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_markAllReadButton);
    headerLayout->addWidget(m_closeButton);

    containerLayout->addWidget(headerFrame);

    // ========== 通知列表区域 ==========
    m_scrollArea = new QScrollArea(container);
    m_scrollArea->setObjectName("notificationScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea#notificationScrollArea {"
        "  background: #FFFFFF;"
        "  border: none;"
        "}"
        "QScrollArea#notificationScrollArea > QWidget > QWidget {"
        "  background: #FFFFFF;"
        "}"
        "QScrollBar:vertical {"
        "  background: #F5F5F5;"
        "  width: 6px;"
        "  border-radius: 3px;"
        "  margin: 4px 2px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #D1D5DB;"
        "  border-radius: 3px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #9CA3AF;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );

    auto *scrollContent = new QWidget(m_scrollArea);
    scrollContent->setObjectName("notificationScrollContent");
    scrollContent->setStyleSheet("background: #FFFFFF;");

    m_listLayout = new QVBoxLayout(scrollContent);
    m_listLayout->setContentsMargins(12, 8, 12, 8);
    m_listLayout->setSpacing(8);
    m_listLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    containerLayout->addWidget(m_scrollArea, 1);

    // ========== 空状态提示 ==========
    m_emptyLabel = new QLabel("暂无通知", m_scrollArea);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 14px;"
        "  color: #9CA3AF;"
        "  padding: 40px;"
        "  background: transparent;"
        "}"
    );
    m_emptyLabel->hide();
}

void NotificationWidget::setupConnections()
{
    // 关闭按钮
    connect(m_closeButton, &QPushButton::clicked, this, &NotificationWidget::hidePopup);

    // 全部已读
    connect(m_markAllReadButton, &QPushButton::clicked, this, &NotificationWidget::onMarkAllAsRead);

    // 监听服务层信号
    if (m_service) {
        connect(m_service, &NotificationService::notificationsReceived,
                this, &NotificationWidget::onNotificationsReceived);
        connect(m_service, &NotificationService::loadingStateChanged,
                this, [this](bool loading) {
            // 可以在这里添加加载状态UI
            Q_UNUSED(loading)
        });
    }
}

void NotificationWidget::showPopup()
{
    if (m_service) {
        m_service->fetchNotifications();
    }
    show();
    raise();
}

void NotificationWidget::hidePopup()
{
    qApp->removeEventFilter(this);
    hide();
    emit popupClosed();
}

void NotificationWidget::refresh()
{
    if (m_service) {
        m_service->fetchNotifications();
    }
}

void NotificationWidget::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);
    // 安装全局事件过滤器，点击外部时关闭
    qApp->installEventFilter(this);
}

bool NotificationWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = static_cast<QMouseEvent*>(event);

        // 检查是否点击了通知项
        QWidget *widget = qobject_cast<QWidget*>(watched);
        if (widget && widget->property("notificationId").isValid()) {
            QString notificationId = widget->property("notificationId").toString();
            if (!notificationId.isEmpty()) {
                onNotificationItemClicked(notificationId);
                return true;
            }
        }

        // 点击弹窗外部关闭
        if (!geometry().contains(mouseEvent->globalPosition().toPoint())) {
            hidePopup();  // hidePopup内部已经移除事件过滤器
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

void NotificationWidget::onNotificationsReceived(const QList<Notification> &notifications)
{
    m_notifications = notifications;
    updateNotificationList(notifications);

    // 更新未读数量
    int unreadCount = 0;
    for (const auto &n : notifications) {
        if (!n.isRead()) {
            unreadCount++;
        }
    }

    if (unreadCount > 0) {
        m_countLabel->setText(QString("(%1 条未读)").arg(unreadCount));
        m_markAllReadButton->setEnabled(true);
    } else {
        m_countLabel->setText("");
        m_markAllReadButton->setEnabled(false);
    }
}

void NotificationWidget::onMarkAllAsRead()
{
    if (m_service) {
        m_service->markAllAsRead();
    }
}

void NotificationWidget::onNotificationItemClicked(const QString &notificationId)
{
    // 查找通知
    for (const auto &n : m_notifications) {
        if (n.id() == notificationId) {
            // 标记为已读
            if (!n.isRead() && m_service) {
                m_service->markAsRead(notificationId);
            }
            emit notificationClicked(n);
            break;
        }
    }
}

void NotificationWidget::onDeleteNotification(const QString &notificationId)
{
    if (m_service) {
        m_service->deleteNotification(notificationId);
    }
}

void NotificationWidget::updateNotificationList(const QList<Notification> &notifications)
{
    // 清空现有列表
    while (m_listLayout->count() > 1) {  // 保留 stretch
        QLayoutItem *item = m_listLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // 显示空状态或列表
    if (notifications.isEmpty()) {
        m_emptyLabel->show();
        m_emptyLabel->raise();
        return;
    }

    m_emptyLabel->hide();

    // 添加通知项
    for (const auto &notification : notifications) {
        QWidget *item = createNotificationItem(notification);
        m_listLayout->insertWidget(m_listLayout->count() - 1, item);
    }
}

QWidget *NotificationWidget::createNotificationItem(const Notification &notification)
{
    auto *item = new QFrame(this);
    item->setObjectName("notificationItem");
    item->setMinimumHeight(kItemMinHeight);
    item->setCursor(Qt::PointingHandCursor);

    QString bgColor = notification.isRead() ? "#FFFFFF" : "#FFF8F8";
    QString borderColor = notification.isRead() ? "#E5E7EB" : QString(StyleConfig::PATRIOTIC_RED_LIGHT);

    item->setStyleSheet(
        "QFrame#notificationItem {"
        "  background: " + bgColor + ";"
        "  border-radius: 12px;"
        "  border: 1px solid " + borderColor + ";"
        "}"
        "QFrame#notificationItem:hover {"
        "  background: #F9FAFB;"
        "  border-color: #D1D5DB;"
        "}"
    );

    auto *layout = new QHBoxLayout(item);
    layout->setContentsMargins(12, 10, 8, 10);
    layout->setSpacing(10);

    // 左侧：类型图标
    auto *typeIcon = new QLabel(item);
    typeIcon->setFixedSize(36, 36);
    typeIcon->setAlignment(Qt::AlignCenter);
    QString typeColor = getTypeColor(notification.type());
    typeIcon->setStyleSheet(
        "QLabel {"
        "  background: " + typeColor + "20;"  // 20% 透明度
        "  color: " + typeColor + ";"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  border-radius: 8px;"
        "}"
    );
    // 用文字代替图标（简洁风格，不用emoji）
    QString typeText;
    switch (notification.type()) {
        case NotificationType::HomeworkSubmission: typeText = "作"; break;
        case NotificationType::LeaveApproval: typeText = "假"; break;
        case NotificationType::GradeRelease: typeText = "绩"; break;
        case NotificationType::SystemAnnouncement: typeText = "公"; break;
    }
    typeIcon->setText(typeText);

    // 中间：内容区域
    auto *contentFrame = new QFrame(item);
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);

    // 标题行
    auto *titleRow = new QHBoxLayout();
    titleRow->setSpacing(8);

    auto *titleLabel = new QLabel(notification.title(), contentFrame);
    titleLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  color: #1A1A1A;"
        "  background: transparent;"
        "}"
    );
    titleLabel->setWordWrap(false);

    // 未读标记（小红点）
    auto *unreadDot = new QLabel(contentFrame);
    unreadDot->setFixedSize(8, 8);
    if (!notification.isRead()) {
        unreadDot->setStyleSheet(
            "QLabel {"
            "  background: " + QString(StyleConfig::PATRIOTIC_RED) + ";"
            "  border-radius: 4px;"
            "}"
        );
    } else {
        unreadDot->hide();
    }

    titleRow->addWidget(titleLabel, 1);
    titleRow->addWidget(unreadDot);
    contentLayout->addLayout(titleRow);

    // 内容预览
    QString preview = notification.content();
    if (preview.length() > 50) {
        preview = preview.left(50) + "...";
    }
    auto *contentLabel = new QLabel(preview, contentFrame);
    contentLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 13px;"
        "  color: #6B7280;"
        "  background: transparent;"
        "}"
    );
    contentLabel->setWordWrap(true);
    contentLayout->addWidget(contentLabel);

    // 时间
    auto *timeLabel = new QLabel(formatTime(notification.createdAt()), contentFrame);
    timeLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 11px;"
        "  color: #9CA3AF;"
        "  background: transparent;"
        "}"
    );
    contentLayout->addWidget(timeLabel);

    layout->addWidget(typeIcon);
    layout->addWidget(contentFrame, 1);

    // 右侧：删除按钮
    auto *deleteBtn = new QPushButton("×", item);
    deleteBtn->setObjectName("deleteNotificationBtn");
    deleteBtn->setFixedSize(24, 24);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet(
        "QPushButton#deleteNotificationBtn {"
        "  background: transparent;"
        "  color: #D1D5DB;"
        "  border: none;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#deleteNotificationBtn:hover {"
        "  color: " + QString(StyleConfig::PATRIOTIC_RED) + ";"
        "}"
    );
    layout->addWidget(deleteBtn, 0, Qt::AlignTop);

    // 连接信号
    QString notificationId = notification.id();

    // 点击整个item
    item->installEventFilter(this);
    item->setProperty("notificationId", notificationId);

    // 使用QFrame的鼠标事件来处理点击
    connect(deleteBtn, &QPushButton::clicked, this, [this, notificationId]() {
        onDeleteNotification(notificationId);
    });

    return item;
}

QString NotificationWidget::getTypeIconPath(NotificationType type) const
{
    // 预留给未来SVG图标使用
    switch (type) {
        case NotificationType::HomeworkSubmission: return ":/icons/homework.svg";
        case NotificationType::LeaveApproval: return ":/icons/leave.svg";
        case NotificationType::GradeRelease: return ":/icons/grade.svg";
        case NotificationType::SystemAnnouncement: return ":/icons/announcement.svg";
    }
    return ":/icons/notification.svg";
}

QString NotificationWidget::getTypeColor(NotificationType type) const
{
    switch (type) {
        case NotificationType::HomeworkSubmission: return StyleConfig::INFO_BLUE;
        case NotificationType::LeaveApproval: return StyleConfig::WARNING_ORANGE;
        case NotificationType::GradeRelease: return StyleConfig::SUCCESS_GREEN;
        case NotificationType::SystemAnnouncement: return StyleConfig::PATRIOTIC_RED;
    }
    return StyleConfig::TEXT_SECONDARY;
}

QString NotificationWidget::formatTime(const QDateTime &dateTime) const
{
    if (!dateTime.isValid()) {
        return "";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = dateTime.secsTo(now);

    if (secs < 60) {
        return "刚刚";
    } else if (secs < 3600) {
        return QString("%1分钟前").arg(secs / 60);
    } else if (secs < 86400) {
        return QString("%1小时前").arg(secs / 3600);
    } else if (secs < 604800) {
        return QString("%1天前").arg(secs / 86400);
    } else {
        return dateTime.toString("MM-dd HH:mm");
    }
}
