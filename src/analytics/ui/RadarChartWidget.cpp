#include "RadarChartWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

RadarChartWidget::RadarChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_fillColor(QColor(225, 29, 72, 80))  // 红色半透明
    , m_lineColor(QColor(225, 29, 72))       // 红色
{
    setMinimumSize(200, 200);
}

void RadarChartWidget::setData(const QVector<QPair<QString, double>> &data)
{
    m_data = data;
    update();
}

void RadarChartWidget::setFillColor(const QColor &color)
{
    m_fillColor = color;
    update();
}

void RadarChartWidget::setLineColor(const QColor &color)
{
    m_lineColor = color;
    update();
}

void RadarChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 计算中心和半径
    QPointF center(width() / 2.0, height() / 2.0);
    double radius = qMin(width(), height()) / 2.0 - 60;  // 留出标签空间

    if (m_data.isEmpty() || radius <= 0) {
        return;
    }

    drawBackground(painter, center, radius);
    drawAxes(painter, center, radius);
    drawData(painter, center, radius);
    drawLabels(painter, center, radius);
}

void RadarChartWidget::drawBackground(QPainter &painter, const QPointF &center, double radius)
{
    int n = m_data.size();
    double angleStep = 2 * M_PI / n;

    // 绘制5层同心多边形
    painter.setPen(QPen(QColor("#E2E8F0"), 1));
    for (int level = 1; level <= 5; ++level) {
        double r = radius * level / 5.0;
        QPolygonF polygon;
        for (int i = 0; i < n; ++i) {
            double angle = -M_PI / 2 + i * angleStep;  // 从顶部开始
            double x = center.x() + r * qCos(angle);
            double y = center.y() + r * qSin(angle);
            polygon << QPointF(x, y);
        }
        polygon << polygon.first();  // 闭合
        painter.drawPolyline(polygon);
    }
}

void RadarChartWidget::drawAxes(QPainter &painter, const QPointF &center, double radius)
{
    int n = m_data.size();
    double angleStep = 2 * M_PI / n;

    painter.setPen(QPen(QColor("#CBD5E0"), 1));
    for (int i = 0; i < n; ++i) {
        double angle = -M_PI / 2 + i * angleStep;
        double x = center.x() + radius * qCos(angle);
        double y = center.y() + radius * qSin(angle);
        painter.drawLine(center, QPointF(x, y));
    }
}

void RadarChartWidget::drawData(QPainter &painter, const QPointF &center, double radius)
{
    if (m_data.isEmpty()) return;

    int n = m_data.size();
    double angleStep = 2 * M_PI / n;

    QPolygonF polygon;
    for (int i = 0; i < n; ++i) {
        double value = qBound(0.0, m_data[i].second, 100.0);
        double r = radius * value / 100.0;
        double angle = -M_PI / 2 + i * angleStep;
        double x = center.x() + r * qCos(angle);
        double y = center.y() + r * qSin(angle);
        polygon << QPointF(x, y);
    }
    polygon << polygon.first();  // 闭合

    // 填充
    painter.setBrush(m_fillColor);
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(polygon);

    // 描边
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(m_lineColor, 2));
    painter.drawPolyline(polygon);

    // 绘制数据点
    painter.setBrush(m_lineColor);
    for (int i = 0; i < polygon.size() - 1; ++i) {
        painter.drawEllipse(polygon[i], 4, 4);
    }
}

void RadarChartWidget::drawLabels(QPainter &painter, const QPointF &center, double radius)
{
    if (m_data.isEmpty()) return;

    int n = m_data.size();
    double angleStep = 2 * M_PI / n;
    double labelRadius = radius + 25;

    painter.setPen(QColor("#4A5568"));
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    for (int i = 0; i < n; ++i) {
        double angle = -M_PI / 2 + i * angleStep;
        double x = center.x() + labelRadius * qCos(angle);
        double y = center.y() + labelRadius * qSin(angle);

        QString label = m_data[i].first;
        // 截断过长的标签
        if (label.length() > 6) {
            label = label.left(5) + "...";
        }

        QRectF textRect(x - 40, y - 10, 80, 20);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }
}
