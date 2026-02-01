#include "CourseClass.h"

CourseClass::CourseClass()
    : m_id(0)
    , m_studentCount(0)
{
}

CourseClass::CourseClass(int id, const QString &name, const QString &teacherId)
    : m_id(id)
    , m_name(name)
    , m_teacherId(teacherId)
    , m_studentCount(0)
{
}

CourseClass CourseClass::fromJson(const QJsonObject &json)
{
    CourseClass cls;
    cls.m_id = json["id"].toInt();
    cls.m_name = json["name"].toString();
    cls.m_teacherId = json["teacher_id"].toString();
    cls.m_studentCount = json["student_count"].toInt();
    cls.m_grade = json["grade"].toString();
    return cls;
}

QJsonObject CourseClass::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["teacher_id"] = m_teacherId;
    json["student_count"] = m_studentCount;
    json["grade"] = m_grade;
    return json;
}

QString CourseClass::displayName() const
{
    if (m_grade.isEmpty()) {
        return m_name;
    }
    return QString("%1 %2").arg(m_grade, m_name);
}
