#ifndef HOTSPOTSERVICE_H
#define HOTSPOTSERVICE_H

#include <QObject>
#include <QList>
#include "../hotspot/NewsItem.h"

// 前向声明
class INewsProvider;
class DifyService;

/**
 * @brief 热点追踪服务
 * 
 * 业务逻辑层，负责：
 * - 管理新闻提供者
 * - 缓存新闻数据
 * - 与 AI 服务联动生成教学内容
 */
class HotspotService : public QObject {
    Q_OBJECT
    
public:
    explicit HotspotService(QObject *parent = nullptr);
    ~HotspotService();
    
    /**
     * @brief 设置新闻提供者（依赖注入）
     * 
     * 未来对接外部新闻库时，只需替换这个提供者即可：
     * @code
     * hotspotService->setNewsProvider(new YourNewsProvider(yourLib));
     * @endcode
     */
    void setNewsProvider(INewsProvider *provider);
    
    /**
     * @brief 获取当前新闻提供者
     */
    INewsProvider* newsProvider() const { return m_newsProvider; }
    
    /**
     * @brief 刷新热点新闻
     * @param category 分类筛选（空字符串表示全部）
     */
    void refreshHotNews(const QString &category = QString());
    
    /**
     * @brief 搜索新闻
     * @param keyword 搜索关键词
     */
    void searchNews(const QString &keyword);
    
    /**
     * @brief 获取新闻详情
     * @param newsId 新闻ID
     */
    void fetchNewsDetail(const QString &newsId);
    
    /**
     * @brief 使用 AI 生成教学内容
     * @param news 新闻数据
     * @param difyService Dify AI 服务实例
     */
    void generateTeachingContent(const NewsItem &news, DifyService *difyService);
    
    /**
     * @brief 获取缓存的新闻列表
     */
    QList<NewsItem> cachedNews() const { return m_cachedNews; }
    
    /**
     * @brief 获取可用分类
     */
    QStringList availableCategories() const;
    
signals:
    /**
     * @brief 热点新闻更新
     */
    void hotNewsUpdated(const QList<NewsItem> &newsList);
    
    /**
     * @brief 新闻详情获取成功
     */
    void newsDetailReceived(const NewsItem &news);
    
    /**
     * @brief AI 生成的教学内容
     */
    void teachingContentGenerated(const QString &content);
    
    /**
     * @brief 发生错误
     */
    void errorOccurred(const QString &error);
    
    /**
     * @brief 加载状态变化
     */
    void loadingStateChanged(bool isLoading);
    
private slots:
    void onNewsListReceived(const QList<NewsItem> &newsList);
    void onNewsDetailReceived(const NewsItem &news);
    void onProviderError(const QString &error);
    
private:
    INewsProvider *m_newsProvider;
    QList<NewsItem> m_cachedNews;
    bool m_ownsProvider;  // 是否拥有 provider 的所有权
};

#endif // HOTSPOTSERVICE_H
