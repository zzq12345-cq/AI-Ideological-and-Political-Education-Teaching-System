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
#include <QSslError>
#include <QSslSocket>

class NetworkRetryHelper;

class SupabaseClient : public QObject
{
    Q_OBJECT

public:
    explicit SupabaseClient(QObject *parent = nullptr);
    ~SupabaseClient();

    // 登录
    void login(const QString &email, const QString &password);
    void refreshSession(const QString &refreshToken);

    // 注册
    void signup(const QString &email, const QString &password, const QString &username);

    // 检查用户是否存在
    void checkUserExists(const QString &email);

    // 密码重置
    void resetPassword(const QString &email);

    // 查询用户角色
    void fetchUserRole(const QString &email);

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

    // 密码重置相关信号
    void passwordResetSuccess(const QString &message);
    void passwordResetFailed(const QString &errorMessage);

    // 角色查询信号
    void roleFetched(const QString &role);

public:
    QString currentUserId() const { return m_currentUserId; }
    QString currentEmail() const { return m_currentEmail; }
    QString currentAccessToken() const { return m_currentAccessToken; }
    QString currentRefreshToken() const { return m_currentRefreshToken; }
    qint64 currentExpiresAt() const { return m_currentExpiresAt; }

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onNetworkError(QNetworkReply::NetworkError error);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void onAuthRequired(QNetworkReply *reply, QAuthenticator *authenticator);

private:
    QNetworkAccessManager *m_networkManager;
    NetworkRetryHelper *m_retryHelper;

    // 发送请求的通用方法
    void sendRequest(const QString &endpoint, const QJsonObject &data, bool isPost = true);
    void sendGetRequest(const QString &endpoint);
    void sendRequestWithManager(QNetworkAccessManager *manager,
                                const QNetworkRequest &request,
                                const QByteArray &data,
                                const QString &method,
                                bool allowDirectFallback);

    // 处理不同类型的响应
    void handleLoginResponse(const QJsonObject &json);
    void handleSignupResponse(const QJsonObject &json);
    void handlePasswordResetResponse(int httpStatus, const QByteArray &data);

    // 解析错误
    QString parseError(const QJsonObject &json);

    QString m_currentUserId;
    QString m_currentEmail;
    QString m_currentAccessToken;
    QString m_currentRefreshToken;
    qint64 m_currentExpiresAt = 0;
};

#endif // SUPABASECLIENT_H
