#ifndef CLASSSTATISTICS_H
#define CLASSSTATISTICS_H

#include <QString>
#include <QJsonObject>

/**
 * @brief 班级统计数据模型
 *
 * 存储班级整体的统计信息
 * 老王说：班级统计就是把一堆数字汇总起来
 */
class ClassStatistics
{
public:
    ClassStatistics();

    // Getters
    int classId() const { return m_classId; }
    int totalStudents() const { return m_totalStudents; }
    double averageScore() const { return m_averageScore; }
    double highestScore() const { return m_highestScore; }
    double lowestScore() const { return m_lowestScore; }
    int excellentCount() const { return m_excellentCount; }  // 优秀人数
    int goodCount() const { return m_goodCount; }            // 良好人数
    int passCount() const { return m_passCount; }            // 及格人数
    int failCount() const { return m_failCount; }            // 不及格人数
    double passRate() const;                                 // 及格率

    // Setters
    void setClassId(int id) { m_classId = id; }
    void setTotalStudents(int count) { m_totalStudents = count; }
    void setAverageScore(double score) { m_averageScore = score; }
    void setHighestScore(double score) { m_highestScore = score; }
    void setLowestScore(double score) { m_lowestScore = score; }
    void setExcellentCount(int count) { m_excellentCount = count; }
    void setGoodCount(int count) { m_goodCount = count; }
    void setPassCount(int count) { m_passCount = count; }
    void setFailCount(int count) { m_failCount = count; }

    // 转换为JSON
    QJsonObject toJson() const;

    // 从JSON构造
    static ClassStatistics fromJson(const QJsonObject &json);

private:
    int m_classId = 0;
    int m_totalStudents = 0;
    double m_averageScore = 0.0;
    double m_highestScore = 0.0;
    double m_lowestScore = 0.0;
    int m_excellentCount = 0;  // 90+
    int m_goodCount = 0;       // 80-89
    int m_passCount = 0;       // 60-79
    int m_failCount = 0;       // <60
};

#endif // CLASSSTATISTICS_H
