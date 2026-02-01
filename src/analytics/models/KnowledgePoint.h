#ifndef KNOWLEDGEPOINT_H
#define KNOWLEDGEPOINT_H

#include <QString>
#include <QJsonObject>

/**
 * @brief 知识点掌握度模型
 *
 * 存储学生/班级对某个知识点的掌握程度
 * 老王说：知识点掌握度就是个百分比，别想太复杂
 */
class KnowledgePoint
{
public:
    KnowledgePoint();
    KnowledgePoint(const QString &name, double masteryRate);

    // 从JSON构造
    static KnowledgePoint fromJson(const QJsonObject &json);

    // 转换为JSON
    QJsonObject toJson() const;

    // Getters
    int id() const { return m_id; }
    QString name() const { return m_name; }
    double masteryRate() const { return m_masteryRate; }
    int questionCount() const { return m_questionCount; }
    QString category() const { return m_category; }

    // Setters
    void setId(int id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setMasteryRate(double rate) { m_masteryRate = rate; }
    void setQuestionCount(int count) { m_questionCount = count; }
    void setCategory(const QString &category) { m_category = category; }

    // 获取掌握等级描述
    QString masteryLevel() const;

    // 获取掌握等级颜色（用于UI显示）
    QString masteryColor() const;

    // 是否为薄弱知识点（掌握率<60%）
    bool isWeak() const { return m_masteryRate < 60.0; }

private:
    int m_id = 0;
    QString m_name;            // 知识点名称
    double m_masteryRate = 0;  // 掌握率 (0-100)
    int m_questionCount = 0;   // 相关题目数量
    QString m_category;        // 分类，如"马克思主义基本原理"
};

#endif // KNOWLEDGEPOINT_H
