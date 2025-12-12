#ifndef MARKDOWNRENDERER_H
#define MARKDOWNRENDERER_H

#include <QString>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QColor>
#include <QFont>
#include <QMap>

/**
 * @brief 基于MD4C的Markdown渲染器
 *
 * 将Markdown文本转换为Qt富文本格式，支持：
 * - 粗体、斜体、删除线
 * - 标题、列表、代码块
 * - 链接、表格、引用
 */
class MarkdownRenderer
{
public:
    MarkdownRenderer();
    ~MarkdownRenderer();

    /**
     * @brief 将Markdown文本转换为Qt富文本HTML
     * @param markdown Markdown格式的文本
     * @return 富文本HTML字符串
     */
    QString renderToHtml(const QString &markdown);

    /**
     * @brief 将Markdown文本转换为QTextDocument
     * @param markdown Markdown格式的文本
     * @return QTextDocument对象
     */
    QTextDocument* renderToDocument(const QString &markdown);

    /**
     * @brief 设置代码块主题颜色
     * @param backgroundColor 代码块背景色
     * @param textColor 代码块文字色
     */
    void setCodeTheme(const QColor &backgroundColor, const QColor &textColor);

    /**
     * @brief 设置链接颜色
     * @param color 链接颜色
     */
    void setLinkColor(const QColor &color);

    /**
     * @brief 设置标题颜色
     * @param level 标题级别(1-6)
     * @param color 颜色
     */
    void setHeadingColor(int level, const QColor &color);

private:
    // 内部渲染数据结构
    struct RenderData {
        QString html;
        QString currentText;
        bool inCodeBlock;
        bool inInlineCode;
        int listLevel;
        QMap<QString, QString> linkUrls;

        RenderData() : inCodeBlock(false), inInlineCode(false), listLevel(0) {}
    };

    // 渲染方法
    void processMarkdown(const QString &markdown, RenderData &data);
    void processTextBlock(const QString &text, RenderData &data);
    void processInlineFormatting(QString &text, RenderData &data);
    void processHeaders(const QString &line, RenderData &data);
    void processLists(const QString &line, RenderData &data);
    void processCodeBlocks(const QString &line, RenderData &data);
    void processLinks(QString &text, RenderData &data);

    // HTML生成方法
    QString escapeHtml(const QString &text) const;
    QString generateHtmlForHeading(const QString &text, int level) const;
    QString generateHtmlForCodeBlock(const QString &text) const;
    QString generateHtmlForInlineCode(const QString &text) const;
    QString generateHtmlForLink(const QString &text, const QString &url) const;
    QString generateHtmlForListItem(const QString &text, bool isOrdered, int level) const;

    // 样式配置
    QColor m_codeBackgroundColor;
    QColor m_codeTextColor;
    QColor m_linkColor;
    QMap<int, QColor> m_headingColors;

    // 工具方法
    bool isHeaderLine(const QString &line, int &level) const;
    bool isOrderedListLine(const QString &line, QString &content) const;
    bool isUnorderedListLine(const QString &line, QString &content) const;
    bool isCodeBlockStart(const QString &line, QString &language) const;
    bool isCodeBlockEnd(const QString &line) const;
    QString stripMarkdownMarkers(const QString &text) const;
};

#endif // MARKDOWNRENDERER_H