#include "ClassManager.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QUuid>
#include <QUrlQuery>
#include <QDebug>

ClassManager* ClassManager::s_instance = nullptr;

ClassManager* ClassManager::instance()
{
    if (!s_instance) {
        s_instance = new ClassManager();
    }
    return s_instance;
}

ClassManager::ClassManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

// ── 教师加载班级 ──
void ClassManager::loadTeacherClasses(const QString &email)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QUrlQuery query;
    query.addQueryItem("select", "*");
    query.addQueryItem("teacher_email", "eq." + email);
    query.addQueryItem("order", "created_at.desc");
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] loadTeacherClasses failed:" << reply->errorString();
            emit error("加载班级失败: " + reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        QList<ClassInfo> classes;
        for (const auto &val : arr) {
            classes.append(ClassInfo::fromJson(val.toObject()));
        }
        m_cached = classes;
        qDebug() << "[ClassManager] 教师班级加载完成:" << classes.size();
        emit classesLoaded(classes);
    });
}

// ── 学生加载已加入班级 ──
void ClassManager::loadStudentClasses(const QString &email)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/class_members");
    QUrlQuery query;
    query.addQueryItem("select", "class_id,classes(*)");
    query.addQueryItem("student_email", "eq." + email);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] loadStudentClasses failed:" << reply->errorString();
            emit error("加载班级失败: " + reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        QList<ClassInfo> classes;
        for (const auto &val : arr) {
            QJsonObject obj = val.toObject();
            QJsonObject classObj = obj["classes"].toObject();
            classes.append(ClassInfo::fromJson(classObj));
        }
        m_cached = classes;
        qDebug() << "[ClassManager] 学生班级加载完成:" << classes.size();
        emit classesLoaded(classes);
    });
}

// ── 创建班级 ──
void ClassManager::createClass(const QString &name, const QString &teacherName, const QString &teacherEmail)
{
    // 生成6位码
    const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QString code;
    auto rng = QRandomGenerator::global();
    for (int i = 0; i < 6; ++i) {
        code += chars[rng->bounded(chars.size())];
    }

    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Prefer: return=representation 让 Supabase 返回插入的行
    request.setRawHeader("Prefer", "return=representation");

    QJsonObject body;
    body["name"] = name;
    body["teacher_email"] = teacherEmail;
    body["teacher_name"] = teacherName;
    body["code"] = code;
    body["status"] = "active";

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, teacherEmail, teacherName]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] createClass failed:" << reply->errorString() << reply->readAll();
            emit error("创建班级失败");
            return;
        }
        QByteArray data = reply->readAll();
        QJsonArray arr = QJsonDocument::fromJson(data).array();
        if (!arr.isEmpty()) {
            ClassInfo info = ClassInfo::fromJson(arr[0].toObject());
            qDebug() << "[ClassManager] 班级创建成功:" << info.name << "code:" << info.code;

            // 将教师加入 class_members
            QUrl memberUrl(SupabaseConfig::supabaseUrl() + "/rest/v1/class_members");
            QNetworkRequest memberReq = NetworkRequestFactory::createAuthRequest(memberUrl);
            memberReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            QJsonObject memberBody;
            memberBody["class_id"] = info.id;
            memberBody["student_email"] = teacherEmail;
            memberBody["student_name"] = teacherName;
            memberBody["student_number"] = "";
            QNetworkReply *memberReply = m_networkManager->post(memberReq, QJsonDocument(memberBody).toJson());
            connect(memberReply, &QNetworkReply::finished, memberReply, &QNetworkReply::deleteLater);

            emit classCreated(info);
        }
    });
}

// ── 刷新班级码 ──
void ClassManager::refreshCode(const QString &classId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/rpc/refresh_class_code");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);

    QJsonObject body;
    body["p_class_id"] = classId;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, classId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] refreshCode failed:" << reply->errorString();
            emit error("刷新班级码失败");
            return;
        }
        QString newCode = QString::fromUtf8(reply->readAll()).trimmed();
        // 去掉 JSON 引号
        if (newCode.startsWith('"')) newCode = newCode.mid(1);
        if (newCode.endsWith('"')) newCode.chop(1);
        qDebug() << "[ClassManager] 班级码刷新:" << newCode;
        emit codeRefreshed(classId, newCode);
    });
}

// ── 学生加入班级 ──
void ClassManager::joinClass(const QString &code, const QString &studentEmail, const QString &studentName, const QString &studentNumber)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/rpc/student_join_class");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);

    QJsonObject body;
    body["p_code"] = code;
    body["p_student_email"] = studentEmail;
    body["p_student_name"] = studentName;
    body["p_student_number"] = studentNumber;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] joinClass failed:" << reply->errorString();
            emit joinResult(false, "加入班级失败: " + reply->errorString(), {});
            return;
        }
        QByteArray data = reply->readAll();
        QJsonObject result = QJsonDocument::fromJson(data).object();

        if (result.contains("error")) {
            emit joinResult(false, result["error"].toString(), {});
        } else {
            QString className = result["class_name"].toString();
            emit joinResult(true, QString("成功加入 \"%1\"！").arg(className), className);
        }
    });
}

// ── 删除班级 ──
void ClassManager::deleteClass(const QString &classId)
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
            qWarning() << "[ClassManager] deleteClass failed:" << reply->errorString();
            emit error("删除班级失败");
            return;
        }
        qDebug() << "[ClassManager] 班级已删除:" << classId;
        emit classDeleted(classId);
    });
}

void ClassManager::loadClassMembers(const QString &classId)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/class_members");
    QUrlQuery query;
    query.addQueryItem("select", "student_email,student_name,student_number");
    query.addQueryItem("class_id", "eq." + classId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, classId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] loadMembers failed:" << reply->errorString();
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<MemberInfo> members;
        for (const auto &val : arr) {
            QJsonObject obj = val.toObject();
            MemberInfo m;
            m.email = obj["student_email"].toString();
            m.name = obj["student_name"].toString();
            m.number = obj["student_number"].toString();
            members.append(m);
        }
        qDebug() << "[ClassManager] 成员列表:" << classId << members.size() << "人";
        emit membersLoaded(classId, members);
    });
}

// ── 踢出学生 ──
void ClassManager::removeMember(const QString &classId, const QString &studentEmail)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/rpc/remove_class_member");
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["p_class_id"] = classId;
    body["p_student_email"] = studentEmail;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, classId, studentEmail]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] removeMember failed:" << reply->errorString();
            emit error("移除学生失败");
            return;
        }
        qDebug() << "[ClassManager] 学生已移除:" << studentEmail;
        emit memberRemoved(classId, studentEmail);
        // 重新加载成员列表
        loadClassMembers(classId);
    });
}

// ── 更新班级信息 ──
void ClassManager::updateClass(const QString &classId, const QString &name, const QString &description, bool isPublic)
{
    QUrl url(SupabaseConfig::supabaseUrl() + "/rest/v1/classes");
    QUrlQuery query;
    query.addQueryItem("id", "eq." + classId);
    url.setQuery(query);

    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "return=representation");

    QJsonObject body;
    body["name"] = name;
    body["description"] = description;
    body["is_public"] = isPublic;

    QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, classId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[ClassManager] updateClass failed:" << reply->errorString();
            emit error("更新班级失败");
            return;
        }
        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        if (!arr.isEmpty()) {
            ClassInfo info = ClassInfo::fromJson(arr[0].toObject());
            qDebug() << "[ClassManager] 班级已更新:" << info.name;
            emit classUpdated(info);
        }
    });
}
