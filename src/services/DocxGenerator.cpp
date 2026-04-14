#include "DocxGenerator.h"
#include "../utils/SimpleZipWriter.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QDebug>
#include <QRegularExpression>

namespace {
struct MarkdownQuestionBlock {
    QString sectionTitle;
    QString number;
    QStringList answerLines;
    QStringList analysisLines;

    bool hasAnswerContent() const
    {
        return !answerLines.isEmpty() || !analysisLines.isEmpty();
    }
};

QString stripInlineMarkdown(QString text)
{
    text.remove(QRegularExpression(R"(\*{1,2})"));
    return text.trimmed();
}

QString normalizedSemanticLine(QString text)
{
    text = text.trimmed();
    text.remove(QRegularExpression(R"(^#{1,4}\s*)"));
    text = stripInlineMarkdown(text);
    if (text.startsWith(QStringLiteral("【")) && text.endsWith(QStringLiteral("】")) && text.size() >= 2) {
        text = text.mid(1, text.size() - 2).trimmed();
    }
    return text;
}

bool isSkippableCommentLine(const QString &line)
{
    if (!(line.startsWith("*") && line.endsWith("*")) || line.startsWith("**") || line.length() <= 2) {
        return false;
    }

    const QString inner = line.mid(1, line.length() - 2);
    return !inner.contains("**") && inner.length() > 10;
}

bool isGlobalAnswerSectionHeader(const QString &line)
{
    const QString text = normalizedSemanticLine(line);
    return text == QStringLiteral("参考答案与解析")
        || text == QStringLiteral("答案与解析")
        || text == QStringLiteral("答案和解析");
}

QString normalizedSectionTitle(const QString &line)
{
    static const QRegularExpression markdownHeaderRe(R"(^(#{1,4})\s+(.+))");
    static const QRegularExpression chineseSectionRe(R"(^[一二三四五六七八九十]+[、\.．]\s*.+)");
    static const QRegularExpression plainSectionRe(
        R"(^(选择题|多选题|判断题|判断说理题|填空题|简答题|论述题|材料分析题|材料论述题|综合题).*)"
    );

    const QRegularExpressionMatch headerMatch = markdownHeaderRe.match(line);
    QString text = headerMatch.hasMatch() ? headerMatch.captured(2).trimmed() : line.trimmed();
    text = stripInlineMarkdown(text);

    if (chineseSectionRe.match(text).hasMatch() || plainSectionRe.match(text).hasMatch()) {
        return text;
    }

    return QString();
}

bool isStrongQuestionStart(const QString &line)
{
    static const QRegularExpression strongQuestionRe(
        R"(^[\*]*\d+\s*[.、\)）]\s*[\*]*\s*.+)"
    );
    return strongQuestionRe.match(line).hasMatch();
}

bool isBracketQuestionStart(const QString &line)
{
    static const QRegularExpression bracketQuestionRe(
        R"(^[\*]*[（(]\s*\d+\s*[）)]\s*[\*]*\s*.+)"
    );
    return bracketQuestionRe.match(line).hasMatch();
}

bool isQuestionStartLine(const QString &line, bool allowBracketed)
{
    if (isStrongQuestionStart(line)) {
        return true;
    }

    return allowBracketed && isBracketQuestionStart(line);
}

QString extractQuestionNumber(const QString &line)
{
    static const QRegularExpression strongQuestionRe(
        R"(^[\*]*(\d+)\s*[.、\)）]\s*[\*]*\s*.+)"
    );
    static const QRegularExpression bracketQuestionRe(
        R"(^[\*]*[（(]\s*(\d+)\s*[）)]\s*[\*]*\s*.+)"
    );

    QRegularExpressionMatch match = strongQuestionRe.match(line);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    match = bracketQuestionRe.match(line);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    return QString();
}

QString answerHeadingXml(const QString &number)
{
    return QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Question"/><w:spacing w:before="240" w:after="80"/></w:pPr>
<w:r><w:rPr><w:b/></w:rPr><w:t>第%1题</w:t></w:r>
</w:p>
)").arg(number);
}

int findQuestionBlockIndexByNumber(const QList<MarkdownQuestionBlock> &blocks, const QString &number)
{
    for (int i = 0; i < blocks.size(); ++i) {
        if (blocks.at(i).number == number) {
            return i;
        }
    }

    return -1;
}

void appendContinuation(QStringList &entries, const QString &line)
{
    const QString text = stripInlineMarkdown(line);
    if (text.isEmpty()) {
        return;
    }

    if (entries.isEmpty()) {
        entries.append(text);
        return;
    }

    entries.last().append(QStringLiteral(" ") + text);
}
}

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
    QStringList typeOrder = {"single_choice", "multiple_choice", "true_false", "fill_blank", "short_answer", "essay", "material_essay"};
    // 题型显示名称
    QMap<QString, QString> typeNames = {
        {"single_choice", "选择题"},
        {"multiple_choice", "多选题"},
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

// ==================== Markdown → DOCX 直接转换 ====================

bool DocxGenerator::generateFromMarkdown(const QString &outputPath, const QString &title, const QString &markdownText)
{
    emit generationStarted();

    if (markdownText.trimmed().isEmpty()) {
        m_lastError = "Markdown 内容为空";
        emit errorOccurred(m_lastError);
        emit generationFinished(false, "");
        return false;
    }

    qDebug() << "[DocxGenerator] 从 Markdown 生成 DOCX:" << title;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        m_lastError = "无法创建临时目录";
        emit errorOccurred(m_lastError);
        emit generationFinished(false, "");
        return false;
    }

    QString basePath = tempDir.path();
    QDir().mkpath(basePath + "/_rels");
    QDir().mkpath(basePath + "/word/_rels");

    if (!createContentTypes(basePath) ||
        !createRels(basePath) ||
        !createDocumentRels(basePath) ||
        !createStyles(basePath) ||
        !createSettings(basePath) ||
        !createDocumentFromMarkdown(basePath, title, markdownText)) {
        emit generationFinished(false, "");
        return false;
    }

    if (!packToZip(basePath, outputPath)) {
        emit generationFinished(false, "");
        return false;
    }

    qDebug() << "[DocxGenerator] Markdown DOCX 生成成功:" << outputPath;
    emit generationFinished(true, outputPath);
    return true;
}

bool DocxGenerator::createDocumentFromMarkdown(const QString &tempDir, const QString &title, const QString &markdownText)
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

    // ---- 第一步：逐题收集正文与答案，避免内联答案把后续题目错误吞入答案区 ----
    QStringList lines = markdownText.split('\n');
    QStringList questionLines;
    QList<MarkdownQuestionBlock> questionBlocks;
    bool skipThinkBlock = false;
    bool foundFirstContent = false;
    QString currentSectionTitle;
    int currentQuestionIndex = -1;
    int currentAnswerQuestionIndex = -1;
    bool inGlobalAnswerSection = false;

    enum class AnswerState {
        None,
        Answer,
        Analysis
    };

    AnswerState answerState = AnswerState::None;

    static const QRegularExpression answerLineRe(R"(^[\*]*【答案】[\*]*\s*[:：]?\s*(.*))");
    static const QRegularExpression analysisLineRe(R"(^[\*]*【解[析释]】[\*]*\s*[:：]?\s*(.*))");
    static const QRegularExpression numberedAnswerLineRe(
        R"(^[\*]*[（(]?(\d+)[）)]?\s*[.、:：]?\s*【答案】\s*(.*))"
    );
    static const QRegularExpression numberedAnalysisLineRe(
        R"(^[\*]*[（(]?(\d+)[）)]?\s*[.、:：]?\s*【解[析释]】\s*(.*))"
    );
    static const QRegularExpression numberedFallbackAnswerLineRe(
        R"(^[\*]*[（(]?(\d+)[）)]?\s*[.、:：]\s*(.+))"
    );

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();

        // 跳过 <think> 块
        if (line.startsWith("<think>")) { skipThinkBlock = true; continue; }
        if (line.startsWith("</think>")) { skipThinkBlock = false; continue; }
        if (skipThinkBlock) continue;

        // 跳过空行
        if (line.isEmpty()) continue;

        // 跳过分割线
        if (line.startsWith("---")) continue;

        // 跳过斜体注释行（AI 的尾注，如 *祝学习愉快！...* ）
        if (isSkippableCommentLine(line)) {
            continue;
        }

        if (isGlobalAnswerSectionHeader(line)) {
            inGlobalAnswerSection = true;
            answerState = AnswerState::None;
            currentAnswerQuestionIndex = -1;
            continue;
        }

        const QString sectionTitle = normalizedSectionTitle(line);

        if (inGlobalAnswerSection) {
            if (!sectionTitle.isEmpty()) {
                continue;
            }

            QRegularExpressionMatch numberedMatch = numberedAnswerLineRe.match(line);
            if (numberedMatch.hasMatch()) {
                currentAnswerQuestionIndex = findQuestionBlockIndexByNumber(questionBlocks, numberedMatch.captured(1));
                if (currentAnswerQuestionIndex >= 0) {
                    questionBlocks[currentAnswerQuestionIndex].answerLines.append(
                        stripInlineMarkdown(numberedMatch.captured(2)));
                    answerState = AnswerState::Answer;
                }
                continue;
            }

            numberedMatch = numberedAnalysisLineRe.match(line);
            if (numberedMatch.hasMatch()) {
                currentAnswerQuestionIndex = findQuestionBlockIndexByNumber(questionBlocks, numberedMatch.captured(1));
                if (currentAnswerQuestionIndex >= 0) {
                    questionBlocks[currentAnswerQuestionIndex].analysisLines.append(
                        stripInlineMarkdown(numberedMatch.captured(2)));
                    answerState = AnswerState::Analysis;
                }
                continue;
            }

            if (currentAnswerQuestionIndex >= 0) {
                QRegularExpressionMatch match = answerLineRe.match(line);
                if (match.hasMatch()) {
                    questionBlocks[currentAnswerQuestionIndex].answerLines.append(
                        stripInlineMarkdown(match.captured(1)));
                    answerState = AnswerState::Answer;
                    continue;
                }

                match = analysisLineRe.match(line);
                if (match.hasMatch()) {
                    questionBlocks[currentAnswerQuestionIndex].analysisLines.append(
                        stripInlineMarkdown(match.captured(1)));
                    answerState = AnswerState::Analysis;
                    continue;
                }
            }

            numberedMatch = numberedFallbackAnswerLineRe.match(line);
            if (numberedMatch.hasMatch()) {
                currentAnswerQuestionIndex = findQuestionBlockIndexByNumber(questionBlocks, numberedMatch.captured(1));
                if (currentAnswerQuestionIndex >= 0) {
                    questionBlocks[currentAnswerQuestionIndex].answerLines.append(
                        stripInlineMarkdown(numberedMatch.captured(2)));
                    answerState = AnswerState::Answer;
                }
                continue;
            }

            if (currentAnswerQuestionIndex >= 0 && answerState == AnswerState::Answer) {
                appendContinuation(questionBlocks[currentAnswerQuestionIndex].answerLines, line);
            } else if (currentAnswerQuestionIndex >= 0 && answerState == AnswerState::Analysis) {
                appendContinuation(questionBlocks[currentAnswerQuestionIndex].analysisLines, line);
            }
            continue;
        }

        const bool allowBracketedQuestion = currentQuestionIndex < 0
            || questionBlocks[currentQuestionIndex].hasAnswerContent();

        if (!foundFirstContent) {
            if (!sectionTitle.isEmpty() || isQuestionStartLine(line, true)) {
                foundFirstContent = true;
            } else {
                continue;
            }
        }

        if (!sectionTitle.isEmpty()) {
            currentSectionTitle = sectionTitle;
            answerState = AnswerState::None;
            questionLines.append(sectionTitle);
            continue;
        }

        if (isQuestionStartLine(line, allowBracketedQuestion)) {
            MarkdownQuestionBlock block;
            block.sectionTitle = currentSectionTitle;
            block.number = extractQuestionNumber(line);
            questionBlocks.append(block);
            currentQuestionIndex = questionBlocks.size() - 1;
            answerState = AnswerState::None;
            questionLines.append(line);
            continue;
        }

        if (currentQuestionIndex < 0) {
            questionLines.append(line);
            continue;
        }

        const QRegularExpressionMatch answerMatch = answerLineRe.match(line);
        if (answerMatch.hasMatch()) {
            questionBlocks[currentQuestionIndex].answerLines.append(stripInlineMarkdown(answerMatch.captured(1)));
            answerState = AnswerState::Answer;
            continue;
        }

        const QRegularExpressionMatch analysisMatch = analysisLineRe.match(line);
        if (analysisMatch.hasMatch()) {
            questionBlocks[currentQuestionIndex].analysisLines.append(stripInlineMarkdown(analysisMatch.captured(1)));
            answerState = AnswerState::Analysis;
            continue;
        }

        if (answerState == AnswerState::Answer) {
            appendContinuation(questionBlocks[currentQuestionIndex].answerLines, line);
            continue;
        }

        if (answerState == AnswerState::Analysis) {
            appendContinuation(questionBlocks[currentQuestionIndex].analysisLines, line);
            continue;
        }

        questionLines.append(line);
    }

    // ---- 第二步：构建文档内容 ----
    QString bodyContent;

    // 文档标题
    bodyContent += QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Title"/></w:pPr>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(title));

    // 副标题（日期 + 提示）
    bodyContent += QString(R"(<w:p>
<w:pPr><w:jc w:val="center"/><w:spacing w:after="400"/></w:pPr>
<w:r><w:rPr><w:color w:val="999999"/><w:sz w:val="22"/></w:rPr>
<w:t>AI 智能出题  |  道德与法治</w:t></w:r>
</w:p>
)");

    // 题目正文部分
    for (const QString &line : questionLines) {
        bodyContent += markdownLineToXml(line);
    }

    bool hasAnyAnswers = false;
    for (const MarkdownQuestionBlock &block : questionBlocks) {
        if (block.hasAnswerContent()) {
            hasAnyAnswers = true;
            break;
        }
    }

    // 如果有答案/解析，插入分页符后添加
    if (hasAnyAnswers) {
        // 分页符
        bodyContent += R"(<w:p>
<w:r><w:br w:type="page"/></w:r>
</w:p>
)";
        // 答案区标题
        bodyContent += R"(<w:p>
<w:pPr><w:pStyle w:val="Title"/></w:pPr>
<w:r><w:t>参考答案与解析</w:t></w:r>
</w:p>
)";

        QString lastRenderedSectionTitle;
        for (int i = 0; i < questionBlocks.size(); ++i) {
            const MarkdownQuestionBlock &block = questionBlocks.at(i);
            if (!block.hasAnswerContent()) {
                continue;
            }

            if (!block.sectionTitle.isEmpty() && block.sectionTitle != lastRenderedSectionTitle) {
                bodyContent += markdownLineToXml(block.sectionTitle);
                lastRenderedSectionTitle = block.sectionTitle;
            }

            const QString questionNumber = block.number.isEmpty()
                ? QString::number(i + 1)
                : block.number;
            bodyContent += answerHeadingXml(questionNumber);

            for (const QString &answerLine : block.answerLines) {
                bodyContent += markdownLineToXml(QStringLiteral("【答案】%1").arg(answerLine));
            }
            for (const QString &analysisLine : block.analysisLines) {
                bodyContent += markdownLineToXml(QStringLiteral("【解析】%1").arg(analysisLine));
            }
        }
    }

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

QString DocxGenerator::markdownLineToXml(const QString &line)
{
    // ---- Markdown 标题 ----
    // ### 标题 → Heading1 样式
    static const QRegularExpression headerRe(R"(^(#{1,4})\s+(.+))");
    QRegularExpressionMatch headerMatch = headerRe.match(line);
    if (headerMatch.hasMatch()) {
        QString text = headerMatch.captured(2).trimmed();
        // 去除 Markdown 粗体标记
        text.remove(QRegularExpression(R"(\*{1,2})"));
        return QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Heading1"/></w:pPr>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(text));
    }

    const QString sectionTitle = normalizedSectionTitle(line);
    if (!sectionTitle.isEmpty()) {
        return QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Heading1"/></w:pPr>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(sectionTitle));
    }

    // ---- 【答案】行 → 绿色加粗 ----
    static const QRegularExpression answerRe(R"(^[\*]*【答案】[\*]*\s*[:：]?\s*(.*))");
    QRegularExpressionMatch ansMatch = answerRe.match(line);
    if (ansMatch.hasMatch()) {
        QString answer = ansMatch.captured(1).trimmed();
        answer.remove(QRegularExpression(R"(\*{1,2})"));
        return QString(R"(<w:p>
<w:pPr><w:spacing w:before="120" w:after="60"/></w:pPr>
<w:r><w:rPr><w:b/><w:color w:val="2E7D32"/></w:rPr><w:t>【答案】%1</w:t></w:r>
</w:p>
)").arg(escapeXml(answer));
    }

    // ---- 【解析】行 → 灰色 ----
    static const QRegularExpression analysisRe(R"(^[\*]*【解[析释]】[\*]*\s*[:：]?\s*(.*))");
    QRegularExpressionMatch anaMatch = analysisRe.match(line);
    if (anaMatch.hasMatch()) {
        QString analysis = anaMatch.captured(1).trimmed();
        analysis.remove(QRegularExpression(R"(\*{1,2})"));
        return QString(R"(<w:p>
<w:pPr><w:spacing w:before="60" w:after="200"/></w:pPr>
<w:r><w:rPr><w:b/><w:color w:val="666666"/></w:rPr><w:t>【解析】</w:t></w:r>
<w:r><w:rPr><w:color w:val="666666"/><w:sz w:val="22"/></w:rPr><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(analysis));
    }

    // ---- 选项行 A. B. C. D. → 缩进 ----
    static const QRegularExpression optionRe(R"(^\s*([A-Ha-h])\s*[.、\)）:：]\s*(.+))");
    QRegularExpressionMatch optMatch = optionRe.match(line);
    if (optMatch.hasMatch()) {
        QString label = optMatch.captured(1).toUpper();
        QString text = optMatch.captured(2).trimmed();
        return QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Option"/></w:pPr>
<w:r><w:t>%1. %2</w:t></w:r>
</w:p>
)").arg(escapeXml(label), escapeXml(text));
    }

    // ---- 带编号的题目行（**1.** 或 1. 等）→ 加粗 ----
    static const QRegularExpression questionRe(
        R"(^[\*]*(\d+)\s*[.、\)）]\s*[\*]*\s*(.+))"
    );
    QRegularExpressionMatch qMatch = questionRe.match(line);
    if (qMatch.hasMatch()) {
        QString num = qMatch.captured(1);
        QString stem = qMatch.captured(2).trimmed();
        stem.remove(QRegularExpression(R"(\*{1,2})"));
        return QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Question"/></w:pPr>
<w:r><w:rPr><w:b/></w:rPr><w:t>%1. </w:t></w:r>
<w:r><w:t>%2</w:t></w:r>
</w:p>
)").arg(escapeXml(num), escapeXml(stem));
    }

    static const QRegularExpression bracketQuestionRe(
        R"(^[\*]*[（(]\s*(\d+)\s*[）)]\s*[\*]*\s*(.+))"
    );
    qMatch = bracketQuestionRe.match(line);
    if (qMatch.hasMatch()) {
        QString num = qMatch.captured(1);
        QString stem = qMatch.captured(2).trimmed();
        stem.remove(QRegularExpression(R"(\*{1,2})"));
        return QString(R"(<w:p>
<w:pPr><w:pStyle w:val="Question"/></w:pPr>
<w:r><w:rPr><w:b/></w:rPr><w:t>（%1）</w:t></w:r>
<w:r><w:t>%2</w:t></w:r>
</w:p>
)").arg(escapeXml(num), escapeXml(stem));
    }

    // ---- 纯加粗行 **text** → 加粗段落 ----
    static const QRegularExpression boldLineRe(R"(^\*{2}(.+)\*{2}\s*$)");
    QRegularExpressionMatch boldMatch = boldLineRe.match(line);
    if (boldMatch.hasMatch()) {
        QString text = boldMatch.captured(1).trimmed();
        return QString(R"(<w:p>
<w:pPr><w:spacing w:before="200" w:after="100"/></w:pPr>
<w:r><w:rPr><w:b/></w:rPr><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(text));
    }

    // ---- 含①②③等多选组合选项 ----
    // 直接作为普通段落处理

    // ---- 普通文本段落 ----
    // 处理行内 **加粗** 标记
    QString plainText = line;
    plainText.remove(QRegularExpression(R"(\*{1,2})"));

    return QString(R"(<w:p>
<w:r><w:t>%1</w:t></w:r>
</w:p>
)").arg(escapeXml(plainText));
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

    if (!SimpleZipWriter::packDirectory(tempDir, outputPath)) {
        m_lastError = QString("ZIP 打包失败: %1").arg(SimpleZipWriter::lastError());
        emit errorOccurred(m_lastError);
        return false;
    }

    return true;
}
