#ifndef COURSECLASS_H
#define COURSECLASS_H

#include <QString>
#include <QJsonObject>

/**
 * @brief 课程班级模型
 *
 * 存储班级信息，用于班级整体分析
 * 老王说：班级就是一群学生的集合，简单明了
 */
class CourseClass
{
public:
    CourseClass();
    CourseClass(int id, const QString &name, const QString &teacherId);

    // 从JSON构造
    static CourseClass fromJson(const QJsonObject &json);

    // 转换为JSON
    QJsonObject toJson() const;

    // Getters
    int id() const { return m_id; }
    QString name() const { return m_name; }
    QString teacherId() const { return m_teacherId; }
    int studentCount() const { return m_studentCount; }
    QString grade() const { return m_grade; }

    // Setters
    void setId(int id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setTeacherId(const QString &teacherId) { m_teacherId = teacherId; }
    void setStudentCount(int count) { m_studentCount = count; }
    void setGrade(const QString &grade) { m_grade = grade; }

    // 获取显示名称（年级 + 班级名）
    QString displayName() const;

private:
    int m_id = 0;
    QString m_name;          // 班级名称，如"初二1班"
    QString m_teacherId;     // 班主任ID
    int m_studentCount = 0;  // 学生人数
    QString m_grade;         // 年级，如"初二"
};

#endif // COURSECLASS_H
