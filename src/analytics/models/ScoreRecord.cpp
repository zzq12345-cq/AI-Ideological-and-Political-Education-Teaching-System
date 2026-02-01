#include "ScoreRecord.h"

ScoreRecord::ScoreRecord()
    : m_id(0)
    , m_studentId(0)
    , m_score(0.0)
    , m_examType(Daily)
    , m_fullScore(100.0)
{
}

ScoreRecord::ScoreRecord(int studentId, const QString &subject, double score, const QDate &date)
    : m_id(0)
    , m_studentId(studentId)
    , m_subject(subject)
    , m_score(score)
    , m_date(date)
    , m_examType(Daily)
    , m_fullScore(100.0)
{
}

ScoreRecord ScoreRecord::fromJson(const QJsonObject &json)
{
    ScoreRecord record;
    record.m_id = json["id"].toInt();
    record.m_studentId = json["student_id"].toInt();
    record.m_subject = json["subject"].toString();
    record.m_score = json["score"].toDouble();
    record.m_date = QDate::fromString(json["date"].toString(), Qt::ISODate);
    record.m_knowledgePoint = json["knowledge_point"].toString();
    record.m_examType = static_cast<ExamType>(json["exam_type"].toInt());
    record.m_fullScore = json["full_score"].toDouble(100.0);
    return record;
}

QJsonObject ScoreRecord::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["student_id"] = m_studentId;
    json["subject"] = m_subject;
    json["score"] = m_score;
    json["date"] = m_date.toString(Qt::ISODate);
    json["knowledge_point"] = m_knowledgePoint;
    json["exam_type"] = static_cast<int>(m_examType);
    json["full_score"] = m_fullScore;
    return json;
}

double ScoreRecord::scoreRate() const
{
    if (m_fullScore <= 0) {
        return 0.0;
    }
    return (m_score / m_fullScore) * 100.0;
}

QString ScoreRecord::examTypeName() const
{
    switch (m_examType) {
    case Daily:   return "日常测验";
    case MidTerm: return "期中考试";
    case Final:   return "期末考试";
    case Quiz:    return "随堂测试";
    default:      return "其他";
    }
}

QString ScoreRecord::gradeLevel() const
{
    double rate = scoreRate();
    if (rate >= 90) return "优秀";
    if (rate >= 80) return "良好";
    if (rate >= 60) return "及格";
    return "不及格";
}
