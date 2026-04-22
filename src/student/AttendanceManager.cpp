#include "AttendanceManager.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QDebug>

AttendanceManager* AttendanceManager::s_instance = nullptr;

AttendanceManager* AttendanceManager::instance()
{
    if (!s_instance) {
        s_instance = new AttendanceManager();
    }
    return s_instance;
}

AttendanceManager::AttendanceManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

// ── SessionInfo ──
AttendanceManager::SessionInfo AttendanceManager::SessionInfo::fromJson(const QJsonObject &json)
{
    SessionInfo s;
    s.id = json["id"].toString();
    s.classId = json["class_id"].toString();
    s.code = json["code"].toString();
    s.status = json["status"].toString();
    s.name = json["name"].toString();
    s.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODateWithMs);
    s.endedAt = QDateTime::fromString(json["ended_at"].toString(), Qt::ISODateWithMs);
    return s;
}

// ── RecordInfo ──
AttendanceManager::RecordInfo AttendanceManager::RecordInfo::fromJson(const QJsonObject &json)
{
    RecordInfo r;
    r.id = json["id"].toString();
    r.sessionId = json["session_id"].toString();
    r.studentEmail = json["student_email"].toString();
    r.studentName = json["student_name"].toString();
    r.status = json["status"].toString();
    r.signedAt = QDateTime::fromString(json["signed_at"].toString(), Qt::ISODateWithMs);
    return r;
}

// ── 教师开始考勤 ──
void AttendanceManager::startAttendance(const QString &classId, const QString &name)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/rpc/start_attendance");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["p_class_id"] = classId;
    body["p_name"] = name;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Attendance] start failed:" << reply->errorString();
            emit error("开始考勤失败: " + reply->errorString());
            return;
        }
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        SessionInfo session = SessionInfo::fromJson(obj);
        qDebug() << "[Attendance] 考勤已开始, code:" << session.code << "name:" << session.name;
        emit attendanceStarted(session);
    });
}

// ── 教师结束考勤 ──
void AttendanceManager::endAttendance(const QString &sessionId, const QString &classId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/rpc/end_attendance");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["p_session_id"] = sessionId;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, sessionId, classId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Attendance] end failed:" << reply->errorString();
            emit error("结束考勤失败");
            return;
        }
        qDebug() << "[Attendance] 考勤已结束:" << sessionId;
        emit attendanceEnded(sessionId, classId);
    });
}

// ── 加载考勤记录 ──
void AttendanceManager::loadSessionRecords(const QString &sessionId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/attendance_records");
    QUrlQuery query;
    query.addQueryItem("select", "id,session_id,student_email,student_name,status,signed_at");
    query.addQueryItem("session_id", "eq." + sessionId);
    query.addQueryItem("order", "signed_at.desc.nullslast,student_name");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, sessionId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Attendance] load records failed:" << reply->errorString();
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<RecordInfo> records;
        for (const auto &val : arr) {
            records.append(RecordInfo::fromJson(val.toObject()));
        }
        qDebug() << "[Attendance] 记录加载:" << records.size();
        emit recordsLoaded(sessionId, records);
    });
}

// ── 修改考勤状态 ──
void AttendanceManager::updateRecordStatus(const QString &recordId, const QString &status)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/rpc/update_attendance_status");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["p_record_id"] = recordId;
    body["p_status"] = status;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, recordId, status]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Attendance] update status failed:" << reply->errorString();
            emit error("修改状态失败");
            return;
        }
        emit recordStatusUpdated(recordId, status);
    });
}

// ── 学生签到 ──
void AttendanceManager::signAttendance(const QString &code, const QString &studentEmail, const QString &studentName)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/rpc/student_sign_attendance");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["p_code"] = code;
    body["p_student_email"] = studentEmail;
    body["p_student_name"] = studentName;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Attendance] sign failed:" << reply->errorString();
            emit signResult(false, "签到失败: " + reply->errorString());
            return;
        }
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        if (obj.contains("error")) {
            emit signResult(false, obj["error"].toString());
        } else {
            QString className = obj["class_name"].toString();
            emit signResult(true, QString("签到成功！%1").arg(className.isEmpty() ? "" : "课程: " + className));
        }
    });
}

// ── 按班级+日期加载考勤 sessions ──
void AttendanceManager::loadSessionsByClassAndDate(const QString &classId, const QDate &date)
{
    Q_UNUSED(date)
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/attendance_sessions");
    QUrlQuery query;
    query.addQueryItem("select", "id,class_id,code,status,name,created_at,ended_at");
    query.addQueryItem("class_id", "eq." + classId);
    query.addQueryItem("order", "created_at.desc");
    query.addQueryItem("limit", "50");
    url.setQuery(query);
    qDebug() << "[Attendance] loadSessions query:" << url.toString();

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Attendance] load sessions failed:" << reply->errorString();
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<SessionInfo> sessions;
        for (const auto &val : arr) {
            sessions.append(SessionInfo::fromJson(val.toObject()));
        }
        qDebug() << "[Attendance] sessions loaded:" << sessions.size();
        emit sessionsLoaded(sessions);
    });
}

// ── 学生查看自己的考勤记录 ──
void AttendanceManager::loadStudentAttendance(const QString &classId, const QString &studentEmail)
{
    // 两步查询：先获取该班级所有 session，再查该学生的 records
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/attendance_sessions");
    QUrlQuery query;
    query.addQueryItem("select", "id,class_id,code,status,name,created_at,ended_at");
    query.addQueryItem("class_id", "eq." + classId);
    query.addQueryItem("order", "created_at.desc");
    query.addQueryItem("limit", "100");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, classId, studentEmail]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Attendance] load student sessions failed:" << reply->errorString();
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<SessionInfo> sessions;
        for (const auto &val : arr) {
            sessions.append(SessionInfo::fromJson(val.toObject()));
        }

        if (sessions.isEmpty()) {
            emit studentAttendanceLoaded(classId, {});
            return;
        }

        // 查该学生所有 records（按 session_id 列表过滤）
        QStringList sessionIds;
        for (const auto &s : sessions) sessionIds << s.id;

        QUrl recUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/attendance_records");
        QUrlQuery recQuery;
        recQuery.addQueryItem("select", "id,session_id,student_email,student_name,status,signed_at");
        recQuery.addQueryItem("student_email", "eq." + studentEmail);
        // 使用 in 过滤
        recQuery.addQueryItem("session_id", "in.(" + sessionIds.join(",") + ")");
        recUrl.setQuery(recQuery);

        QNetworkRequest recRequest = NetworkRequestFactory::createAuthRequest(recUrl);
        QNetworkReply *recReply = m_networkManager->get(recRequest);

        connect(recReply, &QNetworkReply::finished, this, [this, recReply, classId, sessions]() {
            recReply->deleteLater();
            if (recReply->error() != QNetworkReply::NoError) {
                qWarning() << "[Attendance] load student records failed:" << recReply->errorString();
                return;
            }

            // 建立 sessionId → status 映射
            QMap<QString, QString> sessionToStatus;
            QJsonArray recArr = QJsonDocument::fromJson(recReply->readAll()).array();
            for (const auto &val : recArr) {
                QJsonObject obj = val.toObject();
                sessionToStatus[obj["session_id"].toString()] = obj["status"].toString();
            }

            QList<QPair<SessionInfo, QString>> result;
            for (const auto &s : sessions) {
                result.append({s, sessionToStatus.value(s.id, "未记录")});
            }

            qDebug() << "[Attendance] 学生考勤:" << result.size();
            emit studentAttendanceLoaded(classId, result);
        });
    });
}
