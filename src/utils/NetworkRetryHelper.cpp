#include "NetworkRetryHelper.h"
#include <QDebug>
#include <QtMath>

NetworkRetryHelper::NetworkRetryHelper(QNetworkAccessManager *manager,
                                       const RetryPolicy &policy,
                                       QObject *parent)
    : QObject(parent)
    , m_manager(manager)
    , m_policy(policy)
{
}

void NetworkRetryHelper::sendRequest(const QNetworkRequest &request,
                                     const QByteArray &data,
                                     const QString &method)
{
    m_pendingRequest = request;
    m_pendingData = data;
    m_pendingMethod = method;
    m_currentAttempt = 0;

    attemptRequest();
}

void NetworkRetryHelper::attemptRequest()
{
    m_currentAttempt++;

    QNetworkReply *reply = nullptr;

    if (m_pendingMethod == "GET") {
        reply = m_manager->get(m_pendingRequest);
    } else if (m_pendingMethod == "POST") {
        reply = m_manager->post(m_pendingRequest, m_pendingData);
    } else if (m_pendingMethod == "PATCH") {
        reply = m_manager->sendCustomRequest(m_pendingRequest, "PATCH", m_pendingData);
    } else if (m_pendingMethod == "DELETE") {
        reply = m_manager->sendCustomRequest(m_pendingRequest, "DELETE");
    } else if (m_pendingMethod == "PUT") {
        reply = m_manager->put(m_pendingRequest, m_pendingData);
    }

    if (!reply) {
        qWarning() << "[NetworkRetryHelper] 无法创建网络请求";
        return;
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
}

void NetworkRetryHelper::onReplyFinished(QNetworkReply *reply)
{
    if (!reply) return;

    if (shouldRetry(reply)) {
        int delay = currentDelay();
        qDebug() << QString("[NetworkRetryHelper] 第 %1/%2 次重试，%3ms 后执行")
                        .arg(m_currentAttempt)
                        .arg(m_policy.maxRetries)
                        .arg(delay);

        emit retrying(m_currentAttempt, m_policy.maxRetries);

        reply->deleteLater();

        QTimer::singleShot(delay, this, &NetworkRetryHelper::attemptRequest);
        return;
    }

    // 最终结果（成功或耗尽重试）
    emit finished(reply);
}

bool NetworkRetryHelper::shouldRetry(QNetworkReply *reply) const
{
    if (m_currentAttempt >= m_policy.maxRetries) {
        return false;
    }

    // 网络层错误：可重试
    QNetworkReply::NetworkError error = reply->error();
    if (error == QNetworkReply::TimeoutError ||
        error == QNetworkReply::ConnectionRefusedError ||
        error == QNetworkReply::RemoteHostClosedError ||
        error == QNetworkReply::TemporaryNetworkFailureError ||
        error == QNetworkReply::NetworkSessionFailedError) {
        return true;
    }

    // HTTP 状态码：仅重试服务端错误和超时
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (m_policy.retryableHttpCodes.contains(httpStatus)) {
        return true;
    }

    // 4xx 客户端错误、SSL 错误等不重试
    return false;
}

int NetworkRetryHelper::currentDelay() const
{
    // delay = baseDelay * (backoffMultiplier ^ (attempt - 1))
    // 例如: 1000 * 2^0 = 1s, 1000 * 2^1 = 2s, 1000 * 2^2 = 4s
    return static_cast<int>(m_policy.baseDelayMs * qPow(m_policy.backoffMultiplier, m_currentAttempt - 1));
}
