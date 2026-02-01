#ifndef RADARCHARTWIDGET_H
#define RADARCHARTWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 雷达图组件
 *
 * 自定义绘制的雷达图，用于展示知识点掌握度
 * 老王说：Qt Charts没有雷达图，只能自己画一个
 */
class RadarChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RadarChartWidget(QWidget *parent = nullptr);

    // 设置数据：<名称, 数值(0-100)>
    void setData(const QVector<QPair<QString, double>> &data);

    // 设置颜色
    void setFillColor(const QColor &color);
    void setLineColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBackground(QPainter &painter, const QPointF &center, double radius);
    void drawAxes(QPainter &painter, const QPointF &center, double radius);
    void drawData(QPainter &painter, const QPointF &center, double radius);
    void drawLabels(QPainter &painter, const QPointF &center, double radius);

    QVector<QPair<QString, double>> m_data;
    QColor m_fillColor;
    QColor m_lineColor;
};

#endif // RADARCHARTWIDGET_H
