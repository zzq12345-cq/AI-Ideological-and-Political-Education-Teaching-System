#ifndef DATAANALYTICSWIDGET_H
#define DATAANALYTICSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

// Qt Charts
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class DifyService;
class AnalyticsDataService;
class MockDataSource;
class AnalyticsNavigationBar;
class PersonalAnalyticsPage;
class ClassAnalyticsPage;

/**
 * @brief 数据分析报告主界面
 *
 * 展示教学数据可视化、学生画像分析、AI 智能报告
 * 老王说：这玩意儿就是把数据变成好看的图表，别想太复杂
 * 现在支持三个视图：概览、个人分析、班级分析
 */
class DataAnalyticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataAnalyticsWidget(QWidget *parent = nullptr);
    ~DataAnalyticsWidget();

    void setDifyService(DifyService *service);
    void refresh();

private slots:
    void onRefreshClicked();
    void onExportClicked();
    void onGenerateReportClicked();
    void onAIResponseReceived(const QString &response);
    void onAIStreamChunk(const QString &chunk);
    void onDataRefreshed();
    void onViewChanged(int viewType);

private:
    void setupUI();
    void setupStyles();
    void createHeader();
    void createNavigationBar();
    void createOverviewPage();
    void createMetricsCards(QVBoxLayout *layout);
    void createChartsArea(QVBoxLayout *layout);
    void createAIReportArea(QVBoxLayout *layout);
    void updateMetricsDisplay();
    void updateCharts();
    void animatePageSwitch(int newIndex);

    // 创建单个指标卡片
    QFrame* createMetricCard(const QString &iconPath,
                             const QString &title,
                             const QString &value,
                             const QString &change,
                             bool isPositive);

    // UI 组件
    QVBoxLayout *m_mainLayout;
    QFrame *m_headerFrame;
    QWidget *m_metricsContainer;
    QWidget *m_chartsContainer;
    QFrame *m_aiReportFrame;
    QLabel *m_aiReportContent;
    QPushButton *m_refreshBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_generateReportBtn;
    QScrollArea *m_scrollArea;

    // 导航和页面切换
    AnalyticsNavigationBar *m_navigationBar;
    QStackedWidget *m_stackedWidget;
    QWidget *m_overviewPage;
    PersonalAnalyticsPage *m_personalPage;
    ClassAnalyticsPage *m_classPage;

    // 指标标签（用于动态更新）
    QLabel *m_participationValue;
    QLabel *m_participationChange;
    QLabel *m_completionValue;
    QLabel *m_completionChange;
    QLabel *m_achievementValue;
    QLabel *m_achievementChange;

    // 图表组件
    QChartView *m_barChartView;
    QChartView *m_lineChartView;
    QBarSet *m_gradeBarSet;
    QLineSeries *m_participationSeries;
    QLineSeries *m_completionSeries;

    // 服务
    DifyService *m_difyService;
    AnalyticsDataService *m_dataService;
    MockDataSource *m_mockDataSource;
    QString m_currentAIResponse;
    bool m_isGeneratingReport;
};

#endif // DATAANALYTICSWIDGET_H
