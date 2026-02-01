#include "NotificationBadge.h"
#include <QPainter>
#include "../../shared/StyleConfig.h"

NotificationBadge::NotificationBadge(QWidget *parent)
    : QLabel(parent)
    , m_count(0)
{
    setFixedSize(18, 18);
    setAlignment(Qt::AlignCenter);
    hide();  // 默认隐藏
}

void NotificationBadge::setCount(int count)
{
    m_count = count;
    if (count <= 0) {
        hide();
    } else {
        show();
        if (count > 99) {
            setText("99+");
            setFixedWidth(26);
        } else {
            setText(QString::number(count));
            setFixedWidth(18);
        }
    }
    update();
}

void NotificationBadge::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (m_count <= 0) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制红色圆形背景
    painter.setBrush(QColor(StyleConfig::PATRIOTIC_RED));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), height() / 2, height() / 2);

    // 绘制白色文字
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(10);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, text());
}
