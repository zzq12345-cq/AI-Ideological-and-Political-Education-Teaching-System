#include "AnalysisResult.h"

AnalysisResult::AnalysisResult()
    : m_targetId(0)
    , m_averageScore(0.0)
    , m_scoreChange(0.0)
    , m_rank(0)
    , m_totalCount(0)
{
}

double AnalysisResult::rankPercentile() const
{
    if (m_totalCount <= 0 || m_rank <= 0) {
        return 0.0;
    }
    return (static_cast<double>(m_rank) / m_totalCount) * 100.0;
}

QJsonObject AnalysisResult::toJson() const
{
    QJsonObject json;
    json["target_type"] = m_targetType;
    json["target_id"] = m_targetId;
    json["average_score"] = m_averageScore;
    json["score_change"] = m_scoreChange;
    json["rank"] = m_rank;
    json["total_count"] = m_totalCount;
    json["ai_suggestion"] = m_aiSuggestion;

    // 转换extraData
    QJsonObject extra;
    for (auto it = m_extraData.constBegin(); it != m_extraData.constEnd(); ++it) {
        if (it.value().typeId() == QMetaType::Double) {
            extra[it.key()] = it.value().toDouble();
        } else if (it.value().typeId() == QMetaType::QString) {
            extra[it.key()] = it.value().toString();
        } else if (it.value().typeId() == QMetaType::Int) {
            extra[it.key()] = it.value().toInt();
        } else if (it.value().typeId() == QMetaType::Bool) {
            extra[it.key()] = it.value().toBool();
        }
    }
    json["extra_data"] = extra;

    return json;
}
