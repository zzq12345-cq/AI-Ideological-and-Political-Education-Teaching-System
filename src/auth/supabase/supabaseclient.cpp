#include "supabaseclient.h"
#include "supabaseconfig.h"
#include "../../utils/NetworkRequestFactory.h"
#include "../../utils/NetworkRetryHelper.h"
#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

namespace {
QString mapAuthNetworkError(QNetworkReply::NetworkError error, const QString &detail)
{
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        return "连接被拒绝，请检查网络";
    case QNetworkReply::HostNotFoundError:
        return "无法解析服务器地址，请检查网络或 DNS";
    case QNetworkReply::TimeoutError:
        return "连接超时，请检查网络";
    case QNetworkReply::SslHandshakeFailedError:
        return "SSL 握手失败，请检查系统时间或代理证书";
    case QNetworkReply::RemoteHostClosedError:
        return "服务器提前关闭了连接，请稍后重试";
    case QNetworkReply::ProxyConnectionRefusedError:
    case QNetworkReply::ProxyConnectionClosedError:
    case QNetworkReply::ProxyNotFoundError:
    case QNetworkReply::ProxyTimeoutError:
    case QNetworkReply::ProxyAuthenticationRequiredError:
    case QNetworkReply::UnknownProxyError:
        return "代理连接失败，请检查代理设置或关闭代理后重试";
    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::NetworkSessionFailedError:
    case QNetworkReply::UnknownNetworkError:
        return "当前网络不可用，请检查网络环境后重试";
    case QNetworkReply::ProtocolFailure:
        return "网络协议错误，请稍后重试";
    default:
        return detail.isEmpty() ? "网络错误，请稍后重试" : QString("网络错误: %1").arg(detail);
    }
}
}

SupabaseClient::SupabaseClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_retryHelper(new NetworkRetryHelper(m_networkManager,
                                            {2, 1000, 2.0, {500, 502, 503, 504, 408}},
                                            this))
{
    qDebug() << "Supabase客户端初始化...";
    qDebug() << "SSL支持:" << QSslSocket::supportsSsl();
    qDebug() << "SSL版本:" << QSslSocket::sslLibraryVersionString();

    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired,
            this, &SupabaseClient::onAuthRequired);
    connect(m_networkManager, &QNetworkAccessManager::sslErrors,
            this, &SupabaseClient::onSslErrors);
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

void SupabaseClient::resetPassword(const QString &email)
{
    qDebug() << "请求密码重置:" << email;

    QJsonObject body;
    body["email"] = email;

    QString endpoint = SupabaseConfig::SUPABASE_URL + "/auth/v1/recover";
    sendRequest(endpoint, body, true);
}

void SupabaseClient::sendRequest(const QString &endpoint, const QJsonObject &data, bool isPost)
{
    const QUrl endpointUrl(endpoint);
    qDebug() << "=== 发送认证请求 ===";
    qDebug() << "完整URL:" << endpoint;
    qDebug() << "Supabase URL:" << SupabaseConfig::SUPABASE_URL;
    qDebug() << "Anon Key 是否设置:" << (!SupabaseConfig::SUPABASE_ANON_KEY.isEmpty() ? "是" : "否");

    // 使用工厂创建认证请求
    QNetworkRequest request = NetworkRequestFactory::createAuthRequest(QUrl(endpoint));

    QJsonDocument doc(data);
    QByteArray postData = doc.toJson();

    // 通过重试器发送（认证请求最多重试2次）
    auto *retryHelper = new NetworkRetryHelper(m_networkManager,
                                                {2, 1000, 2.0, {500, 502, 503, 504, 408}},
                                                this);
    QString method = isPost ? "POST" : "GET";
    connect(retryHelper, &NetworkRetryHelper::finished, this, [this, retryHelper](QNetworkReply *reply) {
        onReplyFinished(reply);
        retryHelper->deleteLater();
    });
    connect(retryHelper, &NetworkRetryHelper::retrying, this, [](int attempt, int max) {
        qDebug() << QString("认证请求重试 %1/%2").arg(attempt).arg(max);
    });

    retryHelper->sendRequest(request, isPost ? postData : QByteArray(), method);
    qDebug() << "请求已发送（带重试），等待响应...";
}

void SupabaseClient::sendGetRequest(const QString &endpoint)
{
    sendRequest(endpoint, QJsonObject(), false);
}

void SupabaseClient::onReplyFinished(QNetworkReply *reply)
{
    if (!reply) return;

    // 先检查网络错误
    QNetworkReply::NetworkError netError = reply->error();
    QString url = reply->url().toString();

    if (reply->property("sslErrorHandled").toBool()) {
        reply->deleteLater();
        return;
    }

    if (netError != QNetworkReply::NoError) {
        const QString networkErrorMessage = mapAuthNetworkError(netError, reply->errorString());
        qDebug() << "网络错误:" << netError << reply->errorString();
        qDebug() << "网络错误映射:" << networkErrorMessage;

        if (url.contains("/auth/v1/token")) {
            emit loginFailed(networkErrorMessage);
        } else if (url.contains("/auth/v1/signup")) {
            emit signupFailed(networkErrorMessage);
        } else if (url.contains("/auth/v1/recover")) {
            emit passwordResetFailed(networkErrorMessage);
        } else {
            emit loginFailed(networkErrorMessage);
        }
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // 检查是否是认证相关的端点
    const bool isAuthEndpoint = url.contains("/auth/v1/token") || url.contains("/auth/v1/signup") || url.contains("/auth/v1/recover");

    qDebug() << "HTTP状态码:" << httpStatus << "URL:" << reply->url().path();
    if (isAuthEndpoint) {
        qDebug() << "认证接口响应长度:" << data.size();
    } else {
        qDebug() << "响应数据:" << data;
    }

    // 密码重置接口：成功时返回空 body 或简短 JSON
    if (url.contains("/auth/v1/recover")) {
        handlePasswordResetResponse(httpStatus, data);
        reply->deleteLater();
        return;
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
        } else if (url.contains("/auth/v1/recover")) {
            emit passwordResetFailed(error.errorString());
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

void SupabaseClient::handlePasswordResetResponse(int httpStatus, const QByteArray &data)
{
    qDebug() << "处理密码重置响应, HTTP状态:" << httpStatus;

    if (httpStatus == 200) {
        emit passwordResetSuccess("密码重置邮件已发送，请检查您的邮箱。");
        return;
    }

    // 非 200 尝试解析错误信息
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
    if (parseErr.error == QJsonParseError::NoError && doc.isObject()) {
        QString errMsg = parseError(doc.object());
        emit passwordResetFailed(errMsg);
    } else {
        emit passwordResetFailed(QString("密码重置失败（HTTP %1）").arg(httpStatus));
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

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    const QString detail = reply ? reply->errorString() : QString();
    const QString errorString = mapAuthNetworkError(error, detail);

    qDebug() << "网络错误详情:" << errorString;

    const QString url = reply ? reply->url().toString() : QString();
    if (url.contains("/auth/v1/token")) {
        emit loginFailed(errorString);
    } else if (url.contains("/auth/v1/signup")) {
        emit signupFailed(errorString);
    } else if (url.contains("/auth/v1/recover")) {
        emit passwordResetFailed(errorString);
    } else if (url.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
        emit userCheckFailed(errorString);
    } else {
        emit loginFailed(errorString);
    }
}

void SupabaseClient::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    if (NetworkRequestFactory::handleSslErrors(reply, errors, "[SupabaseClient]")) {
        return;
    }

    if (reply) {
        reply->setProperty("sslErrorHandled", true);
    }

    const QString errorMsg = "SSL 证书校验失败，已拒绝建立不安全连接";
    const QString url = reply ? reply->url().toString() : QString();
    if (url.contains("/auth/v1/token")) {
        emit loginFailed(errorMsg);
    } else if (url.contains("/auth/v1/signup")) {
        emit signupFailed(errorMsg);
    } else if (url.contains("/auth/v1/recover")) {
        emit passwordResetFailed(errorMsg);
    } else if (url.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
        emit userCheckFailed(errorMsg);
    } else {
        emit loginFailed(errorMsg);
    }
}
