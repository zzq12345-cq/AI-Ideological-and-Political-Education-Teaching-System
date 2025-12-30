#include "markdownhighlighter.h"

MarkdownHighlighter::MarkdownHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // Header formats
    headerFormat.setForeground(QColor(43, 145, 175));
    headerFormat.setFontWeight(QFont::Bold);

    // Bold format
    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setForeground(QColor(203, 75, 22));

    // Italic format
    italicFormat.setFontItalic(true);
    italicFormat.setForeground(QColor(211, 54, 130));

    // Code format
    codeFormat.setForeground(QColor(133, 153, 0));
    codeFormat.setFontFamily("Courier New");
    codeFormat.setBackground(QColor(235, 237, 240));

    // Link format
    linkFormat.setForeground(QColor(66, 133, 244));
    linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    // List format
    listFormat.setForeground(QColor(88, 110, 117));

    // Headers (# ## ### #### ##### ######)
    HighlightingRule rule;
    rule.pattern = QRegularExpression("^#{1,6}\\s.*");
    rule.format = headerFormat;
    rules.append(rule);

    // Bold (**text** or __text__)
    rule.pattern = QRegularExpression("\\*\\*.*?\\*\\*|__.*?__");
    rule.format = boldFormat;
    rules.append(rule);

    // Italic (*text* or _text_)
    rule.pattern = QRegularExpression("\\*.*?\\*|_.*?_(?!_)");
    rule.format = italicFormat;
    rules.append(rule);

    // Inline code (`code`)
    rule.pattern = QRegularExpression("`.*?`");
    rule.format = codeFormat;
    rules.append(rule);

    // Links [text](url)
    rule.pattern = QRegularExpression("\\[.*?\\]\\(.*?\\)");
    rule.format = linkFormat;
    rules.append(rule);

    // Lists (- or * or + or number.)
    rule.pattern = QRegularExpression("^(\\s*[-*+]\\s|\\s*\\d+\\.\\s)");
    rule.format = listFormat;
    rules.append(rule);
}

void MarkdownHighlighter::highlightBlock(const QString& text)
{
    for (const HighlightingRule& rule : rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
