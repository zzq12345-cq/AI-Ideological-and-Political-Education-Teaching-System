#ifndef CLASSANALYTICSPAGE_H
#define CLASSANALYTICSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>

class DifyService;
class MockDataSource;

/**
 * @brief 班级整体分析页面
 *
 * 展示班级成绩分布、学生排名和薄弱知识点分析
 * 老王说：一个班级的整体情况，一目了然
 */
class ClassAnalyticsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ClassAnalyticsPage(QWidget *parent = nullptr);
    ~ClassAnalyticsPage();

    void setDataSource(MockDataSource *dataSource);
    void setDifyService(DifyService *service);
    void refresh();

signals:
    void classChanged(int classId);

private slots:
    void onClassChanged(int index);
    void onGenerateAdviceClicked();
    void onAIStreamChunk(const QString &chunk);
    void onAIRequestFinished();
    void onExportClicked();

private:
    void setupUI();
    void setupStyles();
    void createHeader();
    void createDistributionChart();
    void createRankingTable();
    QFrame* createWeakPointsChart();
    void createAIAdviceArea();
    void updateCharts();
    void updateRankingTable();
    void updateWeakPointsChart();

    // UI组件
    QVBoxLayout *m_mainLayout;
    QComboBox *m_classCombo;
    QFrame *m_headerFrame;

    // 成绩分布图
    QChartView *m_distributionChartView;
    QBarSet *m_gradeBarSet;

    // 学生排名表格
    QTableWidget *m_rankingTable;

    // 薄弱知识点图表
    QChartView *m_weakPointsChartView;

    // AI建议区域
    QFrame *m_adviceFrame;
    QLabel *m_adviceContent;
    QPushButton *m_generateAdviceBtn;
    QPushButton *m_exportBtn;
    QString m_currentAdvice;
    bool m_isGenerating;

    // 数据
    MockDataSource *m_dataSource;
    DifyService *m_difyService;
    int m_currentClassId;
};

#endif // CLASSANALYTICSPAGE_H
