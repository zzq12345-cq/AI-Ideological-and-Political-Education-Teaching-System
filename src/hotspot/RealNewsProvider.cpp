#include "RealNewsProvider.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QXmlStreamReader>
#include <QUrlQuery>
#include <QDebug>
#include <QDateTime>
#include <QUuid>
#include <QRandomGenerator>

RealNewsProvider::RealNewsProvider(QObject *parent)
    : INewsProvider(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_dataSource(DataSource::All)
    , m_pendingRSSCount(0)
    , m_currentLimit(20)
{
    // 添加默认 RSS 源 - 思政相关
    addRSSSource("人民网-时政", "http://politics.people.com.cn/rss/politics.xml");
    addRSSSource("人民网-理论", "http://theory.people.com.cn/rss/theory.xml");
    addRSSSource("新华网-时政", "http://www.xinhuanet.com/politics/news_politics.xml");
    addRSSSource("央视网-新闻", "http://news.cctv.com/rss/china.xml");
}

RealNewsProvider::~RealNewsProvider()
{
}

void RealNewsProvider::addRSSSource(const QString &name, const QString &url)
{
    m_rssSources.append(qMakePair(name, url));
}

void RealNewsProvider::fetchHotNews(int limit, const QString &category)
{
    m_currentLimit = limit;
    m_currentCategory = category;
    m_aggregatedNews.clear();

    emit loadingStarted();

    switch (m_dataSource) {
    case DataSource::TianXing:
        fetchFromTianXing(limit, category);
        break;
    case DataSource::RSS:
        fetchFromRSS();
        break;
    case DataSource::All:
    default:
        // 先尝试天行 API，如果没有 key 则使用 RSS
        if (!m_tianxingKey.isEmpty()) {
            fetchFromTianXing(limit, category);
        } else {
            fetchFromRSS();
        }
        break;
    }
}

void RealNewsProvider::fetchFromTianXing(int limit, const QString &category)
{
    if (m_tianxingKey.isEmpty()) {
        qWarning() << "[RealNewsProvider] 天行数据 API Key 未设置，切换到 RSS 模式";
        fetchFromRSS();
        return;
    }

    // 天行数据综合新闻接口
    QUrl url("https://apis.tianapi.com/generalnews/index");
    QUrlQuery query;
    query.addQueryItem("key", m_tianxingKey);
    query.addQueryItem("num", QString::number(qMin(limit, 50)));

    // 分类映射
    if (!category.isEmpty()) {
        QString type = categoryToTianXingType(category);
        if (!type.isEmpty()) {
            query.addQueryItem("word", type);
        }
    }

    url.setQuery(query);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onTianXingReplyFinished(reply);
    });
}

void RealNewsProvider::fetchFromRSS()
{
    if (m_rssSources.isEmpty()) {
        emit errorOccurred("没有配置 RSS 数据源");
        emit loadingFinished();
        return;
    }

    m_pendingRSSCount = m_rssSources.size();
    m_aggregatedNews.clear();

    for (const auto &source : m_rssSources) {
        QUrl url(source.second);
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
        request.setRawHeader("User-Agent", "Mozilla/5.0 (compatible; AIPoliticalEducation/1.0)");

        QNetworkReply *reply = m_networkManager->get(request);
        QString sourceName = source.first;

        connect(reply, &QNetworkReply::finished, this, [this, reply, sourceName]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QList<NewsItem> items = parseRSSResponse(data, sourceName);
                m_aggregatedNews.append(items);
            } else {
                qWarning() << "[RealNewsProvider] RSS 获取失败:" << sourceName << reply->errorString();
            }

            reply->deleteLater();
            m_pendingRSSCount--;

            // 所有 RSS 源都处理完毕
            if (m_pendingRSSCount <= 0) {
                // 按时间排序
                std::sort(m_aggregatedNews.begin(), m_aggregatedNews.end(),
                          [](const NewsItem &a, const NewsItem &b) {
                              return a.publishTime > b.publishTime;
                          });

                // 限制数量
                if (m_aggregatedNews.size() > m_currentLimit) {
                    m_aggregatedNews = m_aggregatedNews.mid(0, m_currentLimit);
                }

                // 根据分类筛选
                if (!m_currentCategory.isEmpty()) {
                    QList<NewsItem> filtered;
                    for (const auto &item : m_aggregatedNews) {
                        if (item.category == m_currentCategory) {
                            filtered.append(item);
                        }
                    }
                    m_aggregatedNews = filtered;
                }

                m_cachedNews = m_aggregatedNews;
                emit newsListReceived(m_aggregatedNews);
                emit loadingFinished();
            }
        });
    }
}

void RealNewsProvider::onTianXingReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[RealNewsProvider] 天行 API 请求失败:" << reply->errorString();
        // 降级到 RSS
        fetchFromRSS();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QList<NewsItem> items = parseTianXingResponse(data);

    if (items.isEmpty()) {
        qWarning() << "[RealNewsProvider] 天行 API 返回空数据，切换到 RSS";
        fetchFromRSS();
    } else {
        m_cachedNews = items;
        emit newsListReceived(items);
        emit loadingFinished();
    }

    reply->deleteLater();
}

QList<NewsItem> RealNewsProvider::parseTianXingResponse(const QByteArray &data)
{
    QList<NewsItem> items;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "[RealNewsProvider] JSON 解析失败";
        return items;
    }

    QJsonObject root = doc.object();
    int code = root["code"].toInt();
    if (code != 200) {
        qWarning() << "[RealNewsProvider] API 返回错误:" << root["msg"].toString();
        return items;
    }

    QJsonArray newsList = root["result"].toObject()["newslist"].toArray();
    for (const QJsonValue &val : newsList) {
        QJsonObject obj = val.toObject();

        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = obj["title"].toString();
        item.summary = obj["description"].toString();
        item.content = obj["content"].toString();
        item.source = obj["source"].toString();
        item.imageUrl = obj["picUrl"].toString();

        // 解析时间
        QString timeStr = obj["ctime"].toString();
        item.publishTime = QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss");
        if (!item.publishTime.isValid()) {
            item.publishTime = QDateTime::currentDateTime();
        }

        // 分类判断
        QString url = obj["url"].toString();
        if (url.contains("world") || url.contains("international")) {
            item.category = "国际";
        } else {
            item.category = "国内";
        }

        item.hotScore = 50 + (QRandomGenerator::global()->bounded(50));  // 模拟热度

        if (item.isValid()) {
            items.append(item);
        }
    }

    return items;
}

QList<NewsItem> RealNewsProvider::parseRSSResponse(const QByteArray &data, const QString &sourceName)
{
    QList<NewsItem> items;
    QXmlStreamReader xml(data);

    NewsItem currentItem;
    QString currentElement;
    bool inItem = false;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            currentElement = xml.name().toString();

            if (currentElement == "item" || currentElement == "entry") {
                inItem = true;
                currentItem = NewsItem();
                currentItem.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                currentItem.source = sourceName;

                // 根据来源判断分类
                if (sourceName.contains("国际") || sourceName.contains("world")) {
                    currentItem.category = "国际";
                } else {
                    currentItem.category = "国内";
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (xml.name().toString() == "item" || xml.name().toString() == "entry") {
                inItem = false;
                if (currentItem.isValid()) {
                    // 生成摘要
                    if (currentItem.summary.isEmpty() && !currentItem.content.isEmpty()) {
                        currentItem.summary = currentItem.content.left(100) + "...";
                    }
                    currentItem.hotScore = 30 + (QRandomGenerator::global()->bounded(70));
                    items.append(currentItem);
                }
            }
        } else if (token == QXmlStreamReader::Characters && inItem) {
            QString text = xml.text().toString().trimmed();
            if (text.isEmpty()) continue;

            if (currentElement == "title") {
                currentItem.title = text;
            } else if (currentElement == "description" || currentElement == "summary") {
                // 移除 HTML 标签
                currentItem.summary = text.remove(QRegularExpression("<[^>]*>"));
            } else if (currentElement == "content" || currentElement == "content:encoded") {
                currentItem.content = text.remove(QRegularExpression("<[^>]*>"));
            } else if (currentElement == "pubDate" || currentElement == "published") {
                // 尝试多种日期格式
                currentItem.publishTime = QDateTime::fromString(text, Qt::RFC2822Date);
                if (!currentItem.publishTime.isValid()) {
                    currentItem.publishTime = QDateTime::fromString(text, Qt::ISODate);
                }
                if (!currentItem.publishTime.isValid()) {
                    currentItem.publishTime = QDateTime::currentDateTime();
                }
            } else if (currentElement == "link") {
                // 可以用于获取详情
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "[RealNewsProvider] RSS 解析错误:" << xml.errorString();
    }

    return items;
}

QString RealNewsProvider::categoryToTianXingType(const QString &category)
{
    if (category == "国内") {
        return "国内";
    } else if (category == "国际") {
        return "国际";
    } else if (category == "时政") {
        return "时政";
    } else if (category == "社会") {
        return "社会";
    }
    return "";
}

void RealNewsProvider::searchNews(const QString &keyword)
{
    if (keyword.isEmpty()) {
        emit newsListReceived(m_cachedNews);
        return;
    }

    emit loadingStarted();

    // 本地搜索缓存
    QList<NewsItem> results;
    for (const auto &item : m_cachedNews) {
        if (item.title.contains(keyword, Qt::CaseInsensitive) ||
            item.summary.contains(keyword, Qt::CaseInsensitive) ||
            item.content.contains(keyword, Qt::CaseInsensitive)) {
            results.append(item);
        }
    }

    emit newsListReceived(results);
    emit loadingFinished();
}

void RealNewsProvider::fetchNewsDetail(const QString &newsId)
{
    // 从缓存中查找
    for (const auto &item : m_cachedNews) {
        if (item.id == newsId) {
            emit newsDetailReceived(item);
            return;
        }
    }

    emit errorOccurred("未找到该新闻");
}

void RealNewsProvider::refresh()
{
    fetchHotNews(m_currentLimit, m_currentCategory);
}
