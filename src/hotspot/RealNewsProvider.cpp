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
#include <QRegularExpression>

RealNewsProvider::RealNewsProvider(QObject *parent)
    : INewsProvider(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_dataSource(DataSource::TianXing)  // 使用天行数据API
    , m_pendingRSSCount(0)
    , m_pendingTianXingCount(0)
    , m_currentLimit(20)
{
    // 国内新闻 RSS 源（备用）
    addRSSSource("人民网-时政", "https://politics.people.com.cn/rss/politics.xml");
    addRSSSource("人民网-理论", "https://theory.people.com.cn/rss/theory.xml");
    addRSSSource("新华网-时政", "https://www.xinhuanet.com/politics/news_politics.xml");
    addRSSSource("央视网-新闻", "https://news.cctv.com/rss/china.xml");

    // 国际新闻 RSS 源（feedx.net BBC 中文，带图片）
    addRSSSource("BBC中文-国际", "https://feedx.net/rss/bbc.xml");
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
    case DataSource::HanXiaoHan:
        fetchFromHanXiaoHan();
        break;
    case DataSource::TianXing:
        fetchFromTianXing(limit, category);
        break;
    case DataSource::RSS:
        fetchFromRSS();
        break;
    case DataSource::All:
    default:
        // 优先使用韩小韩（免费），失败再用天行或RSS
        fetchFromHanXiaoHan();
        break;
    }
}

void RealNewsProvider::fetchFromHanXiaoHan()
{
    // 韩小韩热点新闻API - 免费无需Key
    // 文档: https://api.vvhan.com/
    QUrl url("https://api.vvhan.com/api/hotlist/toutiao");

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (compatible; AIPoliticalEducation/1.0)");
    request.setTransferTimeout(15000);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QList<NewsItem> items = parseHanXiaoHanResponse(data);

            if (!items.isEmpty()) {
                // 根据分类筛选
                if (!m_currentCategory.isEmpty() && m_currentCategory != "全部") {
                    QList<NewsItem> filtered;
                    for (const auto &item : items) {
                        if (item.category == m_currentCategory) {
                            filtered.append(item);
                        }
                    }
                    items = filtered;
                }

                // 限制数量
                if (items.size() > m_currentLimit) {
                    items = items.mid(0, m_currentLimit);
                }

                m_cachedNews = items;
                emit newsListReceived(items);
                emit loadingFinished();
            } else {
                qWarning() << "[RealNewsProvider] 韩小韩API返回空数据，切换到RSS";
                fetchFromRSS();
            }
        } else {
            qWarning() << "[RealNewsProvider] 韩小韩API请求失败:" << reply->errorString();
            // 降级到天行或RSS
            if (!m_tianxingKey.isEmpty()) {
                fetchFromTianXing(m_currentLimit, m_currentCategory);
            } else {
                fetchFromRSS();
            }
        }
        reply->deleteLater();
    });
}

QList<NewsItem> RealNewsProvider::parseHanXiaoHanResponse(const QByteArray &data)
{
    QList<NewsItem> items;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "[RealNewsProvider] 韩小韩JSON解析失败";
        return items;
    }

    QJsonObject root = doc.object();
    if (!root["success"].toBool()) {
        qWarning() << "[RealNewsProvider] 韩小韩API返回错误";
        return items;
    }

    QJsonArray dataArray = root["data"].toArray();
    for (const QJsonValue &val : dataArray) {
        QJsonObject obj = val.toObject();

        NewsItem item;
        item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.title = obj["title"].toString();
        item.url = obj["url"].toString();
        item.hotScore = obj["hot"].toString().remove("万").toInt();
        if (item.hotScore == 0) {
            item.hotScore = 50 + QRandomGenerator::global()->bounded(50);
        } else {
            // 转换热度值到0-100范围
            item.hotScore = qMin(100, qMax(10, item.hotScore));
        }

        // 今日头条来源，默认国内
        item.source = "今日头条";
        item.category = "国内";
        item.publishTime = QDateTime::currentDateTime();

        // 摘要用标题
        item.summary = item.title;

        if (item.isValid()) {
            items.append(item);
        }
    }

    qDebug() << "[RealNewsProvider] 韩小韩API返回" << items.size() << "条新闻";
    return items;
}

void RealNewsProvider::fetchFromTianXing(int limit, const QString &category)
{
    if (m_tianxingKey.isEmpty()) {
        qWarning() << "[RealNewsProvider] 天行数据 API Key 未设置，切换到 RSS 模式";
        fetchFromRSS();
        return;
    }

    m_aggregatedNews.clear();

    // 根据分类决定请求策略
    // 国内新闻：用天行 API（有图片）
    // 国际新闻：用 feedx.net BBC RSS（有图片）
    bool needDomestic = (category.isEmpty() || category == "全部" || category == "国内");
    bool needInternational = (category.isEmpty() || category == "全部" || category == "国际" || category == "国外");

    // 计数器：需要完成的请求数
    int pendingCount = 0;
    if (needDomestic) pendingCount++;
    if (needInternational) pendingCount++;
    m_pendingTianXingCount = pendingCount;

    // 请求国内新闻（天行 API）
    if (needDomestic) {
        QUrl url(QString("https://apis.tianapi.com/guonei/index"));
        QUrlQuery query;
        query.addQueryItem("key", m_tianxingKey);
        query.addQueryItem("num", QString::number(qMin(limit, 50)));
        url.setQuery(query);

        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
        request.setTransferTimeout(15000);

        QNetworkReply *reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QList<NewsItem> items = parseTianXingResponse(data, "guonei");
                m_aggregatedNews.append(items);
            } else {
                qWarning() << "[RealNewsProvider] 天行 API 国内新闻请求失败:" << reply->errorString();
            }

            reply->deleteLater();
            m_pendingTianXingCount--;
            finalizeNewsAggregation();
        });
    }

    // 请求国际新闻（feedx.net BBC RSS，有图片）
    if (needInternational) {
        QUrl url("https://feedx.net/rss/bbc.xml");
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
        request.setRawHeader("User-Agent", "Mozilla/5.0 (compatible; AIPoliticalEducation/1.0)");
        request.setTransferTimeout(20000);  // BBC RSS 数据较大，给更长超时

        QNetworkReply *reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply, limit]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QList<NewsItem> items = parseRSSResponse(data, "BBC中文-国际");
                // 限制国际新闻数量
                if (items.size() > limit / 2) {
                    items = items.mid(0, limit / 2);
                }
                m_aggregatedNews.append(items);
                qDebug() << "[RealNewsProvider] BBC RSS 获取成功，" << items.size() << "条国际新闻";
            } else {
                qWarning() << "[RealNewsProvider] BBC RSS 请求失败:" << reply->errorString();
                // 失败时尝试用天行的 world 接口作为备用（虽然没图片）
                // 这里不做备用，直接跳过
            }

            reply->deleteLater();
            m_pendingTianXingCount--;
            finalizeNewsAggregation();
        });
    }
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
        request.setTransferTimeout(15000);

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

QList<NewsItem> RealNewsProvider::parseTianXingResponse(const QByteArray &data, const QString &endpoint)
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
        item.url = obj["url"].toString();

        // 如果 summary 为空，使用标题的前 80 个字符
        if (item.summary.isEmpty()) {
            item.summary = item.title.left(80);
            if (item.title.length() > 80) {
                item.summary += "...";
            }
        }

        // 解析时间
        QString timeStr = obj["ctime"].toString();
        item.publishTime = QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss");
        if (!item.publishTime.isValid()) {
            item.publishTime = QDateTime::currentDateTime();
        }

        // 分类判断 - 基于 endpoint 的默认分类 + 智能检测
        // 只有当从 world 接口获取的新闻明确是国内政务时才改为"国内"
        if (endpoint == "world") {
            // world 接口默认为国际，只有明确的国内政务才改为国内
            // 政府部门关键词（高优先级，明确是国内政务）
            static const QStringList domesticGovIndicators = {
                "教育部", "国务院", "发改委", "财政部", "工信部",
                "农业农村部", "卫生健康委", "商务部", "自然资源部",
                "生态环境部", "交通运输部", "水利部", "人社部", "住建部",
                "文旅部", "科技部", "公安部", "司法部", "民政部",
                "退役军人部", "应急管理部", "市场监管总局", "国家统计局",
                "省委", "市委", "两会", "全国人大", "全国政协",
                "中央政府", "我国", "我省", "我市", "国产"
            };
            // 中国领导人姓名（明确是国内政务新闻）
            static const QStringList chineseLeaders = {
                "习近平", "李强", "王沪宁", "赵乐际", "丁薛祥",
                "李克强", "张国清", "刘国中", "何立峰"
            };

            bool isDomesticNews = false;
            // 检查标题中的政府部门关键词
            for (const QString &indicator : domesticGovIndicators) {
                if (item.title.contains(indicator)) {
                    isDomesticNews = true;
                    break;
                }
            }
            // 检查标题中的中国领导人姓名
            if (!isDomesticNews) {
                for (const QString &name : chineseLeaders) {
                    if (item.title.contains(name)) {
                        isDomesticNews = true;
                        break;
                    }
                }
            }
            item.category = isDomesticNews ? "国内" : "国际";
        } else {
            // guonei 接口默认为国内
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
    QString currentDescription;  // 保存 description 原始内容用于提取图片
    bool inItem = false;

    // 用于从 HTML 中提取图片 URL 的正则表达式
    static QRegularExpression imgRegex(R"(<img[^>]+src\s*=\s*[\"']([^\"']+)[\"'])");

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            currentElement = xml.name().toString();

            if (currentElement == "item" || currentElement == "entry") {
                inItem = true;
                currentItem = NewsItem();
                currentItem.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                currentItem.source = sourceName;
                currentDescription.clear();

                // 根据来源判断分类
                if (sourceName.contains("国际") || sourceName.contains("world") ||
                    sourceName.contains("BBC") || sourceName.contains("bbc")) {
                    currentItem.category = "国际";
                } else {
                    currentItem.category = "国内";
                }
            }

            if (inItem && currentElement == "link") {
                // Atom: <link href="..." />
                const auto href = xml.attributes().value("href").toString().trimmed();
                if (!href.isEmpty()) {
                    currentItem.url = href;
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (xml.name().toString() == "item" || xml.name().toString() == "entry") {
                inItem = false;

                // 如果没有直接的图片 URL，尝试从 description 的 HTML 中提取
                if (currentItem.imageUrl.isEmpty() && !currentDescription.isEmpty()) {
                    QRegularExpressionMatch match = imgRegex.match(currentDescription);
                    if (match.hasMatch()) {
                        currentItem.imageUrl = match.captured(1);
                        // 处理 HTML 转义
                        currentItem.imageUrl = currentItem.imageUrl
                            .replace("&amp;", "&")
                            .replace("&quot;", "\"")
                            .replace("&lt;", "<")
                            .replace("&gt;", ">");
                    }
                }

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
                // 保存原始 HTML 用于提取图片
                currentDescription = text;
                // 移除 HTML 标签作为摘要
                currentItem.summary = text;
                currentItem.summary.remove(QRegularExpression("<[^>]*>"));
                // 处理常见 HTML 实体
                currentItem.summary = currentItem.summary
                    .replace("&amp;", "&")
                    .replace("&nbsp;", " ")
                    .replace("&quot;", "\"")
                    .replace("&lt;", "<")
                    .replace("&gt;", ">")
                    .trimmed();
                // 截断过长的摘要
                if (currentItem.summary.length() > 150) {
                    currentItem.summary = currentItem.summary.left(150) + "...";
                }
            } else if (currentElement == "content" || currentElement == "content:encoded") {
                currentItem.content = text.remove(QRegularExpression("<[^>]*>"));
            } else if (currentElement == "pubDate" || currentElement == "published") {
                // 尝试多种日期格式
                currentItem.publishTime = QDateTime::fromString(text, Qt::RFC2822Date);
                if (!currentItem.publishTime.isValid()) {
                    currentItem.publishTime = QDateTime::fromString(text, Qt::ISODate);
                }
            } else if (currentElement == "link") {
                // RSS: <link>https://...</link>
                if (currentItem.url.isEmpty()) {
                    currentItem.url = text;
                }
            }
        }
        // 注：Qt6 中 CDATA 内容会作为 Characters token 返回，不需要单独处理
    }

    if (xml.hasError()) {
        qWarning() << "[RealNewsProvider] RSS 解析错误:" << xml.errorString();
    }

    qDebug() << "[RealNewsProvider] RSS 解析完成:" << sourceName << "获取" << items.size() << "条新闻";
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

void RealNewsProvider::finalizeNewsAggregation()
{
    // 还有请求没完成，等着
    if (m_pendingTianXingCount > 0) {
        return;
    }

    // 按发布时间排序（新的在前）
    std::sort(m_aggregatedNews.begin(), m_aggregatedNews.end(),
              [](const NewsItem &a, const NewsItem &b) {
                  return a.publishTime > b.publishTime;
              });

    // 根据分类筛选
    if (!m_currentCategory.isEmpty() && m_currentCategory != "全部") {
        QList<NewsItem> filtered;
        for (const auto &item : m_aggregatedNews) {
            if (item.category == m_currentCategory ||
                (m_currentCategory == "国外" && item.category == "国际")) {
                filtered.append(item);
            }
        }
        m_aggregatedNews = filtered;
    }

    // 限制数量
    if (m_aggregatedNews.size() > m_currentLimit) {
        m_aggregatedNews = m_aggregatedNews.mid(0, m_currentLimit);
    }

    m_cachedNews = m_aggregatedNews;
    emit newsListReceived(m_aggregatedNews);
    emit loadingFinished();

    qDebug() << "[RealNewsProvider] 新闻聚合完成，共" << m_aggregatedNews.size() << "条";
}
