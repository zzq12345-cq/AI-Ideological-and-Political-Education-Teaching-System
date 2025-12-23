#include "HotspotService.h"
#include "../hotspot/INewsProvider.h"
#include "../hotspot/MockNewsProvider.h"
#include "DifyService.h"
#include <QDebug>
#include <memory>

HotspotService::HotspotService(QObject *parent)
    : QObject(parent)
    , m_newsProvider(nullptr)
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
    
    qDebug() << "[HotspotService] Refreshing hot news, category:" << category;
    m_newsProvider->fetchHotNews(20, category);
}

void HotspotService::searchNews(const QString &keyword)
{
    if (!m_newsProvider) {
        emit errorOccurred("新闻提供者未设置");
        return;
    }
    
    if (keyword.trimmed().isEmpty()) {
        refreshHotNews();
        return;
    }
    
    qDebug() << "[HotspotService] Searching news:" << keyword;
    m_newsProvider->searchNews(keyword);
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
    return QStringList() << "全部" << "国内" << "国外";
}

void HotspotService::onNewsListReceived(const QList<NewsItem> &newsList)
{
    m_cachedNews = newsList;
    qDebug() << "[HotspotService] Received" << newsList.size() << "news items";
    emit hotNewsUpdated(newsList);
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
