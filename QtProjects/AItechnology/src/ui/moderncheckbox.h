#pragma once

#include <QCheckBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

class ModernCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    explicit ModernCheckBox(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawCheckmark(QPainter &painter, const QRect &rect);

    static constexpr int CHECKBOX_SIZE = 20;
    static constexpr int BORDER_RADIUS = 4;
    static constexpr QColor UNCHECKED_BORDER = QColor(191, 200, 210);  // #BFC8D2
    static constexpr QColor CHECKED_BORDER = QColor(42, 123, 255);     // #2A7BFF
    static constexpr QColor CHECKED_CHECKMARK = QColor(42, 123, 255);  // #2A7BFF
};