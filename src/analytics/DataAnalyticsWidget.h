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

// Qt Charts
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class DifyService;
class AnalyticsDataService;

/**
 * @brief 数据分析报告主界面
 *
 * 展示教学数据可视化、学生画像分析、AI 智能报告
 * 老王说：这玩意儿就是把数据变成好看的图表，别想太复杂
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

private:
    void setupUI();
    void setupStyles();
    void createHeader();
    void createMetricsCards();
    void createChartsArea();
    void createAIReportArea();
    void updateMetricsDisplay();
    void updateCharts();

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
    QString m_currentAIResponse;
    bool m_isGeneratingReport;
};

#endif // DATAANALYTICSWIDGET_H
