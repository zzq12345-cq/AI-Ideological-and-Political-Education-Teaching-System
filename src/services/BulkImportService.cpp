#include "BulkImportService.h"
#include "DocumentReaderService.h"
#include "QuestionParserService.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

BulkImportService::BulkImportService(PaperService *paperService, QObject *parent)
    : QObject(parent)
    , m_paperService(paperService)
    , m_documentReader(new DocumentReaderService(this))
    , m_questionParser(new QuestionParserService(this))
    , m_isImporting(false)
    , m_stopRequested(false)
    , m_totalFiles(0)
    , m_processedFiles(0)
    , m_totalQuestions(0)
    , m_failedFiles(0)
{
    // 连接解析服务信号
    connect(m_questionParser, &QuestionParserService::parseCompleted,
            this, &BulkImportService::onParseCompleted);
    connect(m_questionParser, &QuestionParserService::errorOccurred,
            this, &BulkImportService::onParseError);
    connect(m_questionParser, &QuestionParserService::parseProgress,
            this, [this](const QString &text) {
                emit documentParseProgress(m_currentFileName, text);
            });
}

BulkImportService::~BulkImportService()
{
}

void BulkImportService::setParserApiKey(const QString &apiKey)
{
    m_questionParser->setApiKey(apiKey);
}

bool BulkImportService::isImporting() const
{
    return m_isImporting;
}

void BulkImportService::stopImport()
{
    m_stopRequested = true;
}

void BulkImportService::importFromJSON(const QString &filePath)
{
    qDebug() << "BulkImportService: 开始导入 JSON" << filePath;
    
    QList<PaperQuestion> questions = parseJSONFile(filePath);
    
    if (questions.isEmpty()) {
        emit importError("未找到有效题目");
        return;
    }
    
    emit importStarted(questions.size());
    
    // 连接进度信号
    connect(m_paperService, &PaperService::questionsAdded, this, [this, questions](int count) {
        qDebug() << "BulkImportService: 成功导入" << count << "题";
        emit importCompleted(count, questions.size() - count);
    }, Qt::SingleShotConnection);
    
    connect(m_paperService, &PaperService::questionError, this, [this](const QString &, const QString &error) {
        qDebug() << "BulkImportService: 导入失败" << error;
        emit importError(error);
    }, Qt::SingleShotConnection);
    
    // 批量导入
    m_paperService->addQuestions(questions);
}

void BulkImportService::importFromDocument(const QString &filePath,
                                           const QString &subject,
                                           const QString &grade)
{
    if (m_isImporting) {
        emit importError("正在导入中，请等待完成");
        return;
    }
    
    if (!m_questionParser->isConfigured()) {
        emit importError("未配置 AI 解析服务的 API Key");
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        emit importError(QString("文件不存在: %1").arg(filePath));
        return;
    }
    
    if (!m_documentReader->isSupportedFormat(filePath)) {
        emit importError(QString("不支持的文件格式: %1").arg(fileInfo.suffix()));
        return;
    }
    
    m_isImporting = true;
    m_stopRequested = false;
    m_pendingFiles.clear();
    m_pendingFiles.append(filePath);
    m_totalFiles = 1;
    m_processedFiles = 0;
    m_totalQuestions = 0;
    m_failedFiles = 0;
    m_currentSubject = subject;
    m_currentGrade = grade;
    
    emit importStarted(1);
    
    processNextFile();
}

void BulkImportService::importFromDirectory(const QString &dirPath,
                                            const QString &subject,
                                            const QString &grade)
{
    if (m_isImporting) {
        emit importError("正在导入中，请等待完成");
        return;
    }
    
    if (!m_questionParser->isConfigured()) {
        emit importError("未配置 AI 解析服务的 API Key");
        return;
    }
    
    QDir dir(dirPath);
    if (!dir.exists()) {
        emit importError(QString("目录不存在: %1").arg(dirPath));
        return;
    }
    
    // 收集所有支持的文件
    m_pendingFiles.clear();
    QStringList filters;
    filters << "*.docx";  // 目前只支持 DOCX
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo &fi : files) {
        m_pendingFiles.append(fi.absoluteFilePath());
    }
    
    if (m_pendingFiles.isEmpty()) {
        emit importError("目录中没有找到支持的文档文件");
        return;
    }
    
    qDebug() << "BulkImportService: 找到" << m_pendingFiles.size() << "个文档文件";
    
    m_isImporting = true;
    m_stopRequested = false;
    m_totalFiles = m_pendingFiles.size();
    m_processedFiles = 0;
    m_totalQuestions = 0;
    m_failedFiles = 0;
    m_currentSubject = subject;
    m_currentGrade = grade;
    
    emit importStarted(m_totalFiles);
    
    processNextFile();
}

void BulkImportService::processNextFile()
{
    if (m_stopRequested || m_pendingFiles.isEmpty()) {
        m_isImporting = false;
        emit importCompleted(m_totalQuestions, m_failedFiles);
        return;
    }
    
    QString filePath = m_pendingFiles.takeFirst();
    QFileInfo fileInfo(filePath);
    m_currentFileName = fileInfo.fileName();
    
    qDebug() << "BulkImportService: 处理文件" << m_currentFileName
             << "(" << m_processedFiles + 1 << "/" << m_totalFiles << ")";
    
    emit documentParseStarted(m_currentFileName);
    
    // 直接上传文件到 Dify 进行解析（不再本地提取文本）
    // 这样可以支持文档中的图片识别
    m_questionParser->parseFile(filePath, m_currentSubject, m_currentGrade);
}

void BulkImportService::onParseCompleted(const QList<PaperQuestion> &questions)
{
    qDebug() << "BulkImportService: 解析完成，获得" << questions.size() << "道题目";
    
    if (questions.isEmpty()) {
        m_failedFiles++;
    } else {
        // 添加到数据库
        m_paperService->addQuestions(questions);
        m_totalQuestions += questions.size();
    }
    
    emit documentParseCompleted(m_currentFileName, questions.size());
    
    m_processedFiles++;
    emit importProgress(m_processedFiles, m_totalFiles);
    
    // 继续处理下一个
    processNextFile();
}

void BulkImportService::onParseError(const QString &error)
{
    qDebug() << "BulkImportService: 解析错误" << error;
    
    m_failedFiles++;
    m_processedFiles++;
    emit importProgress(m_processedFiles, m_totalFiles);
    
    // 继续处理下一个
    processNextFile();
}

QList<PaperQuestion> BulkImportService::parseJSONFile(const QString &filePath)
{
    QList<PaperQuestion> questions;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "BulkImportService: 无法打开文件" << filePath;
        emit importError("无法打开文件: " + filePath);
        return questions;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "BulkImportService: JSON 解析错误" << parseError.errorString();
        emit importError("JSON 解析错误: " + parseError.errorString());
        return questions;
    }
    
    if (!doc.isObject()) {
        emit importError("JSON 格式错误：根节点应为对象");
        return questions;
    }
    
    QJsonObject root = doc.object();
    if (!root.contains("questions") || !root["questions"].isArray()) {
        emit importError("JSON 格式错误：缺少 questions 数组");
        return questions;
    }
    
    QJsonArray questionsArray = root["questions"].toArray();
    
    for (const QJsonValue &val : questionsArray) {
        if (!val.isObject()) continue;
        
        PaperQuestion q = PaperQuestion::fromJson(val.toObject());
        
        // 确保公共题目 paper_id 为空
        if (q.visibility == "public") {
            q.paperId = "";
        }
        
        questions.append(q);
    }
    
    qDebug() << "BulkImportService: 成功解析" << questions.size() << "道题目";
    
    return questions;
}
