#ifndef ANALYTICSNAVIGATIONBAR_H
#define ANALYTICSNAVIGATIONBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QVector>

/**
 * @brief 数据分析导航栏
 *
 * 用于切换 概览/个人分析/班级分析 三个视图
 * 老王说：简洁明了的导航，别搞那些花里胡哨的
 */
class AnalyticsNavigationBar : public QWidget
{
    Q_OBJECT

public:
    enum ViewType {
        Overview = 0,   // 概览
        Personal = 1,   // 个人分析
        ClassWide = 2   // 班级分析
    };

    explicit AnalyticsNavigationBar(QWidget *parent = nullptr);
    ~AnalyticsNavigationBar();

    void setCurrentView(ViewType view);
    ViewType currentView() const;

signals:
    void viewChanged(ViewType view);

private slots:
    void onButtonClicked(int id);

private:
    void setupUI();
    void setupStyles();
    QPushButton* createNavButton(const QString &text, const QString &icon);
    void updateButtonStyles();

    QHBoxLayout *m_layout;
    QButtonGroup *m_buttonGroup;
    QVector<QPushButton*> m_buttons;
    ViewType m_currentView;
};

#endif // ANALYTICSNAVIGATIONBAR_H
