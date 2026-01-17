#include "PaperService.h"
#include "../auth/supabase/supabaseconfig.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>

// ===== Paper 结构体实现 =====
Paper Paper::fromJson(const QJsonObject &json)
{
    Paper paper;
    paper.id = json["id"].toString();
    paper.title = json["title"].toString();
    paper.subject = json["subject"].toString();
    paper.grade = json["grade"].toString();
    paper.totalScore = json["total_score"].toInt(100);
    paper.duration = json["duration"].toInt(90);
    paper.paperType = json["paper_type"].toString();
    paper.description = json["description"].toString();
    paper.createdBy = json["created_by"].toString();
    paper.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    paper.updatedAt = QDateTime::fromString(json["updated_at"].toString(), Qt::ISODate);
    return paper;
}

QJsonObject Paper::toJson() const
{
    QJsonObject json;
    if (!id.isEmpty()) json["id"] = id;
    json["title"] = title;
    json["subject"] = subject;
    json["grade"] = grade;
    json["total_score"] = totalScore;
    json["duration"] = duration;
    json["paper_type"] = paperType;
    json["description"] = description;
    if (!createdBy.isEmpty()) json["created_by"] = createdBy;
    return json;
}

// ===== PaperQuestion 结构体实现 =====
PaperQuestion PaperQuestion::fromJson(const QJsonObject &json)
{
    PaperQuestion q;
    q.id = json["id"].toString();
    q.paperId = json["paper_id"].toString();
    q.questionType = json["question_type"].toString();
    q.difficulty = json["difficulty"].toString();
    q.stem = json["stem"].toString();
    q.material = json["material"].toString();  // 材料内容
    q.answer = json["answer"].toString();
    q.explanation = json["explanation"].toString();
    q.score = json["score"].toInt(5);
    q.orderNum = json["order_num"].toInt(0);
    q.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);

    // 解析 options (JSONB -> QStringList)
    if (json.contains("options") && json["options"].isArray()) {
        QJsonArray optArray = json["options"].toArray();
        for (const QJsonValue &val : optArray) {
            q.options.append(val.toString());
        }
    }

    // 解析 sub_questions (材料论述题小问列表)
    if (json.contains("sub_questions") && json["sub_questions"].isArray()) {
        QJsonArray sqArray = json["sub_questions"].toArray();
        for (const QJsonValue &val : sqArray) {
            q.subQuestions.append(val.toString());
        }
    }

    // 解析 sub_answers (材料论述题小问答案)
    if (json.contains("sub_answers") && json["sub_answers"].isArray()) {
        QJsonArray saArray = json["sub_answers"].toArray();
        for (const QJsonValue &val : saArray) {
            q.subAnswers.append(val.toString());
        }
    }

    // 解析 tags (TEXT[] -> QStringList)
    if (json.contains("tags") && json["tags"].isArray()) {
        QJsonArray tagArray = json["tags"].toArray();
        for (const QJsonValue &val : tagArray) {
            q.tags.append(val.toString());
        }
    }

    // 解析新增字段
    q.visibility = json["visibility"].toString("private");
    q.subject = json["subject"].toString();
    q.grade = json["grade"].toString();
    q.chapter = json["chapter"].toString();

    // 解析 knowledge_points (TEXT[] -> QStringList)
    if (json.contains("knowledge_points") && json["knowledge_points"].isArray()) {
        QJsonArray kpArray = json["knowledge_points"].toArray();
        for (const QJsonValue &val : kpArray) {
            q.knowledgePoints.append(val.toString());
        }
    }

    return q;
}

QJsonObject PaperQuestion::toJson() const
{
    QJsonObject json;
    if (!id.isEmpty()) json["id"] = id;
    
    // paper_id: 如果为空则不发送（让数据库使用默认值 null）
    if (!paperId.isEmpty()) {
        json["paper_id"] = paperId;
    }
    // 如果需要显式发送 null，取消下面的注释：
    // else {
    //     json["paper_id"] = QJsonValue::Null;
    // }
    
    json["question_type"] = questionType;
    json["difficulty"] = difficulty;
    json["stem"] = stem;
    if (!material.isEmpty()) json["material"] = material;  // 材料内容
    json["answer"] = answer;
    json["explanation"] = explanation;
    json["score"] = score;
    json["order_num"] = orderNum;

    // 转换 options
    QJsonArray optArray;
    for (const QString &opt : options) {
        optArray.append(opt);
    }
    json["options"] = optArray;

    // 转换 sub_questions (材料论述题小问列表)
    if (!subQuestions.isEmpty()) {
        QJsonArray sqArray;
        for (const QString &sq : subQuestions) {
            sqArray.append(sq);
        }
        json["sub_questions"] = sqArray;
    }

    // 转换 sub_answers (材料论述题小问答案)
    if (!subAnswers.isEmpty()) {
        QJsonArray saArray;
        for (const QString &sa : subAnswers) {
            saArray.append(sa);
        }
        json["sub_answers"] = saArray;
    }

    // 转换 tags
    QJsonArray tagArray;
    for (const QString &tag : tags) {
        tagArray.append(tag);
    }
    json["tags"] = tagArray;

    // 转换新增字段
    json["visibility"] = visibility;
    if (!subject.isEmpty()) json["subject"] = subject;
    if (!grade.isEmpty()) json["grade"] = grade;
    if (!chapter.isEmpty()) json["chapter"] = chapter;
    
    // 转换 knowledge_points
    if (!knowledgePoints.isEmpty()) {
        QJsonArray kpArray;
        for (const QString &kp : knowledgePoints) {
            kpArray.append(kp);
        }
        json["knowledge_points"] = kpArray;
    }

    return json;
}

// ===== PaperService 实现 =====
PaperService::PaperService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &PaperService::onReplyFinished);
}

PaperService::~PaperService()
{
}

void PaperService::setAccessToken(const QString &token)
{
    m_accessToken = token;
    qDebug() << "PaperService: 访问令牌已设置";
}

// ===== 试卷操作 =====
void PaperService::createPaper(const Paper &paper)
{
    QJsonDocument doc(paper.toJson());
    sendRequest("/rest/v1/papers", RequestType::CreatePaper, doc, "POST");
}

void PaperService::getPapers()
{
    sendRequest("/rest/v1/papers?order=created_at.desc", RequestType::GetPapers, QJsonDocument(), "GET");
}

void PaperService::getPaperById(const QString &paperId)
{
    QString endpoint = QString("/rest/v1/papers?id=eq.%1").arg(paperId);
    sendRequest(endpoint, RequestType::GetPaperById, QJsonDocument(), "GET");
}

void PaperService::updatePaper(const Paper &paper)
{
    QString endpoint = QString("/rest/v1/papers?id=eq.%1").arg(paper.id);
    QJsonDocument doc(paper.toJson());
    sendRequest(endpoint, RequestType::UpdatePaper, doc, "PATCH");
}

void PaperService::deletePaper(const QString &paperId)
{
    QString endpoint = QString("/rest/v1/papers?id=eq.%1").arg(paperId);
    sendRequest(endpoint, RequestType::DeletePaper, QJsonDocument(), "DELETE");
}

// ===== 题目操作 =====
void PaperService::addQuestion(const PaperQuestion &question)
{
    QJsonDocument doc(question.toJson());
    sendRequest("/rest/v1/questions", RequestType::AddQuestion, doc, "POST");
}

void PaperService::addQuestions(const QList<PaperQuestion> &questions)
{
    QJsonArray array;
    for (const PaperQuestion &q : questions) {
        array.append(q.toJson());
    }
    QJsonDocument doc(array);
    sendRequest("/rest/v1/questions", RequestType::AddQuestions, doc, "POST");
}

void PaperService::getQuestionsByPaperId(const QString &paperId)
{
    QString endpoint = QString("/rest/v1/questions?paper_id=eq.%1&order=order_num.asc").arg(paperId);
    sendRequest(endpoint, RequestType::GetQuestionsByPaper, QJsonDocument(), "GET");
}

void PaperService::getQuestionById(const QString &questionId)
{
    QString endpoint = QString("/rest/v1/questions?id=eq.%1").arg(questionId);
    sendRequest(endpoint, RequestType::GetQuestionById, QJsonDocument(), "GET");
}

void PaperService::updateQuestion(const PaperQuestion &question)
{
    QString endpoint = QString("/rest/v1/questions?id=eq.%1").arg(question.id);
    QJsonDocument doc(question.toJson());
    sendRequest(endpoint, RequestType::UpdateQuestion, doc, "PATCH");
}

void PaperService::deleteQuestion(const QString &questionId)
{
    QString endpoint = QString("/rest/v1/questions?id=eq.%1").arg(questionId);
    sendRequest(endpoint, RequestType::DeleteQuestion, QJsonDocument(), "DELETE");
}

// ===== 题目检索 =====
void PaperService::searchQuestions(const QuestionSearchCriteria &criteria)
{
    QString endpoint = "/rest/v1/questions?";
    QStringList filters;

    // 可见性筛选（默认只查询公共题库）
    if (criteria.visibility.isEmpty() || criteria.visibility == "public") {
        filters.append("visibility=eq.public");
    } else if (criteria.visibility == "private") {
        filters.append("visibility=eq.private");
    }
    // visibility == "all" 时不添加筛选条件

    // 科目筛选
    if (!criteria.subject.isEmpty()) {
        filters.append(QString("subject=eq.%1").arg(criteria.subject));
    }

    // 年级筛选
    if (!criteria.grade.isEmpty()) {
        filters.append(QString("grade=eq.%1").arg(criteria.grade));
    }

    // 章节筛选
    if (!criteria.chapter.isEmpty()) {
        filters.append(QString("chapter=eq.%1").arg(criteria.chapter));
    }

    // 题型筛选
    if (!criteria.questionType.isEmpty()) {
        filters.append(QString("question_type=eq.%1").arg(criteria.questionType));
    }

    // 难度筛选
    if (!criteria.difficulty.isEmpty()) {
        filters.append(QString("difficulty=eq.%1").arg(criteria.difficulty));
    }

    // 标签筛选（数组包含）
    if (!criteria.tags.isEmpty()) {
        QString tagList = criteria.tags.join(",");
        filters.append(QString("tags=cs.{%1}").arg(tagList));
    }

    // 知识点筛选（数组包含）
    if (!criteria.knowledgePoints.isEmpty()) {
        QString kpList = criteria.knowledgePoints.join(",");
        filters.append(QString("knowledge_points=cs.{%1}").arg(kpList));
    }

    // 题干关键词搜索
    if (!criteria.keyword.isEmpty()) {
        filters.append(QString("stem=ilike.*%1*").arg(criteria.keyword));
    }

    endpoint += filters.join("&");
    sendRequest(endpoint, RequestType::SearchQuestions, QJsonDocument(), "GET");
}

// ===== 私有方法 =====
void PaperService::sendRequest(const QString &endpoint, RequestType type,
                               const QJsonDocument &data, const QString &method)
{
    QString url = SupabaseConfig::SUPABASE_URL + endpoint;
    qDebug() << "PaperService 请求:" << method << url;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("apikey", SupabaseConfig::SUPABASE_ANON_KEY.toUtf8());
    request.setRawHeader("Prefer", "return=representation");  // 返回完整数据

    // 如果有访问令牌，添加认证头
    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    }

    QNetworkReply *reply = nullptr;
    if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "POST") {
        reply = m_networkManager->post(request, data.toJson());
    } else if (method == "PATCH") {
        reply = m_networkManager->sendCustomRequest(request, "PATCH", data.toJson());
    } else if (method == "DELETE") {
        reply = m_networkManager->sendCustomRequest(request, "DELETE");
    }

    if (reply) {
        reply->setProperty("requestType", static_cast<int>(type));
    }
}

void PaperService::onReplyFinished(QNetworkReply *reply)
{
    if (!reply) return;

    RequestType type = static_cast<RequestType>(reply->property("requestType").toInt());
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = reply->errorString();
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray errorData = reply->readAll();
        
        qDebug() << "PaperService 错误:" << errorMsg << "HTTP:" << httpStatus;
        if (!errorData.isEmpty()) {
            qDebug() << "PaperService 错误详情:" << errorData;
        }
        
        switch (type) {
        case RequestType::CreatePaper:
        case RequestType::GetPapers:
        case RequestType::GetPaperById:
        case RequestType::UpdatePaper:
        case RequestType::DeletePaper:
            emit paperError("network", QString("%1 (HTTP %2)").arg(errorMsg).arg(httpStatus));
            break;
        default:
            emit questionError("network", QString("%1 (HTTP %2)").arg(errorMsg).arg(httpStatus));
            break;
        }
        
        reply->deleteLater();
        return;
    }

    handleResponse(reply, type);
    reply->deleteLater();
}

void PaperService::handleResponse(QNetworkReply *reply, RequestType type)
{
    QByteArray data = reply->readAll();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    qDebug() << "PaperService 响应[" << httpStatus << "]:" << data;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON 解析错误:" << parseError.errorString();
        emit paperError("parse", parseError.errorString());
        return;
    }

    // 处理不同类型的响应
    switch (type) {
    case RequestType::CreatePaper: {
        if (doc.isArray() && !doc.array().isEmpty()) {
            Paper paper = Paper::fromJson(doc.array().first().toObject());
            emit paperCreated(paper);
        }
        break;
    }
    case RequestType::GetPapers: {
        QList<Paper> papers;
        if (doc.isArray()) {
            for (const QJsonValue &val : doc.array()) {
                papers.append(Paper::fromJson(val.toObject()));
            }
        }
        emit papersLoaded(papers);
        break;
    }
    case RequestType::GetPaperById: {
        if (doc.isArray() && !doc.array().isEmpty()) {
            Paper paper = Paper::fromJson(doc.array().first().toObject());
            emit paperLoaded(paper);
        }
        break;
    }
    case RequestType::UpdatePaper: {
        if (doc.isArray() && !doc.array().isEmpty()) {
            Paper paper = Paper::fromJson(doc.array().first().toObject());
            emit paperUpdated(paper);
        }
        break;
    }
    case RequestType::DeletePaper: {
        emit paperDeleted("");  // Supabase 不返回删除的ID
        break;
    }
    case RequestType::AddQuestion: {
        if (doc.isArray() && !doc.array().isEmpty()) {
            PaperQuestion question = PaperQuestion::fromJson(doc.array().first().toObject());
            emit questionAdded(question);
        }
        break;
    }
    case RequestType::AddQuestions: {
        if (doc.isArray()) {
            emit questionsAdded(doc.array().size());
        }
        break;
    }
    case RequestType::GetQuestionsByPaper:
    case RequestType::SearchQuestions: {
        QList<PaperQuestion> questions;
        if (doc.isArray()) {
            for (const QJsonValue &val : doc.array()) {
                questions.append(PaperQuestion::fromJson(val.toObject()));
            }
        }
        if (type == RequestType::GetQuestionsByPaper) {
            emit questionsLoaded(questions);
        } else {
            emit searchCompleted(questions);
        }
        break;
    }
    case RequestType::GetQuestionById: {
        if (doc.isArray() && !doc.array().isEmpty()) {
            PaperQuestion question = PaperQuestion::fromJson(doc.array().first().toObject());
            emit questionLoaded(question);
        }
        break;
    }
    case RequestType::UpdateQuestion: {
        if (doc.isArray() && !doc.array().isEmpty()) {
            PaperQuestion question = PaperQuestion::fromJson(doc.array().first().toObject());
            emit questionUpdated(question);
        }
        break;
    }
    case RequestType::DeleteQuestion: {
        emit questionDeleted("");
        break;
    }
    }
}

QString PaperService::parseError(const QJsonDocument &doc)
{
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("message")) {
            return obj["message"].toString();
        }
        if (obj.contains("error")) {
            return obj["error"].toString();
        }
    }
    return "未知错误";
}
