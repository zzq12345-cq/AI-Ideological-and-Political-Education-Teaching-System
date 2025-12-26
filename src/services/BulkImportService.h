#ifndef BULKIMPORTSERVICE_H
#define BULKIMPORTSERVICE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include "PaperService.h"

class DocumentReaderService;
class QuestionParserService;

/**
 * @brief 批量导入服务
 * 
 * 支持从 JSON 文件、DOCX 文档、目录批量导入试题到公共题库
 */
class BulkImportService : public QObject
{
    Q_OBJECT

public:
    explicit BulkImportService(PaperService *paperService, QObject *parent = nullptr);
    ~BulkImportService();
    
    /**
     * @brief 设置 Dify 试题解析工作流的 API Key
     */
    void setParserApiKey(const QString &apiKey);
    
    /**
     * @brief 从 JSON 文件批量导入公共题目
     */
    void importFromJSON(const QString &filePath);
    
    /**
     * @brief 从 DOCX 文档导入试题（使用 AI 解析）
     * @param filePath DOCX 文件路径
     * @param subject 学科（可选）
     * @param grade 年级（可选）
     */
    void importFromDocument(const QString &filePath, 
                           const QString &subject = QString(),
                           const QString &grade = QString());
    
    /**
     * @brief 从目录批量导入所有支持的文档
     * @param dirPath 目录路径
     * @param subject 学科（可选）
     * @param grade 年级（可选）
     */
    void importFromDirectory(const QString &dirPath,
                            const QString &subject = QString(),
                            const QString &grade = QString());
    
    /**
     * @brief 停止当前导入
     */
    void stopImport();
    
    /**
     * @brief 是否正在导入
     */
    bool isImporting() const;
    
signals:
    void importStarted(int totalCount);
    void importProgress(int current, int total);
    void importCompleted(int successCount, int failCount);
    void importError(const QString &error);
    
    // 文档解析相关信号
    void documentParseStarted(const QString &fileName);
    void documentParseProgress(const QString &fileName, const QString &text);
    void documentParseCompleted(const QString &fileName, int questionCount);
    
private slots:
    void onParseCompleted(const QList<PaperQuestion> &questions);
    void onParseError(const QString &error);
    
private:
    PaperService *m_paperService;
    DocumentReaderService *m_documentReader;
    QuestionParserService *m_questionParser;
    
    // 导入状态
    bool m_isImporting;
    bool m_stopRequested;
    QStringList m_pendingFiles;
    int m_totalFiles;
    int m_processedFiles;
    int m_totalQuestions;
    int m_failedFiles;
    
    // 当前处理的元数据
    QString m_currentSubject;
    QString m_currentGrade;
    QString m_currentFileName;
    
    QList<PaperQuestion> parseJSONFile(const QString &filePath);
    void processNextFile();
};

#endif // BULKIMPORTSERVICE_H
