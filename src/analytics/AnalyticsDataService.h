#ifndef ANALYTICSDATASERVICE_H
#define ANALYTICSDATASERVICE_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QRandomGenerator>

/**
 * @brief 教学数据分析服务
 *
 * 提供模拟的教学统计数据，后续可对接真实数据库
 * 老王说：先用假数据把 UI 搞漂亮，真数据以后再说
 */
class AnalyticsDataService : public QObject
{
    Q_OBJECT

public:
    // 单例模式
    static AnalyticsDataService* instance();

    // 核心指标数据结构
    struct MetricData {
        double value;           // 当前值 (百分比)
        double change;          // 较上周变化
        bool isPositive;        // 是否正向变化
    };

    // 成绩分布数据结构
    struct GradeDistribution {
        int excellent;          // 优秀 (90-100)
        int good;               // 良好 (80-89)
        int pass;               // 及格 (60-79)
        int fail;               // 不及格 (<60)
    };

    // 趋势数据点
    struct TrendPoint {
        QDate date;
        double value;
    };

    // 获取核心指标
    MetricData getParticipationRate();   // 课堂参与度
    MetricData getCompletionRate();      // 作业完成率
    MetricData getAchievementRate();     // 目标达成率

    // 获取成绩分布
    GradeDistribution getGradeDistribution();

    // 获取趋势数据 (近30天)
    QVector<TrendPoint> getParticipationTrend();
    QVector<TrendPoint> getCompletionTrend();

    // 刷新数据 (重新生成随机数据)
    void refreshData();

    // 获取摘要文本 (用于 AI 分析)
    QString getDataSummary();

signals:
    void dataRefreshed();

private:
    explicit AnalyticsDataService(QObject *parent = nullptr);
    ~AnalyticsDataService() = default;

    void generateMockData();
    double randomInRange(double min, double max);

    // 缓存的数据
    MetricData m_participation;
    MetricData m_completion;
    MetricData m_achievement;
    GradeDistribution m_gradeDistribution;
    QVector<TrendPoint> m_participationTrend;
    QVector<TrendPoint> m_completionTrend;

    static AnalyticsDataService* s_instance;
};

#endif // ANALYTICSDATASERVICE_H
