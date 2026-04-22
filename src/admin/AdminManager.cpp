#include "AdminManager.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QRandomGenerator>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>

namespace {
void adminLog(const QString &msg) {
    QFile f(QDir::homePath() + "/admin_debug.log");
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream s(&f);
        s << QTime::currentTime().toString("HH:mm:ss") << " " << msg << "\n";
    }
}
}

AdminManager* AdminManager::s_instance = nullptr;

AdminManager* AdminManager::instance()
{
    if (!s_instance) s_instance = new AdminManager();
    return s_instance;
}

AdminManager::AdminManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

// ── 数据结构 fromJson ──
AdminManager::SchoolInfo AdminManager::SchoolInfo::fromJson(const QJsonObject &json)
{
    SchoolInfo s;
    s.id = json["id"].toString();
    s.name = json["name"].toString();
    s.classQuota = json["class_quota"].toInt(5);
    s.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODateWithMs);
    return s;
}

AdminManager::InviteCodeInfo AdminManager::InviteCodeInfo::fromJson(const QJsonObject &json)
{
    InviteCodeInfo c;
    c.id = json["id"].toString();
    c.schoolId = json["school_id"].toString();
    c.code = json["code"].toString();
    c.used = json["used"].toBool(false);
    c.usedByEmail = json["used_by_email"].toString();
    c.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODateWithMs);
    return c;
}

AdminManager::TeacherInfo AdminManager::TeacherInfo::fromJson(const QJsonObject &json)
{
    TeacherInfo t;
    t.email = json["email"].toString();
    t.name = t.email.split('@')[0];
    t.schoolId = json["school_id"].toString();
    return t;
}

AdminManager::ClassBrief AdminManager::ClassBrief::fromJson(const QJsonObject &json)
{
    ClassBrief c;
    c.id = json["id"].toString();
    c.name = json["name"].toString();
    c.teacherName = json["teacher_name"].toString(json["teacher"].toString());
    c.teacherEmail = json["teacher_email"].toString(json["teacherEmail"].toString());
    c.schoolId = json["school_id"].toString();
    c.code = json["code"].toString();
    c.description = json["description"].toString();
    c.status = json["status"].toString("active");
    c.studentCount = json["student_count"].toInt(json["studentCount"].toInt(0));
    c.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODateWithMs);
    return c;
}

// ══════════════════════════════════════
//  学校管理
// ══════════════════════════════════════

void AdminManager::loadSchools()
{
    adminLog("loadSchools");
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/schools");
    QUrlQuery query;
    query.addQueryItem("select", "id,name,class_quota,created_at");
    query.addQueryItem("order", "created_at.desc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            adminLog(QString("loadSchools ERROR: %1").arg(reply->errorString()));
            emit error("加载学校列表失败");
            return;
        }
        QByteArray data = reply->readAll();
        adminLog(QString("loadSchools OK: %1").arg(QString::fromUtf8(data.left(500))));
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        QList<SchoolInfo> list;
        for (const auto &v : arr) list.append(SchoolInfo::fromJson(v.toObject()));

        // 查询每个学校的统计数据（班级数/教师数）
        // 简化：后续通过聚合查询补充，先返回基本信息
        qDebug() << "[Admin] 学校列表:" << list.size();
        emit schoolsLoaded(list);
    });
}

void AdminManager::createSchool(const QString &name, int classQuota)
{
    adminLog(QString("createSchool name=%1 quota=%2").arg(name).arg(classQuota));
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/schools");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "return=representation");

    QJsonObject body;
    body["name"] = name;
    body["class_quota"] = classQuota;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QString errBody = reply->readAll();
            adminLog(QString("createSchool ERROR: %1 body=%2").arg(reply->errorString()).arg(errBody.left(300)));
            emit error("创建学校失败");
            return;
        }
        QByteArray data = reply->readAll();
        adminLog(QString("createSchool OK: %1").arg(QString::fromUtf8(data.left(500))));
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        SchoolInfo info;
        if (!arr.isEmpty()) {
            info = SchoolInfo::fromJson(arr[0].toObject());
        }
        emit schoolCreated(info);
    });
}

void AdminManager::updateSchool(const QString &schoolId, const QString &name, int classQuota)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/schools");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + schoolId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["name"] = name;
    body["class_quota"] = classQuota;

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, schoolId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Admin] updateSchool failed:" << reply->errorString();
            emit error("更新学校失败");
            return;
        }
        qDebug() << "[Admin] 学校已更新:" << schoolId;
        emit schoolUpdated();
    });
}

void AdminManager::deleteSchool(const QString &schoolId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/schools");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + schoolId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, schoolId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Admin] deleteSchool failed:" << reply->errorString();
            emit error("删除学校失败");
            return;
        }
        qDebug() << "[Admin] 学校已删除:" << schoolId;
        emit schoolDeleted(schoolId);
    });
}

// ══════════════════════════════════════
//  邀请码管理
// ══════════════════════════════════════

void AdminManager::loadInviteCodes(const QString &schoolId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/invitation_codes");
    QUrlQuery query;
    query.addQueryItem("select", "id,school_id,code,used,used_by_email,created_at");
    if (!schoolId.isEmpty()) query.addQueryItem("school_id", "eq." + schoolId);
    query.addQueryItem("order", "created_at.desc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Admin] loadInviteCodes failed:" << reply->errorString();
            emit error("加载邀请码失败");
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<InviteCodeInfo> list;
        for (const auto &v : arr) list.append(InviteCodeInfo::fromJson(v.toObject()));
        qDebug() << "[Admin] 邀请码列表:" << list.size();
        emit inviteCodesLoaded(list);
    });
}

void AdminManager::generateInviteCode(const QString &schoolId, int count)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/invitation_codes");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "return=representation");

    QJsonArray bodyArr;
    const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QStringList generatedCodes;
    auto rng = QRandomGenerator::global();

    for (int i = 0; i < count; ++i) {
        QString code;
        for (int j = 0; j < 8; ++j) code += chars[rng->bounded(chars.size())];
        generatedCodes << code;
        QJsonObject item;
        item["school_id"] = schoolId;
        item["code"] = code;
        bodyArr.append(item);
    }

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(bodyArr).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, generatedCodes]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Admin] generateInviteCode failed:" << reply->errorString();
            emit error("生成邀请码失败");
            return;
        }
        qDebug() << "[Admin] 邀请码已生成:" << generatedCodes;
        emit inviteCodeGenerated(generatedCodes);
    });
}

void AdminManager::deleteInviteCode(const QString &codeId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/invitation_codes");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + codeId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, codeId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Admin] deleteInviteCode failed:" << reply->errorString();
            emit error("删除邀请码失败");
            return;
        }
        emit inviteCodeDeleted(codeId);
    });
}

// ══════════════════════════════════════
//  教师管理
// ══════════════════════════════════════

void AdminManager::loadTeachers(const QString &schoolId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/" + SupabaseConfig::USERS_TABLE);
    QUrlQuery query;
    query.addQueryItem("select", "email,role,school_id");
    query.addQueryItem("role", "eq.教师");
    if (!schoolId.isEmpty()) query.addQueryItem("school_id", "eq." + schoolId);
    query.addQueryItem("order", "email.asc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            adminLog(QString("loadTeachers ERROR: %1").arg(reply->errorString()));
            emit error("加载教师列表失败");
            return;
        }
        QByteArray data = reply->readAll();
        adminLog(QString("loadTeachers OK: %1").arg(QString::fromUtf8(data.left(500))));
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        QList<TeacherInfo> list;
        for (const auto &v : arr) list.append(TeacherInfo::fromJson(v.toObject()));
        qDebug() << "[Admin] 教师列表:" << list.size();
        emit teachersLoaded(list);
    });
}

// ══════════════════════════════════════
//  班级管理
// ══════════════════════════════════════

void AdminManager::loadAllClasses()
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QUrlQuery query;
    query.addQueryItem("select", "*");
    query.addQueryItem("order", "created_at.desc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            adminLog(QString("loadAllClasses ERROR: %1").arg(reply->errorString()));
            emit error("加载班级列表失败");
            return;
        }
        QByteArray data = reply->readAll();
        adminLog(QString("loadAllClasses OK: %1").arg(QString::fromUtf8(data.left(500))));
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        QList<ClassBrief> list;
        for (const auto &v : arr) list.append(ClassBrief::fromJson(v.toObject()));
        qDebug() << "[Admin] 全部班级:" << list.size();
        emit allClassesLoaded(list);
    });
}

void AdminManager::loadSchoolClasses(const QString &schoolId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QUrlQuery query;
    query.addQueryItem("select", "*");
    query.addQueryItem("school_id", "eq." + schoolId);
    query.addQueryItem("order", "created_at.desc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, schoolId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Admin] loadSchoolClasses failed:" << reply->errorString();
            emit error("加载班级失败");
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<ClassBrief> list;
        for (const auto &v : arr) list.append(ClassBrief::fromJson(v.toObject()));
        emit schoolClassesLoaded(schoolId, list);
    });
}

void AdminManager::deleteClass(const QString &classId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + classId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, classId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[Admin] deleteClass failed:" << reply->errorString();
            emit error("删除班级失败");
            return;
        }
        emit classDeleted(classId);
    });
}

// ══════════════════════════════════════
//  邀请码验证（设置页用）
// ══════════════════════════════════════

void AdminManager::validateAndUseInviteCode(const QString &code, const QString &userEmail)
{
    // 1. 查邀请码
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/invitation_codes");
    QUrlQuery query;
    query.addQueryItem("select", "id,school_id,code,used");
    query.addQueryItem("code", "eq." + code);
    query.addQueryItem("used", "is.false");
    query.addQueryItem("limit", "1");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, code, userEmail]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit inviteCodeInvalid("验证邀请码失败");
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        if (arr.isEmpty()) {
            emit inviteCodeInvalid("邀请码无效或已被使用");
            return;
        }

        auto info = arr[0].toObject();
        QString codeId = info["id"].toString();
        QString schoolId = info["school_id"].toString();

        // 2. 查学校名
        QUrl schUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/schools");
        QUrlQuery schQuery;
        schQuery.addQueryItem("select", "name");
        schQuery.addQueryItem("id", "eq." + schoolId);
        schUrl.setQuery(schQuery);

        QNetworkRequest schReq = NetworkRequestFactory::createAuthRequest(schUrl);
        QNetworkReply *schReply = m_networkManager->get(schReq);

        connect(schReply, &QNetworkReply::finished, this,
            [this, schReply, codeId, schoolId, userEmail, code]() {
            schReply->deleteLater();
            QString schoolName;
            if (schReply->error() == QNetworkReply::NoError) {
                QJsonArray schArr = QJsonDocument::fromJson(schReply->readAll()).array();
                if (!schArr.isEmpty()) schoolName = schArr[0].toObject()["name"].toString();
            }

            // 3. 标记邀请码已使用
            QUrl patchUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/invitation_codes");
            QUrlQuery patchQuery;
            patchQuery.addQueryItem("id", "eq." + codeId);
            patchUrl.setQuery(patchQuery);

            QNetworkRequest patchReq = NetworkRequestFactory::createAuthRequest(patchUrl);
            patchReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

            QJsonObject patchBody;
            patchBody["used"] = true;
            patchBody["used_by_email"] = userEmail;

            QNetworkReply *patchReply = m_networkManager->sendCustomRequest(
                patchReq, "PATCH", QJsonDocument(patchBody).toJson());

            connect(patchReply, &QNetworkReply::finished, this,
                [this, patchReply, schoolId, userEmail, schoolName]() {
                patchReply->deleteLater();
                if (patchReply->error() != QNetworkReply::NoError) {
                    emit inviteCodeInvalid("激活邀请码失败");
                    return;
                }

                // 4. 更新用户: role=教师, school_id
                QUrl userUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/" + SupabaseConfig::USERS_TABLE);
                QUrlQuery userQuery;
                userQuery.addQueryItem("email", "eq." + userEmail);
                userUrl.setQuery(userQuery);

                QNetworkRequest userReq = NetworkRequestFactory::createAuthRequest(userUrl);
                userReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

                QJsonObject userBody;
                userBody["role"] = "教师";
                userBody["school_id"] = schoolId;

                QNetworkReply *userReply = m_networkManager->sendCustomRequest(
                    userReq, "PATCH", QJsonDocument(userBody).toJson());

                connect(userReply, &QNetworkReply::finished, this,
                    [this, userReply, schoolId, schoolName]() {
                    userReply->deleteLater();
                    if (userReply->error() != QNetworkReply::NoError) {
                        QString errBody = userReply->readAll();
                        adminLog(QString("updateUserRole ERROR: %1 body=%2").arg(userReply->errorString()).arg(errBody.left(300)));
                        emit inviteCodeInvalid("更新角色失败");
                        return;
                    }
                    QByteArray resp = userReply->readAll();
                    adminLog(QString("updateUserRole OK schoolId=%1 resp=%2").arg(schoolId).arg(QString::fromUtf8(resp.left(300))));
                    qDebug() << "[Admin] 邀请码验证成功, 绑定学校:" << schoolName;
                    emit inviteCodeValid(schoolId, schoolName);
                });
            });
        });
    });
}

// ══════════════════════════════════════
//  教师操作
// ══════════════════════════════════════

void AdminManager::assignTeacherToSchool(const QString &email, const QString &schoolId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/" + SupabaseConfig::USERS_TABLE);
    QUrlQuery query;
    query.addQueryItem("email", "eq." + email);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["school_id"] = schoolId;

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { emit error("分配教师失败"); return; }
        emit teacherUpdated();
    });
}

void AdminManager::updateTeacherRole(const QString &email, const QString &role)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/" + SupabaseConfig::USERS_TABLE);
    QUrlQuery query;
    query.addQueryItem("email", "eq." + email);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["role"] = role;

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { emit error("更新角色失败"); return; }
        emit teacherUpdated();
    });
}

void AdminManager::deleteTeacher(const QString &email)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/" + SupabaseConfig::USERS_TABLE);
    QUrlQuery query;
    query.addQueryItem("email", "eq." + email);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, email]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { emit error("删除教师失败"); return; }
        emit teacherDeleted(email);
    });
}

void AdminManager::resetTeacherPassword(const QString &email, const QString &newPassword)
{
    // 用 admin API 列出用户，客户端按 email 过滤
    QUrl listUrl(SupabaseConfig::supabaseUrl() + "/auth/v1/admin/users");
    QUrlQuery listQuery;
    listQuery.addQueryItem("per_page", "1000");
    listUrl.setQuery(listQuery);

    QNetworkRequest listReq(listUrl);
    listReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    listReq.setRawHeader("apikey", SupabaseConfig::supabaseAnonKey().toUtf8());
    listReq.setRawHeader("Authorization", QString("Bearer %1").arg(SupabaseConfig::supabaseServiceKey()).toUtf8());

    QNetworkReply *listReply = m_networkManager->get(listReq);
    connect(listReply, &QNetworkReply::finished, this, [this, listReply, email, newPassword]() {
        listReply->deleteLater();
        if (listReply->error() != QNetworkReply::NoError) {
            emit error("查询教师失败");
            return;
        }
        QJsonObject resp = QJsonDocument::fromJson(listReply->readAll()).object();
        QJsonArray users = resp["users"].toArray();
        QString userId;
        for (const auto &u : users) {
            if (u.toObject()["email"].toString() == email) {
                userId = u.toObject()["id"].toString();
                break;
            }
        }
        if (userId.isEmpty()) {
            emit error("未找到该教师账号");
            return;
        }

        // 用 admin API 重置密码
        QUrl updateUrl(SupabaseConfig::supabaseUrl() + "/auth/v1/admin/users/" + userId);
        QNetworkRequest updateReq(updateUrl);
        updateReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        updateReq.setRawHeader("apikey", SupabaseConfig::supabaseAnonKey().toUtf8());
        updateReq.setRawHeader("Authorization", QString("Bearer %1").arg(SupabaseConfig::supabaseServiceKey()).toUtf8());

        QJsonObject body;
        body["password"] = newPassword;

        QNetworkReply *updateReply = m_networkManager->sendCustomRequest(updateReq, "PUT", QJsonDocument(body).toJson());
        connect(updateReply, &QNetworkReply::finished, this, [this, updateReply, email]() {
            updateReply->deleteLater();
            if (updateReply->error() != QNetworkReply::NoError) {
                emit error("重置密码失败");
                return;
            }
            emit passwordReset(email);
        });
    });
}

// ══════════════════════════════════════
//  班级更新
// ══════════════════════════════════════

void AdminManager::updateClass(const QString &classId, const QString &name, const QString &description, const QString &status)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + classId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    if (!name.isEmpty()) body["name"] = name;
    body["description"] = description;
    body["status"] = status;

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { emit error("更新班级失败"); return; }
        emit classUpdated();
    });
}

// ══════════════════════════════════════
//  总览统计
// ══════════════════════════════════════

void AdminManager::loadOverviewStats()
{
    // 并发查询4个计数
    auto *counts = new QMap<QString, int>();

    auto checkDone = [this, counts]() {
        if (counts->size() == 4) {
            emit overviewStatsLoaded(
                counts->value("schools", 0),
                counts->value("teachers", 0),
                counts->value("classes", 0),
                counts->value("students", 0));
            delete counts;
        }
    };

    // 学校数
    QUrl schUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/schools");
    QUrlQuery schQ; schQ.addQueryItem("select", "id"); schUrl.setQuery(schQ);
    QNetworkReply *r1 = m_networkManager->get(NetworkRequestFactory::createAuthRequest(schUrl));
    connect(r1, &QNetworkReply::finished, this, [r1, counts, checkDone]() {
        r1->deleteLater();
        counts->insert("schools", QJsonDocument::fromJson(r1->readAll()).array().size());
        checkDone();
    });

    // 教师数
    QUrl tUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/" + SupabaseConfig::USERS_TABLE);
    QUrlQuery tQ; tQ.addQueryItem("select", "email"); tQ.addQueryItem("role", "eq.教师"); tUrl.setQuery(tQ);
    QNetworkReply *r2 = m_networkManager->get(NetworkRequestFactory::createAuthRequest(tUrl));
    connect(r2, &QNetworkReply::finished, this, [r2, counts, checkDone]() {
        r2->deleteLater();
        counts->insert("teachers", QJsonDocument::fromJson(r2->readAll()).array().size());
        checkDone();
    });

    // 班级数
    QUrl cUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QUrlQuery cQ; cQ.addQueryItem("select", "id"); cUrl.setQuery(cQ);
    QNetworkReply *r3 = m_networkManager->get(NetworkRequestFactory::createAuthRequest(cUrl));
    connect(r3, &QNetworkReply::finished, this, [r3, counts, checkDone]() {
        r3->deleteLater();
        counts->insert("classes", QJsonDocument::fromJson(r3->readAll()).array().size());
        checkDone();
    });

    // 学生数
    QUrl sUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/class_members");
    QUrlQuery sQ; sQ.addQueryItem("select", "student_email"); sUrl.setQuery(sQ);
    QNetworkReply *r4 = m_networkManager->get(NetworkRequestFactory::createAuthRequest(sUrl));
    connect(r4, &QNetworkReply::finished, this, [r4, counts, checkDone]() {
        r4->deleteLater();
        QSet<QString> unique;
        for (const auto &v : QJsonDocument::fromJson(r4->readAll()).array())
            unique.insert(v.toObject()["student_email"].toString());
        counts->insert("students", unique.size());
        checkDone();
    });
}
