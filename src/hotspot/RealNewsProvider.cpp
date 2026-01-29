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
#include <QStringDecoder>

RealNewsProvider::RealNewsProvider(QObject *parent)
    : INewsProvider(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_dataSource(DataSource::TianXing)  // 默认使用天行 API（数据最新）
    , m_pendingRSSCount(0)
    , m_pendingTianXingCount(0)
    , m_currentLimit(20)
{
    // 天行 API Key（国内新闻主数据源）
    m_tianxingKey = "aa65efc9316cdcc1a4baf14ba175fc39";

    // RSS 源作为备用
    addRSSSource("人民日报", "http://www.people.com.cn/rss/politics.xml");

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

    // 根据分类智能选择数据源
    // 国际新闻 → BBC RSS（有图片）
    // 国内新闻 → 天行 API（数据最新，来源为人民日报/新华社/央视等官媒）
    if (category == "国际") {
        qDebug() << "[RealNewsProvider] 分类为国际，使用 BBC RSS";
        fetchFromRSS();
        return;
    }

    // 国内新闻或全部：使用天行 API
    qDebug() << "[RealNewsProvider] 国内新闻使用天行API（人民日报、新华社、央视新闻）";
    fetchFromTianXing(limit, category);
}

void RealNewsProvider::fetchFromTouTiao()
{
    // 使用网易新闻国内频道 API - 图文匹配
    // 接口返回 JSONP 格式，需要解析
    QUrl url("https://temp.163.com/special/00804KVA/cm_guonei.js?callback=data_callback");

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36");
    request.setTransferTimeout(15000);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QList<NewsItem> items = parseTouTiaoResponse(data);

            // 筛选思政类新闻，过滤社会新闻
            items = filterPoliticalNews(items);

            if (!items.isEmpty()) {
                // 限制数量
                if (items.size() > m_currentLimit) {
                    items = items.mid(0, m_currentLimit);
                }

                m_cachedNews = items;
                emit newsListReceived(items);
                emit loadingFinished();
                qDebug() << "[RealNewsProvider] 思政新闻筛选完成，" << items.size() << "条新闻";
            } else {
                qWarning() << "[RealNewsProvider] 无思政相关新闻，降级到今日头条热榜";
                // 降级到今日头条热榜
                fetchFromTouTiaoHotBoard();
            }
        } else {
            qWarning() << "[RealNewsProvider] 网易API请求失败:" << reply->errorString();
            fetchFromTouTiaoHotBoard();
        }
        reply->deleteLater();
    });
}

// 筛选思政类新闻 - 严格过滤社会类新闻
QList<NewsItem> RealNewsProvider::filterPoliticalNews(const QList<NewsItem> &items)
{
    // 时政热点关键词 - 党政、理论、政策、领导人、重要会议等
    static const QStringList politicalKeywords = {
        // 领导人
        "习近平", "李强", "赵乐际", "王沪宁", "蔡奇", "丁薛祥", "李希",
        "总书记", "国家主席", "总理", "委员长", "政协主席", "国家副主席",
        // 机构
        "中央", "国务院", "全国人大", "全国政协", "中纪委", "中组部",
        "外交部", "国防部", "发改委", "教育部", "科技部", "工信部",
        "公安部", "民政部", "司法部", "财政部", "人社部", "自然资源部",
        "生态环境部", "住建部", "交通运输部", "水利部", "农业农村部",
        "商务部", "文旅部", "卫健委", "退役军人部", "应急管理部",
        "中国人民银行", "审计署", "国资委", "海关总署", "税务总局",
        "市场监管", "广电总局", "体育总局", "统计局", "医保局",
        // 会议活动
        "两会", "党代会", "中央全会", "政治局", "常委会", "座谈会",
        "工作会议", "中央经济", "深改委", "国家安全", "中央财经",
        // 政策理论
        "改革", "政策", "法治", "依法治国", "从严治党", "党建",
        "思想政治", "意识形态", "马克思", "社会主义", "新时代",
        "中国特色", "现代化", "高质量发展", "共同富裕", "乡村振兴",
        "一带一路", "双循环", "碳达峰", "碳中和", "数字中国",
        // 外交国防
        "外交", "国防", "军队", "解放军", "武警", "国际关系",
        "中美", "中俄", "中欧", "台湾", "港澳", "统一",
        // 其他时政
        "省委", "市委", "党委", "人大代表", "政协委员",
        "纪检", "巡视", "反腐", "廉政", "作风建设",
        // 补充：经济、科技政策相关
        "经济工作", "金融监管", "科技创新", "产业政策", "区域发展"
    };

    // 社会新闻排除关键词 - 严格过滤（老王加强版）
    static const QStringList excludeKeywords = {
        // 事故灾难类
        "车祸", "事故", "坠楼", "跳楼", "自杀", "凶杀", "命案", "火灾",
        "爆炸", "塌方", "坍塌", "伤亡", "遇难", "死亡", "身亡", "溺水",
        "触电", "中毒", "煤气", "意外", "惨剧",
        // 犯罪类
        "骗子", "诈骗", "盗窃", "抢劫", "绑架", "失踪", "贩毒", "贪污",
        "偷窃", "强奸", "猥亵", "杀人", "杀害", "谋杀", "行凶", "砍人",
        "报警", "逮捕", "抓捕", "犯罪", "嫌疑人", "作案", "案件", "刑事",
        // 娱乐八卦类
        "出轨", "离婚", "小三", "婆媳", "家暴", "吵架", "恋情", "分手",
        "网红", "明星", "八卦", "绯闻", "整容", "炫富", "豪宅", "豪车",
        "结婚", "订婚", "热恋", "复合", "前夫", "前妻", "恋爱",
        // 博彩赌博类
        "彩票", "赌博", "酒驾", "醉驾", "超速", "违章", "中奖", "博彩",
        // 生活娱乐类
        "宠物", "萌宠", "吃播", "减肥", "健身", "美食", "旅游",
        "直播", "带货", "网购", "团购", "探店", "打卡", "测评",
        // 社会琐事类
        "吵架", "口角", "纠纷", "邻居", "停车", "物业", "业主",
        "打架", "斗殴", "聚众", "闹事", "争执", "争吵", "冲突",
        // 奇闻异事类
        "路人", "围观", "现场", "目击", "爆料", "曝光", "揭秘",
        "惊现", "惊人", "震惊", "吓人", "离奇", "诡异", "诡异",
        "奇葩", "奇怪", "罕见", "罕见", "匪夷所思", "不可思议",
        // 医疗健康类（个人案例）
        "患者", "病人", "手术", "肿瘤", "癌症", "确诊", "治疗", "医院",
        // 消费维权类（个案）
        "投诉", "维权", "退款", "赔偿", "索赔", "质量问题", "假冒",
        // 家庭矛盾类
        "继母", "继父", "婆婆", "儿媳", "女婿", "岳父", "岳母",
        // 情感故事类
        "表白", "求婚", "情侣", "夫妻", "相亲", "约会", "男友", "女友",
        // 校园社会类（非教育政策）
        "学生打架", "校园霸凌", "师生冲突", "早恋",
        // 其他社会八卦
        "网友热议", "引发热议", "网传", "有人", "某男", "某女",
        "一男子", "一女子", "男童", "女童", "老人", "老太",
        // 标题党关键词
        "太", "竟然", "居然", "竟", "万万没想到", "没想到"
    };

    QList<NewsItem> filtered;

    // 官方权威媒体列表 - 扩充更多官媒
    static const QStringList officialSources = {
        "人民日报", "新华社", "新华网", "央视", "CCTV", "中国日报",
        "光明日报", "经济日报", "解放军报", "中国青年报", "中国纪检监察报",
        "求是", "半月谈", "环球时报", "参考消息", "中国政府网",
        "人民网", "央广网", "中国网", "中国新闻网", "学习强国",
        "央视新闻", "新华社", "人民政协网", "法制日报", "科技日报"
    };

    for (const NewsItem &item : items) {
        QString text = item.title + " " + item.summary;

        // 第一步：严格排除社会新闻（优先级最高）
        bool excluded = false;
        for (const QString &keyword : excludeKeywords) {
            if (text.contains(keyword, Qt::CaseInsensitive)) {
                excluded = true;
                qDebug() << "[RealNewsProvider] 排除社会新闻:" << item.title.left(30)
                         << " (关键词:" << keyword << ")";
                break;
            }
        }
        if (excluded) continue;

        // 第二步：检查是否包含时政关键词
        bool isPolitical = false;
        QString matchedKeyword;
        for (const QString &keyword : politicalKeywords) {
            if (text.contains(keyword, Qt::CaseInsensitive)) {
                isPolitical = true;
                matchedKeyword = keyword;
                break;
            }
        }

        // 第三步：检查来源是否是官方权威媒体（补充筛选）
        bool isOfficialSource = false;
        for (const QString &source : officialSources) {
            if (item.source.contains(source, Qt::CaseInsensitive)) {
                isOfficialSource = true;
                break;
            }
        }

        // 通过条件：必须同时满足 (时政关键词 OR 官方媒体) 且不包含排除关键词
        if (isPolitical || isOfficialSource) {
            filtered.append(item);
            qDebug() << "[RealNewsProvider] 保留时政新闻:" << item.title.left(40)
                     << " (关键词:" << matchedKeyword << " 官媒:" << isOfficialSource << ")";
        } else {
            qDebug() << "[RealNewsProvider] 过滤非时政:" << item.title.left(40);
        }
    }

    qDebug() << "[RealNewsProvider] 时政热点筛选完成: 原" << items.size()
             << "条 → 筛选后" << filtered.size() << "条";
    return filtered;
}

QList<NewsItem> RealNewsProvider::parseTouTiaoResponse(const QByteArray &data)
{
    QList<NewsItem> items;

    // 网易新闻返回 JSONP 格式: data_callback([...])
    // 数据编码为 GBK，使用 QStringDecoder (Qt6)
    QStringDecoder decoder("GBK");
    if (!decoder.isValid()) {
        qWarning() << "[RealNewsProvider] GBK 解码器创建失败，尝试 GB18030";
        decoder = QStringDecoder("GB18030");
    }

    QString text = decoder(data);

    if (text.isEmpty()) {
        qWarning() << "[RealNewsProvider] GBK 解码后文本为空，原始数据长度:" << data.size();
        return items;
    }

    qDebug() << "[RealNewsProvider] 解码后文本长度:" << text.length() << "前100字符:" << text.left(100);

    // 提取 JSON 数组部分（使用非贪婪匹配，避免匹配到最后的括号）
    QRegularExpression jsonRegex(R"(data_callback\((.*)\)\s*$)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = jsonRegex.match(text);

    if (!match.hasMatch()) {
        qWarning() << "[RealNewsProvider] 网易新闻 JSONP 解析失败，文本开头:" << text.left(200);
        return items;
    }
    
    QString jsonStr = match.captured(1);
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "[RealNewsProvider] 网易新闻 JSON 解析失败";
        return items;
    }

    QJsonArray dataArray = doc.array();

    for (const QJsonValue &val : dataArray) {
        QJsonObject obj = val.toObject();

        NewsItem item;
        item.id = obj["docid"].toString();
        if (item.id.isEmpty()) {
            item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        item.title = obj["title"].toString();
        
        // 获取图片 URL (网易使用 imgurl 或 imgsrc)
        item.imageUrl = obj["imgurl"].toString();
        if (item.imageUrl.isEmpty()) {
            item.imageUrl = obj["imgsrc"].toString();
        }
        
        // 确保图片 URL 是完整的
        if (!item.imageUrl.isEmpty() && !item.imageUrl.startsWith("http")) {
            item.imageUrl = "http:" + item.imageUrl;
        }
        
        // 热度值（如果没有则设为0，不再随机生成）
        item.hotScore = obj["hot"].toVariant().toInt();
        if (item.hotScore <= 0) item.hotScore = 0;
        
        // 链接
        item.url = obj["url"].toString();
        if (item.url.isEmpty()) {
            item.url = obj["docurl"].toString();
        }
        
        // 来源和分类
        item.source = obj["source"].toString();
        if (item.source.isEmpty()) {
            item.source = "网易新闻";
        }
        item.category = "国内";
        
        // 摘要
        item.summary = obj["digest"].toString();
        if (item.summary.isEmpty()) {
            item.summary = item.title;
        }
        
        // 发布时间
        QString timeStr = obj["ptime"].toString();
        item.publishTime = QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss");
        if (!item.publishTime.isValid()) {
            item.publishTime = QDateTime::currentDateTime();
        }

        if (item.isValid()) {
            items.append(item);
        }
    }

    qDebug() << "[RealNewsProvider] 网易新闻解析完成，" << items.size() << "条新闻";
    return items;
}

void RealNewsProvider::fetchFromTouTiaoHotBoard()
{
    // 今日头条热榜 API - 备用（也需要筛选时政新闻）
    QUrl url("https://www.toutiao.com/hot-event/hot-board/?origin=toutiao_pc");

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36");
    request.setTransferTimeout(15000);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QList<NewsItem> items = parseTouTiaoHotBoardResponse(data);

            // 同样需要筛选时政新闻！
            items = filterPoliticalNews(items);

            if (!items.isEmpty()) {
                if (items.size() > m_currentLimit) {
                    items = items.mid(0, m_currentLimit);
                }
                m_cachedNews = items;
                emit newsListReceived(items);
                emit loadingFinished();
                qDebug() << "[RealNewsProvider] 今日头条热榜时政筛选完成，" << items.size() << "条新闻";
            } else {
                qWarning() << "[RealNewsProvider] 今日头条热榜无时政新闻，降级到韩小韩";
                fetchFromHanXiaoHan();
            }
        } else {
            qWarning() << "[RealNewsProvider] 今日头条热榜请求失败:" << reply->errorString();
            fetchFromHanXiaoHan();
        }
        reply->deleteLater();
    });
}

QList<NewsItem> RealNewsProvider::parseTouTiaoHotBoardResponse(const QByteArray &data)
{
    QList<NewsItem> items;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return items;
    }

    QJsonObject root = doc.object();
    QJsonArray dataArray = root["data"].toArray();

    for (const QJsonValue &val : dataArray) {
        QJsonObject obj = val.toObject();

        NewsItem item;
        item.id = obj["ClusterId"].toString();
        if (item.id.isEmpty()) {
            item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        item.title = obj["Title"].toString();
        
        QJsonObject imgObj = obj["Image"].toObject();
        item.imageUrl = imgObj["url"].toString();
        
        qint64 hotValue = obj["HotValue"].toVariant().toLongLong();
        item.hotScore = qMin(100, qMax(10, static_cast<int>(hotValue / 100000)));
        
        QString clusterId = obj["ClusterId"].toString();
        item.url = QString("https://www.toutiao.com/trending/%1/").arg(clusterId);
        
        item.source = "今日头条";
        item.category = "国内";
        item.summary = item.title;
        item.publishTime = QDateTime::currentDateTime();

        if (item.isValid()) {
            items.append(item);
        }
    }

    return items;
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

    m_aggregatedNews.clear();

    // 根据分类筛选要加载的 RSS 源
    QList<QPair<QString, QString>> sourcesToFetch;

    for (const auto &source : m_rssSources) {
        bool isInternational = source.first.contains("国际") ||
                              source.first.contains("BBC") ||
                              source.first.contains("world");

        // 如果是"国际"分类，只加载国际源
        if (m_currentCategory == "国际") {
            if (isInternational) {
                sourcesToFetch.append(source);
            }
        }
        // 如果是"国内"分类，只加载国内源
        else if (m_currentCategory == "国内") {
            if (!isInternational) {
                sourcesToFetch.append(source);
            }
        }
        // "全部"或空，加载所有源
        else {
            sourcesToFetch.append(source);
        }
    }

    if (sourcesToFetch.isEmpty()) {
        emit errorOccurred("没有适合该分类的 RSS 数据源");
        emit loadingFinished();
        return;
    }

    m_pendingRSSCount = sourcesToFetch.size();
    qDebug() << "[RealNewsProvider] 加载 RSS 源，分类:" << m_currentCategory << "源数量:" << m_pendingRSSCount;

    for (const auto &source : sourcesToFetch) {
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
                qDebug() << "[RealNewsProvider] RSS 源" << sourceName << "获取成功，" << items.size() << "条新闻";
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

                m_cachedNews = m_aggregatedNews;
                emit newsListReceived(m_aggregatedNews);
                emit loadingFinished();

                qDebug() << "[RealNewsProvider] RSS 新闻聚合完成，共" << m_aggregatedNews.size() << "条";
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
        // 国内新闻不需要图片，只要内容
        item.imageUrl = QString();  // 清空图片URL
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

        item.hotScore = 0; // 真实数据无热度，不再随机生成

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
                    currentItem.hotScore = 0; // 真实数据无热度，不再随机生成

                    // 如果是国际新闻源，过滤掉涉及中国的新闻
                    if (currentItem.category == "国际") {
                        static const QStringList chinaRelatedKeywords = {
                            "中国", "中共", "中方", "北京", "习近平", "李强",
                            "台湾", "台北", "香港", "澳门", "新疆", "西藏",
                            "中美", "中俄", "中欧", "中日", "中韩", "中印",
                            "华为", "大疆", "TikTok", "字节跳动", "阿里", "腾讯",
                            "人民币", "一带一路", "孔子学院"
                        };

                        bool isChinaRelated = false;
                        QString fullText = currentItem.title + " " + currentItem.summary;

                        for (const QString &keyword : chinaRelatedKeywords) {
                            if (fullText.contains(keyword, Qt::CaseInsensitive)) {
                                isChinaRelated = true;
                                break;
                            }
                        }

                        // 只添加非中国相关的国际新闻
                        if (!isChinaRelated) {
                            items.append(currentItem);
                        }
                    } else {
                        // 国内新闻正常添加
                        items.append(currentItem);
                    }
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

    // 去重：基于 URL 和标题去重（避免多个源返回相同新闻）
    // 性能优化：使用 QSet 快速查找，时间复杂度 O(n)
    static constexpr int TITLE_PREFIX_LENGTH = 30;      // 标题前缀匹配长度
    static constexpr int MIN_PREFIX_MATCH_LENGTH = 15;  // 最小前缀匹配长度

    QList<NewsItem> deduplicatedNews;
    QSet<QString> seenUrls;        // 已见过的 URL
    QSet<QString> seenTitles;      // 已见过的标题（规范化后）
    QSet<QString> seenPrefixes;    // 已见过的标题前缀（性能优化）

    for (const NewsItem &item : m_aggregatedNews) {
        bool isDuplicate = false;

        // 规范化标题：去除空格、转小写
        QString normalizedTitle = item.title.simplified().toLower();
        QString titlePrefix = normalizedTitle.left(TITLE_PREFIX_LENGTH);

        // 检查1：URL 完全相同
        if (!item.url.isEmpty() && seenUrls.contains(item.url)) {
            isDuplicate = true;
            qDebug() << "[RealNewsProvider] 去重: URL重复 -" << item.title.left(40);
        }
        // 检查2：标题完全相同
        else if (seenTitles.contains(normalizedTitle)) {
            isDuplicate = true;
            qDebug() << "[RealNewsProvider] 去重: 标题重复 -" << item.title.left(40);
        }
        // 检查3：标题前缀相同（性能优化：O(1) 查找）
        else if (titlePrefix.length() >= MIN_PREFIX_MATCH_LENGTH &&
                 seenPrefixes.contains(titlePrefix)) {
            isDuplicate = true;
            qDebug() << "[RealNewsProvider] 去重: 前缀重复 -" << item.title.left(40);
        }

        // 如果不是重复，加入结果并记录
        if (!isDuplicate) {
            deduplicatedNews.append(item);
            if (!item.url.isEmpty()) {
                seenUrls.insert(item.url);
            }
            seenTitles.insert(normalizedTitle);
            if (normalizedTitle.length() >= TITLE_PREFIX_LENGTH) {
                seenPrefixes.insert(titlePrefix);
            }
        }
    }

    qDebug() << "[RealNewsProvider] 去重: 原" << m_aggregatedNews.size()
             << "条 → 去重后" << deduplicatedNews.size() << "条";
    m_aggregatedNews = deduplicatedNews;

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
