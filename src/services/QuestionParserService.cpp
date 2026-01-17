#include "QuestionParserService.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QDebug>
#include <QSslConfiguration>
#include <QSslSocket>
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
}

QuestionParserService::~QuestionParserService()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    if (m_uploadReply) {
        m_uploadReply->abort();
        m_uploadReply->deleteLater();
    }
    if (m_uploadFile) {
        m_uploadFile->close();
        delete m_uploadFile;
    }
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
    
    // 取消之前的请求
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    // 保存元数据
    m_currentSubject = subject;
    m_currentGrade = grade;
    m_fullResponse.clear();
    m_streamBuffer.clear();
    
    // 构建 Dify 工作流 API 请求
    // 使用 /workflows/run 端点
    QUrl url(m_baseUrl + "/workflows/run");
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    
    // 配置 SSL
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(300000);  // 5分钟超时（解析可能需要较长时间）
    
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
    
    // 保存元数据
    m_currentFilePath = filePath;
    m_currentSubject = subject;
    m_currentGrade = grade;
    m_fullResponse.clear();
    m_streamBuffer.clear();
    m_uploadedFileId.clear();
    
    emit parseStarted();
    
    // 开始上传文件
    uploadFileToDify(filePath);
}

void QuestionParserService::uploadFileToDify(const QString &filePath)
{
    qDebug() << "[QuestionParserService] 开始上传文件:" << filePath;
    
    // 清理之前的上传
    if (m_uploadReply) {
        m_uploadReply->abort();
        m_uploadReply->deleteLater();
        m_uploadReply = nullptr;
    }
    if (m_uploadFile) {
        m_uploadFile->close();
        delete m_uploadFile;
        m_uploadFile = nullptr;
    }
    
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
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    
    // 配置 SSL
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(120000);  // 2分钟上传超时
    
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
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    m_fullResponse.clear();
    m_streamBuffer.clear();
    
    // 构建工作流请求
    QUrl url(m_baseUrl + "/workflows/run");
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    
    // 配置 SSL
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(300000);  // 5分钟解析超时
    
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
        parseStreamResponse(data);
    }
}

void QuestionParserService::onReplyFinished()
{
    if (!m_currentReply) return;
    
    QNetworkReply::NetworkError error = m_currentReply->error();
    
    if (error != QNetworkReply::NoError && 
        error != QNetworkReply::RemoteHostClosedError) {
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
        
        // 从完整响应中提取 JSON
        QList<PaperQuestion> questions = parseJsonResponse(m_fullResponse);
        emit parseCompleted(questions);
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void QuestionParserService::parseStreamResponse(const QByteArray &data)
{
    QString buffer = m_streamBuffer + QString::fromUtf8(data);
    QStringList lines = buffer.split('\n');
    m_streamBuffer.clear();
    
    // 保留不完整的最后一行
    if (!buffer.endsWith('\n') && !lines.isEmpty()) {
        m_streamBuffer = lines.takeLast();
    }
    
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;
        
        if (!trimmed.startsWith("data: ")) {
            // 如果不是 SSE 格式，可能是错误信息
            qDebug() << "[QuestionParserService] 非 SSE 数据:" << trimmed;
            m_fullResponse += trimmed; // 暂时保存，以防是纯文本响应
            continue;
        }
        
        QString jsonStr = trimmed.mid(6);
        if (jsonStr.isEmpty() || jsonStr == "[DONE]") {
            continue;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (doc.isNull() || !doc.isObject()) {
            continue;
        }
        
        QJsonObject obj = doc.object();
        QString event = obj["event"].toString();
        
        if (event == "text_chunk") {
            // 提取文本块
            QJsonObject dataObj = obj["data"].toObject();
            QString text = dataObj["text"].toString();
            if (text.isEmpty()) {
                text = obj["text"].toString();
            }
            
            m_fullResponse += text;
            emit parseProgress(text);
            
        } else if (event == "workflow_finished") {
            // 工作流完成，提取最终输出
            QJsonObject dataObj = obj["data"].toObject();
            QJsonObject outputs = dataObj["outputs"].toObject();
            
            // 打印详细调试信息
            qDebug() << "[QuestionParserService] workflow_finished 数据:";
            qDebug() << "[QuestionParserService] data 字段:" << QJsonDocument(dataObj).toJson(QJsonDocument::Compact);
            qDebug() << "[QuestionParserService] outputs 字段:" << QJsonDocument(outputs).toJson(QJsonDocument::Compact);
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
                // 遍历 outputs 找到第一个字符串值
                for (const QString &key : outputs.keys()) {
                    QJsonValue val = outputs[key];
                    qDebug() << "[QuestionParserService] 检查字段:" << key << "类型:" << val.type();
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
            } else {
                qDebug() << "[QuestionParserService] 警告：outputs 中没有找到有效输出";
            }
            
            qDebug() << "[QuestionParserService] 工作流完成";
        }
    }
}

QList<PaperQuestion> QuestionParserService::parseJsonResponse(const QString &jsonText)
{
    QList<PaperQuestion> questions;

    // 尝试从响应中提取 JSON
    QString cleanJson = jsonText.trimmed();

    // 移除 <think>...</think> 标签及其内容（AI 的思考过程）
    QRegularExpression thinkRe("<think>.*?</think>", QRegularExpression::DotMatchesEverythingOption);
    cleanJson.remove(thinkRe);
    cleanJson = cleanJson.trimmed();

    // 检查是否是工作流直接插入数据库的报告格式
    // 格式示例：数据插入报告\n- 总计: 15\n- 成功: 6\n- 失败: 9
    QRegularExpression successRe("成功[：:]\\s*(\\d+)");
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
    QRegularExpression codeBlockRe("```(?:json)?\\s*([\\s\\S]*?)```");
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
