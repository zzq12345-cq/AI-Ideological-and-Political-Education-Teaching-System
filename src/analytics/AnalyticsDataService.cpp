#include "AnalyticsDataService.h"
#include <QDebug>

AnalyticsDataService* AnalyticsDataService::s_instance = nullptr;

AnalyticsDataService* AnalyticsDataService::instance()
{
    if (!s_instance) {
        s_instance = new AnalyticsDataService();
    }
    return s_instance;
}

AnalyticsDataService::AnalyticsDataService(QObject *parent)
    : QObject(parent)
{
    generateMockData();
}

void AnalyticsDataService::generateMockData()
{
    qDebug() << "[AnalyticsDataService] 生成模拟教学数据...";

    // 课堂参与度 75-95%
    m_participation.value = randomInRange(75.0, 95.0);
    m_participation.change = randomInRange(-5.0, 8.0);
    m_participation.isPositive = m_participation.change >= 0;

    // 作业完成率 80-98%
    m_completion.value = randomInRange(80.0, 98.0);
    m_completion.change = randomInRange(-3.0, 5.0);
    m_completion.isPositive = m_completion.change >= 0;

    // 目标达成率 65-90%
    m_achievement.value = randomInRange(65.0, 90.0);
    m_achievement.change = randomInRange(-4.0, 10.0);
    m_achievement.isPositive = m_achievement.change >= 0;

    // 成绩分布 (总数100人)
    int total = 100;
    m_gradeDistribution.excellent = QRandomGenerator::global()->bounded(15, 30);
    m_gradeDistribution.good = QRandomGenerator::global()->bounded(25, 40);
    m_gradeDistribution.fail = QRandomGenerator::global()->bounded(3, 12);
    m_gradeDistribution.pass = total - m_gradeDistribution.excellent
                                     - m_gradeDistribution.good
                                     - m_gradeDistribution.fail;

    // 生成30天趋势数据
    m_participationTrend.clear();
    m_completionTrend.clear();

    QDate today = QDate::currentDate();
    double baseParticipation = randomInRange(70.0, 80.0);
    double baseCompletion = randomInRange(85.0, 90.0);

    for (int i = 29; i >= 0; --i) {
        QDate date = today.addDays(-i);

        // 参与度趋势 - 带波动的上升趋势
        TrendPoint pPoint;
        pPoint.date = date;
        pPoint.value = baseParticipation + (29 - i) * 0.3 + randomInRange(-5.0, 5.0);
        pPoint.value = qBound(60.0, pPoint.value, 100.0);
        m_participationTrend.append(pPoint);

        // 完成率趋势 - 相对稳定
        TrendPoint cPoint;
        cPoint.date = date;
        cPoint.value = baseCompletion + randomInRange(-3.0, 3.0);
        cPoint.value = qBound(75.0, cPoint.value, 100.0);
        m_completionTrend.append(cPoint);
    }

    qDebug() << "[AnalyticsDataService] 数据生成完成:"
             << "参与度=" << QString::number(m_participation.value, 'f', 1) << "%"
             << "完成率=" << QString::number(m_completion.value, 'f', 1) << "%"
             << "达标率=" << QString::number(m_achievement.value, 'f', 1) << "%";
}

double AnalyticsDataService::randomInRange(double min, double max)
{
    return min + QRandomGenerator::global()->generateDouble() * (max - min);
}

AnalyticsDataService::MetricData AnalyticsDataService::getParticipationRate()
{
    return m_participation;
}

AnalyticsDataService::MetricData AnalyticsDataService::getCompletionRate()
{
    return m_completion;
}

AnalyticsDataService::MetricData AnalyticsDataService::getAchievementRate()
{
    return m_achievement;
}

AnalyticsDataService::GradeDistribution AnalyticsDataService::getGradeDistribution()
{
    return m_gradeDistribution;
}

QVector<AnalyticsDataService::TrendPoint> AnalyticsDataService::getParticipationTrend()
{
    return m_participationTrend;
}

QVector<AnalyticsDataService::TrendPoint> AnalyticsDataService::getCompletionTrend()
{
    return m_completionTrend;
}

void AnalyticsDataService::refreshData()
{
    generateMockData();
    emit dataRefreshed();
}

QString AnalyticsDataService::getDataSummary()
{
    return QString(
        "【课堂参与度】%1%（较上周 %2%3%）\n"
        "【作业完成率】%4%（较上周 %5%6%）\n"
        "【目标达成率】%7%（较上周 %8%9%）\n\n"
        "【成绩分布】\n"
        "- 优秀(90-100分): %10人\n"
        "- 良好(80-89分): %11人\n"
        "- 及格(60-79分): %12人\n"
        "- 不及格(<60分): %13人"
    ).arg(QString::number(m_participation.value, 'f', 1))
     .arg(m_participation.isPositive ? "+" : "")
     .arg(QString::number(m_participation.change, 'f', 1))
     .arg(QString::number(m_completion.value, 'f', 1))
     .arg(m_completion.isPositive ? "+" : "")
     .arg(QString::number(m_completion.change, 'f', 1))
     .arg(QString::number(m_achievement.value, 'f', 1))
     .arg(m_achievement.isPositive ? "+" : "")
     .arg(QString::number(m_achievement.change, 'f', 1))
     .arg(m_gradeDistribution.excellent)
     .arg(m_gradeDistribution.good)
     .arg(m_gradeDistribution.pass)
     .arg(m_gradeDistribution.fail);
}
