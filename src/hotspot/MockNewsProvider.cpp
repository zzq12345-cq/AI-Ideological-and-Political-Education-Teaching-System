#include "MockNewsProvider.h"
#include <QDebug>
#include <QUuid>

MockNewsProvider::MockNewsProvider(QObject *parent)
    : INewsProvider(parent)
{
    // 初始化时生成模拟数据
    m_mockNews = generateMockData();
}

void MockNewsProvider::fetchHotNews(int limit, const QString &category)
{
    emit loadingStarted();
    qDebug() << "[MockNewsProvider] Fetching hot news, limit:" << limit << "category:" << category;
    
    QList<NewsItem> result;
    
    for (const NewsItem &news : m_mockNews) {
        if (result.size() >= limit) break;
        
        // 分类筛选
        if (!category.isEmpty() && news.category != category) {
            continue;
        }
        
        result.append(news);
    }
    
    // 模拟网络延迟
    QTimer::singleShot(300, this, [this, result]() {
        emit newsListReceived(result);
        emit loadingFinished();
    });
}

void MockNewsProvider::searchNews(const QString &keyword)
{
    emit loadingStarted();
    qDebug() << "[MockNewsProvider] Searching news with keyword:" << keyword;
    
    QList<NewsItem> result;
    
    for (const NewsItem &news : m_mockNews) {
        if (news.title.contains(keyword, Qt::CaseInsensitive) ||
            news.summary.contains(keyword, Qt::CaseInsensitive) ||
            news.keywords.contains(keyword, Qt::CaseInsensitive)) {
            result.append(news);
        }
    }
    
    QTimer::singleShot(200, this, [this, result]() {
        emit newsListReceived(result);
        emit loadingFinished();
    });
}

void MockNewsProvider::fetchNewsDetail(const QString &newsId)
{
    emit loadingStarted();
    qDebug() << "[MockNewsProvider] Fetching news detail:" << newsId;
    
    for (const NewsItem &news : m_mockNews) {
        if (news.id == newsId) {
            QTimer::singleShot(100, this, [this, news]() {
                emit newsDetailReceived(news);
                emit loadingFinished();
            });
            return;
        }
    }
    
    emit errorOccurred("新闻不存在");
    emit loadingFinished();
}

void MockNewsProvider::refresh()
{
    qDebug() << "[MockNewsProvider] Refreshing news data...";
    fetchHotNews(20);
}

QList<NewsItem> MockNewsProvider::generateMockData()
{
    QList<NewsItem> news;
    
    // 模拟国内新闻
    {
        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = "中共中央政治局召开会议 分析研究当前经济形势和经济工作";
        item.summary = "会议强调，要全面贯彻落实党的二十大精神，坚持稳中求进工作总基调，完整、准确、全面贯彻新发展理念，加快构建新发展格局，着力推动高质量发展。";
        item.content = item.summary + "\n\n会议指出，今年以来，在以习近平同志为核心的党中央坚强领导下，各地区各部门认真贯彻落实党中央决策部署，经济运行持续回升向好，高质量发展稳步推进，社会大局保持稳定。";
        item.source = "新华社";
        item.category = "国内";
        item.publishTime = QDateTime::currentDateTime().addDays(-1);
        item.hotScore = 98;
        item.keywords = QStringList() << "政治局会议" << "经济形势" << "高质量发展";
        news.append(item);
    }
    
    {
        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = "全国人大常委会法工委：加快推进重点领域立法";
        item.summary = "全国人大常委会法工委发言人表示，将坚持科学立法、民主立法、依法立法，不断完善中国特色社会主义法律体系。";
        item.content = item.summary;
        item.source = "人民日报";
        item.category = "国内";
        item.publishTime = QDateTime::currentDateTime().addDays(-2);
        item.hotScore = 85;
        item.keywords = QStringList() << "立法" << "法治" << "人大";
        news.append(item);
    }
    
    {
        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = "我国科技创新能力持续提升 多项关键核心技术取得突破";
        item.summary = "从量子计算到人工智能，从载人航天到深海探测，我国在多个前沿科技领域取得重要进展，科技自立自强迈出坚实步伐。";
        item.content = item.summary;
        item.source = "央视新闻";
        item.category = "国内";
        item.publishTime = QDateTime::currentDateTime().addDays(-3);
        item.hotScore = 92;
        item.keywords = QStringList() << "科技创新" << "自主可控" << "核心技术";
        news.append(item);
    }
    
    // 模拟国外新闻
    {
        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = "中国与多国加强战略对话 推动构建人类命运共同体";
        item.summary = "外交部发言人表示，中国始终是世界和平的建设者、全球发展的贡献者、国际秩序的维护者。";
        item.content = item.summary + "\n\n中方愿同各方一道，践行全球发展倡议、全球安全倡议、全球文明倡议，推动构建人类命运共同体。";
        item.source = "外交部";
        item.category = "国外";
        item.publishTime = QDateTime::currentDateTime().addDays(-1);
        item.hotScore = 90;
        item.keywords = QStringList() << "外交" << "人类命运共同体" << "全球治理";
        news.append(item);
    }
    
    {
        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = "\"一带一路\"高质量发展取得新成效 惠及沿线国家人民";
        item.summary = "共建\"一带一路\"倡议提出十年来，中国已同150多个国家和30多个国际组织签署合作文件，形成一批标志性项目和惠民工程。";
        item.content = item.summary;
        item.source = "新华网";
        item.category = "国外";
        item.publishTime = QDateTime::currentDateTime().addDays(-2);
        item.hotScore = 88;
        item.keywords = QStringList() << "一带一路" << "国际合作" << "互联互通";
        news.append(item);
    }
    
    {
        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = "金砖国家合作机制不断深化 新兴市场国家合作潜力巨大";
        item.summary = "金砖国家领导人会晤取得丰硕成果，各方一致同意加强在经贸、科技、人文等领域的务实合作。";
        item.content = item.summary;
        item.source = "环球时报";
        item.category = "国外";
        item.publishTime = QDateTime::currentDateTime().addDays(-4);
        item.hotScore = 82;
        item.keywords = QStringList() << "金砖国家" << "多边合作" << "新兴市场";
        news.append(item);
    }
    
    // 按热度排序
    std::sort(news.begin(), news.end(), [](const NewsItem &a, const NewsItem &b) {
        return a.hotScore > b.hotScore;
    });
    
    qDebug() << "[MockNewsProvider] Generated" << news.size() << "mock news items";
    return news;
}
