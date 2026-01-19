#ifndef DOCXGENERATOR_H
#define DOCXGENERATOR_H

#include <QObject>
#include <QString>
#include <QList>
#include "PaperService.h"

/**
 * @brief DOCX 文件生成器
 *
 * 生成真实的 .docx 文件（Office Open XML 格式）
 * .docx 本质上是一个 ZIP 文件，包含多个 XML 文件
 */
class DocxGenerator : public QObject
{
    Q_OBJECT

public:
    explicit DocxGenerator(QObject *parent = nullptr);

    /**
     * @brief 生成试卷 DOCX 文件
     * @param outputPath 输出文件路径
     * @param paperTitle 试卷标题
     * @param questions 试题列表
     * @return 成功返回 true
     */
    bool generatePaper(const QString &outputPath, const QString &paperTitle, const QList<PaperQuestion> &questions);

    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const { return m_lastError; }

signals:
    void generationStarted();
    void generationFinished(bool success, const QString &filePath);
    void errorOccurred(const QString &error);

private:
    // 创建 DOCX 所需的 XML 文件
    bool createContentTypes(const QString &tempDir);
    bool createRels(const QString &tempDir);
    bool createDocumentRels(const QString &tempDir);
    bool createDocument(const QString &tempDir, const QString &paperTitle, const QList<PaperQuestion> &questions);
    bool createStyles(const QString &tempDir);
    bool createSettings(const QString &tempDir);

    // 生成试题内容 XML
    QString generateQuestionXml(const PaperQuestion &question, int index);
    QString generateOptionsXml(const QStringList &options);
    QString escapeXml(const QString &text);

    // 打包为 ZIP
    bool packToZip(const QString &tempDir, const QString &outputPath);

    QString m_lastError;
};

#endif // DOCXGENERATOR_H
