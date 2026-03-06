#ifndef LAYOUTUTILS_H
#define LAYOUTUTILS_H

#include <QLayout>
#include <QLayoutItem>
#include <QWidget>

namespace LayoutUtils {

/**
 * @brief 清除布局中的所有子项（widget + spacer）
 * @param layout 要清除的布局
 * @param keepStretch 如果 true，保留最后一个 stretch item
 */
inline void clearLayout(QLayout *layout, bool keepStretch = false)
{
    if (!layout) return;

    int stopAt = keepStretch ? 1 : 0;
    while (layout->count() > stopAt) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

} // namespace LayoutUtils

#endif // LAYOUTUTILS_H
