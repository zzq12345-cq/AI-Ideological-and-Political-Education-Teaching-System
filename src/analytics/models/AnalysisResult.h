#ifndef ANALYSISRESULT_H
#define ANALYSISRESULT_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QVariantMap>

/**
 * @brief 分析结果汇总模型
 *
 * 存储个人或班级的分析结果摘要
 * 老王说：把分析结果打包好，方便传来传去
 */
class AnalysisResult
{
public:
    AnalysisResult();

    // Getters
    QString targetType() const { return m_targetType; }  // "student" 或 "class"
    int targetId() const { return m_targetId; }
    double averageScore() const { return m_averageScore; }
    double scoreChange() const { return m_scoreChange; }  // 较上期变化
    int rank() const { return m_rank; }
    int totalCount() const { return m_totalCount; }  // 总人数（用于计算排名百分比）
    QString aiSuggestion() const { return m_aiSuggestion; }
    QVariantMap extraData() const { return m_extraData; }

    // Setters
    void setTargetType(const QString &type) { m_targetType = type; }
    void setTargetId(int id) { m_targetId = id; }
    void setAverageScore(double score) { m_averageScore = score; }
    void setScoreChange(double change) { m_scoreChange = change; }
    void setRank(int rank) { m_rank = rank; }
    void setTotalCount(int count) { m_totalCount = count; }
    void setAiSuggestion(const QString &suggestion) { m_aiSuggestion = suggestion; }
    void setExtraData(const QVariantMap &data) { m_extraData = data; }

    // 计算排名百分比（前X%）
    double rankPercentile() const;

    // 是否进步（scoreChange > 0）
    bool isImproved() const { return m_scoreChange > 0; }

    // 转换为JSON
    QJsonObject toJson() const;

private:
    QString m_targetType;     // "student" 或 "class"
    int m_targetId = 0;
    double m_averageScore = 0.0;
    double m_scoreChange = 0.0;
    int m_rank = 0;
    int m_totalCount = 0;
    QString m_aiSuggestion;
    QVariantMap m_extraData;  // 扩展数据
};

#endif // ANALYSISRESULT_H
