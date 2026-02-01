#ifndef PERSONALANALYTICSPAGE_H
#define PERSONALANALYTICSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class DifyService;
class MockDataSource;
class Student;

/**
 * @brief 个人学情分析页面
 *
 * 展示单个学生的成绩趋势、知识点掌握度和AI建议
 * 老王说：一个学生的数据分析，简单明了
 */
class PersonalAnalyticsPage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalAnalyticsPage(QWidget *parent = nullptr);
    ~PersonalAnalyticsPage();

    void setDataSource(MockDataSource *dataSource);
    void setDifyService(DifyService *service);
    void refresh();

signals:
    void studentChanged(int studentId);

private slots:
    void onClassChanged(int index);
    void onStudentChanged(int index);
    void onGenerateAdviceClicked();
    void onAIStreamChunk(const QString &chunk);
    void onAIRequestFinished();

private:
    void setupUI();
    void setupStyles();
    void createHeader();
    void createScoreTrendChart();
    void createKnowledgeRadarChart();
    void createAIAdviceArea();
    void updateStudentList();
    void updateCharts();
    void updateRadarChart();

    // UI组件
    QVBoxLayout *m_mainLayout;
    QComboBox *m_classCombo;
    QComboBox *m_studentCombo;
    QFrame *m_headerFrame;

    // 成绩趋势图
    QChartView *m_trendChartView;
    QLineSeries *m_scoreSeries;
    QLineSeries *m_classAvgSeries;  // 班级平均线

    // 知识点雷达图（使用自定义绘制）
    QWidget *m_radarWidget;

    // AI建议区域
    QFrame *m_adviceFrame;
    QLabel *m_adviceContent;
    QPushButton *m_generateAdviceBtn;
    QString m_currentAdvice;
    bool m_isGenerating;

    // 数据
    MockDataSource *m_dataSource;
    DifyService *m_difyService;
    int m_currentStudentId;
    int m_currentClassId;
};

#endif // PERSONALANALYTICSPAGE_H
