#include "DocxGenerator.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>

DocxGenerator::DocxGenerator(QObject *parent)
    : QObject(parent)
{
}

bool DocxGenerator::generatePaper(const QString &outputPath, const QString &paperTitle, const QList<PaperQuestion> &questions)
{
    emit generationStarted();

    if (questions.isEmpty()) {
        m_lastError = "试题列表为空";
        emit errorOccurred(m_lastError);
        emit generationFinished(false, "");
        return false;
    }

    qDebug() << "[DocxGenerator] Generating paper:" << paperTitle << "with" << questions.size() << "questions";

    // 创建临时目录
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        m_lastError = "无法创建临时目录";
        emit errorOccurred(m_lastError);
        emit generationFinished(false, "");
        return false;
    }

    QString basePath = tempDir.path();

    // 创建 DOCX 目录结构
    QDir().mkpath(basePath + "/_rels");
    QDir().mkpath(basePath + "/word/_rels");

    // 创建必要的 XML 文件
    if (!createContentTypes(basePath) ||
        !createRels(basePath) ||
        !createDocumentRels(basePath) ||
        !createStyles(basePath) ||
        !createSettings(basePath) ||
        !createDocument(basePath, paperTitle, questions)) {
        emit generationFinished(false, "");
        return false;
    }

    // 打包为 ZIP（DOCX）
    if (!packToZip(basePath, outputPath)) {
        emit generationFinished(false, "");
        return false;
    }

    qDebug() << "[DocxGenerator] DOCX generated successfully:" << outputPath;
    emit generationFinished(true, outputPath);
    return true;
}

bool DocxGenerator::createContentTypes(const QString &tempDir)
{
    QString content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>)";

    QFile file(tempDir + "/[Content_Types].xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 [Content_Types].xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

bool DocxGenerator::createRels(const QString &tempDir)
{
    QString content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>)";

    QFile file(tempDir + "/_rels/.rels");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 .rels";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

bool DocxGenerator::createDocumentRels(const QString &tempDir)
{
    QString content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>)";

    QFile file(tempDir + "/word/_rels/document.xml.rels");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 document.xml.rels";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

bool DocxGenerator::createStyles(const QString &tempDir)
{
    QString content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults>
<w:rPrDefault>
<w:rPr>
<w:rFonts w:ascii="宋体" w:eastAsia="宋体" w:hAnsi="宋体"/>
<w:sz w:val="24"/>
<w:szCs w:val="24"/>
<w:lang w:val="zh-CN" w:eastAsia="zh-CN"/>
</w:rPr>
</w:rPrDefault>
<w:pPrDefault>
<w:pPr>
<w:spacing w:after="200" w:line="276" w:lineRule="auto"/>
</w:pPr>
</w:pPrDefault>
</w:docDefaults>
<w:style w:type="paragraph" w:styleId="Title">
<w:name w:val="Title"/>
<w:pPr>
<w:jc w:val="center"/>
<w:spacing w:after="400"/>
</w:pPr>
<w:rPr>
<w:b/>
<w:sz w:val="44"/>
<w:szCs w:val="44"/>
</w:rPr>
</w:style>
<w:style w:type="paragraph" w:styleId="Heading1">
<w:name w:val="Heading 1"/>
<w:pPr>
<w:spacing w:before="400" w:after="200"/>
</w:pPr>
<w:rPr>
<w:b/>
<w:sz w:val="32"/>
<w:szCs w:val="32"/>
</w:rPr>
</w:style>
<w:style w:type="paragraph" w:styleId="Question">
<w:name w:val="Question"/>
<w:pPr>
<w:spacing w:before="200" w:after="100"/>
</w:pPr>
<w:rPr>
<w:sz w:val="24"/>
<w:szCs w:val="24"/>
</w:rPr>
</w:style>
<w:style w:type="paragraph" w:styleId="Option">
<w:name w:val="Option"/>
<w:pPr>
<w:ind w:left="480"/>
<w:spacing w:after="60"/>
</w:pPr>
<w:rPr>
<w:sz w:val="24"/>
<w:szCs w:val="24"/>
</w:rPr>
</w:style>
</w:styles>)";

    QFile file(tempDir + "/word/styles.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 styles.xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

bool DocxGenerator::createSettings(const QString &tempDir)
{
    QString content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:zoom w:percent="100"/>
<w:defaultTabStop w:val="720"/>
<w:characterSpacingControl w:val="doNotCompress"/>
</w:settings>)";

    QFile file(tempDir + "/word/settings.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 settings.xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

bool DocxGenerator::createDocument(const QString &tempDir, const QString &paperTitle, const QList<PaperQuestion> &questions)
{
    QString documentHeader = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:body>
)";

    QString documentFooter = R"(
<w:sectPr>
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>
</w:sectPr>
</w:body>
</w:document>)";

    // 构建文档内容
    QString bodyContent;

    // 标题
    bodyContent += QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Title"/></w:pPr>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(paperTitle));

    // 分组题目
    QMap<QString, QList<QPair<int, PaperQuestion>>> groupedQuestions;
    int globalIndex = 0;
    for (const PaperQuestion &q : questions) {
        globalIndex++;
        groupedQuestions[q.questionType].append(qMakePair(globalIndex, q));
    }

    // 题型顺序
    QStringList typeOrder = {"single_choice", "multi_choice", "true_false", "fill_blank", "short_answer", "essay", "material_essay"};
    // 题型显示名称
    QMap<QString, QString> typeNames = {
        {"single_choice", "选择题"},
        {"multi_choice", "多选题"},
        {"true_false", "判断题"},
        {"fill_blank", "填空题"},
        {"short_answer", "简答题"},
        {"essay", "论述题"},
        {"material_essay", "材料分析题"}
    };

    for (const QString &type : typeOrder) {
        if (!groupedQuestions.contains(type) || groupedQuestions[type].isEmpty()) {
            continue;
        }

        // 题型标题
        QString typeName = typeNames.value(type, type);
        bodyContent += QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Heading1"/></w:pPr>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(typeName));

        // 题目
        for (const auto &pair : groupedQuestions[type]) {
            bodyContent += generateQuestionXml(pair.second, pair.first);
        }
    }

    // 合并文档
    QString fullDocument = documentHeader + bodyContent + documentFooter;

    QFile file(tempDir + "/word/document.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 document.xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << fullDocument;
    file.close();
    return true;
}

QString DocxGenerator::generateQuestionXml(const PaperQuestion &question, int index)
{
    QString xml;

    // 题目内容
    QString questionText = QString("%1. %2").arg(index).arg(question.stem);
    xml += QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Question"/></w:pPr>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(questionText));

    // 选项（如果有）
    if (!question.options.isEmpty()) {
        xml += generateOptionsXml(question.options);
    }

    return xml;
}

QString DocxGenerator::generateOptionsXml(const QStringList &options)
{
    QString xml;
    QStringList labels = {"A", "B", "C", "D", "E", "F", "G", "H"};
    // 正则匹配已有的选项前缀（如 "A." "A、" "A:" 等）
    QRegularExpression prefixPattern("^[A-Ha-h][.、:：]\\s*");

    for (int i = 0; i < options.size() && i < labels.size(); ++i) {
        QString optionText = options[i];

        // 检查选项是否已经包含字母前缀，如果有则不再添加
        if (!prefixPattern.match(optionText).hasMatch()) {
            optionText = QString("%1. %2").arg(labels[i]).arg(optionText);
        }

        xml += QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Option"/></w:pPr>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(optionText));
    }

    return xml;
}

QString DocxGenerator::escapeXml(const QString &text)
{
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&apos;");
    return escaped;
}

bool DocxGenerator::packToZip(const QString &tempDir, const QString &outputPath)
{
    // 确保输出目录存在
    QFileInfo outputInfo(outputPath);
    QDir().mkpath(outputInfo.absolutePath());

    // 删除已存在的文件
    if (QFile::exists(outputPath)) {
        QFile::remove(outputPath);
    }

    // 使用 zip 命令打包
    QProcess zipProcess;
    zipProcess.setWorkingDirectory(tempDir);

    QStringList args;
    args << "-r" << outputPath << ".";

    zipProcess.start("zip", args);
    if (!zipProcess.waitForFinished(30000)) {
        m_lastError = "ZIP 打包超时";
        emit errorOccurred(m_lastError);
        return false;
    }

    if (zipProcess.exitCode() != 0) {
        m_lastError = QString("ZIP 打包失败: %1").arg(QString::fromUtf8(zipProcess.readAllStandardError()));
        emit errorOccurred(m_lastError);
        return false;
    }

    return true;
}
