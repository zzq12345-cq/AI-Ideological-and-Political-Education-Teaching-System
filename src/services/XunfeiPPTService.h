#ifndef XUNFEIPPTSERVICE_H
#define XUNFEIPPTSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QTimer>

/**
 * @brief 讯飞智文 PPT 生成服务
 * 
 * 调用讯飞开放平台的智能 PPT 生成 API
 * 支持根据文本主题自动生成精美 PPT
 */
class XunfeiPPTService : public QObject
{
    Q_OBJECT

public:
    explicit XunfeiPPTService(QObject *parent = nullptr);
    ~XunfeiPPTService();

    /**
     * @brief 设置 API 密钥
     * @param appId 应用 ID
     * @param apiSecret API 密钥
     */
    void setCredentials(const QString &appId, const QString &apiSecret);

    /**
     * @brief 生成 PPT
     * @param query 用户的 PPT 生成需求描述（最大 8000 字）
     * @param author 作者名（可选）
     * @param themeId 主题 ID（可选，-1 表示默认）
     */
    void generatePPT(const QString &query, const QString &author = "", int themeId = -1);

    /**
     * @brief 获取可用主题列表
     */
    void fetchThemes();

    /**
     * @brief 取消当前生成任务
     */
    void cancel();

signals:
    /**
     * @brief 生成开始
     */
    void generationStarted();

    /**
     * @brief 生成进度更新
     * @param progress 进度百分比 (0-100)
     * @param message 进度消息
     */
    void progressUpdated(int progress, const QString &message);

    /**
     * @brief 生成完成
     * @param pptUrl PPT 下载链接
     * @param coverUrl 封面图片链接
     */
    void generationFinished(const QString &pptUrl, const QString &coverUrl);

    /**
     * @brief 生成失败
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

    /**
     * @brief 主题列表获取完成
     * @param themes 主题列表 [{id, name, coverUrl}...]
     */
    void themesReceived(const QJsonArray &themes);

private slots:
    void onCreateReply();
    void onProgressReply();
    void checkProgress();

private:
    /**
     * @brief 生成请求签名
     * @param timestamp 时间戳
     * @return 签名字符串
     */
    QString generateSignature(qint64 timestamp);

    /**
     * @brief 创建请求头
     * @return 包含认证信息的请求头
     */
    QMap<QString, QString> createHeaders();

    QString m_appId;
    QString m_apiSecret;
    QString m_baseUrl;
    QString m_currentSid;  // 当前任务 ID

    QNetworkAccessManager *m_networkManager;
    QTimer *m_progressTimer;
    bool m_cancelled;
    int m_progressErrorCount = 0;
    int m_progressMissingUrlCount = 0;
    int m_maxProgressRetries = 3;
};

#endif // XUNFEIPPTSERVICE_H
