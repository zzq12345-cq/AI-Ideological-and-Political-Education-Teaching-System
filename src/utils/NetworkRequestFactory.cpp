#include "NetworkRequestFactory.h"
#include "../auth/supabase/supabaseconfig.h"
#include <QDebug>

// ===== 公共辅助方法 =====

bool NetworkRequestFactory::allowInsecureSslForDebug()
{
    const QString value = qEnvironmentVariable("ALLOW_INSECURE_SSL").trimmed().toLower();
    return value == "1" || value == "true" || value == "yes";
}

// ===== 内部辅助 =====

void NetworkRequestFactory::applyBaseConfig(QNetworkRequest &request)
{
    // 全局禁用 HTTP/2，避免 macOS 上的协议错误
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
}

void NetworkRequestFactory::applySslConfig(QNetworkRequest &request)
{
    if (allowInsecureSslForDebug()) {
        QSslConfiguration sslConfig = request.sslConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(sslConfig);
    }
}

// ===== 工厂方法 =====

QNetworkRequest NetworkRequestFactory::createDifyRequest(const QUrl &url,
                                                         const QString &apiKey,
                                                         int timeout)
{
    QNetworkRequest request(url);

    // 请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    // 基础配置
    applyBaseConfig(request);
    applySslConfig(request);

    // 超时
    request.setTransferTimeout(timeout);

    // 重定向策略
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
#endif

    return request;
}

QNetworkRequest NetworkRequestFactory::createDifyUploadRequest(const QUrl &url,
                                                               const QString &apiKey,
                                                               int timeout)
{
    QNetworkRequest request(url);

    // 文件上传不设 Content-Type，由 QHttpMultiPart 自动处理
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    // 基础配置
    applyBaseConfig(request);
    applySslConfig(request);

    // 超时
    request.setTransferTimeout(timeout);

    return request;
}

QNetworkRequest NetworkRequestFactory::createSupabaseRequest(const QString &endpoint,
                                                             const QString &accessToken,
                                                             bool preferRepresentation,
                                                             int timeout)
{
    QUrl url(SupabaseConfig::SUPABASE_URL + endpoint);
    QNetworkRequest request(url);

    // 请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("apikey", SupabaseConfig::SUPABASE_ANON_KEY.toUtf8());

    // 认证: 优先使用用户令牌，否则使用 anon key
    if (!accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(accessToken).toUtf8());
    } else {
        request.setRawHeader("Authorization",
                             QString("Bearer %1").arg(SupabaseConfig::SUPABASE_ANON_KEY).toUtf8());
    }

    if (preferRepresentation) {
        request.setRawHeader("Prefer", "return=representation");
    }

    // 基础配置
    applyBaseConfig(request);
    applySslConfig(request);

    // 超时
    request.setTransferTimeout(timeout);

    return request;
}

QNetworkRequest NetworkRequestFactory::createGeneralRequest(const QUrl &url, int timeout)
{
    QNetworkRequest request(url);

    // 基础配置
    applyBaseConfig(request);
    applySslConfig(request);

    // 超时
    request.setTransferTimeout(timeout);

    return request;
}

QNetworkRequest NetworkRequestFactory::createAuthRequest(const QUrl &url, int timeout)
{
    QNetworkRequest request(url);

    // 请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("apikey", SupabaseConfig::SUPABASE_ANON_KEY.toUtf8());

    // 基础配置
    applyBaseConfig(request);
    applySslConfig(request);

    // 超时
    request.setTransferTimeout(timeout);

    return request;
}
