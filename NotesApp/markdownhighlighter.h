#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    MarkdownHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> rules;

    QTextCharFormat headerFormat;
    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat codeFormat;
    QTextCharFormat linkFormat;
    QTextCharFormat listFormat;
};

#endif // MARKDOWNHIGHLIGHTER_H
