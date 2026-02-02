#include "CurriculumService.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>

CurriculumService* CurriculumService::s_instance = nullptr;

CurriculumService::CurriculumService(QObject *parent)
    : QObject(parent)
    , m_loaded(false)
{
}

CurriculumService::~CurriculumService()
{
}

CurriculumService* CurriculumService::instance()
{
    if (!s_instance) {
        s_instance = new CurriculumService();
    }
    return s_instance;
}

bool CurriculumService::loadCurriculum(const QString &jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[CurriculumService] 艹！打不开课程配置文件:" << jsonPath;
        emit loadError(QString("无法打开课程配置文件: %1").arg(jsonPath));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[CurriculumService] JSON解析出错:" << parseError.errorString();
        emit loadError(QString("JSON解析错误: %1").arg(parseError.errorString()));
        return false;
    }

    QJsonObject root = doc.object();
    m_metadata = root["metadata"].toObject();

    // 解析年级数据
    QJsonArray gradesArray = root["grades"].toArray();
    parseGrades(gradesArray);

    m_loaded = true;
    qInfo() << "[CurriculumService] 课程目录加载成功！"
            << "年级:" << m_grades.size()
            << "总课时:" << getTotalLessons();

    emit curriculumLoaded();
    return true;
}

void CurriculumService::parseGrades(const QJsonArray &gradesArray)
{
    m_grades.clear();

    for (const QJsonValue &gradeVal : gradesArray) {
        QJsonObject gradeObj = gradeVal.toObject();
        GradeInfo grade;
        grade.gradeId = gradeObj["grade_id"].toInt();
        grade.gradeName = gradeObj["grade"].toString();

        // 解析学期
        QJsonArray semestersArray = gradeObj["semesters"].toArray();
        for (const QJsonValue &semVal : semestersArray) {
            QJsonObject semObj = semVal.toObject();
            SemesterInfo semester;
            semester.semesterId = semObj["semester_id"].toInt();
            semester.semester = semObj["semester"].toString();

            // 解析单元
            QJsonArray unitsArray = semObj["units"].toArray();
            for (const QJsonValue &unitVal : unitsArray) {
                QJsonObject unitObj = unitVal.toObject();
                UnitInfo unit;
                unit.unitId = unitObj["unit_id"].toInt();
                unit.unitName = unitObj["unit_name"].toString();

                // 解析课时
                QJsonArray lessonsArray = unitObj["lessons"].toArray();
                for (const QJsonValue &lessonVal : lessonsArray) {
                    QJsonObject lessonObj = lessonVal.toObject();
                    LessonInfo lesson;
                    lesson.lessonId = lessonObj["lesson_id"].toInt();
                    lesson.lessonName = lessonObj["lesson_name"].toString();
                    lesson.unitName = unit.unitName;
                    lesson.gradeSemester = grade.gradeName + semester.semester;

                    // 解析小节
                    QJsonArray sectionsArray = lessonObj["sections"].toArray();
                    for (const QJsonValue &secVal : sectionsArray) {
                        lesson.sections.append(secVal.toString());
                    }

                    unit.lessons.append(lesson);
                }

                semester.units.append(unit);
            }

            grade.semesters.append(semester);
        }

        m_grades.append(grade);
    }
}

QStringList CurriculumService::getGrades() const
{
    QStringList result;
    for (const GradeInfo &grade : m_grades) {
        result.append(grade.gradeName);
    }
    return result;
}

QStringList CurriculumService::getSemesters(const QString &grade) const
{
    QStringList result;
    const GradeInfo* gradeInfo = findGrade(grade);
    if (gradeInfo) {
        for (const SemesterInfo &sem : gradeInfo->semesters) {
            result.append(sem.semester);
        }
    }
    return result;
}

QStringList CurriculumService::getUnits(const QString &grade, const QString &semester) const
{
    QStringList result;
    const GradeInfo* gradeInfo = findGrade(grade);
    if (gradeInfo) {
        for (const SemesterInfo &sem : gradeInfo->semesters) {
            if (sem.semester == semester) {
                for (const UnitInfo &unit : sem.units) {
                    result.append(unit.unitName);
                }
                break;
            }
        }
    }
    return result;
}

QStringList CurriculumService::getLessons(const QString &grade, const QString &semester,
                                          const QString &unit) const
{
    QStringList result;
    const GradeInfo* gradeInfo = findGrade(grade);
    if (gradeInfo) {
        for (const SemesterInfo &sem : gradeInfo->semesters) {
            if (sem.semester == semester) {
                for (const UnitInfo &u : sem.units) {
                    if (u.unitName == unit) {
                        for (const LessonInfo &lesson : u.lessons) {
                            result.append(lesson.lessonName);
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    return result;
}

CurriculumService::LessonInfo CurriculumService::getLessonInfo(
    const QString &grade, const QString &semester,
    const QString &unit, const QString &lesson) const
{
    LessonInfo result;
    const GradeInfo* gradeInfo = findGrade(grade);
    if (gradeInfo) {
        for (const SemesterInfo &sem : gradeInfo->semesters) {
            if (sem.semester == semester) {
                for (const UnitInfo &u : sem.units) {
                    if (u.unitName == unit) {
                        for (const LessonInfo &l : u.lessons) {
                            if (l.lessonName == lesson) {
                                return l;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

QStringList CurriculumService::getSections(const QString &grade, const QString &semester,
                                           const QString &unit, const QString &lesson) const
{
    LessonInfo info = getLessonInfo(grade, semester, unit, lesson);
    return info.sections;
}

QString CurriculumService::buildLessonTitle(const QString &grade, const QString &semester,
                                            const QString &unit, const QString &lesson) const
{
    // 格式: "七年级上册 第一单元 成长的节拍 - 第一课 中学时代"
    return QString("%1%2 %3 - %4")
        .arg(grade)
        .arg(semester)
        .arg(unit)
        .arg(lesson);
}

int CurriculumService::getTotalGrades() const
{
    return m_grades.size();
}

int CurriculumService::getTotalUnits() const
{
    int count = 0;
    for (const GradeInfo &grade : m_grades) {
        for (const SemesterInfo &sem : grade.semesters) {
            count += sem.units.size();
        }
    }
    return count;
}

int CurriculumService::getTotalLessons() const
{
    int count = 0;
    for (const GradeInfo &grade : m_grades) {
        for (const SemesterInfo &sem : grade.semesters) {
            for (const UnitInfo &unit : sem.units) {
                count += unit.lessons.size();
            }
        }
    }
    return count;
}

QList<CurriculumService::LessonInfo> CurriculumService::searchLessons(const QString &keyword) const
{
    QList<LessonInfo> result;
    QString lowerKeyword = keyword.toLower();

    for (const GradeInfo &grade : m_grades) {
        for (const SemesterInfo &sem : grade.semesters) {
            for (const UnitInfo &unit : sem.units) {
                for (const LessonInfo &lesson : unit.lessons) {
                    // 匹配课时名称
                    if (lesson.lessonName.toLower().contains(lowerKeyword)) {
                        result.append(lesson);
                        continue;
                    }
                    // 匹配单元名称
                    if (lesson.unitName.toLower().contains(lowerKeyword)) {
                        result.append(lesson);
                        continue;
                    }
                    // 匹配小节名称
                    for (const QString &sec : lesson.sections) {
                        if (sec.toLower().contains(lowerKeyword)) {
                            result.append(lesson);
                            break;
                        }
                    }
                }
            }
        }
    }

    return result;
}

CurriculumService::GradeInfo* CurriculumService::findGrade(const QString &gradeName)
{
    for (int i = 0; i < m_grades.size(); ++i) {
        if (m_grades[i].gradeName == gradeName) {
            return &m_grades[i];
        }
    }
    return nullptr;
}

const CurriculumService::GradeInfo* CurriculumService::findGrade(const QString &gradeName) const
{
    for (int i = 0; i < m_grades.size(); ++i) {
        if (m_grades[i].gradeName == gradeName) {
            return &m_grades[i];
        }
    }
    return nullptr;
}
