#include "QuestionParserService.h"
#include "../utils/NetworkRequestFactory.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QDebug>
#include <QRegularExpression>
#include <QHttpMultiPart>
#include <QMimeDatabase>
#include <QFileInfo>

QuestionParserService::QuestionParserService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_uploadReply(nullptr)
    , m_uploadFile(nullptr)
    , m_baseUrl("https://api.dify.ai/v1")
{
    // 初始化 SSE 解析器回调
    m_sseParser.setEventHandler([this](const QString &event, const QJsonObject &obj) {
        handleSseEvent(event, obj);
    });
}

QuestionParserService::~QuestionParserService()
{
    cancelActiveOperation();
}

void QuestionParserService::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void QuestionParserService::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = baseUrl;
}

QString QuestionParserService::lastError() const
{
    return m_lastError;
}

bool QuestionParserService::isConfigured() const
{
    return !m_apiKey.isEmpty();
}

void QuestionParserService::cancelCurrentReply()
{
    if (!m_currentReply) {
        return;
    }

    m_currentReply->disconnect(this);
    m_currentReply->abort();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void QuestionParserService::cancelUploadReply()
{
    if (m_uploadReply) {
        m_uploadReply->disconnect(this);
        m_uploadReply->abort();
        m_uploadReply->deleteLater();
        m_uploadReply = nullptr;
    }

    if (m_uploadFile) {
        m_uploadFile->close();
        delete m_uploadFile;
        m_uploadFile = nullptr;
    }
}

void QuestionParserService::cancelActiveOperation()
{
    cancelCurrentReply();
    cancelUploadReply();
}

void QuestionParserService::parseDocument(const QString &documentText,
                                          const QString &subject,
                                          const QString &grade)
{
    m_lastError.clear();
    
    if (m_apiKey.isEmpty()) {
        m_lastError = "API Key 未设置";
        emit errorOccurred(m_lastError);
        return;
    }
    
    if (documentText.trimmed().isEmpty()) {
        m_lastError = "文档内容为空";
        emit errorOccurred(m_lastError);
        return;
    }
    
    cancelActiveOperation();
    
    // 保存元数据
    m_currentSubject = subject;
    m_currentGrade = grade;
    m_fullResponse.clear();
    m_sseParser.reset();
    m_hasStreamError = false;

    // 构建 Dify 工作流 API 请求
    // 使用 /workflows/run 端点
    QUrl url(m_baseUrl + "/workflows/run");
    QNetworkRequest request = NetworkRequestFactory::createDifyRequest(url, m_apiKey, 300000);
    
    // 构建请求体
    // 输入变量名与 Dify 工作流中定义的变量名一致
    QJsonObject inputs;
    inputs["document_text"] = documentText;
    
    qDebug() << "[QuestionParserService] 输入变量: document_text, 长度:" << documentText.length();
    
    QJsonObject body;
    body["inputs"] = inputs;
    body["response_mode"] = "streaming";
    body["user"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson();
    
    qDebug() << "[QuestionParserService] 发送解析请求，文档长度:" << documentText.length();
    qDebug() << "[QuestionParserService] URL:" << url.toString();
    
    emit parseStarted();
    
    m_currentReply = m_networkManager->post(request, jsonData);
    
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &QuestionParserService::onReplyFinished);
    connect(m_currentReply, &QNetworkReply::readyRead, 
            this, &QuestionParserService::onReadyRead);
}

// ==================== 文件上传模式 ====================

void QuestionParserService::parseFile(const QString &filePath,
                                      const QString &subject,
                                      const QString &grade)
{
    m_lastError.clear();
    
    if (m_apiKey.isEmpty()) {
        m_lastError = "API Key 未设置";
        emit errorOccurred(m_lastError);
        return;
    }
    
    if (!QFile::exists(filePath)) {
        m_lastError = "文件不存在: " + filePath;
        emit errorOccurred(m_lastError);
        return;
    }
    
    cancelActiveOperation();

    // 保存元数据
    m_currentFilePath = filePath;
    m_currentSubject = subject;
    m_currentGrade = grade;
    m_fullResponse.clear();
    m_sseParser.reset();
    m_hasStreamError = false;
    m_uploadedFileId.clear();

    emit parseStarted();
    
    // 开始上传文件
    uploadFileToDify(filePath);
}

void QuestionParserService::uploadFileToDify(const QString &filePath)
{
    qDebug() << "[QuestionParserService] 开始上传文件:" << filePath;
    
    cancelUploadReply();
    
    // 打开文件
    m_uploadFile = new QFile(filePath);
    if (!m_uploadFile->open(QIODevice::ReadOnly)) {
        m_lastError = "无法打开文件: " + filePath;
        emit errorOccurred(m_lastError);
        delete m_uploadFile;
        m_uploadFile = nullptr;
        return;
    }
    
    // 构建 multipart 请求
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    // 添加文件部分
    QHttpPart filePart;
    QFileInfo fileInfo(filePath);
    QMimeDatabase mimeDb;
    QString mimeType = mimeDb.mimeTypeForFile(fileInfo).name();
    
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mimeType));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QString("form-data; name=\"file\"; filename=\"%1\"")
                                .arg(fileInfo.fileName())));
    filePart.setBodyDevice(m_uploadFile);
    m_uploadFile->setParent(multiPart);  // multiPart 会负责删除文件
    m_uploadFile = nullptr;  // 不再需要手动管理
    multiPart->append(filePart);
    
    // 添加 user 参数
    QHttpPart userPart;
    userPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"user\""));
    userPart.setBody(QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8());
    multiPart->append(userPart);
    
    // 创建请求
    QUrl url(m_baseUrl + "/files/upload");
    QNetworkRequest request = NetworkRequestFactory::createDifyUploadRequest(url, m_apiKey, 120000);
    
    qDebug() << "[QuestionParserService] 上传 URL:" << url.toString();
    qDebug() << "[QuestionParserService] 文件 MIME 类型:" << mimeType;
    
    m_uploadReply = m_networkManager->post(request, multiPart);
    multiPart->setParent(m_uploadReply);  // 请求完成后自动删除
    
    connect(m_uploadReply, &QNetworkReply::finished,
            this, &QuestionParserService::onUploadFinished);
    connect(m_uploadReply, &QNetworkReply::uploadProgress,
            this, &QuestionParserService::onUploadProgress);
}

void QuestionParserService::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    emit uploadProgress(bytesSent, bytesTotal);
}

void QuestionParserService::onUploadFinished()
{
    if (!m_uploadReply) return;
    
    QNetworkReply::NetworkError error = m_uploadReply->error();
    QByteArray responseData = m_uploadReply->readAll();
    
    m_uploadReply->deleteLater();
    m_uploadReply = nullptr;
    
    if (error != QNetworkReply::NoError) {
        m_lastError = QString("文件上传失败: %1").arg(QString(responseData));
        qWarning() << "[QuestionParserService] 上传错误:" << m_lastError;
        emit errorOccurred(m_lastError);
        return;
        
    }
    
    // 解析响应获取文件 ID
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        m_lastError = "上传响应格式错误";
        emit errorOccurred(m_lastError);
        return;
    }
    
    QJsonObject obj = doc.object();
    m_uploadedFileId = obj["id"].toString();
    
    if (m_uploadedFileId.isEmpty()) {
        m_lastError = "未能获取上传文件 ID";
        emit errorOccurred(m_lastError);
        return;
    }
    
    qDebug() << "[QuestionParserService] 文件上传成功，ID:" << m_uploadedFileId;
    
    // 调用工作流
    callWorkflowWithFile(m_uploadedFileId);
}

void QuestionParserService::callWorkflowWithFile(const QString &uploadFileId)
{
    qDebug() << "[QuestionParserService] 调用工作流，文件 ID:" << uploadFileId;
    
    // 取消之前的请求
    cancelCurrentReply();
    
    m_fullResponse.clear();
    m_sseParser.reset();
    m_hasStreamError = false;

    // 构建工作流请求
    QUrl url(m_baseUrl + "/workflows/run");
    QNetworkRequest request = NetworkRequestFactory::createDifyRequest(url, m_apiKey, 600000);
    
    // 构建请求体 - 文件需要放在 inputs 中，变量名与 Dify 工作流开始节点定义的一致
    // Dify 工作流中定义的文件输入变量名是 "up"，类型是文件列表（数组）
    QJsonObject fileObj;
    fileObj["type"] = "document";
    fileObj["transfer_method"] = "local_file";
    fileObj["upload_file_id"] = uploadFileId;

    // 文件列表格式：将文件对象放入数组中
    QJsonArray fileArray;
    fileArray.append(fileObj);

    QJsonObject inputs;
    inputs["up"] = fileArray;  // 变量名与 Dify 工作流开始节点定义的一致
    
    QJsonObject body;
    body["inputs"] = inputs;
    body["response_mode"] = "streaming";
    body["user"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson();
    
    qDebug() << "[QuestionParserService] 工作流请求体:" << QString(jsonData);
    
    m_currentReply = m_networkManager->post(request, jsonData);
    
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &QuestionParserService::onReplyFinished);
    connect(m_currentReply, &QNetworkReply::readyRead, 
            this, &QuestionParserService::onReadyRead);
}

void QuestionParserService::onReadyRead()
{
    if (!m_currentReply) return;

    QByteArray data = m_currentReply->readAll();
    if (!data.isEmpty()) {
        m_sseParser.feed(data);
    }
}

void QuestionParserService::onReplyFinished()
{
    if (!m_currentReply) return;
    
    QNetworkReply::NetworkError error = m_currentReply->error();
    
    if (error != QNetworkReply::NoError &&
        error != QNetworkReply::RemoteHostClosedError) {
        if (m_sseParser.hasPendingData()) {
            qDebug() << "[QuestionParserService] Flush pending stream on error";
            m_sseParser.flush();
        }
        QString errorMsg = m_currentReply->errorString();
        int httpStatus = m_currentReply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
            
        QByteArray errorData = m_currentReply->readAll();
        qDebug() << "[QuestionParserService] 请求错误:" << error 
                 << "HTTP 状态:" << httpStatus;
        
        // 尝试从错误响应中提取具体信息
        if (!errorData.isEmpty()) {
            qDebug() << "[QuestionParserService] 错误响应:" << QString::fromUtf8(errorData);
            QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
            if (errorDoc.isObject()) {
                QJsonObject errorObj = errorDoc.object();
                if (errorObj.contains("message")) {
                    errorMsg = errorObj["message"].toString();
                } else if (errorObj.contains("code")) {
                    errorMsg = QString("错误代码: %1").arg(errorObj["code"].toString());
                }
            }
        }
        
        m_lastError = QString("请求失败: %1 (HTTP %2)").arg(errorMsg).arg(httpStatus);
        emit errorOccurred(m_lastError);
    } else {
        // 解析完成，处理完整响应
        qDebug() << "[QuestionParserService] 请求完成，响应长度:"
                 << m_fullResponse.length();

        if (m_sseParser.hasPendingData()) {
            qDebug() << "[QuestionParserService] Flush pending stream on finish";
            m_sseParser.flush();
        }

        if (m_hasStreamError) {
            qDebug() << "[QuestionParserService] 已发生流式错误，跳过 parseCompleted";
        } else {
            // 检查是否收到了有效数据
            if (m_fullResponse.isEmpty()) {
                m_lastError = "工作流超时或未返回数据。请检查 Dify 工作流配置和网络连接。";
                qWarning() << "[QuestionParserService]" << m_lastError;
                emit errorOccurred(m_lastError);
                m_currentReply->deleteLater();
                m_currentReply = nullptr;
                return;
            }

            // 从完整响应中提取 JSON
            m_lastError.clear();
            QList<PaperQuestion> questions = parseJsonResponse(m_fullResponse);
            if (questions.isEmpty()) {
                if (m_lastError.isEmpty()) {
                    m_lastError = "未解析到有效题目";
                }
                emit errorOccurred(m_lastError);
            } else {
                emit parseCompleted(questions);
            }
        }
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

// SSE 事件业务处理（由 SseStreamParser 回调）
void QuestionParserService::handleSseEvent(const QString &event, const QJsonObject &obj)
{
    if (event == "text_chunk") {
        QJsonObject dataObj = obj["data"].toObject();
        QString text = dataObj["text"].toString();
        if (text.isEmpty()) {
            text = obj["text"].toString();
        }
        m_fullResponse += text;
        emit parseProgress(text);

    } else if (event == "workflow_finished") {
        QJsonObject dataObj = obj["data"].toObject();
        QJsonObject outputs = dataObj["outputs"].toObject();

        qDebug() << "[QuestionParserService] workflow_finished";
        qDebug() << "[QuestionParserService] outputs 所有键:" << outputs.keys();

        // 工作流输出可能在 outputs.result 或其他字段
        QString result = outputs["result"].toString();
        if (result.isEmpty()) {
            result = outputs["text"].toString();
        }
        if (result.isEmpty()) {
            result = outputs["output"].toString();
        }
        if (result.isEmpty()) {
            for (const QString &key : outputs.keys()) {
                QJsonValue val = outputs[key];
                if (val.isString() && !val.toString().isEmpty()) {
                    result = val.toString();
                    qDebug() << "[QuestionParserService] 使用字段:" << key;
                    break;
                }
            }
        }

        if (!result.isEmpty()) {
            m_fullResponse = result;
            qDebug() << "[QuestionParserService] 获取到输出，长度:" << result.length();
        }

    } else if (event == "workflow_started") {
        qDebug() << "[QuestionParserService] 工作流已启动";

    } else if (event == "node_started" || event == "node_finished") {
        QJsonObject dataObj = obj["data"].toObject();
        QString nodeTitle = dataObj["title"].toString();
        if (!nodeTitle.isEmpty()) {
            qDebug() << "[QuestionParserService] 节点" << (event == "node_started" ? "开始" : "完成") << ":" << nodeTitle;
        }

    } else if (event == "error") {
        QJsonObject dataObj = obj["data"].toObject();
        QString errorMsg = dataObj["message"].toString();
        if (errorMsg.isEmpty()) {
            errorMsg = obj["message"].toString();
        }
        m_lastError = QString("工作流错误: %1").arg(errorMsg);
        m_hasStreamError = true;
        qWarning() << "[QuestionParserService]" << m_lastError;
        emit errorOccurred(m_lastError);
    }
}

QList<PaperQuestion> QuestionParserService::parseJsonResponse(const QString &jsonText)
{
    QList<PaperQuestion> questions;

    // 尝试从响应中提取 JSON
    QString cleanJson = jsonText.trimmed();

    // 移除 <think>...</think> 标签及其内容（AI 的思考过程）
    static const QRegularExpression thinkRe("<think>.*?</think>", QRegularExpression::DotMatchesEverythingOption);
    cleanJson.remove(thinkRe);
    cleanJson = cleanJson.trimmed();

    // 检查是否是工作流直接插入数据库的报告格式
    // 格式示例：数据插入报告\n- 总计: 15\n- 成功: 6\n- 失败: 9
    static const QRegularExpression successRe("成功[：:]\\s*(\\d+)");
    QRegularExpressionMatch successMatch = successRe.match(cleanJson);
    if (successMatch.hasMatch()) {
        int successCount = successMatch.captured(1).toInt();
        qDebug() << "[QuestionParserService] 工作流已直接插入数据库，成功:" << successCount << "道题目";

        // 创建虚拟题目列表，仅用于计数
        for (int i = 0; i < successCount; i++) {
            PaperQuestion q;
            q.stem = QString("已由工作流插入 #%1").arg(i + 1);
            questions.append(q);
        }
        return questions;
    }

    // 如果响应被 markdown 代码块包裹，提取其中的 JSON
    static const QRegularExpression codeBlockRe("```(?:json)?\\s*([\\s\\S]*?)```");
    QRegularExpressionMatch match = codeBlockRe.match(cleanJson);
    if (match.hasMatch()) {
        cleanJson = match.captured(1).trimmed();
    }
    
    // 尝试找到 JSON 对象或数组的起始位置
    int jsonStart = cleanJson.indexOf('{');
    int arrayStart = cleanJson.indexOf('[');
    
    if (jsonStart == -1 && arrayStart == -1) {
        m_lastError = "响应中未找到有效的 JSON";
        qDebug() << "[QuestionParserService]" << m_lastError;
        qDebug() << "[QuestionParserService] 原始响应:" << jsonText.left(500);
        return questions;
    }
    
    // 选择最早出现的位置
    int startPos = jsonStart;
    if (arrayStart != -1 && (jsonStart == -1 || arrayStart < jsonStart)) {
        startPos = arrayStart;
    }
    
    cleanJson = cleanJson.mid(startPos);
    
    // 查找 JSON 结束位置，处理末尾有额外内容的情况
    // 通过匹配括号来找到正确的结束位置
    int braceCount = 0;
    int bracketCount = 0;
    int endPos = -1;
    bool inString = false;
    bool escaped = false;
    
    for (int i = 0; i < cleanJson.length(); i++) {
        QChar c = cleanJson.at(i);
        
        if (escaped) {
            escaped = false;
            continue;
        }
        
        if (c == '\\') {
            escaped = true;
            continue;
        }
        
        if (c == '"') {
            inString = !inString;
            continue;
        }
        
        if (inString) continue;
        
        if (c == '{') braceCount++;
        else if (c == '}') {
            braceCount--;
            if (braceCount == 0 && bracketCount == 0) {
                endPos = i + 1;
                break;
            }
        }
        else if (c == '[') bracketCount++;
        else if (c == ']') {
            bracketCount--;
            if (braceCount == 0 && bracketCount == 0) {
                endPos = i + 1;
                break;
            }
        }
    }
    
    if (endPos > 0 && endPos < cleanJson.length()) {
        qDebug() << "[QuestionParserService] 截断 JSON 末尾额外内容，从" << cleanJson.length() << "截断到" << endPos;
        cleanJson = cleanJson.left(endPos);
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(cleanJson.toUtf8(), &parseError);
    
    // 如果解析失败，尝试处理双重转义的情况
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "[QuestionParserService] 首次解析失败:" << parseError.errorString();
        
        // 尝试反转义：将 \\" 变为 "，将 \\\\ 变为 \\，将 \\n 变为 \n
        QString unescaped = cleanJson;
        unescaped.replace("\\\"", "\"");
        unescaped.replace("\\\\", "\\");
        unescaped.replace("\\n", "\n");
        unescaped.replace("\\t", "\t");
        
        // 重新找到 JSON 起始位置
        int newStart = unescaped.indexOf('{');
        int newArrayStart = unescaped.indexOf('[');
        if (newArrayStart != -1 && (newStart == -1 || newArrayStart < newStart)) {
            newStart = newArrayStart;
        }
        if (newStart != -1) {
            unescaped = unescaped.mid(newStart);
        }
        
        doc = QJsonDocument::fromJson(unescaped.toUtf8(), &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            m_lastError = QString("JSON 解析错误: %1").arg(parseError.errorString());
            qDebug() << "[QuestionParserService]" << m_lastError;
            qDebug() << "[QuestionParserService] 尝试解析的内容(前500字符):" << unescaped.left(500);
            return questions;
        }
        
        qDebug() << "[QuestionParserService] 反转义后解析成功";
    }
    
    // 解析 JSON 结构
    QJsonArray questionsArray;
    
    if (doc.isArray()) {
        questionsArray = doc.array();
    } else if (doc.isObject()) {
        QJsonObject root = doc.object();
        // 尝试多种可能的键名
        if (root.contains("questions")) {
            questionsArray = root["questions"].toArray();
        } else if (root.contains("data")) {
            questionsArray = root["data"].toArray();
        } else if (root.contains("items")) {
            questionsArray = root["items"].toArray();
        }
    }
    
    qDebug() << "[QuestionParserService] 解析到" << questionsArray.size() << "道题目";
    
    for (const QJsonValue &val : questionsArray) {
        if (!val.isObject()) continue;
        
        QJsonObject qObj = val.toObject();
        PaperQuestion q;
        
        // 必填字段
        q.stem = qObj["stem"].toString();
        if (q.stem.isEmpty()) {
            q.stem = qObj["question"].toString();
        }
        if (q.stem.isEmpty()) {
            q.stem = qObj["content"].toString();
        }
        
        if (q.stem.isEmpty()) {
            continue;  // 跳过没有题干的数据
        }
        
        // 题目类型 - 支持多种字段名格式
        q.questionType = qObj["question_type"].toString();  // Dify 输出的格式
        if (q.questionType.isEmpty()) {
            q.questionType = qObj["questionType"].toString();  // 驼峰格式
        }
        if (q.questionType.isEmpty()) {
            q.questionType = qObj["type"].toString();
        }
        if (q.questionType.isEmpty()) {
            q.questionType = "short_answer";  // 默认简答题
        }
        
        // 中文题型映射到英文（数据库约束要求英文值）
        static const QMap<QString, QString> typeMapping = {
            {"单选题", "single_choice"},
            {"多选题", "multiple_choice"},
            {"填空题", "fill_blank"},
            {"判断说理题", "true_false"},
            {"判断题", "true_false"},
            {"材料论述题", "material_essay"},
            {"简答题", "short_answer"},
            {"论述题", "short_answer"},
            {"材料分析题", "material_essay"}
        };
        if (typeMapping.contains(q.questionType)) {
            q.questionType = typeMapping[q.questionType];
        }

        // 材料内容（材料论述题专用）
        q.material = qObj["material"].toString();
        if (q.material.isEmpty()) {
            q.material = qObj["materials"].toString();
        }

        // 小问列表（材料论述题专用）
        if (qObj.contains("sub_questions")) {
            QJsonArray sqArr = qObj["sub_questions"].toArray();
            for (const QJsonValue &sq : sqArr) {
                q.subQuestions.append(sq.toString());
            }
        }
        if (qObj.contains("subQuestions")) {
            QJsonArray sqArr = qObj["subQuestions"].toArray();
            for (const QJsonValue &sq : sqArr) {
                q.subQuestions.append(sq.toString());
            }
        }

        // 小问答案（材料论述题专用）
        if (qObj.contains("sub_answers")) {
            QJsonArray saArr = qObj["sub_answers"].toArray();
            for (const QJsonValue &sa : saArr) {
                q.subAnswers.append(sa.toString());
            }
        }
        if (qObj.contains("subAnswers")) {
            QJsonArray saArr = qObj["subAnswers"].toArray();
            for (const QJsonValue &sa : saArr) {
                q.subAnswers.append(sa.toString());
            }
        }

        // 选项
        if (qObj.contains("options")) {
            QJsonArray optArr = qObj["options"].toArray();
            for (const QJsonValue &opt : optArr) {
                q.options.append(opt.toString());
            }
        }
        
        // 答案
        q.answer = qObj["answer"].toString();
        
        // 解析
        q.explanation = qObj["explanation"].toString();
        if (q.explanation.isEmpty()) {
            q.explanation = qObj["analysis"].toString();
        }
        
        // 难度
        q.difficulty = qObj["difficulty"].toString();
        if (q.difficulty.isEmpty()) {
            q.difficulty = "medium";
        }
        
        // 分数
        q.score = qObj["score"].toInt(5);
        
        // 章节和知识点
        q.chapter = qObj["chapter"].toString();
        if (qObj.contains("knowledgePoints")) {
            QJsonArray kpArr = qObj["knowledgePoints"].toArray();
            for (const QJsonValue &kp : kpArr) {
                q.knowledgePoints.append(kp.toString());
            }
        }
        
        // 标签
        if (qObj.contains("tags")) {
            QJsonArray tagArr = qObj["tags"].toArray();
            for (const QJsonValue &tag : tagArr) {
                q.tags.append(tag.toString());
            }
        }
        
        // 设置为公共题目
        q.visibility = "public";
        
        // 使用传入的元数据
        if (q.subject.isEmpty() && !m_currentSubject.isEmpty()) {
            q.subject = m_currentSubject;
        }
        if (q.grade.isEmpty() && !m_currentGrade.isEmpty()) {
            q.grade = m_currentGrade;
        }
        
        questions.append(q);
    }
    
    qDebug() << "[QuestionParserService] 成功解析" << questions.size() << "道有效题目";
    
    return questions;
}

// ==================== 本地 Markdown 解析（无需 Dify API） ====================

QList<PaperQuestion> QuestionParserService::parseMarkdownToQuestions(const QString &markdownText)
{
    QList<PaperQuestion> questions;

    if (markdownText.trimmed().isEmpty()) {
        qDebug() << "[QuestionParserService::parseMarkdown] 输入为空";
        return questions;
    }

    // 按行分割
    QStringList lines = markdownText.split('\n');

    // 调试：将原始输入转储到文件
    {
        QFile dumpFile("/tmp/ai_markdown_dump.txt");
        if (dumpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&dumpFile);
            out << markdownText;
            dumpFile.close();
            qDebug() << "[parseMarkdown] 原始内容已转储到 /tmp/ai_markdown_dump.txt，共" << lines.size() << "行";
        }
    }

    // ---- 正则模式 ----
    // 题目编号：如 "1." "1、" "1)" "（1）" "第1题" "**1.**" 等
    static const QRegularExpression questionNumRe(
        R"(^[\s*]*(?:\*{0,2})(?:(?:第\s*)?(\d+)\s*[.、\)）题]|(\d+)\s*[.、\)）]\s*(?:\*{0,2}))(.*))",
        QRegularExpression::MultilineOption
    );

    // 选项行：A. / A、/ A) / A: 等（大小写均可）
    static const QRegularExpression optionRe(
        R"(^\s*([A-Ha-h])\s*[.、\)）:：]\s*(.+))"
    );

    // 【答案】标记
    static const QRegularExpression answerRe(
        R"(^\s*[\*]*【答案】[\*]*\s*[:：]?\s*(.*))"
    );

    // 【解析】标记
    static const QRegularExpression analysisRe(
        R"(^\s*[\*]*【解[析释]】[\*]*\s*[:：]?\s*(.*))"
    );

    // 题型段落标题：如 "## 一、选择题" / "**选择题**" / "一、单选题" 等
    static const QRegularExpression sectionTypeRe(
        R"((?:#{1,4}\s+)?(?:[一二三四五六七八九十]+[、.]?\s*)?[\*]*\s*(单选题|多选题|选择题|判断题|判断说理题|填空题|简答题|论述题|材料分析题|材料论述题)\s*[\*]*)"
    );

    // 题型关键字到内部类型的映射
    static const QMap<QString, QString> typeMapping = {
        {"单选题", "single_choice"},
        {"选择题", "single_choice"},
        {"多选题", "multiple_choice"},
        {"填空题", "fill_blank"},
        {"判断说理题", "true_false"},
        {"判断题", "true_false"},
        {"材料论述题", "material_essay"},
        {"材料分析题", "material_essay"},
        {"简答题", "short_answer"},
        {"论述题", "short_answer"}
    };

    // ---- 解析状态机 ----
    QString currentSectionType;          // 当前段落的题型
    PaperQuestion currentQuestion;
    bool inQuestion = false;             // 是否正在构建一道题
    bool inAnalysis = false;             // 后续行追加到解析
    bool inStem = false;                 // 后续行追加到题干

    auto flushQuestion = [&]() {
        if (!inQuestion) return;
        // 清理题干中的 Markdown 加粗/标记
        currentQuestion.stem = currentQuestion.stem.trimmed();
        currentQuestion.stem.remove(QRegularExpression(R"(\*{1,2})"));

        if (!currentQuestion.stem.isEmpty()) {
            // 设置默认题型
            if (currentQuestion.questionType.isEmpty()) {
                if (!currentQuestion.options.isEmpty()) {
                    currentQuestion.questionType = "single_choice";
                } else {
                    currentQuestion.questionType = "short_answer";
                }
            }
            if (currentQuestion.difficulty.isEmpty()) {
                currentQuestion.difficulty = "medium";
            }
            currentQuestion.visibility = "public";
            currentQuestion.score = 5;
            questions.append(currentQuestion);
        }
        currentQuestion = PaperQuestion();
        inQuestion = false;
        inAnalysis = false;
        inStem = false;
    };

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];
        QString trimmed = line.trimmed();

        // 跳过空行
        if (trimmed.isEmpty()) {
            inStem = false;      // 空行中断题干续行
            continue;
        }

        // 跳过 <think> 标签
        if (trimmed.startsWith("<think>") || trimmed.startsWith("</think>")) {
            continue;
        }

        // 检测段落题型标题（如 "一、选择题"）
        QRegularExpressionMatch sectionMatch = sectionTypeRe.match(trimmed);
        if (sectionMatch.hasMatch()) {
            QString typeText = sectionMatch.captured(1);
            currentSectionType = typeMapping.value(typeText, "short_answer");
            qDebug() << "[parseMarkdown] 检测到题型段落:" << typeText << "->" << currentSectionType;
            inStem = false;
            continue;
        }

        // 检测【答案】
        QRegularExpressionMatch ansMatch = answerRe.match(trimmed);
        if (ansMatch.hasMatch() && inQuestion) {
            currentQuestion.answer = ansMatch.captured(1).trimmed();
            // 清除 Markdown 粗体
            currentQuestion.answer.remove(QRegularExpression(R"(\*{1,2})"));
            inAnalysis = false;
            inStem = false;
            continue;
        }

        // 检测【解析】
        QRegularExpressionMatch anaMatch = analysisRe.match(trimmed);
        if (anaMatch.hasMatch() && inQuestion) {
            currentQuestion.explanation = anaMatch.captured(1).trimmed();
            inAnalysis = true;
            inStem = false;
            continue;
        }

        // 检测选项行
        QRegularExpressionMatch optMatch = optionRe.match(trimmed);
        if (optMatch.hasMatch() && inQuestion) {
            currentQuestion.options.append(optMatch.captured(2).trimmed());
            inAnalysis = false;
            inStem = false;
            continue;
        }

        // 检测题目编号行（新一道题的开始）
        QRegularExpressionMatch qMatch = questionNumRe.match(trimmed);
        if (qMatch.hasMatch()) {
            // 先把上一道题入库
            flushQuestion();

            inQuestion = true;
            inStem = true;
            inAnalysis = false;

            // 提取题干：编号后面的内容
            QString stemPart = qMatch.captured(3).trimmed();

            // 也可能形如 "1. **（选择题）** 以下关于..."，要提取内嵌题型
            static const QRegularExpression inlineTypeRe(
                R"([\(（]?\s*(单选题|多选题|选择题|判断题|判断说理题|填空题|简答题|论述题|材料分析题|材料论述题)\s*[\)）]?\s*)"
            );
            QRegularExpressionMatch inlineTypeMatch = inlineTypeRe.match(stemPart);
            if (inlineTypeMatch.hasMatch()) {
                QString inlineType = inlineTypeMatch.captured(1);
                currentQuestion.questionType = typeMapping.value(inlineType, "short_answer");
                // 从题干中移除内嵌题型标注
                stemPart.remove(inlineTypeMatch.capturedStart(), inlineTypeMatch.capturedLength());
                stemPart = stemPart.trimmed();
            } else if (!currentSectionType.isEmpty()) {
                // 继承段落题型
                currentQuestion.questionType = currentSectionType;
            }

            currentQuestion.stem = stemPart;
            continue;
        }

        // 后续追加行
        if (inQuestion) {
            if (inAnalysis) {
                if (!currentQuestion.explanation.isEmpty()) {
                    currentQuestion.explanation += "\n";
                }
                currentQuestion.explanation += trimmed;
            } else if (inStem) {
                // 多行题干
                if (!currentQuestion.stem.isEmpty()) {
                    currentQuestion.stem += "\n";
                }
                currentQuestion.stem += trimmed;
            }
        }
    }

    // 最后一道题入库
    flushQuestion();

    qDebug() << "[QuestionParserService::parseMarkdown] 本地解析完成，共"
             << questions.size() << "道题目";

    return questions;
}
