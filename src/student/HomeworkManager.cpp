#include "HomeworkManager.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QDebug>

HomeworkManager* HomeworkManager::s_instance = nullptr;

HomeworkManager* HomeworkManager::instance()
{
    if (!s_instance) s_instance = new HomeworkManager();
    return s_instance;
}

HomeworkManager::HomeworkManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

// ── AssignmentInfo ──
HomeworkManager::AssignmentInfo HomeworkManager::AssignmentInfo::fromJson(const QJsonObject &json)
{
    AssignmentInfo a;
    a.id = json["id"].toString();
    a.classId = json["class_id"].toString();
    a.teacherEmail = json["teacher_email"].toString();
    a.title = json["title"].toString();
    a.description = json["description"].toString();
    a.totalScore = json["total_score"].toInt(100);
    a.status = json["status"].toInt(2);
    a.endTime = QDateTime::fromString(json["end_time"].toString(), Qt::ISODateWithMs);
    a.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODateWithMs);
    return a;
}

// ── SubmissionInfo ──
HomeworkManager::SubmissionInfo HomeworkManager::SubmissionInfo::fromJson(const QJsonObject &json)
{
    SubmissionInfo s;
    s.id = json["id"].toString();
    s.assignmentId = json["assignment_id"].toString();
    s.studentEmail = json["student_email"].toString();
    s.studentName = json["student_name"].toString();
    s.content = json["content"].toString();
    s.feedback = json["feedback"].toString();
    s.fileUrl = json["file_url"].toString();
    s.score = json["score"].toInt(-1);
    s.status = json["status"].toInt(1);
    s.allowResubmit = json["allow_resubmit"].toBool(false);
    s.submitTime = QDateTime::fromString(json["submit_time"].toString(), Qt::ISODateWithMs);
    s.gradeTime = QDateTime::fromString(json["grade_time"].toString(), Qt::ISODateWithMs);
    return s;
}

// ── 创建作业 ──
void HomeworkManager::createAssignment(const QString &classId, const QString &teacherEmail,
                                        const QString &title, const QString &description,
                                        int totalScore, const QDateTime &endTime)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/assignments");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "return=representation");

    QJsonObject body;
    body["class_id"] = classId;
    body["teacher_email"] = teacherEmail;
    body["title"] = title;
    body["description"] = description;
    body["total_score"] = totalScore;
    body["end_time"] = endTime.toString(Qt::ISODate);
    body["status"] = 2;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Homework] create failed:" << reply->errorString();
            emit error("创建作业失败");
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        if (!arr.isEmpty()) {
            AssignmentInfo info = AssignmentInfo::fromJson(arr[0].toObject());
            qDebug() << "[Homework] 作业已创建:" << info.title;
            emit assignmentCreated(info);
        }
    });
}

// ── 加载作业列表 ──
void HomeworkManager::loadAssignments(const QString &classId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/assignments");
    QUrlQuery query;
    query.addQueryItem("select", "id,class_id,teacher_email,title,description,total_score,status,end_time,created_at");
    query.addQueryItem("class_id", "eq." + classId);
    query.addQueryItem("order", "created_at.desc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Homework] load failed:" << reply->errorString();
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<AssignmentInfo> list;
        for (const auto &val : arr) {
            list.append(AssignmentInfo::fromJson(val.toObject()));
        }
        qDebug() << "[Homework] 作业列表:" << list.size();
        emit assignmentsLoaded(list);
    });
}

// ── 加载提交 ──
void HomeworkManager::loadSubmissions(const QString &assignmentId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/submissions");
    QUrlQuery query;
    query.addQueryItem("select", "id,assignment_id,student_email,student_name,content,score,feedback,file_url,allow_resubmit,status,submit_time,grade_time");
    query.addQueryItem("assignment_id", "eq." + assignmentId);
    query.addQueryItem("order", "submit_time.desc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, assignmentId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Homework] load submissions failed:" << reply->errorString();
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<SubmissionInfo> list;
        for (const auto &val : arr) {
            list.append(SubmissionInfo::fromJson(val.toObject()));
        }
        qDebug() << "[Homework] 提交列表:" << list.size();
        emit submissionsLoaded(assignmentId, list);
    });
}

// ── 删除作业 ──
void HomeworkManager::deleteAssignment(const QString &assignmentId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/assignments");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + assignmentId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, assignmentId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Homework] delete failed:" << reply->errorString();
            emit error("删除作业失败");
            return;
        }
        qDebug() << "[Homework] 作业已删除:" << assignmentId;
        emit assignmentDeleted(assignmentId);
    });
}

// ── 批改 ──
void HomeworkManager::gradeSubmission(const QString &submissionId, int score, const QString &feedback,
                                      bool allowResubmit)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/submissions");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + submissionId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["score"] = score;
    body["feedback"] = feedback;
    body["status"] = 2;
    body["grade_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    body["allow_resubmit"] = allowResubmit;

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, submissionId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Homework] grade failed:" << reply->errorString();
            emit error("批改失败");
            return;
        }
        qDebug() << "[Homework] 已批改:" << submissionId;
        emit submissionGraded(submissionId);
    });
}

// ── 学生提交作业 ──
void HomeworkManager::submitHomework(const QString &assignmentId, const QString &studentEmail,
                                      const QString &studentName, const QString &content,
                                      const QString &fileUrl)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/submissions");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "return=representation");

    QJsonObject body;
    body["assignment_id"] = assignmentId;
    body["student_email"] = studentEmail;
    body["student_name"] = studentName;
    body["content"] = content;
    body["status"] = 1;
    body["submit_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    if (!fileUrl.isEmpty()) body["file_url"] = fileUrl;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, assignmentId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Homework] submit failed:" << reply->errorString();
            emit error("提交作业失败: " + reply->readAll());
            return;
        }
        QByteArray respData = reply->readAll();
        qDebug() << "[Homework] submit response:" << respData.left(500);
        emit homeworkSubmitted(assignmentId);
    });
}

// ── 追加提交（重交） ──
void HomeworkManager::resubmitHomework(const QString &submissionId,
                                        const QString &newContent,
                                        const QString &newFileUrl)
{
    // 先读取当前提交内容
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/submissions");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + submissionId);
    query.addQueryItem("select", "content,file_url");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, submissionId, newContent, newFileUrl]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Homework] resubmit read failed:" << reply->errorString();
            emit error("追加提交失败");
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        if (arr.isEmpty()) { emit error("提交记录不存在"); return; }

        QString oldContent = arr[0].toObject()["content"].toString();
        QString oldFileUrl = arr[0].toObject()["file_url"].toString();

        // 追加内容
        QString mergedContent = oldContent;
        if (!newContent.isEmpty()) {
            if (!mergedContent.isEmpty()) mergedContent += "\n\n--- 追加 ---\n\n";
            mergedContent += newContent;
        }
        // 追加文件 URL
        QString mergedFileUrl = oldFileUrl;
        if (!newFileUrl.isEmpty()) {
            if (!mergedFileUrl.isEmpty()) mergedFileUrl += ",";
            mergedFileUrl += newFileUrl;
        }

        // PATCH 更新
        QUrl patchUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/submissions");
        QUrlQuery patchQuery;
        patchQuery.addQueryItem("id", "eq." + submissionId);
        patchUrl.setQuery(patchQuery);

        QNetworkRequest patchReq = NetworkRequestFactory::createAuthRequest(patchUrl);
        patchReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject body;
        body["content"] = mergedContent;
        if (!mergedFileUrl.isEmpty()) body["file_url"] = mergedFileUrl;
        body["status"] = 1; // 重置为待批改
        body["allow_resubmit"] = false; // 重置
        body["submit_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        QNetworkReply *patchReply = m_networkManager->sendCustomRequest(
            patchReq, "PATCH", QJsonDocument(body).toJson());

        connect(patchReply, &QNetworkReply::finished, this, [this, patchReply, submissionId]() {
            patchReply->deleteLater();
            if (patchReply->error() != QNetworkReply::NoError) {
                qWarning() << "[Homework] resubmit patch failed:" << patchReply->errorString();
                emit error("追加提交失败");
                return;
            }
            qDebug() << "[Homework] 追加提交成功:" << submissionId;
            // 用 assignmentId 触发刷新 — 这里发 homeworkSubmitted 信号，assignmentId 通过 submissionId 查不到
            // 改为直接发一个通用刷新信号
            emit homeworkSubmitted(QString());
        });
    });
}
