#ifndef STUDENT_H
#define STUDENT_H

#include <QObject>
#include <QString>
#include <QJsonObject>

/**
 * @brief 学生模型
 *
 * 存储学生基本信息，用于个人学情分析
 * 老王说：一个学生就是一条数据，别搞那么复杂
 */
class Student
{
public:
    Student();
    Student(int id, const QString &name, int classId, const QString &studentNo);

    // 从JSON构造
    static Student fromJson(const QJsonObject &json);

    // 转换为JSON
    QJsonObject toJson() const;

    // Getters
    int id() const { return m_id; }
    QString name() const { return m_name; }
    int classId() const { return m_classId; }
    QString studentNo() const { return m_studentNo; }
    QString avatar() const { return m_avatar; }

    // Setters
    void setId(int id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setClassId(int classId) { m_classId = classId; }
    void setStudentNo(const QString &studentNo) { m_studentNo = studentNo; }
    void setAvatar(const QString &avatar) { m_avatar = avatar; }

    // 获取显示名称（姓名 + 学号）
    QString displayName() const;

    // 获取头像首字（用于默认头像显示）
    QString avatarInitial() const;

private:
    int m_id = 0;
    QString m_name;
    int m_classId = 0;
    QString m_studentNo;
    QString m_avatar;  // 头像URL，可选
};

#endif // STUDENT_H
