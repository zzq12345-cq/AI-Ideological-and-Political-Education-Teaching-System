#include "ZhipuPPTAgentService.h"
#include "../utils/NetworkRequestFactory.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSvgRenderer>
#include <QPainter>
#include <QRegularExpression>
#include <QDebug>

// ============================================================================
// 构造 / 析构
// ============================================================================

ZhipuPPTAgentService::ZhipuPPTAgentService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_baseUrl("https://open.bigmodel.cn/api/coding/paas/v4")
{
    qDebug() << "[PPTAgent] Service initialized";
}

ZhipuPPTAgentService::~ZhipuPPTAgentService()
{
    cancel();
}

// ============================================================================
// 公共接口
// ============================================================================

void ZhipuPPTAgentService::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void ZhipuPPTAgentService::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = baseUrl;
}

void ZhipuPPTAgentService::generate(const QMap<QString, QString> &params)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("智谱 API Key 未设置");
        return;
    }

    // 重置状态
    m_cancelled = false;
    m_params = params;
    m_outline = QJsonObject();
    m_pagePlans.clear();
    m_svgCodes.clear();
    m_previewImages.clear();
    m_currentSvgIndex = 0;
    m_totalPages = 0;

    // 构建主题描述
    m_topic = buildTopicDescription(params);
    qDebug() << "[PPTAgent] Starting generation for topic:" << m_topic;

    // 开始阶段1: 大纲生成
    setState(State::GeneratingOutline);
    emit progressUpdated(0, "阶段1/3: 大纲生成", "正在分析课程主题，生成PPT结构大纲...");
    startOutlineGeneration(m_topic);
}

void ZhipuPPTAgentService::cancel()
{
    m_cancelled = true;
    QPointer<QNetworkReply> reply = m_currentReply;
    m_currentReply.clear();

    if (reply) {
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }
    if (m_state != State::Idle && m_state != State::Finished && m_state != State::Failed) {
        setState(State::Idle);
    }
}

// ============================================================================
// 主题描述构建
// ============================================================================

QString ZhipuPPTAgentService::buildTopicDescription(const QMap<QString, QString> &params) const
{
    QString topic;
    const QString textbook = params.value("textbook", "人教版");
    const QString grade = params.value("grade", "八年级");
    const QString chapter = params.value("chapter", "");
    const QString duration = params.value("duration", "45分钟标准课时");
    const QString focus = params.value("contentFocus", "");

    topic = QString("%1 %2 思想政治课 %3").arg(textbook, grade, chapter);

    if (!focus.isEmpty()) {
        topic += QString("\n\n内容侧重要求：%1").arg(focus);
    }
    topic += QString("\n课时长度：%1").arg(duration);

    // 附加用户原始请求，保留页数等细节要求
    const QString userRequest = params.value("userRequest");
    if (!userRequest.isEmpty()) {
        topic += QString("\n\n用户原始需求：%1").arg(userRequest);
    }

    return topic;
}

int ZhipuPPTAgentService::extractRequestedPageCount(const QString &request) const
{
    if (request.trimmed().isEmpty()) {
        return 0;
    }

    const QRegularExpression re(R"(([0-9]+)\s*(?:页|张)(?:PPT|ppt|课件|幻灯片|演示文稿)?)");
    const QRegularExpressionMatch match = re.match(request);
    if (!match.hasMatch()) {
        return 0;
    }

    bool ok = false;
    const int count = match.captured(1).toInt(&ok);
    return ok && count > 0 ? count : 0;
}

// ============================================================================
// 网络请求基础
// ============================================================================

QNetworkRequest ZhipuPPTAgentService::createRequest() const
{
    QUrl url(m_baseUrl + "/chat/completions");
    QNetworkRequest request(url);

    // Bearer 认证
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    // 禁用 HTTP/2（macOS 兼容）
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    // 超时
    request.setTransferTimeout(180000); // 3 分钟

    // SSL 配置：始终放宽验证
    // 智谱 API (open.bigmodel.cn) 是可信端点，macOS SecureTransport 可能
    // 对其证书链报 SSL 错误，导致 handleSslErrors() 调用 abort() 中止请求
    {
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(sslConfig);
    }

    return request;
}

QNetworkReply* ZhipuPPTAgentService::callZhipuApi(const QString &model,
                                                     const QString &systemPrompt,
                                                     const QString &userMessage,
                                                     double temperature,
                                                     int maxTokens)
{
    QNetworkRequest request = createRequest();

    QJsonObject body;
    body["model"] = model;
    body["stream"] = false;
    body["temperature"] = temperature;
    body["max_tokens"] = maxTokens;

    QJsonArray messages;

    if (!systemPrompt.isEmpty()) {
        QJsonObject sysMsg;
        sysMsg["role"] = "system";
        sysMsg["content"] = systemPrompt;
        messages.append(sysMsg);
    }

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userMessage;
    messages.append(userMsg);

    body["messages"] = messages;

    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    qDebug() << "[PPTAgent] API call to model:" << model
             << "body size:" << data.size();

    QNetworkReply *reply = m_networkManager->post(request, data);
    
    // 统一处理 SSL 错误，防止被中间人/代理直接断开（表现为 RemoteHostClosedError）
    connect(reply, &QNetworkReply::sslErrors,
            this, [reply](const QList<QSslError> &errors) {
        if (!NetworkRequestFactory::handleSslErrors(reply, errors, "[PPTAgent]")) {
            qWarning() << "[PPTAgent] SSL errors correctly rejected.";
        }
    });
    
    return reply;
}

QString ZhipuPPTAgentService::extractContent(const QByteArray &responseData) const
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[PPTAgent] JSON parse error:" << parseError.errorString();
        return QString();
    }

    QJsonObject obj = doc.object();

    // 检查错误
    if (obj.contains("error")) {
        QJsonObject errorObj = obj["error"].toObject();
        qWarning() << "[PPTAgent] API error:" << errorObj["message"].toString();
        return QString();
    }

    // 提取 choices[0].message.content
    QJsonArray choices = obj["choices"].toArray();
    if (choices.isEmpty()) {
        qWarning() << "[PPTAgent] No choices in response";
        return QString();
    }

    QJsonObject choice = choices[0].toObject();
    QJsonObject message = choice["message"].toObject();
    return message["content"].toString();
}

// ============================================================================
// 阶段1: 大纲生成
// ============================================================================

void ZhipuPPTAgentService::startOutlineGeneration(const QString &topic)
{
    QString userMsg = QString(
        "请为以下思政课堂教学内容生成PPT大纲：\n\n%1\n\n"
        "要求：\n"
        "- 页数根据用户需求决定，如果用户指定了页数则严格遵守，否则默认 8-12 页\n"
        "- 必须包含封面页和结束页\n"
        "- 每个页面的 content 数组中放入该页要展示的核心要点（2-4 条）\n"
        "- 结合思政教育的特点，体现社会主义核心价值观"
    ).arg(topic);

    m_currentReply = callZhipuApi(MODEL_TEXT, outlineSystemPrompt(), userMsg, 0.7, 4096);
    qDebug() << "[PPTAgent] 发起大纲生成请求，reply=" << m_currentReply;
    if (m_currentReply) {
        connect(m_currentReply, &QNetworkReply::finished,
                this, &ZhipuPPTAgentService::onOutlineReplyFinished);
        connect(m_currentReply, &QNetworkReply::errorOccurred,
                this, [](QNetworkReply::NetworkError code) {
            qDebug() << "[PPTAgent] 大纲请求底层错误发生，代码:" << code;
        });
    }
}

void ZhipuPPTAgentService::onOutlineReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    qDebug() << "[PPTAgent] onOutlineReplyFinished 触发，HTTP状态码:" 
             << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
             << "错误码:" << reply->error();

    reply->deleteLater();
    m_currentReply = nullptr;

    // 用户主动取消时静默退出
    if (m_cancelled) {
        qDebug() << "[PPTAgent] 大纲请求已被用户取消，静默退出";
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        // 对 OperationCanceledError 提供更友好的描述
        QString errDetail = (reply->error() == QNetworkReply::OperationCanceledError)
            ? "请求被中止（可能是 SSL 证书问题或网络不稳定）"
            : reply->errorString();
        QString err = QString("大纲生成失败: %1 (错误码: %2)").arg(errDetail).arg(reply->error());
        qWarning() << "[PPTAgent]" << err;
        setState(State::Failed);
        emit errorOccurred(err);
        return;
    }

    QByteArray data = reply->readAll();
    qDebug() << "[PPTAgent] Outline response length:" << data.size();

    QString content = extractContent(data);
    if (content.isEmpty()) {
        setState(State::Failed);
        emit errorOccurred("大纲生成失败: AI 返回内容为空");
        return;
    }

    // 解析大纲 JSON
    m_outline = parseOutlineJson(content);
    if (m_outline.isEmpty()) {
        // 如果 JSON 解析失败，把原始文本作为简单大纲
        qWarning() << "[PPTAgent] Failed to parse outline JSON, using fallback";
        // 创建一个简单的回退大纲
        QJsonObject fallback;
        QJsonObject cover;
        cover["title"] = m_params.value("chapter", "思政课堂");
        cover["sub_title"] = m_params.value("grade", "") + " " + m_params.value("textbook", "");
        fallback["cover"] = cover;

        QJsonArray parts;
        QJsonObject part;
        part["part_title"] = "主要内容";
        QJsonArray pages;
        // 从原始文本中提取要点
        QStringList lines = content.split('\n', Qt::SkipEmptyParts);
        for (int i = 0; i < qMin(6, lines.size()); ++i) {
            QJsonObject page;
            page["title"] = lines[i].trimmed().left(30);
            page["content"] = QJsonArray({"要点1", "要点2"});
            pages.append(page);
        }
        part["pages"] = pages;
        parts.append(part);
        fallback["parts"] = parts;

        QJsonObject endPage;
        endPage["title"] = "总结与展望";
        fallback["end_page"] = endPage;

        m_outline = fallback;
    }

    emit outlineGenerated(m_outline);
    qDebug() << "[PPTAgent] Outline generated successfully";

    // 计算总页数
    m_totalPages = 2; // 封面 + 结束页
    if (m_outline.contains("table_of_contents")) {
        m_totalPages++; // 目录页
    }
    QJsonArray parts = m_outline["parts"].toArray();
    for (const QJsonValue &partVal : parts) {
        QJsonObject part = partVal.toObject();
        m_totalPages += part["pages"].toArray().size();
    }

    const int requestedPageCount = extractRequestedPageCount(m_params.value("userRequest"));
    if (requestedPageCount > 0) {
        m_totalPages = requestedPageCount;
    } else {
        m_totalPages = qMax(m_totalPages, 4); // 默认至少 4 页
    }

    emit progressUpdated(25, "阶段1/3: 大纲生成完成",
                         QString("已规划 %1 页PPT结构").arg(m_totalPages));

    // 进入阶段2: 策划稿
    setState(State::GeneratingPlan);
    emit progressUpdated(30, "阶段2/3: 策划稿生成", "正在为每页规划版面内容...");
    startPlanGeneration();
}

QJsonObject ZhipuPPTAgentService::parseOutlineJson(const QString &response) const
{
    // 尝试提取 [PPT_OUTLINE]...[/PPT_OUTLINE] 标记内的 JSON
    static const QRegularExpression outlineRe(
        R"(\[PPT_OUTLINE\]\s*([\s\S]*?)\s*\[/PPT_OUTLINE\])",
        QRegularExpression::CaseInsensitiveOption);

    QString jsonStr;
    QRegularExpressionMatch match = outlineRe.match(response);
    if (match.hasMatch()) {
        jsonStr = match.captured(1).trimmed();
    } else {
        // 回退: 尝试找 ```json ... ``` 之间的内容
        static const QRegularExpression codeBlockRe(R"(```(?:json)?\s*([\s\S]*?)```)");
        match = codeBlockRe.match(response);
        if (match.hasMatch()) {
            jsonStr = match.captured(1).trimmed();
        } else {
            // 最后回退: 找第一个 { 到最后一个 }
            int start = response.indexOf('{');
            int end = response.lastIndexOf('}');
            if (start >= 0 && end > start) {
                jsonStr = response.mid(start, end - start + 1);
            }
        }
    }

    if (jsonStr.isEmpty()) {
        return QJsonObject();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[PPTAgent] Outline JSON parse error:" << parseError.errorString();
        return QJsonObject();
    }

    QJsonObject obj = doc.object();
    // 支持顶层 ppt_outline 包装
    if (obj.contains("ppt_outline")) {
        return obj["ppt_outline"].toObject();
    }
    return obj;
}

// ============================================================================
// 阶段2: 策划稿生成
// ============================================================================

void ZhipuPPTAgentService::startPlanGeneration()
{
    // 将大纲序列化为文本
    QJsonDocument outlineDoc(m_outline);
    QString outlineText = QString::fromUtf8(outlineDoc.toJson(QJsonDocument::Indented));

    QString userMsg = QString(
        "以下是一份思政课堂PPT的大纲 JSON：\n\n%1\n\n"
        "请为每一页（包括封面、目录、内容页、结束页）生成一段简洁的策划描述，说明：\n"
        "1. 该页的核心呈现信息\n"
        "2. 建议的版面布局（如：大标题居中、两栏对比、三卡片并列、数据图表等）\n"
        "3. 建议配色指引（整体采用党政红为主色调，#C00000）\n\n"
        "输出格式：每页用 \"===第N页===\" 分隔，紧跟策划内容。"
    ).arg(outlineText);

    m_currentReply = callZhipuApi(MODEL_TEXT, planSystemPrompt(), userMsg, 0.6, 8192);
    if (m_currentReply) {
        connect(m_currentReply, &QNetworkReply::finished,
                this, &ZhipuPPTAgentService::onPlanReplyFinished);
    }
}

void ZhipuPPTAgentService::onPlanReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();
    m_currentReply = nullptr;

    if (m_cancelled) {
        qDebug() << "[PPTAgent] 策划稿请求已被用户取消，静默退出";
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        QString errDetail = (reply->error() == QNetworkReply::OperationCanceledError)
            ? "请求被中止（可能是 SSL 证书问题或网络不稳定）"
            : reply->errorString();
        QString err = QString("策划稿生成失败: %1").arg(errDetail);
        qWarning() << "[PPTAgent]" << err;
        setState(State::Failed);
        emit errorOccurred(err);
        return;
    }

    QByteArray data = reply->readAll();
    QString content = extractContent(data);
    if (content.isEmpty()) {
        setState(State::Failed);
        emit errorOccurred("策划稿生成失败: AI 返回内容为空");
        return;
    }

    qDebug() << "[PPTAgent] Plan response length:" << content.length();

    // 按 "===第N页===" 分割
    static const QRegularExpression pageSplitRe(R"(===\s*第\d+页\s*===)");
    QStringList parts = content.split(pageSplitRe, Qt::SkipEmptyParts);

    m_pagePlans.clear();
    for (const QString &part : parts) {
        QString trimmed = part.trimmed();
        if (!trimmed.isEmpty()) {
            m_pagePlans.append(trimmed);
        }
    }

    // 如果分割失败，把整个内容当作一个策划
    if (m_pagePlans.isEmpty()) {
        m_pagePlans.append(content);
    }

    // 确保页数匹配
    while (m_pagePlans.size() < m_totalPages) {
        m_pagePlans.append(m_pagePlans.last());
    }

    qDebug() << "[PPTAgent] Plan generated:" << m_pagePlans.size() << "pages";
    emit progressUpdated(55, "阶段2/3: 策划稿完成",
                         QString("已完成 %1 页版面策划").arg(m_pagePlans.size()));

    // 进入阶段3: SVG 设计
    setState(State::GeneratingSVG);
    emit progressUpdated(60, "阶段3/3: SVG 设计生成", "开始生成精美页面设计...");
    m_currentSvgIndex = 0;
    startSvgGeneration();
}

// ============================================================================
// 阶段3: SVG 设计生成（逐页）
// ============================================================================

void ZhipuPPTAgentService::startSvgGeneration()
{
    generateNextSvg();
}

void ZhipuPPTAgentService::generateNextSvg()
{
    if (m_cancelled) return;

    if (m_currentSvgIndex >= m_totalPages) {
        // 全部生成完成
        setState(State::Finished);
        emit progressUpdated(100, "生成完成", QString("共生成 %1 页PPT").arg(m_svgCodes.size()));
        emit allSlidesGenerated(m_svgCodes, m_previewImages);
        return;
    }

    int progress = 60 + (m_currentSvgIndex * 40 / m_totalPages);
    emit progressUpdated(progress, "阶段3/3: SVG 设计生成",
                         QString("正在设计第 %1/%2 页...").arg(m_currentSvgIndex + 1).arg(m_totalPages));

    // 构建当前页的内容源
    QString pageContent = buildPageContent(m_currentSvgIndex);

    QString userMsg = QString(
        "请你根据以下思政课堂PPT页面策划，生成一张完整的 SVG 页面代码。\n\n"
        "页面策划内容：\n%1\n\n"
        "要求：\n"
        "- SVG viewBox 必须是 0 0 1280 720\n"
        "- 主色调使用党政红 #C00000，背景色 #1A1A2E 或白色 #FFFFFF\n"
        "- 采用便当网格(Bento Grid)卡片式布局\n"
        "- 文字使用中文，字体使用 Microsoft YaHei 或 SimHei\n"
        "- 只输出 <svg>...</svg> 代码，不要输出其他内容"
    ).arg(pageContent);

    m_currentReply = callZhipuApi(MODEL_CODE, svgSystemPrompt(), userMsg, 0.8, 16384);
    if (m_currentReply) {
        connect(m_currentReply, &QNetworkReply::finished,
                this, &ZhipuPPTAgentService::onSvgReplyFinished);
    }
}

void ZhipuPPTAgentService::onSvgReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();
    m_currentReply = nullptr;

    if (m_cancelled) {
        qDebug() << "[PPTAgent] SVG请求已被用户取消，静默退出";
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[PPTAgent] SVG generation error for page" << m_currentSvgIndex
                   << ":" << reply->errorString();
        // 生成一个错误占位 SVG
        QString fallbackSvg = QString(
            "<svg viewBox=\"0 0 1280 720\" xmlns=\"http://www.w3.org/2000/svg\">"
            "<rect width=\"1280\" height=\"720\" fill=\"#1A1A2E\"/>"
            "<text x=\"640\" y=\"360\" text-anchor=\"middle\" fill=\"#C00000\" "
            "font-size=\"36\" font-family=\"SimHei\">第 %1 页生成失败</text></svg>"
        ).arg(m_currentSvgIndex + 1);
        m_svgCodes.append(fallbackSvg);
        m_previewImages.append(renderSvgToImage(fallbackSvg));
    } else {
        QByteArray data = reply->readAll();
        QString content = extractContent(data);
        QString svgCode = extractSvgCode(content);

        if (svgCode.isEmpty()) {
            qWarning() << "[PPTAgent] No SVG found in response for page" << m_currentSvgIndex;
            svgCode = QString(
                "<svg viewBox=\"0 0 1280 720\" xmlns=\"http://www.w3.org/2000/svg\">"
                "<rect width=\"1280\" height=\"720\" fill=\"#FFFFFF\"/>"
                "<text x=\"640\" y=\"320\" text-anchor=\"middle\" fill=\"#333\" "
                "font-size=\"32\" font-family=\"SimHei\">第 %1 页</text>"
                "<text x=\"640\" y=\"380\" text-anchor=\"middle\" fill=\"#999\" "
                "font-size=\"20\" font-family=\"SimHei\">内容生成中...</text></svg>"
            ).arg(m_currentSvgIndex + 1);
        }

        m_svgCodes.append(svgCode);
        QImage preview = renderSvgToImage(svgCode);
        m_previewImages.append(preview);

        emit slideGenerated(m_currentSvgIndex, svgCode, preview);
    }

    // 继续下一页
    m_currentSvgIndex++;
    generateNextSvg();
}

// ============================================================================
// 辅助方法
// ============================================================================

QString ZhipuPPTAgentService::extractSvgCode(const QString &response) const
{
    // 先尝试 ```svg ... ``` 代码块
    static const QRegularExpression codeBlockRe(R"(```(?:svg|xml)?\s*([\s\S]*?)```)");
    QRegularExpressionMatch match = codeBlockRe.match(response);
    if (match.hasMatch()) {
        QString code = match.captured(1).trimmed();
        if (code.contains("<svg")) {
            return code;
        }
    }

    // 直接查找 <svg ... </svg>
    static const QRegularExpression svgRe(R"(<svg[\s\S]*?</svg>)",
                                           QRegularExpression::CaseInsensitiveOption);
    match = svgRe.match(response);
    if (match.hasMatch()) {
        return match.captured(0);
    }

    return QString();
}

QImage ZhipuPPTAgentService::renderSvgToImage(const QString &svgCode, int width, int height) const
{
    QSvgRenderer renderer(svgCode.toUtf8());
    if (!renderer.isValid()) {
        qWarning() << "[PPTAgent] Invalid SVG, creating placeholder";
        QImage img(width, height, QImage::Format_ARGB32);
        img.fill(Qt::darkGray);
        QPainter p(&img);
        p.setPen(Qt::white);
        p.setFont(QFont("SimHei", 24));
        p.drawText(img.rect(), Qt::AlignCenter, "SVG 渲染失败");
        return img;
    }

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderer.render(&painter);
    return image;
}

void ZhipuPPTAgentService::setState(State newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

// ============================================================================
// 页面内容构建（为 SVG 生成提供素材）
// ============================================================================

QString ZhipuPPTAgentService::buildPageContent(int pageIndex) const
{
    QString result;

    // 使用策划稿内容
    if (pageIndex < m_pagePlans.size()) {
        result = m_pagePlans[pageIndex];
    }

    // 如果策划稿不够详细，补充大纲信息
    if (result.length() < 50) {
        if (pageIndex == 0) {
            // 封面页
            QJsonObject cover = m_outline["cover"].toObject();
            result += QString("\n封面页 - 标题: %1, 副标题: %2")
                .arg(cover["title"].toString(),
                     cover["sub_title"].toString());
        } else if (pageIndex == m_totalPages - 1) {
            // 结束页
            QJsonObject endPage = m_outline["end_page"].toObject();
            result += QString("\n结束页 - 标题: %1")
                .arg(endPage["title"].toString());
        } else {
            // 内容页，从大纲 parts 中提取
            QJsonArray parts = m_outline["parts"].toArray();
            int contentPageIndex = pageIndex - 1; // 减去封面
            if (m_outline.contains("table_of_contents")) {
                contentPageIndex--; // 减去目录
            }

            int pageCounter = 0;
            for (const QJsonValue &partVal : parts) {
                QJsonObject part = partVal.toObject();
                QJsonArray pages = part["pages"].toArray();
                for (const QJsonValue &pageVal : pages) {
                    if (pageCounter == contentPageIndex) {
                        QJsonObject page = pageVal.toObject();
                        result += QString("\n内容页 - 标题: %1")
                            .arg(page["title"].toString());
                        QJsonArray contentArr = page["content"].toArray();
                        for (const QJsonValue &item : contentArr) {
                            result += "\n  • " + item.toString();
                        }
                        break;
                    }
                    pageCounter++;
                }
            }
        }
    }

    return result;
}

// ============================================================================
// Prompt 模板（源自 Linux.do 文章，适配思政课堂）
// ============================================================================

QString ZhipuPPTAgentService::outlineSystemPrompt()
{
    return QStringLiteral(
        "# Role: 顶级的PPT结构架构师\n"
        "## Profile\n"
        "- 版本：2.0 (Context-Aware)\n"
        "- 专业：PPT逻辑结构设计\n"
        "- 特长：运用金字塔原理构建清晰的演示逻辑\n"
        "\n"
        "## Goals\n"
        "基于用户提供的思政课堂教学主题，设计一份逻辑严密、层次清晰的PPT大纲。\n"
        "\n"
        "## Core Methodology: 金字塔原理\n"
        "1. 结论先行：每个部分以核心观点开篇\n"
        "2. 以上统下：上层观点是下层内容的总结\n"
        "3. 归类分组：同一层级的内容属于同一逻辑范畴\n"
        "4. 逻辑递进：内容按照某种逻辑顺序展开\n"
        "\n"
        "## 思政课堂特别要求\n"
        "- 紧密结合社会主义核心价值观\n"
        "- 体现爱国主义教育、法治教育等思政要素\n"
        "- 贴近学生实际生活，增强课堂感染力\n"
        "\n"
        "## 输出规范\n"
        "请严格按照以下JSON格式输出，结果用[PPT_OUTLINE]和[/PPT_OUTLINE]包裹：\n"
        "[PPT_OUTLINE]\n"
        "{\n"
        "  \"ppt_outline\": {\n"
        "    \"cover\": {\n"
        "      \"title\": \"引人注目的主标题\",\n"
        "      \"sub_title\": \"副标题\",\n"
        "      \"content\": []\n"
        "    },\n"
        "    \"table_of_contents\": {\n"
        "      \"title\": \"目录\",\n"
        "      \"content\": [\"第一部分标题\", \"第二部分标题\"]\n"
        "    },\n"
        "    \"parts\": [\n"
        "      {\n"
        "        \"part_title\": \"第一部分：章节标题\",\n"
        "        \"pages\": [\n"
        "          { \"title\": \"页面标题1\", \"content\": [\"要点1\", \"要点2\"] },\n"
        "          { \"title\": \"页面标题2\", \"content\": [\"要点1\", \"要点2\"] }\n"
        "        ]\n"
        "      }\n"
        "    ],\n"
        "    \"end_page\": {\n"
        "      \"title\": \"总结与展望\",\n"
        "      \"content\": []\n"
        "    }\n"
        "  }\n"
        "}\n"
        "[/PPT_OUTLINE]\n"
        "\n"
        "## Constraints\n"
        "1. 必须严格遵循JSON格式。\n"
        "2. 页数根据用户需求决定。如果用户指定了页数则严格遵守（如用户要2页就只生成2页），否则默认 8-12 页。"
    );
}

QString ZhipuPPTAgentService::planSystemPrompt()
{
    return QStringLiteral(
        "# Role: PPT 策划师\n"
        "## 背景\n"
        "你是一位顶级PPT设计公司的策划师。你的工作是在大纲和设计之间架起桥梁：\n"
        "将结构化大纲转化为详细的版面规划，让设计师（AI）能一页页高效执行。\n"
        "\n"
        "## 输出要求\n"
        "为每一页提供简洁的策划描述，包括：\n"
        "1. 核心呈现信息：该页需要展示的关键文字内容\n"
        "2. 版面布局建议：标题位置、内容分区、卡片数量和尺寸\n"
        "3. 配色指引：主色 #C00000（党政红），辅色建议\n"
        "\n"
        "## 格式\n"
        "每页用 \"===第N页===\" 分隔，N 从 1 开始。"
    );
}

QString ZhipuPPTAgentService::svgSystemPrompt()
{
    return QStringLiteral(
        "作为精通信息架构与 SVG 编码的专家，你的任务是将完整的文字内容转化为一张高质量、"
        "结构化、具备高级感、简洁感和专业感的 SVG 演示文稿页面。\n\n"
        "要求如下：\n"
        "1. 画布: SVG viewBox 必须是 0 0 1280 720。\n"
        "2. 内容页的便当网格 (Bento Grid) 布局\n"
        "   这是一种灵活的网格系统，其布局应由内容本身的需求驱动，而非僵硬的模板。\n"
        "   通过组合不同尺寸的卡片，创造出动态且视觉有趣的布局。\n"
        "   - 核心原则:\n"
        "     - 灵活性: 卡片数量不固定。可以是 1, 2, 3, 4, 5 或更多个。\n"
        "     - 层级感: 使用卡片尺寸建立视觉层级。最重要的信息放在最大的卡片上。\n"
        "     - 留白: 在所有卡片之间保持至少 20px 的间距。\n"
        "   - 布局组合示例:\n"
        "     - 单一焦点: 一张大卡片覆盖大部分区域 (w=1200, h=580)。\n"
        "     - 两栏布局: 50/50 对称或非对称（2/3 + 1/3）。\n"
        "     - 三栏布局: 三张等宽的卡片，适合并列比较。\n"
        "     - 顶部英雄式: 顶部一张宽幅卡片，下方是2-4个较小卡片网格。\n"
        "     - 混合网格: 自由混合各种尺寸的卡片。\n\n"
        "3. 颜色主题:\n"
        "   - 主色: #C00000（党政红）\n"
        "   - 背景色: #1A1A2E（深蓝黑）或 #FFFFFF（白色）\n"
        "   - 卡片背景: rgba(255,255,255,0.08) 或 #F5F5F5\n"
        "   - 文字颜色: #FFFFFF（深色背景）或 #333333（浅色背景）\n\n"
        "4. 字体: font-family 使用 \"Microsoft YaHei\", \"SimHei\", sans-serif\n"
        "5. 只输出 <svg>...</svg> 代码，不要包含 ```svg 标记或其他说明文字。\n"
        "6. 所有文字必须使用中文。"
    );
}
