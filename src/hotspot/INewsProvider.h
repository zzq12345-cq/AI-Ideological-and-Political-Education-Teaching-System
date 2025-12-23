#ifndef INEWSPROVIDER_H
#define INEWSPROVIDER_H

#include <QObject>
#include <QList>
#include "NewsItem.h"

/**
 * @brief 新闻提供者抽象接口
 * 
 * 这个接口定义了获取新闻数据的标准方法。
 * 未来对接外部新闻库时，只需实现此接口即可。
 * 
 * 使用方式：
 * 1. 继承 INewsProvider
 * 2. 实现 fetchHotNews、searchNews、fetchNewsDetail 方法
 * 3. 在方法内调用外部库获取数据
 * 4. 通过信号发送结果
 */
class INewsProvider : public QObject {
    Q_OBJECT
    
public:
    explicit INewsProvider(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~INewsProvider() = default;
    
    /**
     * @brief 获取热点新闻列表
     * @param limit 返回数量上限
     * @param category 分类筛选（空字符串表示全部）
     */
    virtual void fetchHotNews(int limit = 20, const QString &category = QString()) = 0;
    
    /**
     * @brief 搜索新闻
     * @param keyword 搜索关键词
     */
    virtual void searchNews(const QString &keyword) = 0;
    
    /**
     * @brief 获取新闻详情
     * @param newsId 新闻ID
     */
    virtual void fetchNewsDetail(const QString &newsId) = 0;
    
    /**
     * @brief 刷新数据（重新获取最新热点）
     */
    virtual void refresh() = 0;
    
signals:
    /**
     * @brief 新闻列表获取成功
     * @param newsList 新闻列表
     */
    void newsListReceived(const QList<NewsItem> &newsList);
    
    /**
     * @brief 新闻详情获取成功
     * @param news 新闻详情
     */
    void newsDetailReceived(const NewsItem &news);
    
    /**
     * @brief 发生错误
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);
    
    /**
     * @brief 开始加载
     */
    void loadingStarted();
    
    /**
     * @brief 加载完成
     */
    void loadingFinished();
};

#endif // INEWSPROVIDER_H
