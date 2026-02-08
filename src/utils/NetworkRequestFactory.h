#ifndef NETWORKREQUESTFACTORY_H
#define NETWORKREQUESTFACTORY_H

#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QString>
#include <QUrl>

/**
 * @brief 网络请求工厂类 - 统一创建已配置的 QNetworkRequest
 *
 * 纯静态工具类，与 SupabaseConfig 风格一致，不持有状态。
 * 各服务继续自己管理 QNetworkAccessManager，本类只负责创建请求对象。
 *
 * 统一配置项：
 * - SSL: 默认严格校验，ALLOW_INSECURE_SSL=1 时降级为 VerifyNone（仅开发调试）
 * - HTTP/2: 全局禁用（避免 macOS 上的协议错误）
 * - 超时: 各方法提供合理默认值，调用方可覆盖
 * - 重定向: Dify 请求启用 NoLessSafeRedirectPolicy
 */
class NetworkRequestFactory
{
public:
    // ===== 工厂方法 =====

    /**
     * @brief 创建 Dify API 请求（Bearer + JSON + SSL + HTTP/2 + 超时 + 重定向）
     * @param url 完整请求 URL
     * @param apiKey Dify API Key
     * @param timeout 超时毫秒数，默认 120s
     */
    static QNetworkRequest createDifyRequest(const QUrl &url,
                                             const QString &apiKey,
                                             int timeout = 120000);

    /**
     * @brief 创建 Dify 文件上传请求（不设 Content-Type，由 QHttpMultiPart 自动处理）
     * @param url 完整请求 URL
     * @param apiKey Dify API Key
     * @param timeout 超时毫秒数，默认 120s
     */
    static QNetworkRequest createDifyUploadRequest(const QUrl &url,
                                                   const QString &apiKey,
                                                   int timeout = 120000);

    /**
     * @brief 创建 Supabase REST 请求（内部拼接 SupabaseConfig::SUPABASE_URL）
     * @param endpoint REST 端点路径（如 "/rest/v1/notifications?..."）
     * @param accessToken 可选的用户访问令牌（为空则使用 anon key）
     * @param preferRepresentation 是否设置 Prefer: return=representation
     */
    static QNetworkRequest createSupabaseRequest(const QString &endpoint,
                                                 const QString &accessToken = QString(),
                                                 bool preferRepresentation = true);

    /**
     * @brief 创建通用请求（HTTP/2 禁用 + 超时）
     * @param url 完整请求 URL
     * @param timeout 超时毫秒数，默认 30s
     */
    static QNetworkRequest createGeneralRequest(const QUrl &url,
                                                int timeout = 30000);

    // ===== 公共辅助方法 =====

    /**
     * @brief 检查是否允许不安全 SSL（读取 ALLOW_INSECURE_SSL 环境变量）
     *
     * 供各服务的 onSslErrors 回调使用，统一判断逻辑。
     * @return true 表示开发调试模式，允许忽略 SSL 错误
     */
    static bool allowInsecureSslForDebug();

private:
    // 禁止实例化
    NetworkRequestFactory() = default;

    // ===== 内部辅助 =====

    /** 禁用 HTTP/2 */
    static void applyBaseConfig(QNetworkRequest &request);

    /** 如果调试开关开启，配置不安全 SSL */
    static void applySslConfig(QNetworkRequest &request);
};

#endif // NETWORKREQUESTFACTORY_H
