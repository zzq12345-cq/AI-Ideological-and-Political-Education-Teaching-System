#include "MarkdownRenderer.h"
#include <QRegularExpression>
#include <QTextDocument>
#include <QDebug>

MarkdownRenderer::MarkdownRenderer()
    : m_codeBackgroundColor("#f6f8fa")
    , m_codeTextColor("#d73a49")
    , m_linkColor("#0366d6")
{
    // 设置默认标题颜色
    m_headingColors[1] = QColor("#24292e"); // H1 - 最深
    m_headingColors[2] = QColor("#24292e"); // H2
    m_headingColors[3] = QColor("#24292e"); // H3
    m_headingColors[4] = QColor("#24292e"); // H4
    m_headingColors[5] = QColor("#24292e"); // H5
    m_headingColors[6] = QColor("#586069"); // H6 - 最浅
}

MarkdownRenderer::~MarkdownRenderer()
{
}

QString MarkdownRenderer::renderToHtml(const QString &markdown)
{
    if (markdown.isEmpty()) {
        return QString();
    }

    RenderData data;
    processMarkdown(markdown, data);
    return data.html;
}

QTextDocument* MarkdownRenderer::renderToDocument(const QString &markdown)
{
    QTextDocument *doc = new QTextDocument();
    QString html = renderToHtml(markdown);

    // 设置基础样式
    QString fullHtml = QString("<!DOCTYPE html>"
                             "<html>"
                             "<head>"
                             "<style>"
                             "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif; "
                             "font-size: 14px; line-height: 1.6; color: #24292e; margin: 8px; }"
                             "pre { background-color: %1; padding: 12px; border-radius: 6px; overflow-x: auto; }"
                             "code { background-color: %1; color: %2; padding: 2px 4px; border-radius: 3px; font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace; }"
                             "a { color: %3; text-decoration: none; }"
                             "a:hover { text-decoration: underline; }"
                             "ul, ol { padding-left: 20px; margin: 8px 0; }"
                             "li { margin: 2px 0; }"
                             "blockquote { border-left: 4px solid #dfe2e5; padding-left: 16px; margin: 8px 0; color: #6a737d; }"
                             "</style>"
                             "</head>"
                             "<body>%4</body>"
                             "</html>")
                             .arg(m_codeBackgroundColor.name())
                             .arg(m_codeTextColor.name())
                             .arg(m_linkColor.name())
                             .arg(html);

    doc->setHtml(fullHtml);
    return doc;
}

void MarkdownRenderer::setCodeTheme(const QColor &backgroundColor, const QColor &textColor)
{
    m_codeBackgroundColor = backgroundColor;
    m_codeTextColor = textColor;
}

void MarkdownRenderer::setLinkColor(const QColor &color)
{
    m_linkColor = color;
}

void MarkdownRenderer::setHeadingColor(int level, const QColor &color)
{
    if (level >= 1 && level <= 6) {
        m_headingColors[level] = color;
    }
}

void MarkdownRenderer::processMarkdown(const QString &markdown, RenderData &data)
{
    QStringList lines = markdown.split('\n');
    QString codeBlockLanguage;
    bool inParagraph = false;
    bool inList = false;
    bool listIsOrdered = false;

    auto closeParagraph = [&]() {
        if (inParagraph) {
            data.html += "</p>";
            inParagraph = false;
        }
    };

    auto closeList = [&]() {
        if (inList) {
            data.html += listIsOrdered ? "</ol>" : "</ul>";
            inList = false;
        }
    };

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        bool lineProcessed = false;

        // 处理代码块
        if (data.inCodeBlock) {
            if (isCodeBlockEnd(line)) {
                data.html += generateHtmlForCodeBlock(data.currentText);
                data.currentText.clear();
                data.inCodeBlock = false;
                codeBlockLanguage.clear();
                lineProcessed = true;
            } else {
                data.currentText += line + "\n";
                lineProcessed = true;
            }
        } else if (isCodeBlockStart(line, codeBlockLanguage)) {
            closeParagraph();
            closeList();
            data.inCodeBlock = true;
            data.currentText.clear();
            lineProcessed = true;
        }

        // 如果不是代码块内的内容，继续其他处理
        if (!lineProcessed && !data.inCodeBlock) {
            // 处理空行
            if (line.trimmed().isEmpty()) {
                closeParagraph();
                closeList();
                continue;
            }

            // 处理标题
            int headingLevel;
            if (isHeaderLine(line, headingLevel)) {
                closeParagraph();
                closeList();
                QString headingText = stripMarkdownMarkers(line);
                data.html += generateHtmlForHeading(headingText, headingLevel);
                continue;
            }

            // 处理列表
            QString listContent;
            bool isOrdered = isOrderedListLine(line, listContent);
            bool isUnordered = !isOrdered && isUnorderedListLine(line, listContent);

            if (isOrdered || isUnordered) {
                closeParagraph();

                if (!inList || listIsOrdered != isOrdered) {
                    closeList();
                    listIsOrdered = isOrdered;
                    data.html += listIsOrdered ? "<ol style=\"margin: 8px 0; padding-left: 20px;\">" : "<ul style=\"margin: 8px 0; padding-left: 20px;\">";
                    inList = true;
                }
                data.html += generateHtmlForListItem(listContent, isOrdered, 1);
                continue;
            }

            // 处理引用块
            if (line.startsWith("> ")) {
                closeParagraph();
                closeList();
                QString quoteText = line.mid(2);
                processInlineFormatting(quoteText, data);
                data.html += "<blockquote style=\"border-left: 4px solid #dfe2e5; padding-left: 16px; margin: 8px 0; color: #6a737d;\">" + quoteText + "</blockquote>";
                continue;
            }

            // 处理普通文本段落
            closeList();
            if (!inParagraph) {
                data.html += "<p style=\"margin: 8px 0;\">";
                inParagraph = true;
            }

            QString processedLine = line;
            processInlineFormatting(processedLine, data);
            data.html += processedLine + " ";
        }
    }

    // 结束未关闭的段落
    if (inParagraph) {
        data.html += "</p>";
    }

    // 结束未关闭的列表
    if (inList) {
        data.html += listIsOrdered ? "</ol>" : "</ul>";
    }

    // 结束未关闭的代码块
    if (data.inCodeBlock) {
        data.html += generateHtmlForCodeBlock(data.currentText);
    }
}

void MarkdownRenderer::processInlineFormatting(QString &text, RenderData &data)
{
    text = escapeHtml(text);

    // 处理行内代码
    QRegularExpression inlineCodeRegex("`([^`]+)`");
    QRegularExpressionMatchIterator it = inlineCodeRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString codeText = match.captured(1);
        text.replace(match.captured(0), generateHtmlForInlineCode(codeText));
    }

    // 处理粗体
    QRegularExpression boldRegex("\\*\\*([^\\*]+)\\*\\*");
    it = boldRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString boldText = match.captured(1);
        text.replace(match.captured(0), "<strong>" + boldText + "</strong>");
    }

    // 处理斜体
    QRegularExpression italicRegex("\\*([^\\*]+)\\*");
    it = italicRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString italicText = match.captured(1);
        text.replace(match.captured(0), "<em>" + italicText + "</em>");
    }

    // 处理删除线
    QRegularExpression strikethroughRegex("~~([^~]+)~~");
    it = strikethroughRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString strikeText = match.captured(1);
        text.replace(match.captured(0), "<del>" + strikeText + "</del>");
    }

    // 处理链接
    processLinks(text, data);
}

void MarkdownRenderer::processLinks(QString &text, RenderData &/*data*/)
{
    QRegularExpression linkRegex("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    QRegularExpressionMatchIterator it = linkRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString linkText = match.captured(1);
        QString linkUrl = match.captured(2);
        text.replace(match.captured(0), generateHtmlForLink(linkText, linkUrl));
    }
}

QString MarkdownRenderer::escapeHtml(const QString &text) const
{
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    return escaped;
}

QString MarkdownRenderer::generateHtmlForHeading(const QString &text, int level) const
{
    QColor color = m_headingColors.value(level, QColor("#24292e"));
    QString colorStyle = QString("color: %1;").arg(color.name());
    QString fontSize;

    switch (level) {
    case 1: fontSize = "24px; font-weight: 600;"; break;
    case 2: fontSize = "20px; font-weight: 600;"; break;
    case 3: fontSize = "16px; font-weight: 600;"; break;
    case 4: fontSize = "14px; font-weight: 600;"; break;
    case 5: fontSize = "13px; font-weight: 600;"; break;
    case 6: fontSize = "12px; font-weight: 600;"; break;
    default: fontSize = "14px; font-weight: 600;"; break;
    }

    QString style = colorStyle + " font-size: " + fontSize + " margin: 16px 0 8px 0;";
    return QString("<h%1 style=\"%2\">%3</h%1>").arg(level).arg(style).arg(text);
}

QString MarkdownRenderer::generateHtmlForCodeBlock(const QString &text) const
{
    QString escapedText = escapeHtml(text.trimmed());
    QString style = QString("background-color: %1; color: %2; padding: 12px; border-radius: 6px; font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace; white-space: pre-wrap; margin: 8px 0;")
                     .arg(m_codeBackgroundColor.name())
                     .arg(m_codeTextColor.name());
    return QString("<pre style=\"%1\">%2</pre>").arg(style).arg(escapedText);
}

QString MarkdownRenderer::generateHtmlForInlineCode(const QString &text) const
{
    QString style = QString("background-color: %1; color: %2; padding: 2px 4px; border-radius: 3px; font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;")
                     .arg(m_codeBackgroundColor.name())
                     .arg(m_codeTextColor.name());
    return QString("<code style=\"%1\">%2</code>").arg(style).arg(text);
}

QString MarkdownRenderer::generateHtmlForLink(const QString &text, const QString &url) const
{
    QString style = QString("color: %1; text-decoration: none;").arg(m_linkColor.name());
    return QString("<a href=\"%1\" style=\"%2\" target=\"_blank\">%3</a>").arg(url).arg(style).arg(text);
}

QString MarkdownRenderer::generateHtmlForListItem(const QString &text, bool isOrdered, int level) const
{
    QString style = QString("margin: 4px 0; padding-left: %1px;").arg(level * 20);
    QString processedText = text;

    // 处理行内格式
    RenderData tempData;
    MarkdownRenderer* tempRenderer = const_cast<MarkdownRenderer*>(this);
    tempRenderer->processInlineFormatting(processedText, tempData);

    if (isOrdered) {
        return QString("<li style=\"%1\">%2</li>").arg(style).arg(processedText);
    } else {
        return QString("<li style=\"list-style-type: disc; %1\">%2</li>").arg(style).arg(processedText);
    }
}

bool MarkdownRenderer::isHeaderLine(const QString &line, int &level) const
{
    int count = 0;
    for (int i = 0; i < line.length() && line[i] == '#'; ++i) {
        count++;
    }
    if (count > 0 && count <= 6 && count < line.length() && line[count] == ' ') {
        level = count;
        return true;
    }
    return false;
}

bool MarkdownRenderer::isOrderedListLine(const QString &line, QString &content) const
{
    QRegularExpression regex(R"(^\s*(\d+)\.\s+(.*))");
    QRegularExpressionMatch match = regex.match(line);
    if (match.hasMatch()) {
        content = match.captured(2);
        return true;
    }
    return false;
}

bool MarkdownRenderer::isUnorderedListLine(const QString &line, QString &content) const
{
    if (line.startsWith("- ") || line.startsWith("* ") || line.startsWith("+ ")) {
        content = line.mid(2);
        return true;
    }
    return false;
}

bool MarkdownRenderer::isCodeBlockStart(const QString &line, QString &language) const
{
    if (line.startsWith("```")) {
        language = line.mid(3).trimmed();
        return true;
    }
    return false;
}

bool MarkdownRenderer::isCodeBlockEnd(const QString &line) const
{
    return line.startsWith("```");
}

QString MarkdownRenderer::stripMarkdownMarkers(const QString &text) const
{
    QString result = text;

    // 移除标题标记
    if (result.startsWith("#")) {
        int spaceIndex = result.indexOf(' ');
        if (spaceIndex > 0) {
            result = result.mid(spaceIndex + 1);
        }
    }

    return result.trimmed();
}
