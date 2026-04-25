#ifndef HELPCENTERWIDGET_H
#define HELPCENTERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTextEdit>
#include <QVector>

/**
 * @brief 帮助中心页面
 *
 * 提供系统使用帮助，包括快速入门、功能指引、FAQ、快捷键速查、
 * 关于系统和反馈支持等板块。支持关键词搜索快速定位。
 */
class HelpCenterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HelpCenterWidget(QWidget *parent = nullptr);
    ~HelpCenterWidget();

private slots:
    void onSearchTextChanged(const QString &text);
    void onSubmitFeedback();

private:
    void setupUI();

    // 各板块创建
    QWidget* createHeaderSection();
    QWidget* createQuickStartSection();
    QWidget* createFeatureGuideSection();
    QWidget* createFAQSection();
    QWidget* createShortcutsSection();
    QWidget* createAboutSection();
    QWidget* createFeedbackSection();

    // Accordion 折叠面板辅助
    QWidget* createAccordionItem(const QString &title, const QString &content);

    // 搜索相关
    void performSearch(const QString &keyword);
    void resetAllSections();

    // UI 组件
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_contentWidget = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QLineEdit *m_searchInput = nullptr;
    QLabel *m_searchResultLabel = nullptr;
    QTextEdit *m_feedbackEdit = nullptr;

    // 可搜索的折叠面板列表
    struct AccordionEntry {
        QWidget *container;     // 整个折叠项容器
        QPushButton *titleBtn;  // 标题按钮
        QWidget *contentWidget; // 内容区域
        QString titleText;      // 标题文本（用于搜索）
        QString contentText;    // 内容文本（用于搜索）
    };
    QVector<AccordionEntry> m_accordionItems;

    // 各板块容器（用于搜索显示/隐藏）
    QWidget *m_quickStartSection = nullptr;
    QWidget *m_featureGuideSection = nullptr;
    QWidget *m_faqSection = nullptr;
    QWidget *m_shortcutsSection = nullptr;
    QWidget *m_aboutSection = nullptr;
    QWidget *m_feedbackSection = nullptr;
};

#endif // HELPCENTERWIDGET_H
