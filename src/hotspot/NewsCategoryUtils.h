#ifndef NEWSCATEGORYUTILS_H
#define NEWSCATEGORYUTILS_H

#include <QList>
#include <QString>
#include <QStringList>

#include "NewsItem.h"

namespace NewsCategoryUtils {

QString normalizeCategory(const QString &category);
QStringList allCategories();
bool isRemoteCategory(const QString &category);
QList<NewsItem> filterNewsByCategory(const QList<NewsItem> &items, const QString &category);
QList<NewsItem> searchNews(const QList<NewsItem> &items, const QString &keyword);

} // namespace NewsCategoryUtils

#endif // NEWSCATEGORYUTILS_H
