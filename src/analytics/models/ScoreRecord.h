#ifndef SCORERECORD_H
#define SCORERECORD_H

#include <QString>
#include <QDate>
#include <QJsonObject>

/**
 * @brief 成绩记录模型
 *
 * 存储单次考试/测验的成绩记录
 * 老王说：一次考试一条记录，多简单
 */
class ScoreRecord
{
public:
    // 考试类型枚举
    enum ExamType {
        Daily = 0,      // 日常测验
        MidTerm = 1,    // 期中考试
        Final = 2,      // 期末考试
        Quiz = 3        // 随堂测试
    };

    ScoreRecord();
    ScoreRecord(int studentId, const QString &subject, double score, const QDate &date);

    // 从JSON构造
    static ScoreRecord fromJson(const QJsonObject &json);

    // 转换为JSON
    QJsonObject toJson() const;

    // Getters
    int id() const { return m_id; }
    int studentId() const { return m_studentId; }
    QString subject() const { return m_subject; }
    double score() const { return m_score; }
    QDate date() const { return m_date; }
    QString knowledgePoint() const { return m_knowledgePoint; }
    ExamType examType() const { return m_examType; }
    double fullScore() const { return m_fullScore; }

    // Setters
    void setId(int id) { m_id = id; }
    void setStudentId(int studentId) { m_studentId = studentId; }
    void setSubject(const QString &subject) { m_subject = subject; }
    void setScore(double score) { m_score = score; }
    void setDate(const QDate &date) { m_date = date; }
    void setKnowledgePoint(const QString &kp) { m_knowledgePoint = kp; }
    void setExamType(ExamType type) { m_examType = type; }
    void setFullScore(double fullScore) { m_fullScore = fullScore; }

    // 计算得分率 (0-100)
    double scoreRate() const;

    // 获取考试类型名称
    QString examTypeName() const;

    // 获取成绩等级 (优秀/良好/及格/不及格)
    QString gradeLevel() const;

private:
    int m_id = 0;
    int m_studentId = 0;
    QString m_subject;        // 科目
    double m_score = 0.0;     // 得分
    QDate m_date;             // 考试日期
    QString m_knowledgePoint; // 关联知识点
    ExamType m_examType = Daily;
    double m_fullScore = 100.0;  // 满分
};

#endif // SCORERECORD_H
