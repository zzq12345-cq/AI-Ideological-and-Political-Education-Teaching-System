#ifndef ZHIPUPPTAGENTSERVICE_H
#define ZHIPUPPTAGENTSERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QImage>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief 智谱 PPT Agent 服务 — 基于 GLM 大模型的三阶段 PPT 生成流水线
 *
 * 完整工作流（参考 Linux.do PPT Agent 思路）：
 *   阶段1: 大纲生成（glm-5.1） — 根据主题生成 JSON 大纲
 *   阶段2: 策划稿（glm-5.1）   — 为每页生成版面规划
 *   阶段3: SVG 设计（glm-4.6）  — 为每页生成 1280×720 SVG 代码
 *
 * SVG 代码会通过 QSvgRenderer 渲染为 QImage 用于预览。
 */
class ZhipuPPTAgentService : public QObject
{
    Q_OBJECT

public:
    /// Agent 运行状态
    enum class State {
        Idle,
        GeneratingOutline,    // 阶段1: 大纲
        GeneratingPlan,       // 阶段2: 策划稿
        GeneratingSVG,        // 阶段3: SVG 设计
        Finished,
        Failed
    };
    Q_ENUM(State)

    explicit ZhipuPPTAgentService(QObject *parent = nullptr);
    ~ZhipuPPTAgentService();

    /// 设置 API Key
    void setApiKey(const QString &apiKey);

    /// 设置 API 基础 URL（默认 https://open.bigmodel.cn/api/coding/paas/v4）
    void setBaseUrl(const QString &baseUrl);

    /**
     * @brief 启动 PPT 生成
     * @param params 包含 textbook/grade/chapter/template/duration/contentFocus 等
     */
    void generate(const QMap<QString, QString> &params);

    /// 取消正在进行的生成
    void cancel();

    /// 获取当前状态
    State currentState() const { return m_state; }

    /// 获取已生成的 SVG 代码列表
    QStringList svgCodes() const { return m_svgCodes; }

    /// 获取已生成的预览图列表
    QVector<QImage> previewImages() const { return m_previewImages; }

signals:
    /// 进度更新（percent 0-100, stage 阶段描述, detail 详细信息）
    void progressUpdated(int percent, const QString &stage, const QString &detail);

    /// 单页 SVG 生成完成
    void slideGenerated(int index, const QString &svgCode, const QImage &preview);

    /// 全部生成完成
    void allSlidesGenerated(const QStringList &svgCodes, const QVector<QImage> &previews);

    /// 大纲生成完成
    void outlineGenerated(const QJsonObject &outline);

    /// 发生错误
    void errorOccurred(const QString &error);

    /// 状态变更
    void stateChanged(ZhipuPPTAgentService::State newState);

private slots:
    void onOutlineReplyFinished();
    void onPlanReplyFinished();
    void onSvgReplyFinished();

private:
    // 构建主题描述文本
    QString buildTopicDescription(const QMap<QString, QString> &params) const;
    int extractRequestedPageCount(const QString &request) const;

    // 阶段1: 发送大纲生成请求
    void startOutlineGeneration(const QString &topic);

    // 阶段2: 发送策划稿请求
    void startPlanGeneration();

    // 阶段3: 发送 SVG 设计请求（逐页）
    void startSvgGeneration();
    void generateNextSvg();

    // 构建单页内容描述（供 SVG 生成使用）
    QString buildPageContent(int pageIndex) const;

    // 统一的 API 调用方法
    QNetworkReply* callZhipuApi(const QString &model,
                                 const QString &systemPrompt,
                                 const QString &userMessage,
                                 double temperature = 0.7,
                                 int maxTokens = 8192);

    // 创建已配置的网络请求
    QNetworkRequest createRequest() const;

    // 解析大纲 JSON
    QJsonObject parseOutlineJson(const QString &response) const;

    // 从 API 响应中提取文本内容
    QString extractContent(const QByteArray &responseData) const;

    // SVG 渲染为 QImage
    QImage renderSvgToImage(const QString &svgCode, int width = 1280, int height = 720) const;

    // 提取 SVG 代码（从 AI 回复中提取 <svg>...</svg>）
    QString extractSvgCode(const QString &response) const;

    // 状态管理
    void setState(State newState);

    // 成员变量
    QNetworkAccessManager *m_networkManager;
    QString m_apiKey;
    QString m_baseUrl;
    State m_state = State::Idle;
    bool m_cancelled = false;

    // 模型名称
    static constexpr const char* MODEL_TEXT = "glm-5.1";   // 文本生成
    static constexpr const char* MODEL_CODE = "glm-4.6";   // SVG 代码生成

    // 生成过程中的数据
    QString m_topic;               // 用户主题
    QMap<QString, QString> m_params; // 原始参数
    QJsonObject m_outline;         // 阶段1 大纲
    QStringList m_pagePlans;       // 阶段2 策划稿（每页一段描述）
    QStringList m_svgCodes;        // 阶段3 SVG 代码
    QVector<QImage> m_previewImages; // 预览图
    int m_currentSvgIndex = 0;     // 当前正在生成的 SVG 页索引
    int m_totalPages = 0;          // 总页数
    QPointer<QNetworkReply> m_currentReply;

    // Prompts
    static QString outlineSystemPrompt();
    static QString planSystemPrompt();
    static QString svgSystemPrompt();
};

#endif // ZHIPUPPTAGENTSERVICE_H
