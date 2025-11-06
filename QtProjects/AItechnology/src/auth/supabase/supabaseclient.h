#ifndef SUPABASECLIENT_H
#define SUPABASECLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QAuthenticator>
#include <QTimer>

class SupabaseClient : public QObject
{
    Q_OBJECT

public:
    explicit SupabaseClient(QObject *parent = nullptr);
    ~SupabaseClient();

    // 登录
    void login(const QString &email, const QString &password);

    // 注册
    void signup(const QString &email, const QString &password, const QString &username);

    // 检查用户是否存在
    void checkUserExists(const QString &email);

signals:
    // 登录相关信号
    void loginSuccess(const QString &userId, const QString &email);
    void loginFailed(const QString &errorMessage);

    // 注册相关信号
    void signupSuccess(const QString &message);
    void signupFailed(const QString &errorMessage);

    // 用户检查相关信号
    void userExists(bool exists);
    void userCheckFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onAuthRequired(QNetworkReply *reply, QAuthenticator *authenticator);

private:
    QNetworkAccessManager *m_networkManager;

    // 发送请求的通用方法
    void sendRequest(const QString &endpoint, const QJsonObject &data, bool isPost = true);
    void sendGetRequest(const QString &endpoint);

    // 处理不同类型的响应
    void handleLoginResponse(const QJsonObject &json);
    void handleSignupResponse(const QJsonObject &json);
    void handleUserCheckResponse(const QJsonObject &json);

    // 解析错误
    QString parseError(const QJsonObject &json);
};

#endif // SUPABASECLIENT_H
