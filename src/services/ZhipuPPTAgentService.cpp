#include "ZhipuPPTAgentService.h"
#include "../config/AiConfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QNetworkProxy>
#include <QUrl>
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
    , m_baseUrl(AiConfig::zhipuBaseUrl())
{
    // 从环境变量配置 HTTP 代理（Qt 不会自动读取 http_proxy/https_proxy）
    QString proxyUrl = qEnvironmentVariable("https_proxy");
    if (proxyUrl.isEmpty()) {
        proxyUrl = qEnvironmentVariable("http_proxy");
    }
    if (!proxyUrl.isEmpty()) {
        QUrl pUrl(proxyUrl);
        if (pUrl.isValid() && !pUrl.host().isEmpty()) {
            QNetworkProxy proxy(QNetworkProxy::HttpProxy,
                                pUrl.host(),
                                static_cast<quint16>(pUrl.port(8080)));
            m_networkManager->setProxy(proxy);
            qDebug() << "[PPTAgent] Using proxy:" << pUrl.host() << ":" << pUrl.port();
        }
    }

    qDebug() << "[PPTAgent] Service initialized, baseUrl:" << m_baseUrl
             << "apiKey length:" << AiConfig::zhipuApiKey().length();
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
        emit errorOccurred("BigModel API Key 未设置");
        return;
    }

    // 重置状态
    m_cancelled = false;
    m_params = params;
    m_outline = QJsonObject();
    m_pagePlans.clear();
    m_pageLayouts = QJsonArray();
    m_useLayoutDriven = false;
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

    topic = QString("%1 %2 中学《道德与法治》课 %3").arg(textbook, grade, chapter);

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

QNetworkRequest ZhipuPPTAgentService::createRequest(const QString &baseUrlOverride) const
{
    const QString requestBaseUrl = baseUrlOverride.isEmpty() ? m_baseUrl : baseUrlOverride;
    QUrl url(requestBaseUrl + "/chat/completions");
    QNetworkRequest request(url);

    // Bearer 认证
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    // 禁用 HTTP/2（macOS 兼容）
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    // 超时
    request.setTransferTimeout(180000); // 3 分钟

    // SSL 配置：始终放宽验证，兼容本地代理和自定义 OpenAI-compatible 端点。
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
                                                     int maxTokens,
                                                     bool useStructuredUserContent,
                                                     bool disableThinking,
                                                     const QString &baseUrlOverride)
{
    QNetworkRequest request = createRequest(baseUrlOverride);

    QJsonObject body;
    body["model"] = model;
    body["stream"] = false;
    body["temperature"] = temperature;
    body["max_tokens"] = maxTokens;
    if (disableThinking) {
        body["thinking"] = QJsonObject{{"type", "disabled"}};
    }

    QJsonArray messages;

    if (!systemPrompt.isEmpty()) {
        QJsonObject sysMsg;
        sysMsg["role"] = "system";
        sysMsg["content"] = systemPrompt;
        messages.append(sysMsg);
    }

    QJsonObject userMsg;
    userMsg["role"] = "user";
    if (useStructuredUserContent) {
        QJsonArray contentParts;
        contentParts.append(QJsonObject{
            {"type", "text"},
            {"text", userMessage}
        });
        userMsg["content"] = contentParts;
    } else {
        userMsg["content"] = userMessage;
    }
    messages.append(userMsg);

    body["messages"] = messages;

    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    qDebug() << "[PPTAgent] API call to model:" << model
             << "url:" << request.url().toString()
             << "body size:" << data.size();

    QNetworkReply *reply = m_networkManager->post(request, data);
    
    // 无条件忽略 SSL 错误（已在 createRequest 中设置 VerifyNone）
    connect(reply, &QNetworkReply::sslErrors,
            this, [reply](const QList<QSslError> &errors) {
        qDebug() << "[PPTAgent] Ignoring SSL errors:" << errors.size();
        reply->ignoreSslErrors(errors);
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
        "请为以下中学《道德与法治》课堂教学内容生成PPT大纲：\n\n%1\n\n"
        "要求：\n"
        "- 页数根据用户需求决定，如果用户指定了页数则严格遵守，否则默认 8-12 页\n"
        "- 必须包含封面页和结束页\n"
        "- 每个页面的 content 数组中放入该页要展示的核心要点（2-4 条）\n"
        "- 内容必须贴合中学生理解水平和课堂表达\n"
        "- 结合思政教育的特点，体现社会主义核心价值观"
    ).arg(topic);

    // 注入用户偏好（如果有）
    const QString scene = m_params.value("pref_scene");
    const QString style = m_params.value("pref_style");
    const QString focus = m_params.value("pref_focus");
    const QString pace  = m_params.value("pref_pace");
    if (!scene.isEmpty() || !style.isEmpty() || !focus.isEmpty() || !pace.isEmpty()) {
        userMsg += QLatin1String("\n\n用户偏好：");
        if (!scene.isEmpty()) userMsg += QString("\n- 授课场景：%1").arg(scene);
        if (!style.isEmpty()) userMsg += QString("\n- 表达风格：%1").arg(style);
        if (!focus.isEmpty()) userMsg += QString("\n- 内容重点：%1").arg(focus);
        if (!pace.isEmpty())  userMsg += QString("\n- 呈现节奏：%1").arg(pace);
    }

    m_currentReply = callZhipuApi(MODEL_TEXT, outlineSystemPrompt(), userMsg, 0.7, 4096,
                                  false, true);
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
    emit artifactGenerated(
        "大纲 JSON",
        "json",
        QString::fromUtf8(QJsonDocument(m_outline).toJson(QJsonDocument::Indented)));
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

    // 系统提示需要注入总页数
    QString sysPrompt = planSystemPrompt().arg(m_totalPages);

    QString userMsg = QString(
        "以下是一份中学《道德与法治》课堂PPT的大纲 JSON（共 %1 页）：\n\n%2\n\n"
        "请严格按照上述格式输出布局指令 JSON。"
    ).arg(m_totalPages).arg(outlineText);

    // 注入用户偏好（如果有）
    const QString scene = m_params.value("pref_scene");
    const QString style = m_params.value("pref_style");
    const QString focus = m_params.value("pref_focus");
    const QString pace  = m_params.value("pref_pace");
    if (!scene.isEmpty() || !style.isEmpty() || !focus.isEmpty() || !pace.isEmpty()) {
        userMsg += QLatin1String("\n\n用户偏好要求：");
        if (!scene.isEmpty()) userMsg += QString("\n- 授课场景：%1").arg(scene);
        if (!style.isEmpty()) userMsg += QString("\n- 表达风格：%1").arg(style);
        if (!focus.isEmpty()) userMsg += QString("\n- 内容侧重：%1").arg(focus);
        if (!pace.isEmpty())  userMsg += QString("\n- 呈现节奏：%1").arg(pace);
    }

    m_currentReply = callZhipuApi(MODEL_TEXT, sysPrompt, userMsg, 0.6, 8192,
                                  false, true);
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

    // 优先尝试解析结构化 JSON 布局指令
    QJsonObject layoutObj = parseLayoutJson(content);
    QJsonArray layoutPages = layoutObj["pages"].toArray();
    if (layoutPages.size() == m_totalPages) {
        m_pageLayouts = layoutPages;
        m_useLayoutDriven = true;
        emit artifactGenerated("页面布局 JSON", "json",
                               QString::fromUtf8(QJsonDocument(layoutObj).toJson(QJsonDocument::Indented)));
        qDebug() << "[PPTAgent] Layout-driven mode: parsed" << layoutPages.size() << "page layouts";
    } else {
        // 回退旧流程
        m_useLayoutDriven = false;
        emit artifactGenerated("页面策划文本", "text", content);
        qWarning() << "[PPTAgent] JSON layout pages mismatch. expected="
                   << m_totalPages << "got=" << layoutPages.size() << ", falling back to text plan";
        m_pagePlans = splitPlanPages(content, m_totalPages);
        if (m_pagePlans.size() != m_totalPages) {
            qWarning() << "[PPTAgent] 版面策划分页数量异常，改用大纲兜底";
            m_pagePlans.clear();
            for (int i = 0; i < m_totalPages; ++i) {
                m_pagePlans.append(buildOutlineOnlyPageContent(i));
            }
        }
    }

    qDebug() << "[PPTAgent] Plan generated, layout-driven=" << m_useLayoutDriven;
    emit progressUpdated(55, "阶段2/3: 策划稿完成",
                         QString("已完成 %1 页版面策划").arg(m_totalPages));

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
    // 始终使用 BigModel 逐页 AI 生成 SVG
    qDebug() << "[PPTAgent] Using AI-driven SVG generation (BigModel)";
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

    m_svgRetriedWithStandardEndpoint = false;
    m_svgRetriedForRender = false;

    // 构建当前页的内容源，并限制长度，避免将整份策划误塞进单页请求。
    const QString pageContent = clampSvgPromptContent(buildPageContent(m_currentSvgIndex));
    emit artifactGenerated(QString("第 %1 页页面策划").arg(m_currentSvgIndex + 1),
                           "text", pageContent);

    QString userMsg = QString(
        "请你根据以下中学《道德与法治》课堂PPT页面策划，生成一张完整的 SVG 页面代码。\n\n"
        "页面策划内容：\n%1\n\n"
        "要求：\n"
        "- SVG viewBox 必须是 0 0 1280 720\n"
        "- 面向中学《道德与法治》课堂，整体气质要端正、清爽、适合课堂教学与公开课展示\n"
        "- 主色调使用党政红 #C00000，背景和大色块以白色、米白、浅红、浅金、浅灰等淡色为主\n"
        "- 禁止黑色、深灰、深蓝等大面积深色背景，不要做暗黑、科技黑、商务黑金风格\n"
        "- 正文和说明文字优先使用深灰色 #333333 / #555555，不要使用大面积白字压深色底\n"
        "- 采用便当网格(Bento Grid)卡片式布局\n"
        "- 文字使用中文，字体使用 Microsoft YaHei 或 SimHei\n"
        "- 必须生成 Qt QSvgRenderer 可渲染的基础 SVG\n"
        "- 只使用 svg、rect、text、tspan、line、circle、ellipse、path、polygon 标签\n"
        "- 禁止 style、class、defs、filter、mask、clipPath、pattern、image、foreignObject\n"
        "- 禁止 CSS、动画、渐变、阴影、HTML、外链资源和 url(#...) 引用\n"
        "- 颜色使用十六进制色值；透明度用 opacity，不要使用 rgba()\n"
        "- 样式必须写成 SVG 原生属性，例如 fill、stroke、font-size、font-family、opacity\n"
        "- 只输出 <svg>...</svg> 代码，不要输出其他内容"
    ).arg(pageContent);

    // 将系统约束与页面内容合并为单条 text-part user 消息。
    m_currentSvgPrompt = svgSystemPrompt() + "\n\n任务输入：\n" + userMsg;
    requestCurrentSvgPage(false);
}

void ZhipuPPTAgentService::requestCurrentSvgPage(bool useStandardEndpoint)
{
    const QString overrideBaseUrl = useStandardEndpoint
        ? QString::fromUtf8(STANDARD_PAASE_URL)
        : QString();

    m_currentReply = callZhipuApi(
        MODEL_CODE,
        QString(),
        m_currentSvgPrompt,
        0.6,
        8192,
        true,
        true,
        overrideBaseUrl);

    if (m_currentReply) {
        connect(m_currentReply, &QNetworkReply::finished,
                this, &ZhipuPPTAgentService::onSvgReplyFinished);
    }
}

void ZhipuPPTAgentService::onSvgReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray responseData = reply->readAll();
    reply->deleteLater();
    m_currentReply = nullptr;

    if (m_cancelled) {
        qDebug() << "[PPTAgent] SVG请求已被用户取消，静默退出";
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[PPTAgent] SVG generation error for page" << m_currentSvgIndex
                   << "http=" << httpStatus
                   << "detail=" << reply->errorString()
                   << "body=" << QString::fromUtf8(responseData.left(600));

        if (httpStatus == 400 && !m_svgRetriedWithStandardEndpoint) {
            // coding/paas 偶发 400 时，自动切到标准 paas 端点再试一次，保持 turbo 不变。
            m_svgRetriedWithStandardEndpoint = true;
            emit progressUpdated(60 + (m_currentSvgIndex * 40 / m_totalPages),
                                 "阶段3/3: SVG 设计生成",
                                 QString("第 %1 页请求异常，正在切换标准接口重试...").arg(m_currentSvgIndex + 1));
            requestCurrentSvgPage(true);
            return;
        }

        qWarning() << "[PPTAgent] SVG generation error for page" << m_currentSvgIndex
                   << ":" << reply->errorString();
        // 生成一个错误占位 SVG
        QString fallbackSvg = QString(
            "<svg viewBox=\"0 0 1280 720\" xmlns=\"http://www.w3.org/2000/svg\">"
            "<rect width=\"1280\" height=\"720\" fill=\"#FFF9F2\"/>"
            "<text x=\"640\" y=\"360\" text-anchor=\"middle\" fill=\"#C00000\" "
            "font-size=\"36\" font-family=\"SimHei\">第 %1 页生成失败</text></svg>"
        ).arg(m_currentSvgIndex + 1);
        emit artifactGenerated(QString("第 %1 页错误占位 SVG").arg(m_currentSvgIndex + 1),
                               "svg", fallbackSvg);
        m_svgCodes.append(fallbackSvg);
        m_previewImages.append(renderSvgToImage(fallbackSvg));
    } else {
        QString content = extractContent(responseData);
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

        emit artifactGenerated(QString("第 %1 页 SVG 代码").arg(m_currentSvgIndex + 1),
                               "svg", svgCode);
        bool renderOk = false;
        QImage preview = renderSvgToImage(svgCode, 1280, 720, &renderOk);
        if (!renderOk && !m_svgRetriedForRender) {
            qWarning() << "[PPTAgent] SVG render failed for page" << m_currentSvgIndex
                       << ", retrying with stricter Qt-safe prompt";
            m_svgRetriedForRender = true;
            emit progressUpdated(60 + (m_currentSvgIndex * 40 / m_totalPages),
                                 "阶段3/3: SVG 设计生成",
                                 QString("第 %1 页渲染失败，正在生成兼容版本...")
                                     .arg(m_currentSvgIndex + 1));
            emit artifactGenerated(QString("第 %1 页渲染重试说明").arg(m_currentSvgIndex + 1),
                                   "text",
                                   "上一版 SVG 无法被 Qt 渲染，正在要求模型输出更简单的兼容 SVG。");
            m_currentSvgPrompt += QStringLiteral(
                "\n\n重要修正：上一版 SVG 无法被 Qt QSvgRenderer 渲染。"
                "请重新输出一版更简单的 Qt 兼容 SVG："
                "禁止 style、class、defs、filter、mask、clipPath、pattern、image、foreignObject、"
                "渐变、阴影、动画、CSS、HTML、外链资源和 url(#...) 引用；"
                "只使用基础 SVG 标签和 fill/stroke/font-size/font-family/opacity 等原生属性；"
                "只输出 <svg>...</svg>。");
            requestCurrentSvgPage(m_svgRetriedWithStandardEndpoint);
            return;
        }

        m_svgCodes.append(svgCode);
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

QString ZhipuPPTAgentService::sanitizeSvg(const QString &svgCode) const
{
    QString svg = svgCode.trimmed();

    // 1. 补全缺失的 xmlns 声明（QSvgRenderer 必需）
    if (!svg.contains(QLatin1String("xmlns"))) {
        svg.replace(QRegularExpression(R"(<svg\b)"),
                    R"(<svg xmlns="http://www.w3.org/2000/svg")");
    }

    // 2. 移除 <script> 标签
    static const QRegularExpression scriptRe(R"(<script[\s\S]*?</script>)",
                                              QRegularExpression::CaseInsensitiveOption);
    svg.remove(scriptRe);

    // 3. 移除 <image> 标签引用外部资源（QSvgRenderer 无法加载外部图片）
    static const QRegularExpression imageRe(R"(<image\b[^>]*(?:href|xlink:href)=[^>]*>(?:</image>)?)",
                                             QRegularExpression::CaseInsensitiveOption);
    svg.remove(imageRe);

    // 4. 移除 foreignObject（QSvgRenderer 不支持）
    static const QRegularExpression foreignRe(
        R"(<foreignObject[\s\S]*?</foreignObject>)",
        QRegularExpression::CaseInsensitiveOption);
    svg.remove(foreignRe);

    // 5. 移除 Qt 渲染器容易失败的复杂 SVG 定义块。
    const QStringList unsupportedBlocks = {
        "defs", "filter", "mask", "clipPath", "pattern"
    };
    for (const QString &tag : unsupportedBlocks) {
        const QRegularExpression blockRe(
            QStringLiteral("<%1\\b[\\s\\S]*?</%1>").arg(tag),
            QRegularExpression::CaseInsensitiveOption);
        svg.remove(blockRe);
    }

    // 6. 移除 class 和 url(#...) 引用，避免引用已被清掉的样式或定义。
    static const QRegularExpression classAttrRe(
        R"(\sclass\s*=\s*(?:"[^"]*"|'[^']*'))",
        QRegularExpression::CaseInsensitiveOption);
    svg.remove(classAttrRe);

    static const QRegularExpression urlAttrDoubleRe(
        R"(\s[\w:-]+\s*=\s*"[^"]*url\(#.*?\)[^"]*")",
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression urlAttrSingleRe(
        R"(\s[\w:-]+\s*=\s*'[^']*url\(#.*?\)[^']*')",
        QRegularExpression::CaseInsensitiveOption);
    svg.remove(urlAttrDoubleRe);
    svg.remove(urlAttrSingleRe);

    // 7. 清理 <style> 块
    static const QRegularExpression styleBlockRe(
        R"(<style[\s\S]*?</style>)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = styleBlockRe.globalMatch(svg);
    // 收集替换对（从后向前替换，避免位移问题）
    QVector<QPair<int, QPair<int, QString>>> replacements;
    while (it.hasNext()) {
        auto match = it.next();
        QString styleBlock = match.captured(0);
        QString cleaned = styleBlock;

        // 7a. 移除 @keyframes 块
        static const QRegularExpression keyframesRe(
            R"(@keyframes\s+[\w-]+\s*\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\})",
            QRegularExpression::CaseInsensitiveOption);
        cleaned.remove(keyframesRe);

        // 7b. 移除 @media 块
        static const QRegularExpression mediaRe(
            R"(@media\s+[^{]*\{[\s\S]*?\}\s*\})",
            QRegularExpression::CaseInsensitiveOption);
        cleaned.remove(mediaRe);

        // 7c. 移除 @import
        static const QRegularExpression importRe(
            R"(@import\s+[^;]+;)",
            QRegularExpression::CaseInsensitiveOption);
        cleaned.remove(importRe);

        // 7d. 移除 Qt SVG 不支持的 CSS 属性（保留 opacity，它是合法的）
        static const QRegularExpression unsupportedPropRe(
            R"([a-zA-Z-]+\s*:\s*[^;}]*(?:flex|grid|gap|pointer-events|cursor|backdrop-filter|box-shadow|text-shadow|overflow|clip-path|mask|animation|transition|transform-origin|object-fit|z-index|position\s*:\s*(?:absolute|relative|fixed))[^;}]*;?)",
            QRegularExpression::CaseInsensitiveOption);
        cleaned.remove(unsupportedPropRe);

        if (cleaned != styleBlock) {
            replacements.append({static_cast<int>(match.capturedStart()),
                                 {static_cast<int>(match.capturedLength()), cleaned}});
        }
    }
    // 从后向前替换
    for (int i = replacements.size() - 1; i >= 0; --i) {
        svg.replace(replacements[i].first, replacements[i].second.first, replacements[i].second.second);
    }

    // 8. 移除内联样式中的不支持属性（保留 opacity）
    static const QRegularExpression inlineStyleRe("style=\\\"([^\\\"]*)\\\"");
    it = inlineStyleRe.globalMatch(svg);
    replacements.clear();
    while (it.hasNext()) {
        auto match = it.next();
        QString styleVal = match.captured(1);
        QString cleaned = styleVal;
        static const QRegularExpression badInlinePropRe(
            R"((?:flex|grid|gap|pointer-events|cursor|backdrop-filter|box-shadow|text-shadow|overflow|clip-path|mask|animation|transition|transform-origin|object-fit|z-index|position\s*:\s*(?:absolute|relative|fixed))[^;]*;?)",
            QRegularExpression::CaseInsensitiveOption);
        cleaned.remove(badInlinePropRe);
        // 处理 CSS filter 属性。
        static const QRegularExpression cssFilterRe(
            R"(filter\s*:\s*(?!url\()[^;]*;?)",
            QRegularExpression::CaseInsensitiveOption);
        cleaned.remove(cssFilterRe);
        if (cleaned != styleVal) {
            replacements.append({static_cast<int>(match.capturedStart()),
                                 {static_cast<int>(match.capturedLength()),
                                 QStringLiteral("style=\"%1\"").arg(cleaned)}});
        }
    }
    for (int i = replacements.size() - 1; i >= 0; --i) {
        svg.replace(replacements[i].first, replacements[i].second.first, replacements[i].second.second);
    }

    // 9. 移除 HTML 实体引用。
    svg.replace("&nbsp;", " ");
    svg.replace("&mdash;", QString(QChar(0x2014)));
    svg.replace("&ndash;", QString(QChar(0x2013)));
    svg.replace("&ldquo;", QString(QChar(0x201C)));
    svg.replace("&rdquo;", QString(QChar(0x201D)));
    svg.replace("&lsquo;", QString(QChar(0x2018)));
    svg.replace("&rsquo;", QString(QChar(0x2019)));
    svg.replace("&hellip;", QString(QChar(0x2026)));
    svg.replace("&bull;", QString(QChar(0x2022)));

    // 10. 将 rgba() 改成 Qt SVG 更稳定的十六进制颜色。
    static const QRegularExpression rgbaRe(
        R"(rgba\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(?:0|1|0?\.\d+)\s*\))",
        QRegularExpression::CaseInsensitiveOption);
    it = rgbaRe.globalMatch(svg);
    replacements.clear();
    while (it.hasNext()) {
        auto match = it.next();
        const int r = qBound(0, match.captured(1).toInt(), 255);
        const int g = qBound(0, match.captured(2).toInt(), 255);
        const int b = qBound(0, match.captured(3).toInt(), 255);
        const QString hex = QString("#%1%2%3")
                                .arg(r, 2, 16, QLatin1Char('0'))
                                .arg(g, 2, 16, QLatin1Char('0'))
                                .arg(b, 2, 16, QLatin1Char('0'))
                                .toUpper();
        replacements.append({static_cast<int>(match.capturedStart()),
                             {static_cast<int>(match.capturedLength()), hex}});
    }
    for (int i = replacements.size() - 1; i >= 0; --i) {
        svg.replace(replacements[i].first, replacements[i].second.first, replacements[i].second.second);
    }

    return svg;
}

QImage ZhipuPPTAgentService::renderSvgToImage(const QString &svgCode, int width, int height,
                                              bool *ok) const
{
    if (ok) {
        *ok = false;
    }

    // 先尝试原始 SVG
    QString svg = sanitizeSvg(svgCode);
    QSvgRenderer renderer(svg.toUtf8());

    // 如果仍然无效，尝试更激进的清理
    if (!renderer.isValid()) {
        qWarning() << "[PPTAgent] SVG still invalid after sanitize, trying aggressive cleanup";
        // 移除所有 <style> 块作为最后手段
        static const QRegularExpression allStyleRe(R"(<style[\s\S]*?</style>)");
        QString aggressive = svg;
        aggressive.remove(allStyleRe);
        renderer.load(aggressive.toUtf8());
        if (renderer.isValid()) {
            svg = aggressive;
        }
    }

    if (!renderer.isValid()) {
        qWarning() << "[PPTAgent] Invalid SVG after all cleanup attempts, creating placeholder";
        QImage img(width, height, QImage::Format_ARGB32);
        img.fill(QColor("#FFF9F2"));
        QPainter p(&img);
        p.setPen(QColor("#C00000"));
        p.setFont(QFont("SimHei", 24));
        p.drawText(img.rect(), Qt::AlignCenter, "SVG 渲染失败");
        return img;
    }

    if (ok) {
        *ok = true;
    }

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderer.render(&painter);
    return image;
}

// ============================================================================
// 布局驱动的 SVG 生成
// ============================================================================

QJsonObject ZhipuPPTAgentService::parseLayoutJson(const QString &response) const
{
    // 尝试提取 [PPT_LAYOUT]...[/PPT_LAYOUT]
    static const QRegularExpression layoutRe(
        R"(\[PPT_LAYOUT\]\s*([\s\S]*?)\s*\[/PPT_LAYOUT\])",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = layoutRe.match(response);

    QString jsonStr;
    if (match.hasMatch()) {
        jsonStr = match.captured(1).trimmed();
    } else {
        // 回退: ```json ... ```
        static const QRegularExpression codeBlockRe(R"(```(?:json)?\s*([\s\S]*?)```)");
        match = codeBlockRe.match(response);
        if (match.hasMatch()) {
            jsonStr = match.captured(1).trimmed();
        } else {
            int start = response.indexOf('{');
            int end = response.lastIndexOf('}');
            if (start >= 0 && end > start) {
                jsonStr = response.mid(start, end - start + 1);
            }
        }
    }

    if (jsonStr.isEmpty()) return QJsonObject();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[PPTAgent] Layout JSON parse error:" << parseError.errorString();
        return QJsonObject();
    }
    return doc.object();
}

QString ZhipuPPTAgentService::escapeXml(const QString &text) const
{
    QString s = text;
    s.replace("&", "&amp;");
    s.replace("<", "&lt;");
    s.replace(">", "&gt;");
    s.replace("\"", "&quot;");
    return s;
}

QString ZhipuPPTAgentService::wrapText(const QString &text, int charsPerLine) const
{
    if (text.length() <= charsPerLine) return escapeXml(text);
    QStringList lines;
    QString remaining = text;
    while (remaining.length() > charsPerLine) {
        lines.append(escapeXml(remaining.left(charsPerLine)));
        remaining = remaining.mid(charsPerLine);
    }
    if (!remaining.isEmpty()) {
        lines.append(escapeXml(remaining));
    }
    return lines.join(QStringLiteral("</tspan><tspan x=\"0\" dy=\"28\">"));
}

QString ZhipuPPTAgentService::buildSvgFromLayout(const QJsonObject &pageLayout, int pageIndex) const
{
    const QString type = pageLayout["type"].toString("content");

    if (type == "cover") return buildCoverSvg(pageLayout);
    if (type == "toc")   return buildTocSvg(pageLayout);
    if (type == "end")   return buildEndSvg(pageLayout);

    // 默认 content
    return buildContentSvg(pageLayout, pageIndex);
}

QString ZhipuPPTAgentService::buildCoverSvg(const QJsonObject &layout) const
{
    const QString title = escapeXml(layout["title"].toString("思政课堂"));
    const QString subtitle = escapeXml(layout["subtitle"].toString());

    return QStringLiteral(
        "<svg viewBox=\"0 0 1280 720\" xmlns=\"http://www.w3.org/2000/svg\">"
        // 整体背景渐变
        "<defs>"
        "<linearGradient id=\"cbg\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"1\">"
        "<stop offset=\"0%\" stop-color=\"#FFFFFF\"/>"
        "<stop offset=\"100%\" stop-color=\"#FFF8F0\"/>"
        "</linearGradient>"
        "</defs>"
        "<rect width=\"1280\" height=\"720\" fill=\"url(#cbg)\"/>"
        // 左侧大色块装饰
        "<rect x=\"0\" y=\"0\" width=\"320\" height=\"720\" fill=\"#C00000\"/>"
        "<rect x=\"300\" y=\"0\" width=\"20\" height=\"720\" fill=\"#A00000\"/>"
        // 左侧装饰圆
        "<circle cx=\"160\" cy=\"200\" r=\"60\" fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"3\" opacity=\"0.3\"/>"
        "<circle cx=\"100\" cy=\"500\" r=\"40\" fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"2\" opacity=\"0.2\"/>"
        // 左侧竖排装饰文字
        "<text x=\"160\" y=\"420\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"20\" fill=\"#FFFFFF\" opacity=\"0.5\" writing-mode=\"tb\">道德与法治</text>"
        // 主标题（右区域居中）
        "<text x=\"800\" y=\"310\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"52\" font-weight=\"bold\" fill=\"#333333\">%1</text>"
        // 副标题
        "<text x=\"800\" y=\"380\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"22\" fill=\"#888888\">%2</text>"
        // 分隔线
        "<line x1=\"640\" y1=\"420\" x2=\"960\" y2=\"420\" stroke=\"#C00000\" stroke-width=\"3\"/>"
        // 底部装饰
        "<rect x=\"320\" y=\"680\" width=\"960\" height=\"40\" fill=\"#FFF0E0\"/>"
        "<text x=\"800\" y=\"707\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"14\" fill=\"#C00000\">中学思政课堂</text>"
        "</svg>"
    ).arg(title, subtitle);
}

QString ZhipuPPTAgentService::buildTocSvg(const QJsonObject &layout) const
{
    const QString pageTitle = escapeXml(layout["title"].toString("目录"));
    QJsonArray items = layout["items"].toArray();

    QString svg =
        "<svg viewBox=\"0 0 1280 720\" xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect width=\"1280\" height=\"720\" fill=\"#FFFFFF\"/>"
        // 顶部红色区域
        "<rect x=\"0\" y=\"0\" width=\"1280\" height=\"140\" fill=\"#C00000\"/>"
        "<text x=\"640\" y=\"90\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"40\" font-weight=\"bold\" fill=\"#FFFFFF\">%1</text>"
        // 底部浅色装饰
        "<rect x=\"0\" y=\"680\" width=\"1280\" height=\"40\" fill=\"#FFF5F0\"/>";

    const int count = qMin(items.size(), 6);
    if (count <= 3) {
        // 横排大卡片
        const int cardW = (1100 - (count - 1) * 30) / count;
        const int cardH = 440;
        const int startY = 180;
        for (int i = 0; i < count; ++i) {
            const int x = 90 + i * (cardW + 30);
            const QString item = escapeXml(items[i].toString());
            svg += QString(
                "<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"#FFF9F5\" rx=\"12\" stroke=\"#E8C8B0\" stroke-width=\"1\"/>"
                "<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"8\" fill=\"#C00000\" rx=\"4\"/>"
                "<circle cx=\"%5\" cy=\"%6\" r=\"28\" fill=\"#C00000\"/>"
                "<text x=\"%5\" y=\"%7\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
                "font-size=\"22\" fill=\"#FFFFFF\" font-weight=\"bold\">%8</text>"
                "<text x=\"%5\" y=\"%9\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
                "font-size=\"20\" font-weight=\"bold\" fill=\"#333333\" lengthAdjust=\"spacingAndGlyphs\" textLength=\"%10\">%11</text>"
            ).arg(x).arg(startY).arg(cardW).arg(cardH)
             .arg(x + cardW / 2).arg(startY + 80).arg(startY + 88)
             .arg(i + 1)
             .arg(startY + 160)
             .arg(qMin(cardW - 40, 300))
             .arg(item);
        }
    } else {
        // 左侧竖排列表
        const int startY = 180;
        const int itemH = 75;
        for (int i = 0; i < count; ++i) {
            const int y = startY + i * itemH;
            const QString item = escapeXml(items[i].toString());
            const QString accentBg = (i % 2 == 0) ? "#FFF5F0" : "#FFFFFF";
            svg += QString(
                "<rect x=\"80\" y=\"%1\" width=\"1120\" height=\"%2\" fill=\"%3\" rx=\"8\"/>"
                "<rect x=\"80\" y=\"%1\" width=\"60\" height=\"%2\" fill=\"#C00000\" rx=\"8\"/>"
                "<text x=\"110\" y=\"%4\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
                "font-size=\"22\" fill=\"#FFFFFF\" font-weight=\"bold\">%5</text>"
                "<text x=\"170\" y=\"%4\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
                "font-size=\"22\" fill=\"#333333\" font-weight=\"bold\">%6</text>"
            ).arg(y).arg(itemH - 10).arg(accentBg)
             .arg(y + (itemH - 10) / 2 + 8)
             .arg(i + 1).arg(item);
        }
    }

    svg += "</svg>";
    return svg.arg(pageTitle);
}

QString ZhipuPPTAgentService::buildContentSvg(const QJsonObject &layout, int pageIndex) const
{
    const QString pageTitle = escapeXml(layout["title"].toString("课堂内容"));
    QJsonArray cards = layout["cards"].toArray();
    const int cardCount = qMax(1, qMin(cards.size(), 5));

    // 页面配色方案：根据 pageIndex 循环选用不同的侧边装饰色
    const QString sideColors[] = {"#C00000", "#B8860B", "#2E7D32", "#1565C0", "#8E24AA"};
    const QString cardAccents[] = {"#C00000", "#D4380D", "#B8860B", "#2E7D32", "#1565C0"};
    const QString cardBgs[] = {"#FFFFFF", "#FFF7F7", "#F9F6F1", "#F5FAF5", "#F5F8FF"};
    const QString sideColor = sideColors[pageIndex % 5];

    // 公共头部：标题栏 + 左侧装饰
    auto header = [&](QString &svg) {
        svg += QString(
            // 左侧装饰条
            "<rect x=\"0\" y=\"0\" width=\"10\" height=\"720\" fill=\"%1\"/>"
            // 顶部标题栏背景
            "<rect x=\"10\" y=\"0\" width=\"1270\" height=\"90\" fill=\"#FAFAFA\"/>"
            "<rect x=\"10\" y=\"86\" width=\"1270\" height=\"4\" fill=\"%1\" opacity=\"0.3\"/>"
            // 标题文字
            "<text x=\"60\" y=\"55\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
            "font-size=\"30\" font-weight=\"bold\" fill=\"#333333\">%2</text>"
            // 页码
            "<text x=\"1220\" y=\"55\" text-anchor=\"end\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
            "font-size=\"14\" fill=\"#AAAAAA\">%3 / %4</text>"
        ).arg(sideColor, pageTitle).arg(pageIndex + 1).arg(m_totalPages);
    };

    // 单个卡片内容渲染器
    auto renderCard = [&](int x, int y, int w, int h, const QJsonObject &card, int accentIdx) {
        const QString cTitle = escapeXml(card["title"].toString());
        const QString cContent = escapeXml(card["content"].toString());
        const QString accent = cardAccents[accentIdx % 5];
        const QString bg = cardBgs[accentIdx % 5];
        QString s;
        s += QString(
            "<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"%5\" rx=\"10\" stroke=\"#E8E8E8\" stroke-width=\"1\"/>"
            // 左侧色条
            "<rect x=\"%1\" y=\"%2\" width=\"5\" height=\"%4\" fill=\"%6\" rx=\"2\"/>"
        ).arg(x).arg(y).arg(w).arg(h).arg(bg).arg(accent);

        // 卡片标题
        s += QString(
            "<text x=\"%1\" y=\"%2\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
            "font-size=\"20\" font-weight=\"bold\" fill=\"%3\">%4</text>"
        ).arg(x + 20).arg(y + 35).arg(accent).arg(cTitle);

        // 卡片内容（支持长文本换行）
        if (cContent.length() > 28) {
            // 长文本：用 tspan 换行
            s += QString(
                "<text x=\"%1\" y=\"%2\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
                "font-size=\"16\" fill=\"#555555\">"
                "<tspan x=\"%1\" dy=\"0\">%3</tspan>"
                "<tspan x=\"%1\" dy=\"24\">%4</tspan>"
                "</text>"
            ).arg(x + 20).arg(y + 70)
             .arg(wrapText(cContent.left(28), 28))
             .arg(wrapText(cContent.mid(28), 28));
        } else {
            s += QString(
                "<text x=\"%1\" y=\"%2\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
                "font-size=\"16\" fill=\"#555555\">%3</text>"
            ).arg(x + 20).arg(y + 70).arg(escapeXml(cContent));
        }
        return s;
    };

    const int contentTop = 100;
    const int contentH = 600;
    const int margin = 30;
    const int gap = 20;
    const int usableW = 1280 - margin * 2 - 10; // 减去左侧装饰条

    QString svg =
        "<svg viewBox=\"0 0 1280 720\" xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect width=\"1280\" height=\"720\" fill=\"#FFFFFF\"/>";

    header(svg);

    // 根据卡片数量选择不同布局
    if (cardCount == 1) {
        // 布局A: 居中大卡片 + 引号装饰
        QJsonObject card = cards[0].toObject();
        const QString cTitle = escapeXml(card["title"].toString());
        const QString cContent = escapeXml(card["content"].toString());
        svg += QString(
            // 大卡片
            "<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"#FFFDF8\" rx=\"14\" stroke=\"#E8D8C8\" stroke-width=\"1\"/>"
            "<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"8\" fill=\"%5\" rx=\"4\"/>"
            // 引号装饰
            "<text x=\"%6\" y=\"%7\" font-family=\"Georgia,serif\" font-size=\"80\" fill=\"%5\" opacity=\"0.15\">\"</text>"
            // 标题
            "<text x=\"%8\" y=\"%9\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
            "font-size=\"28\" font-weight=\"bold\" fill=\"#333333\">%10</text>"
            // 内容
            "<text x=\"%8\" y=\"%11\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
            "font-size=\"20\" fill=\"#555555\">%12</text>"
        ).arg(margin + 10).arg(contentTop).arg(usableW).arg(contentH)
         .arg(sideColor)
         .arg(margin + 40).arg(contentTop + 100)
         .arg(margin + 60).arg(contentTop + 120)
         .arg(cTitle)
         .arg(contentTop + 200)
         .arg(escapeXml(cContent));

    } else if (cardCount == 2) {
        // 布局B: 左右对比两栏
        const int cardW = (usableW - gap) / 2;
        for (int i = 0; i < 2; ++i) {
            const int x = margin + 10 + i * (cardW + gap);
            svg += renderCard(x, contentTop, cardW, contentH, cards[i].toObject(), pageIndex * 2 + i);
        }

    } else if (cardCount == 3) {
        // 布局C: 三栏并列
        const int cardW = (usableW - 2 * gap) / 3;
        for (int i = 0; i < 3; ++i) {
            const int x = margin + 10 + i * (cardW + gap);
            svg += renderCard(x, contentTop, cardW, contentH, cards[i].toObject(), pageIndex + i);
        }

    } else if (cardCount == 4) {
        // 布局D: 2x2 网格
        const int cardW = (usableW - gap) / 2;
        const int cardH = (contentH - gap) / 2;
        for (int i = 0; i < 4; ++i) {
            const int col = i % 2;
            const int row = i / 2;
            const int x = margin + 10 + col * (cardW + gap);
            const int y = contentTop + row * (cardH + gap);
            svg += renderCard(x, y, cardW, cardH, cards[i].toObject(), pageIndex + i);
        }

    } else {
        // 布局E: 左侧大卡片 + 右侧 2x2 小网格 (5 cards)
        const int leftW = usableW * 2 / 5;
        const int rightW = usableW - leftW - gap;
        const int smallW = (rightW - gap) / 2;
        const int smallH = (contentH - gap) / 2;

        // 左侧大卡片
        svg += renderCard(margin + 10, contentTop, leftW, contentH, cards[0].toObject(), pageIndex);

        // 右侧 2x2
        for (int i = 0; i < 4 && i + 1 < cardCount; ++i) {
            const int col = i % 2;
            const int row = i / 2;
            const int x = margin + 10 + leftW + gap + col * (smallW + gap);
            const int y = contentTop + row * (smallH + gap);
            svg += renderCard(x, y, smallW, smallH, cards[i + 1].toObject(), pageIndex + i + 1);
        }
    }

    svg += "</svg>";
    return svg;
}

QString ZhipuPPTAgentService::buildEndSvg(const QJsonObject &layout) const
{
    const QString title = escapeXml(layout["title"].toString("感谢聆听"));
    const QString subtitle = escapeXml(layout["subtitle"].toString("共同进步，一起成长"));

    return QStringLiteral(
        "<svg viewBox=\"0 0 1280 720\" xmlns=\"http://www.w3.org/2000/svg\">"
        // 背景渐变
        "<defs>"
        "<linearGradient id=\"ebg\" x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\">"
        "<stop offset=\"0%\" stop-color=\"#FFFFFF\"/>"
        "<stop offset=\"100%\" stop-color=\"#FFF5F0\"/>"
        "</linearGradient>"
        "</defs>"
        "<rect width=\"1280\" height=\"720\" fill=\"url(#ebg)\"/>"
        // 右侧大装饰色块
        "<rect x=\"960\" y=\"0\" width=\"320\" height=\"720\" fill=\"#C00000\"/>"
        "<rect x=\"950\" y=\"0\" width=\"10\" height=\"720\" fill=\"#A00000\"/>"
        // 右侧装饰圆
        "<circle cx=\"1120\" cy=\"180\" r=\"50\" fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"2\" opacity=\"0.3\"/>"
        "<circle cx=\"1050\" cy=\"550\" r=\"35\" fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"2\" opacity=\"0.2\"/>"
        // 左上装饰
        "<rect x=\"60\" y=\"60\" width=\"100\" height=\"4\" fill=\"#C00000\" rx=\"2\"/>"
        "<rect x=\"60\" y=\"60\" width=\"4\" height=\"100\" fill=\"#C00000\" rx=\"2\"/>"
        // 主标题
        "<text x=\"480\" y=\"320\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"50\" font-weight=\"bold\" fill=\"#333333\">%1</text>"
        // 副标题
        "<text x=\"480\" y=\"390\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"22\" fill=\"#888888\">%2</text>"
        // 分隔线
        "<line x1=\"300\" y1=\"430\" x2=\"660\" y2=\"430\" stroke=\"#C00000\" stroke-width=\"2\"/>"
        // 底部点缀
        "<rect x=\"60\" y=\"660\" width=\"860\" height=\"1\" fill=\"#E0D0C0\"/>"
        "<text x=\"480\" y=\"690\" text-anchor=\"middle\" font-family=\"Microsoft YaHei,SimHei,sans-serif\" "
        "font-size=\"14\" fill=\"#C00000\">中学思政课堂</text>"
        "</svg>"
    ).arg(title, subtitle);
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
        result += buildOutlineOnlyPageContent(pageIndex);
    }

    return result;
}

QString ZhipuPPTAgentService::buildOutlineOnlyPageContent(int pageIndex) const
{
    QString result;

    if (pageIndex == 0) {
        const QJsonObject cover = m_outline["cover"].toObject();
        result += QString("封面页\n标题：%1\n副标题：%2")
            .arg(cover["title"].toString(),
                 cover["sub_title"].toString());
        return result;
    }

    if (pageIndex == m_totalPages - 1) {
        const QJsonObject endPage = m_outline["end_page"].toObject();
        result += QString("结束页\n标题：%1\n展示一句课堂总结或感谢语")
            .arg(endPage["title"].toString());
        return result;
    }

    if (m_outline.contains("table_of_contents") && pageIndex == 1) {
        const QJsonObject toc = m_outline["table_of_contents"].toObject();
        result += QString("目录页\n标题：%1").arg(toc["title"].toString("目录"));
        const QJsonArray tocItems = toc["content"].toArray();
        for (const QJsonValue &item : tocItems) {
            result += "\n- " + item.toString();
        }
        return result;
    }

    const QJsonArray parts = m_outline["parts"].toArray();
    int contentPageIndex = pageIndex - 1; // 减去封面
    if (m_outline.contains("table_of_contents")) {
        contentPageIndex--; // 减去目录
    }

    int pageCounter = 0;
    for (const QJsonValue &partVal : parts) {
        const QJsonObject part = partVal.toObject();
        const QJsonArray pages = part["pages"].toArray();
        for (const QJsonValue &pageVal : pages) {
            if (pageCounter == contentPageIndex) {
                const QJsonObject page = pageVal.toObject();
                result += QString("内容页\n所属部分：%1\n标题：%2")
                    .arg(part["part_title"].toString(),
                         page["title"].toString());
                const QJsonArray contentArr = page["content"].toArray();
                for (const QJsonValue &item : contentArr) {
                    result += "\n- " + item.toString();
                }
                return result;
            }
            pageCounter++;
        }
    }

    return QString("内容页\n标题：课堂重点\n- 提炼知识点\n- 保持课堂讲授风格");
}

QString ZhipuPPTAgentService::clampSvgPromptContent(const QString &text, int maxChars) const
{
    QString normalized = text.trimmed();
    normalized.replace(QRegularExpression(R"(\n{3,})"), "\n\n");
    if (normalized.size() <= maxChars) {
        return normalized;
    }

    QString truncated = normalized.left(maxChars).trimmed();
    truncated += "\n\n（其余策划细节已省略，请优先保留标题、要点、版式和淡色思政风格完成 SVG 设计。）";
    qWarning() << "[PPTAgent] 单页 SVG 提示过长，已截断。original="
               << normalized.size() << "truncated=" << truncated.size();
    return truncated;
}

QStringList ZhipuPPTAgentService::splitPlanPages(const QString &content, int expectedPages) const
{
    const QString normalized = content.trimmed();
    if (normalized.isEmpty()) {
        return {};
    }

    static const QRegularExpression markerRe(
        R"((?:^|\n)\s*(?:===\s*)?第\s*(?:\d+|[一二三四五六七八九十百]+)\s*页(?:\s*===|[\:\：])?)");

    QList<QRegularExpressionMatch> markers;
    QRegularExpressionMatchIterator it = markerRe.globalMatch(normalized);
    while (it.hasNext()) {
        markers.append(it.next());
    }

    if (markers.isEmpty()) {
        return {};
    }

    QStringList result;
    for (int i = 0; i < markers.size(); ++i) {
        const int start = markers[i].capturedEnd();
        const int end = (i + 1 < markers.size()) ? markers[i + 1].capturedStart() : normalized.size();
        const QString pageText = normalized.mid(start, end - start).trimmed();
        if (!pageText.isEmpty()) {
            result.append(pageText);
        }
    }

    if (expectedPages > 0 && result.size() != expectedPages) {
        return {};
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
        "## 中学《道德与法治》课堂特别要求\n"
        "- 紧密结合社会主义核心价值观\n"
        "- 体现爱国主义教育、法治教育、规则意识、公民责任等核心主题\n"
        "- 贴近中学生实际生活，增强课堂感染力与可理解性\n"
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
        "# Role: 中学《道德与法治》PPT 布局规划师\n"
        "\n"
        "## 任务\n"
        "根据 PPT 大纲，为每一页生成结构化 JSON 布局指令。每页指定页面类型、布局方式和卡片内容。\n"
        "\n"
        "## 输出格式\n"
        "严格输出以下 JSON，用 [PPT_LAYOUT] 和 [/PPT_LAYOUT] 包裹：\n"
        "[PPT_LAYOUT]\n"
        "{\n"
        "  \"pages\": [\n"
        "    {\n"
        "      \"type\": \"cover\",\n"
        "      \"title\": \"课件标题\",\n"
        "      \"subtitle\": \"副标题（年级+教材）\"\n"
        "    },\n"
        "    {\n"
        "      \"type\": \"toc\",\n"
        "      \"title\": \"目录\",\n"
        "      \"items\": [\"第一部分\", \"第二部分\", \"第三部分\"]\n"
        "    },\n"
        "    {\n"
        "      \"type\": \"content\",\n"
        "      \"title\": \"页面标题\",\n"
        "      \"cards\": [\n"
        "        {\"title\": \"要点1标题\", \"content\": \"要点1的具体阐述文字\"},\n"
        "        {\"title\": \"要点2标题\", \"content\": \"要点2的具体阐述文字\"},\n"
        "        {\"title\": \"要点3标题\", \"content\": \"要点3的具体阐述文字\"}\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"type\": \"end\",\n"
        "      \"title\": \"感谢聆听\",\n"
        "      \"subtitle\": \"共勉语句\"\n"
        "    }\n"
        "  ]\n"
        "}\n"
        "[/PPT_LAYOUT]\n"
        "\n"
        "## 页面类型\n"
        "- cover: 封面页，只需 title + subtitle\n"
        "- toc: 目录页，需要 items 数组列出各部分标题\n"
        "- content: 内容页，需要 cards 数组（2-5 个卡片）\n"
        "- end: 结束页，只需 title + subtitle\n"
        "\n"
        "## 约束\n"
        "- pages 数量必须与大纲一致（共 %1 页）\n"
        "- 第1页必须是 cover，最后一页必须是 end\n"
        "- 每个 card.title 不超过 15 字\n"
        "- 每个 card.content 不超过 80 字\n"
        "- 内容必须贴合中学《道德与法治》课堂，体现思政教育特点\n"
        "- 只输出 JSON，不要输出其他说明文字\n"
    );
}

QString ZhipuPPTAgentService::svgSystemPrompt()
{
    return QStringLiteral(
        "作为精通信息架构与 SVG 编码的专家，你的任务是将完整的文字内容转化为一张高质量、"
        "结构化、适用于中学《道德与法治》课堂的 SVG 演示文稿页面。\n\n"
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
        "3. 视觉风格:\n"
        "   - 适合中学《道德与法治》课堂，整体端正、清爽、温和，兼顾思政课堂气质\n"
        "   - 可采用淡色课堂风格或思政风格，但必须保持明亮，不得做暗黑风格\n"
        "   - 禁止黑色、深灰、深蓝等大面积深色背景，禁止赛博风、科技暗黑风、商务黑金风\n\n"
        "4. 颜色主题:\n"
        "   - 主色: #C00000（党政红），用于标题、重点信息、分割线、少量装饰\n"
        "   - 背景色: #FFFFFF、#FFF9F2、#F8F4EF、#F7F3EA 等淡色\n"
        "   - 卡片背景: #FFFFFF、#FFF7F7、#F9F6F1、#F5F5F5\n"
        "   - 文字颜色: #333333、#555555，避免大面积白字压深底\n\n"
        "5. 字体: font-family 使用 \"Microsoft YaHei\", \"SimHei\", sans-serif\n"
        "6. 只输出 <svg>...</svg> 代码，不要包含 ```svg 标记或其他说明文字。\n"
        "7. 所有文字必须使用中文。"
    );
}
