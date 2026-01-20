#ifndef NEWSITEM_H
#define NEWSITEM_H

#include <QString>
#include <QDateTime>
#include <QStringList>

/**
 * @brief 新闻数据结构
 * 用于存储政治热点新闻的基本信息
 */
struct NewsItem {
    QString id;              // 唯一标识
    QString title;           // 标题
    QString summary;         // 摘要
    QString content;         // 完整内容
    QString source;          // 来源
    QString category;        // 分类（国内、国外）
    QString imageUrl;        // 封面图片 URL
    QString url;             // 新闻原文链接
    QDateTime publishTime;   // 发布时间
    int hotScore;            // 热度评分 (0-100)
    QStringList keywords;    // 关键词标签
    
    NewsItem() : hotScore(0) {}
    
    bool isValid() const {
        return !id.isEmpty() && !title.isEmpty();
    }
};

#endif // NEWSITEM_H
