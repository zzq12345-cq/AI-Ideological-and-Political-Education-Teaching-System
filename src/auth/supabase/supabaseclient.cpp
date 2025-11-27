#include "supabaseclient.h"
#include "supabaseconfig.h"
#include <QDebug>
#include <QMessageBox>

SupabaseClient::SupabaseClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    qDebug() << "Supabase客户端初始化...";
    qDebug() << "SSL支持:" << QSslSocket::supportsSsl();
    qDebug() << "SSL版本:" << QSslSocket::sslLibraryVersionString();
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

    QString endpoint = SupabaseConfig::SUPABASE_URL + "/rest/v1/" + SupabaseConfig::USERS_TABLE +
                      "?select=id&email=eq." + email;

    sendGetRequest(endpoint);
}

void SupabaseClient::sendRequest(const QString &endpoint, const QJsonObject &data, bool isPost)
{
    qDebug() << "发送请求到:" << endpoint;

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
        emit loginFailed("无法创建网络请求");
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

    qDebug() << "HTTP状态码:" << httpStatus;
    qDebug() << "响应数据:" << data;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << error.errorString();
        reply->deleteLater();
        return;
    }

    QJsonObject json = doc.object();

    // 检查是否是认证相关的端点
    QString url = reply->url().toString();

    if (url.contains("/auth/v1/token")) {
        handleLoginResponse(json);
    } else if (url.contains("/auth/v1/signup")) {
        handleSignupResponse(json);
    } else if (url.contains("/rest/v1/" + SupabaseConfig::USERS_TABLE)) {
        handleUserCheckResponse(json);
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
    qDebug() << "处理登录响应:" << json;

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
    qDebug() << "处理注册响应:" << json;

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

void SupabaseClient::handleUserCheckResponse(const QJsonObject &json)
{
    qDebug() << "处理用户检查响应:" << json;

    // 如果返回的是数组且有元素，说明用户存在
    if (json.contains("")) {
        // 检查是否是数组格式
        // 假设Supabase REST API返回数组格式
        QJsonValue usersValue = json.value("");
        if (usersValue.isArray()) {
            QJsonArray users = usersValue.toArray();
            bool exists = !users.isEmpty();
            qDebug() << "用户存在状态:" << exists;
            emit userExists(exists);
        } else if (json.contains("error")) {
            QString error = json["error"].toString();
            qDebug() << "用户检查失败:" << error;
            emit userCheckFailed(error);
        } else {
            // 假设不存在
            emit userExists(false);
        }
    } else if (json.contains("error")) {
        QString error = json["error"].toString();
        qDebug() << "用户检查失败:" << error;
        emit userCheckFailed(error);
    } else {
        // 假设不存在
        emit userExists(false);
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
    emit loginFailed(errorString);
}

void SupabaseClient::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "SSL错误数量:" << errors.size();
    for (const QSslError &error : errors) {
        qDebug() << "SSL错误:" << error.errorString();
    }

    // 为了调试，暂时忽略SSL错误
    // 在生产环境中应该严格验证SSL证书
    emit loginFailed("SSL连接错误: " + (errors.isEmpty() ? "未知SSL错误" : errors.first().errorString()));
}
