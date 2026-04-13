#include "HotspotService.h"
#include "../hotspot/INewsProvider.h"
#include "../hotspot/MockNewsProvider.h"
#include "../hotspot/NewsCategoryUtils.h"
#include "DifyService.h"
#include <QDebug>
#include <memory>

namespace {

constexpr int kCategoryDisplayLimit = 26;
constexpr int kFullFeedCacheLimit = 50;
constexpr int kThematicLocalThreshold = 8;

QList<NewsItem> limitForDisplay(const QList<NewsItem> &items, const QString &category)
{
    if (NewsCategoryUtils::isRemoteCategory(category) && items.size() > kCategoryDisplayLimit) {
        return items.mid(0, kCategoryDisplayLimit);
    }
    return items;
}

} // namespace

HotspotService::HotspotService(QObject *parent)
    : QObject(parent)
    , m_newsProvider(nullptr)
    , m_hasLoadedCategoryData(false)
    , m_ownsProvider(false)
{
    // 默认使用模拟数据提供者
    setNewsProvider(new MockNewsProvider(this));
    m_ownsProvider = true;
}

HotspotService::~HotspotService()
{
    // 如果拥有 provider 的所有权，需要在这里清理
    // 由于 provider 是 QObject 子对象，会自动清理
}

void HotspotService::setNewsProvider(INewsProvider *provider)
{
    if (m_newsProvider) {
        disconnect(m_newsProvider, nullptr, this, nullptr);
        if (m_ownsProvider && m_newsProvider->parent() == this) {
            delete m_newsProvider;
        }
    }
    
    m_newsProvider = provider;
    m_ownsProvider = false;
    m_cachedNews.clear();
    m_fullNewsCache.clear();
    m_activeCategory.clear();
    m_loadedCategory.clear();
    m_lastRequestedCategory.clear();
    m_hasLoadedCategoryData = false;
    
    if (m_newsProvider) {
        connect(m_newsProvider, &INewsProvider::newsListReceived,
                this, &HotspotService::onNewsListReceived);
        connect(m_newsProvider, &INewsProvider::newsDetailReceived,
                this, &HotspotService::onNewsDetailReceived);
        connect(m_newsProvider, &INewsProvider::errorOccurred,
                this, &HotspotService::onProviderError);
        connect(m_newsProvider, &INewsProvider::loadingStarted,
                this, [this]() { emit loadingStateChanged(true); });
        connect(m_newsProvider, &INewsProvider::loadingFinished,
                this, [this]() { emit loadingStateChanged(false); });
                
        qDebug() << "[HotspotService] News provider set:" << provider->metaObject()->className();
    }
}

void HotspotService::refreshHotNews(const QString &category)
{
    if (!m_newsProvider) {
        emit errorOccurred("新闻提供者未设置");
        return;
    }

    m_activeCategory = NewsCategoryUtils::normalizeCategory(category);
    qDebug() << "[HotspotService] Refreshing hot news, category:" << m_activeCategory;

    const bool requestDedicatedCategory =
        !m_activeCategory.isEmpty() && !NewsCategoryUtils::isRemoteCategory(m_activeCategory);
    const bool needsFullFeedCache = m_activeCategory.isEmpty();
    const int requestLimit = (needsFullFeedCache || requestDedicatedCategory)
        ? kFullFeedCacheLimit
        : kCategoryDisplayLimit;
    const QString requestCategory = needsFullFeedCache ? QString() : m_activeCategory;

    m_lastRequestedCategory = requestCategory;
    m_newsProvider->fetchHotNews(requestLimit, requestCategory);
}

void HotspotService::setActiveCategory(const QString &category)
{
    if (!m_newsProvider) {
        emit errorOccurred("新闻提供者未设置");
        return;
    }

    m_activeCategory = NewsCategoryUtils::normalizeCategory(category);
    qDebug() << "[HotspotService] Switching active category to:" << m_activeCategory;

    if (m_hasLoadedCategoryData &&
        m_loadedCategory == m_activeCategory &&
        (NewsCategoryUtils::isRemoteCategory(m_activeCategory) || m_lastRequestedCategory == m_activeCategory)) {
        emit hotNewsUpdated(m_cachedNews);
        return;
    }

    if (NewsCategoryUtils::isRemoteCategory(m_activeCategory)) {
        const int requestLimit = m_activeCategory.isEmpty() ? kFullFeedCacheLimit : kCategoryDisplayLimit;
        m_lastRequestedCategory = m_activeCategory;
        m_newsProvider->fetchHotNews(requestLimit, m_activeCategory);
        return;
    }

    QList<NewsItem> localFiltered;
    if (!m_fullNewsCache.isEmpty()) {
        localFiltered = NewsCategoryUtils::filterNewsByCategory(m_fullNewsCache, m_activeCategory);
        m_cachedNews = localFiltered;
        m_loadedCategory = m_activeCategory;
        m_hasLoadedCategoryData = true;
        emit hotNewsUpdated(m_cachedNews);
    }

    const bool needsDedicatedFetch = m_fullNewsCache.isEmpty() || localFiltered.size() < kThematicLocalThreshold;
    if (needsDedicatedFetch) {
        qDebug() << "[HotspotService] 本地分类结果较少，拉取专属数据源:" << m_activeCategory;
        m_lastRequestedCategory = m_activeCategory;
        m_newsProvider->fetchHotNews(kFullFeedCacheLimit, m_activeCategory);
    }
}

void HotspotService::searchNews(const QString &keyword)
{
    if (keyword.trimmed().isEmpty()) {
        setActiveCategory(m_activeCategory);
        return;
    }

    qDebug() << "[HotspotService] Searching news:" << keyword;
    emit hotNewsUpdated(NewsCategoryUtils::searchNews(m_cachedNews, keyword));
}

void HotspotService::fetchNewsDetail(const QString &newsId)
{
    if (!m_newsProvider) {
        emit errorOccurred("新闻提供者未设置");
        return;
    }
    
    qDebug() << "[HotspotService] Fetching news detail:" << newsId;
    m_newsProvider->fetchNewsDetail(newsId);
}

void HotspotService::generateTeachingContent(const NewsItem &news, DifyService *difyService)
{
    if (!difyService) {
        emit errorOccurred("AI 服务未设置");
        return;
    }
    
    // 构建教学内容生成提示
    QString prompt = QString(
        "请根据以下时政新闻，生成一份适合思政课堂使用的教学案例分析。\n\n"
        "【新闻标题】%1\n"
        "【新闻来源】%2\n"
        "【新闻摘要】%3\n"
        "【关键词】%4\n\n"
        "请按以下格式输出：\n"
        "## 案例背景\n"
        "简要介绍新闻背景\n\n"
        "## 思政价值\n"
        "分析该新闻蕴含的思政教育价值\n\n"
        "## 讨论话题\n"
        "设计2-3个适合课堂讨论的话题\n\n"
        "## 延伸思考\n"
        "引导学生进行深入思考的问题"
    ).arg(news.title)
     .arg(news.source)
     .arg(news.summary)
     .arg(news.keywords.join("、"));
    
    qDebug() << "[HotspotService] Generating teaching content for:" << news.title;
    
    // 使用共享指针存储连接，避免未初始化警告
    auto connPtr = std::make_shared<QMetaObject::Connection>();
    *connPtr = connect(difyService, &DifyService::messageReceived,
        this, [this, connPtr](const QString &response) {
            disconnect(*connPtr);
            emit teachingContentGenerated(response);
        });
    
    difyService->sendMessage(prompt);
}

QStringList HotspotService::availableCategories() const
{
    return NewsCategoryUtils::allCategories();
}

void HotspotService::onNewsListReceived(const QList<NewsItem> &newsList)
{
    const bool requestedSpecificThematic =
        !m_lastRequestedCategory.isEmpty() && !NewsCategoryUtils::isRemoteCategory(m_lastRequestedCategory);

    if (m_lastRequestedCategory.isEmpty()) {
        m_fullNewsCache = newsList;
    }

    QList<NewsItem> displayNews;
    if (requestedSpecificThematic) {
        if (newsList.isEmpty() && !m_cachedNews.isEmpty() && m_loadedCategory == m_activeCategory) {
            qDebug() << "[HotspotService] 专属源返回空结果，保留当前分类缓存:" << m_activeCategory;
            emit hotNewsUpdated(m_cachedNews);
            return;
        }
        displayNews = newsList;
    } else if (NewsCategoryUtils::isRemoteCategory(m_activeCategory)) {
        displayNews = limitForDisplay(newsList, m_activeCategory);
    } else {
        const QList<NewsItem> &sourceNews = m_fullNewsCache.isEmpty() ? newsList : m_fullNewsCache;
        displayNews = NewsCategoryUtils::filterNewsByCategory(sourceNews, m_activeCategory);
    }

    m_cachedNews = displayNews;
    m_loadedCategory = m_activeCategory;
    m_hasLoadedCategoryData = true;

    qDebug() << "[HotspotService] Received" << newsList.size() << "news items, display" << displayNews.size();
    emit hotNewsUpdated(displayNews);
}

void HotspotService::onNewsDetailReceived(const NewsItem &news)
{
    emit newsDetailReceived(news);
}

void HotspotService::onProviderError(const QString &error)
{
    qDebug() << "[HotspotService] Provider error:" << error;
    emit errorOccurred(error);
}
