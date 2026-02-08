#include "supabaseclient.h"
#include "supabaseconfig.h"
#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

namespace {
bool allowInsecureSslForDebug()
{
    const QString value = qEnvironmentVariable("ALLOW_INSECURE_SSL").trimmed().toLower();
    return value == "1" || value == "true" || value == "yes";
}
}

SupabaseClient::SupabaseClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    qDebug() << "Supabase客户端初始化...";
    qDebug() << "SSL支持:" << QSslSocket::supportsSsl();
    qDebug() << "SSL版本:" << QSslSocket::sslLibraryVersionString();

    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired,
            this, &SupabaseClient::onAuthRequired);
}

SupabaseClient::~SupabaseClient()
{
}

void SupabaseClient::login(const QString &email, const QString &password)
{
    qDebug() << "尝试登录:" << email;

    QJsonObject authData;
    authData["email"] = email;
    authData["password"] = password;

    QString endpoint = SupabaseConfig::SUPABASE_URL + "/auth/v1/token?grant_type=password";
    sendRequest(endpoint, authData, true);
}

void SupabaseClient::signup(const QString &email, const QString &password, const QString &username)
{
    qDebug() << "尝试注册:" << email;

    QJsonObject authData;
    authData["email"] = email;
    authData["password"] = password;
    authData["data"] = QJsonObject::fromVariantMap(QVariantMap{
        {"username", username}
    });

    QString endpoint = SupabaseConfig::SUPABASE_URL + "/auth/v1/signup";
    sendRequest(endpoint, authData, true);
}

void SupabaseClient::checkUserExists(const QString &email)
{
    qDebug() << "检查用户是否存在:" << email;

    QUrl endpoint(SupabaseConfig::SUPABASE_URL + "/rest/v1/" + SupabaseConfig::USERS_TABLE);
    QUrlQuery query;
    query.addQueryItem("select", "id");
    query.addQueryItem("email", "eq." + email);
    endpoint.setQuery(query);

    sendGetRequest(endpoint.toString(QUrl::FullyEncoded));
}

void SupabaseClient::sendRequest(const QString &endpoint, const QJsonObject &data, bool isPost)
{
    const QUrl endpointUrl(endpoint);
    qDebug() << "发送请求到:" << endpointUrl.path();

    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("apikey", SupabaseConfig::SUPABASE_ANON_KEY.toUtf8());

    QJsonDocument doc(data);
    QByteArray postData = doc.toJson();

    QNetworkReply *reply = nullptr;
    if (isPost) {
        reply = m_networkManager->post(request, postData);
    } else {
        reply = m_networkManager->get(request);
    }

    if (reply) {
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            onReplyFinished(reply);
        });
        connect(reply, &QNetworkReply::errorOccurred, this, &SupabaseClient::onNetworkError);
        connect(reply, &QNetworkReply::sslErrors, this, &SupabaseClient::onSslErrors);
        qDebug() << "请求已发送，等待响应...";
    } else {
        qDebug() << "错误：无法创建网络请求";
        if (endpoint.contains("/auth/v1/token")) {
            emit loginFailed("无法创建网络请求");
        } else if (endpoint.contains("/auth/v1/signup")) {
            emit signupFailed("无法创建网络请求");
        } else if (endpoint.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
            emit userCheckFailed("无法创建网络请求");
        } else {
            emit loginFailed("无法创建网络请求");
        }
    }
}

void SupabaseClient::sendGetRequest(const QString &endpoint)
{
    sendRequest(endpoint, QJsonObject(), false);
}

void SupabaseClient::onReplyFinished(QNetworkReply *reply)
{
    if (!reply) return;

    QByteArray data = reply->readAll();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // 检查是否是认证相关的端点
    QString url = reply->url().toString();
    const bool isAuthEndpoint = url.contains("/auth/v1/token") || url.contains("/auth/v1/signup");

    qDebug() << "HTTP状态码:" << httpStatus << "URL:" << reply->url().path();
    if (isAuthEndpoint) {
        qDebug() << "认证接口响应长度:" << data.size();
    } else {
        qDebug() << "响应数据:" << data;
    }

    // 用户检查接口返回的是数组，需要特殊处理
    if (url.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "JSON解析错误:" << error.errorString();
            emit userCheckFailed(error.errorString());
        } else if (doc.isArray()) {
            // Supabase REST API返回数组
            bool exists = !doc.array().isEmpty();
            qDebug() << "用户存在状态:" << exists;
            emit userExists(exists);
        } else if (doc.isObject() && doc.object().contains("error")) {
            QString errorMsg = doc.object()["error"].toString();
            emit userCheckFailed(errorMsg);
        } else {
            emit userExists(false);
        }
        reply->deleteLater();
        return;
    }

    // 其他接口返回对象
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << error.errorString();
        if (url.contains("/auth/v1/token")) {
            emit loginFailed(error.errorString());
        } else if (url.contains("/auth/v1/signup")) {
            emit signupFailed(error.errorString());
        } else if (url.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
            emit userCheckFailed(error.errorString());
        }
        reply->deleteLater();
        return;
    }

    QJsonObject json = doc.object();

    if (url.contains("/auth/v1/token")) {
        handleLoginResponse(json);
    } else if (url.contains("/auth/v1/signup")) {
        handleSignupResponse(json);
    }

    reply->deleteLater();
}

void SupabaseClient::onAuthRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    qDebug() << "认证请求...";
    authenticator->setUser(SupabaseConfig::SUPABASE_ANON_KEY);
    authenticator->setPassword("");
}

void SupabaseClient::handleLoginResponse(const QJsonObject &json)
{
    qDebug() << "处理登录响应";

    if (json.contains("access_token")) {
        QString userId = json["user"].toObject()["id"].toString();
        QString email = json["user"].toObject()["email"].toString();

        qDebug() << "登录成功! 用户ID:" << userId << "邮箱:" << email;
        emit loginSuccess(userId, email);
    } else if (json.contains("error_description")) {
        QString error = json["error_description"].toString();
        qDebug() << "登录失败:" << error;
        emit loginFailed(error);
    } else {
        QString error = parseError(json);
        qDebug() << "登录失败:" << error;
        emit loginFailed(error);
    }
}

void SupabaseClient::handleSignupResponse(const QJsonObject &json)
{
    qDebug() << "处理注册响应";

    if (json.contains("access_token")) {
        QString userId = json["user"].toObject()["id"].toString();
        qDebug() << "注册成功! 用户ID:" << userId;
        emit signupSuccess("注册成功！请检查您的邮箱进行验证。");
    } else if (json.contains("msg")) {
        QString error = json["msg"].toString();
        qDebug() << "注册失败:" << error;
        emit signupFailed(error);
    } else {
        QString error = parseError(json);
        qDebug() << "注册失败:" << error;
        emit signupFailed(error);
    }
}

QString SupabaseClient::parseError(const QJsonObject &json)
{
    if (json.contains("error_description")) {
        return json["error_description"].toString();
    } else if (json.contains("error")) {
        return json["error"].toString();
    } else if (json.contains("msg")) {
        return json["msg"].toString();
    } else {
        return "未知错误";
    }
}

void SupabaseClient::onNetworkError(QNetworkReply::NetworkError error)
{
    qDebug() << "网络错误:" << error;

    QString errorString;
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorString = "连接被拒绝，请检查网络连接";
        break;
    case QNetworkReply::HostNotFoundError:
        errorString = "主机未找到，请检查服务器地址";
        break;
    case QNetworkReply::TimeoutError:
        errorString = "连接超时，请检查网络连接";
        break;
    case QNetworkReply::SslHandshakeFailedError:
        errorString = "SSL握手失败，可能存在证书问题";
        break;
    case QNetworkReply::ProtocolFailure:
        errorString = "协议错误，服务器返回了无效响应";
        break;
    case QNetworkReply::UnknownServerError:
        errorString = "未知服务器错误";
        break;
    default:
        errorString = QString("网络错误：%1").arg(error);
        break;
    }

    qDebug() << "网络错误详情:" << errorString;

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    const QString url = reply ? reply->url().toString() : QString();
    if (url.contains("/auth/v1/token")) {
        emit loginFailed(errorString);
    } else if (url.contains("/auth/v1/signup")) {
        emit signupFailed(errorString);
    } else if (url.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
        emit userCheckFailed(errorString);
    } else {
        emit loginFailed(errorString);
    }
}

void SupabaseClient::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "SSL错误数量:" << errors.size();
    for (const QSslError &error : errors) {
        qDebug() << "SSL错误:" << error.errorString();
    }

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply && allowInsecureSslForDebug()) {
        qWarning() << "ALLOW_INSECURE_SSL 已启用，忽略 SSL 错误（仅用于开发调试）";
        reply->ignoreSslErrors(errors);
        return;
    }

    if (reply) {
        reply->abort();
    }

    const QString errorMsg = "SSL 证书校验失败，已拒绝建立不安全连接";
    const QString url = reply ? reply->url().toString() : QString();
    if (url.contains("/auth/v1/token")) {
        emit loginFailed(errorMsg);
    } else if (url.contains("/auth/v1/signup")) {
        emit signupFailed(errorMsg);
    } else if (url.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
        emit userCheckFailed(errorMsg);
    } else {
        emit loginFailed(errorMsg);
    }
}
