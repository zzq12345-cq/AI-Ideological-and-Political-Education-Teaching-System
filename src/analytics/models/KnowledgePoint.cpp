#include "KnowledgePoint.h"

KnowledgePoint::KnowledgePoint()
    : m_id(0)
    , m_masteryRate(0.0)
    , m_questionCount(0)
{
}

KnowledgePoint::KnowledgePoint(const QString &name, double masteryRate)
    : m_id(0)
    , m_name(name)
    , m_masteryRate(masteryRate)
    , m_questionCount(0)
{
}

KnowledgePoint KnowledgePoint::fromJson(const QJsonObject &json)
{
    KnowledgePoint kp;
    kp.m_id = json["id"].toInt();
    kp.m_name = json["name"].toString();
    kp.m_masteryRate = json["mastery_rate"].toDouble();
    kp.m_questionCount = json["question_count"].toInt();
    kp.m_category = json["category"].toString();
    return kp;
}

QJsonObject KnowledgePoint::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["mastery_rate"] = m_masteryRate;
    json["question_count"] = m_questionCount;
    json["category"] = m_category;
    return json;
}

QString KnowledgePoint::masteryLevel() const
{
    if (m_masteryRate >= 90) return "精通";
    if (m_masteryRate >= 80) return "熟练";
    if (m_masteryRate >= 60) return "掌握";
    if (m_masteryRate >= 40) return "了解";
    return "薄弱";
}

QString KnowledgePoint::masteryColor() const
{
    if (m_masteryRate >= 90) return "#38A169";  // 绿色 - 精通
    if (m_masteryRate >= 80) return "#3182CE";  // 蓝色 - 熟练
    if (m_masteryRate >= 60) return "#DD6B20";  // 橙色 - 掌握
    if (m_masteryRate >= 40) return "#D69E2E";  // 黄色 - 了解
    return "#E53E3E";  // 红色 - 薄弱
}
