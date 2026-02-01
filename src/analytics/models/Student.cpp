#include "Student.h"
#include <QJsonDocument>

Student::Student()
    : m_id(0)
    , m_classId(0)
{
}

Student::Student(int id, const QString &name, int classId, const QString &studentNo)
    : m_id(id)
    , m_name(name)
    , m_classId(classId)
    , m_studentNo(studentNo)
{
}

Student Student::fromJson(const QJsonObject &json)
{
    Student student;
    student.m_id = json["id"].toInt();
    student.m_name = json["name"].toString();
    student.m_classId = json["class_id"].toInt();
    student.m_studentNo = json["student_no"].toString();
    student.m_avatar = json["avatar"].toString();
    return student;
}

QJsonObject Student::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["class_id"] = m_classId;
    json["student_no"] = m_studentNo;
    if (!m_avatar.isEmpty()) {
        json["avatar"] = m_avatar;
    }
    return json;
}

QString Student::displayName() const
{
    if (m_studentNo.isEmpty()) {
        return m_name;
    }
    return QString("%1 (%2)").arg(m_name, m_studentNo);
}

QString Student::avatarInitial() const
{
    if (m_name.isEmpty()) {
        return "?";
    }
    return m_name.left(1);
}
