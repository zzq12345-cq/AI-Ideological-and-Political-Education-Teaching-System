#include "moderncheckbox.h"

ModernCheckBox::ModernCheckBox(QWidget *parent)
    : QCheckBox(parent)
{
    setFixedSize(CHECKBOX_SIZE + 8, CHECKBOX_SIZE + 8);  // 添加边距
    setCursor(Qt::PointingHandCursor);
    setStyleSheet("QCheckBox { background: transparent; border: none; }");
}

void ModernCheckBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制区域（居中）
    QRect checkRect(4, 4, CHECKBOX_SIZE, CHECKBOX_SIZE);

    // 绘制背景
    painter.fillRect(checkRect, Qt::white);

    // 绘制边框
    QPainterPath path;
    path.addRoundedRect(checkRect, BORDER_RADIUS, BORDER_RADIUS);

    if (isChecked()) {
        // 选中状态：蓝色边框 + 勾号
        painter.setPen(QPen(CHECKED_BORDER, 2));
        painter.drawPath(path);
        drawCheckmark(painter, checkRect);
    } else {
        // 未选中状态：灰色边框
        painter.setPen(QPen(UNCHECKED_BORDER, 1.5));
        painter.drawPath(path);
    }
}

void ModernCheckBox::drawCheckmark(QPainter &painter, const QRect &rect)
{
    painter.setPen(QPen(CHECKED_CHECKMARK, 2.5));
    painter.setBrush(Qt::NoBrush);

    // 14px 勾号的坐标（适配20px方框）
    const QPointF points[] = {
        QPointF(rect.left() + 4, rect.center().y()),
        QPointF(rect.left() + 7.5, rect.bottom() - 3),
        QPointF(rect.right() - 3, rect.top() + 4)
    };

    painter.drawLine(points[0], points[1]);
    painter.drawLine(points[1], points[2]);
}