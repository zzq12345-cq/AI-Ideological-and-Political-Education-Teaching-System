#include "XunfeiPPTService.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QDateTime>
#include <QDebug>

XunfeiPPTService::XunfeiPPTService(QObject *parent)
    : QObject(parent)
    , m_baseUrl("https://zwapi.xfyun.cn")
    , m_networkManager(new QNetworkAccessManager(this))
    , m_progressTimer(new QTimer(this))
    , m_cancelled(false)
{
    // 默认使用环境变量中的凭证
    m_appId = qEnvironmentVariable("XUNFEI_APP_ID", "dbf537bf");
    m_apiSecret = qEnvironmentVariable("XUNFEI_API_SECRET", "Y2QyMWU3OTBlNzMyODE1Mjc5MzI5YzU5");
    
    // 进度轮询定时器
    m_progressTimer->setInterval(3000);  // 每 3 秒查询一次进度
    connect(m_progressTimer, &QTimer::timeout, this, &XunfeiPPTService::checkProgress);
    
    qDebug() << "[XunfeiPPTService] Initialized with appId:" << m_appId;
}

XunfeiPPTService::~XunfeiPPTService()
{
    cancel();
}

void XunfeiPPTService::setCredentials(const QString &appId, const QString &apiSecret)
{
    m_appId = appId;
    m_apiSecret = apiSecret;
    qDebug() << "[XunfeiPPTService] Credentials updated";
}

QString XunfeiPPTService::generateSignature(qint64 timestamp)
{
    // 签名算法：
    // 1. 将 appId 和 timestamp 拼接后进行 MD5 加密
    // 2. 使用 apiSecret 对 MD5 结果进行 HMAC-SHA1 加密
    // 3. 结果进行 Base64 编码
    
    QString toSign = m_appId + QString::number(timestamp);
    QByteArray md5Hash = QCryptographicHash::hash(toSign.toUtf8(), QCryptographicHash::Md5).toHex();
    
    QMessageAuthenticationCode hmac(QCryptographicHash::Sha1);
    hmac.setKey(m_apiSecret.toUtf8());
    hmac.addData(md5Hash);
    
    return hmac.result().toBase64();
}

QMap<QString, QString> XunfeiPPTService::createHeaders()
{
    qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    QString signature = generateSignature(timestamp);
    
    QMap<QString, QString> headers;
    headers["appId"] = m_appId;
    headers["timestamp"] = QString::number(timestamp);
    headers["signature"] = signature;
    headers["Content-Type"] = "application/json";
    
    return headers;
}

void XunfeiPPTService::generatePPT(const QString &query, const QString &author, int themeId)
{
    if (query.isEmpty()) {
        emit errorOccurred("生成请求不能为空");
        return;
    }
    
    m_cancelled = false;
    m_progressErrorCount = 0;
    m_progressMissingUrlCount = 0;
    m_progressTimer->stop();
    emit generationStarted();
    emit progressUpdated(0, "正在创建 PPT 生成任务...");
    
    // 构建请求
    QUrl url(m_baseUrl + "/api/aippt/create");
    QNetworkRequest request(url);
    
    // 设置请求头
    QMap<QString, QString> headers = createHeaders();
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    // 构建请求体
    QJsonObject body;
    body["query"] = query;
    if (!author.isEmpty()) {
        body["author"] = author;
    }
    if (themeId > 0) {
        body["theme"] = themeId;
    }
    body["isCardNote"] = true;  // 生成演讲备注
    
    QJsonDocument doc(body);
    QByteArray data = doc.toJson();
    
    qDebug() << "[XunfeiPPTService] Creating PPT with query:" << query.left(50) << "...";
    qDebug() << "[XunfeiPPTService] Request URL:" << url.toString();
    
    QNetworkReply *reply = m_networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &XunfeiPPTService::onCreateReply);
}

void XunfeiPPTService::onCreateReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (m_cancelled) {
        return;
    }
    
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("网络错误: %1").arg(reply->errorString()));
        return;
    }
    
    QByteArray responseData = reply->readAll();
    qDebug() << "[XunfeiPPTService] Create response:" << responseData;
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject obj = doc.object();
    
    int code = obj["code"].toInt();
    if (code != 0) {
        QString desc = obj["desc"].toString();
        emit errorOccurred(QString("创建失败: %1 (code: %2)").arg(desc).arg(code));
        return;
    }
    
    // 获取任务 ID
    QJsonObject data = obj["data"].toObject();
    m_currentSid = data["sid"].toString();
    
    if (m_currentSid.isEmpty()) {
        emit errorOccurred("未获取到任务 ID");
        return;
    }
    
    qDebug() << "[XunfeiPPTService] Task created, sid:" << m_currentSid;
    emit progressUpdated(10, "PPT 生成任务已创建，正在生成中...");
    
    // 开始轮询进度
    m_progressTimer->start();
}

void XunfeiPPTService::checkProgress()
{
    if (m_currentSid.isEmpty() || m_cancelled) {
        m_progressTimer->stop();
        return;
    }
    
    // 构建进度查询请求
    QUrl url(m_baseUrl + "/api/aippt/progress");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("sid", m_currentSid);
    url.setQuery(urlQuery);
    
    QNetworkRequest request(url);
    
    // 设置请求头
    QMap<QString, QString> headers = createHeaders();
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    QNetworkReply *reply = m_networkManager->get(request);
    if (reply) {
        reply->setProperty("sid", m_currentSid);
    }
    connect(reply, &QNetworkReply::finished, this, &XunfeiPPTService::onProgressReply);
}

void XunfeiPPTService::onProgressReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();

    if (m_cancelled) {
        m_progressTimer->stop();
        return;
    }

    const QString replySid = reply->property("sid").toString();
    if (!replySid.isEmpty() && replySid != m_currentSid) {
        qDebug() << "[XunfeiPPTService] 忽略旧任务进度回复:" << replySid;
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[XunfeiPPTService] Progress query error:" << reply->errorString();
        m_progressErrorCount++;
        if (m_progressErrorCount >= m_maxProgressRetries) {
            m_progressTimer->stop();
            emit errorOccurred(QString("进度查询失败: %1").arg(reply->errorString()));
        }
        return;
    }
    
    QByteArray responseData = reply->readAll();
    qDebug() << "[XunfeiPPTService] Progress response:" << responseData;
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject obj = doc.object();
    
    int code = obj["code"].toInt();
    if (code != 0) {
        QString desc = obj["desc"].toString();
        m_progressErrorCount++;
        if (m_progressErrorCount >= m_maxProgressRetries) {
            m_progressTimer->stop();
            emit errorOccurred(QString("进度查询失败: %1").arg(desc));
        }
        return;
    }
    
    QJsonObject data = obj["data"].toObject();
    int process = data["process"].toInt();
    QString pptUrl = data["pptUrl"].toString();
    QString coverUrl = data["coverUrl"].toString();
    
    emit progressUpdated(process, QString("正在生成 PPT... %1%").arg(process));
    
    // 检查是否完成
    if (process >= 100) {
        if (!pptUrl.isEmpty()) {
            m_progressTimer->stop();
            qDebug() << "[XunfeiPPTService] PPT generated:" << pptUrl;
            emit generationFinished(pptUrl, coverUrl);
        } else {
            m_progressMissingUrlCount++;
            if (m_progressMissingUrlCount >= m_maxProgressRetries) {
                m_progressTimer->stop();
                emit errorOccurred("生成完成但未返回 PPT 链接");
            }
        }
    }
}

void XunfeiPPTService::fetchThemes()
{
    QUrl url(m_baseUrl + "/api/aippt/themeList");
    QNetworkRequest request(url);
    
    QMap<QString, QString> headers = createHeaders();
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[XunfeiPPTService] Theme fetch error:" << reply->errorString();
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        
        if (obj["code"].toInt() == 0) {
            QJsonArray themes = obj["data"].toObject()["themeList"].toArray();
            emit themesReceived(themes);
        }
    });
}

void XunfeiPPTService::cancel()
{
    m_cancelled = true;
    m_progressTimer->stop();
    m_currentSid.clear();
}
