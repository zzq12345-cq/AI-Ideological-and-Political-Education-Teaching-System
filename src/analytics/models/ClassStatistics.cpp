#include "ClassStatistics.h"

ClassStatistics::ClassStatistics()
    : m_classId(0)
    , m_totalStudents(0)
    , m_averageScore(0.0)
    , m_highestScore(0.0)
    , m_lowestScore(0.0)
    , m_excellentCount(0)
    , m_goodCount(0)
    , m_passCount(0)
    , m_failCount(0)
{
}

double ClassStatistics::passRate() const
{
    if (m_totalStudents <= 0) {
        return 0.0;
    }
    int passedStudents = m_excellentCount + m_goodCount + m_passCount;
    return (static_cast<double>(passedStudents) / m_totalStudents) * 100.0;
}

QJsonObject ClassStatistics::toJson() const
{
    QJsonObject json;
    json["class_id"] = m_classId;
    json["total_students"] = m_totalStudents;
    json["average_score"] = m_averageScore;
    json["highest_score"] = m_highestScore;
    json["lowest_score"] = m_lowestScore;
    json["excellent_count"] = m_excellentCount;
    json["good_count"] = m_goodCount;
    json["pass_count"] = m_passCount;
    json["fail_count"] = m_failCount;
    json["pass_rate"] = passRate();
    return json;
}

ClassStatistics ClassStatistics::fromJson(const QJsonObject &json)
{
    ClassStatistics stats;
    stats.m_classId = json["class_id"].toInt();
    stats.m_totalStudents = json["total_students"].toInt();
    stats.m_averageScore = json["average_score"].toDouble();
    stats.m_highestScore = json["highest_score"].toDouble();
    stats.m_lowestScore = json["lowest_score"].toDouble();
    stats.m_excellentCount = json["excellent_count"].toInt();
    stats.m_goodCount = json["good_count"].toInt();
    stats.m_passCount = json["pass_count"].toInt();
    stats.m_failCount = json["fail_count"].toInt();
    return stats;
}
