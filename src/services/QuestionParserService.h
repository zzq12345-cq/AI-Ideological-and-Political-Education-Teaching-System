#ifndef QUESTIONPARSERSERVICE_H
#define QUESTIONPARSERSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include "PaperService.h"

/**
 * @brief 试题解析服务
 * 
 * 使用 Dify 工作流 API 将文档解析为结构化的试题数据
 * 支持两种模式：
 * 1. 文本模式：传递已提取的文本
 * 2. 文件模式：上传原始文件，使用 Dify 文档提取器
 */
class QuestionParserService : public QObject
{
    Q_OBJECT

public:
    explicit QuestionParserService(QObject *parent = nullptr);
    ~QuestionParserService();
    
    /**
     * @brief 设置 Dify 工作流的 API Key
     */
    void setApiKey(const QString &apiKey);
    
    /**
     * @brief 设置 Dify API 基础 URL
     */
    void setBaseUrl(const QString &baseUrl);
    
    /**
     * @brief 解析文档文本为试题列表（旧方法，保持兼容）
     * @param documentText 文档的纯文本内容
     * @param subject 学科（可选）
     * @param grade 年级（可选）
     */
    void parseDocument(const QString &documentText, 
                      const QString &subject = QString(),
                      const QString &grade = QString());
    
    /**
     * @brief 上传文件并解析为试题列表（新方法）
     * @param filePath 文件路径（DOCX、PDF 等）
     * @param subject 学科（可选）
     * @param grade 年级（可选）
     */
    void parseFile(const QString &filePath,
                   const QString &subject = QString(),
                   const QString &grade = QString());
    
    /**
     * @brief 获取最后一次操作的错误信息
     */
    QString lastError() const;
    
    /**
     * @brief 检查服务是否已配置
     */
    bool isConfigured() const;

signals:
    /**
     * @brief 解析开始
     */
    void parseStarted();
    
    /**
     * @brief 文件上传进度
     */
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    
    /**
     * @brief 解析完成
     * @param questions 解析出的试题列表
     */
    void parseCompleted(const QList<PaperQuestion> &questions);
    
    /**
     * @brief 解析进度（流式响应中）
     * @param text 当前接收到的文本
     */
    void parseProgress(const QString &text);
    
    /**
     * @brief 发生错误
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();
    void onReadyRead();
    void onUploadFinished();
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
    /**
     * @brief 上传文件到 Dify
     */
    void uploadFileToDify(const QString &filePath);
    
    /**
     * @brief 调用工作流（使用已上传的文件）
     */
    void callWorkflowWithFile(const QString &uploadFileId);
    
    /**
     * @brief 解析 Dify 返回的 JSON 结果
     */
    QList<PaperQuestion> parseJsonResponse(const QString &jsonText);
    
    /**
     * @brief 解析 SSE 流式响应
     */
    void parseStreamResponse(const QByteArray &data);
    
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply;
    QNetworkReply *m_uploadReply;
    QString m_apiKey;
    QString m_baseUrl;
    QString m_lastError;
    QString m_fullResponse;
    QString m_streamBuffer;
    
    // 当前解析的元数据
    QString m_currentSubject;
    QString m_currentGrade;
    
    // 文件上传相关
    QString m_currentFilePath;
    QString m_uploadedFileId;
    QFile *m_uploadFile;
};

#endif // QUESTIONPARSERSERVICE_H
