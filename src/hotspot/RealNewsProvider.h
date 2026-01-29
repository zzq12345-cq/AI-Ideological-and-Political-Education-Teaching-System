#ifndef REALNEWSPROVIDER_H
#define REALNEWSPROVIDER_H

#include "INewsProvider.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
 * @brief 真实新闻提供者
 *
 * 对接多个新闻数据源：
 * 1. 今日头条热榜 API - 免费热点新闻（带图片，推荐）
 * 2. 韩小韩 API - 免费热点新闻（无需Key）
 * 3. 天行数据 API - 综合新闻（需Key，图文不匹配）
 * 4. RSS 订阅 - 人民网、新华网（备用）
 */
class RealNewsProvider : public INewsProvider {
    Q_OBJECT

public:
    // 数据源类型
    enum class DataSource {
        TouTiao,     // 今日头条热榜（推荐，图文匹配）
        HanXiaoHan,  // 韩小韩（免费）
        TianXing,    // 天行数据
        JuHe,        // 聚合数据
        RSS,         // RSS 订阅
        All          // 聚合所有源
    };

    explicit RealNewsProvider(QObject *parent = nullptr);
    ~RealNewsProvider() override;

    /**
     * @brief 设置 API Key
     */
    void setTianXingApiKey(const QString &key) { m_tianxingKey = key; }
    void setJuHeApiKey(const QString &key) { m_juheKey = key; }

    /**
     * @brief 设置数据源
     */
    void setDataSource(DataSource source) { m_dataSource = source; }

    /**
     * @brief 添加 RSS 源
     */
    void addRSSSource(const QString &name, const QString &url);

    // INewsProvider 接口实现
    void fetchHotNews(int limit = 20, const QString &category = QString()) override;
    void searchNews(const QString &keyword) override;
    void fetchNewsDetail(const QString &newsId) override;
    void refresh() override;

private slots:
    void onTianXingReplyFinished(QNetworkReply *reply);

private:
    void fetchFromTouTiao();  // 网易新闻国内频道（主要）
    void fetchFromTouTiaoHotBoard();  // 今日头条热榜（备用）
    void fetchFromHanXiaoHan();
    void fetchFromTianXing(int limit, const QString &category);
    void fetchFromRSS();
    void finalizeNewsAggregation();  // 聚合完成后统一处理
    QList<NewsItem> filterPoliticalNews(const QList<NewsItem> &items);  // 筛选思政新闻
    QList<NewsItem> parseTouTiaoResponse(const QByteArray &data);  // 解析网易新闻
    QList<NewsItem> parseTouTiaoHotBoardResponse(const QByteArray &data);  // 解析今日头条热榜
    QList<NewsItem> parseHanXiaoHanResponse(const QByteArray &data);
    QList<NewsItem> parseTianXingResponse(const QByteArray &data, const QString &endpoint = QString());
    QList<NewsItem> parseRSSResponse(const QByteArray &data, const QString &sourceName);
    QString categoryToTianXingType(const QString &category);

    QNetworkAccessManager *m_networkManager;
    QString m_tianxingKey;
    QString m_juheKey;
    DataSource m_dataSource;

    // RSS 源列表: <名称, URL>
    QList<QPair<QString, QString>> m_rssSources;

    // 待处理的 RSS 响应计数
    int m_pendingRSSCount;
    int m_pendingTianXingCount;  // 天行 API 请求计数
    QList<NewsItem> m_aggregatedNews;

    // 缓存
    QList<NewsItem> m_cachedNews;
    int m_currentLimit;
    QString m_currentCategory;
};

#endif // REALNEWSPROVIDER_H
