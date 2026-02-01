#ifndef NOTIFICATIONBADGE_H
#define NOTIFICATIONBADGE_H

#include <QLabel>

/**
 * @brief 未读数量小红点组件
 * 老王说：简单的小红点，count=0时隐藏
 */
class NotificationBadge : public QLabel
{
    Q_OBJECT

public:
    explicit NotificationBadge(QWidget *parent = nullptr);

    void setCount(int count);
    int count() const { return m_count; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_count = 0;
};

#endif // NOTIFICATIONBADGE_H
