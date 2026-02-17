#ifndef NETWORKRETRYHELPER_H
#define NETWORKRETRYHELPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QTimer>

/**
 * @brief 轻量级网络重试封装
 *
 * 以组合方式嵌入现有服务，不改变信号/槽架构。
 * 支持指数退避策略和可配置重试条件。
 */
class NetworkRetryHelper : public QObject
{
    Q_OBJECT

public:
    struct RetryPolicy {
        int maxRetries;
        int baseDelayMs;                  // 首次重试延迟
        double backoffMultiplier;         // 退避倍数
        QSet<int> retryableHttpCodes;

        RetryPolicy()
            : maxRetries(3), baseDelayMs(1000), backoffMultiplier(2.0)
            , retryableHttpCodes({500, 502, 503, 504, 408}) {}
        RetryPolicy(int retries, int delay, double multiplier, QSet<int> codes)
            : maxRetries(retries), baseDelayMs(delay)
            , backoffMultiplier(multiplier), retryableHttpCodes(std::move(codes)) {}
    };

    explicit NetworkRetryHelper(QNetworkAccessManager *manager,
                                const RetryPolicy &policy = RetryPolicy(),
                                QObject *parent = nullptr);

    // 发起可重试请求
    void sendRequest(const QNetworkRequest &request,
                     const QByteArray &data,
                     const QString &method);

signals:
    void finished(QNetworkReply *reply);          // 最终结果（成功或耗尽重试）
    void retrying(int attempt, int maxRetries);   // 正在重试通知

private:
    void attemptRequest();
    void onReplyFinished(QNetworkReply *reply);
    bool shouldRetry(QNetworkReply *reply) const;
    int currentDelay() const;

    QNetworkAccessManager *m_manager;
    RetryPolicy m_policy;

    // 当前请求上下文
    QNetworkRequest m_pendingRequest;
    QByteArray m_pendingData;
    QString m_pendingMethod;
    int m_currentAttempt = 0;
};

#endif // NETWORKRETRYHELPER_H
