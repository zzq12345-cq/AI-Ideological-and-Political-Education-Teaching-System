#include "MockDataSource.h"
#include <QDateTime>

MockDataSource::MockDataSource()
{
    // 初始化知识点名称
    m_knowledgePointNames = {
        "马克思主义基本原理",
        "中国特色社会主义理论",
        "习近平新时代中国特色社会主义思想",
        "党史学习教育",
        "社会主义核心价值观",
        "中华优秀传统文化",
        "时政热点分析"
    };

    generateMockData();
}

void MockDataSource::generateMockData()
{
    m_students.clear();
    m_classes.clear();
    m_scores.clear();

    // 生成3个班级
    QStringList classNames = {"初二1班", "初二2班", "初二3班"};
    for (int i = 0; i < 3; ++i) {
        CourseClass cls;
        cls.setId(i + 1);
        cls.setName(classNames[i]);
        cls.setGrade("初二");
        cls.setTeacherId("teacher_001");
        cls.setStudentCount(30);
        m_classes.append(cls);
    }

    // 为每个班级生成30个学生
    QStringList surnames = {"张", "李", "王", "刘", "陈", "杨", "赵", "黄", "周", "吴",
                            "徐", "孙", "胡", "朱", "高", "林", "何", "郭", "马", "罗"};
    QStringList names = {"伟", "芳", "娜", "秀英", "敏", "静", "丽", "强", "磊", "洋",
                         "勇", "艳", "杰", "涛", "明", "军", "鹏", "辉", "华", "平"};

    int studentId = 1;
    for (int classIdx = 0; classIdx < 3; ++classIdx) {
        for (int i = 0; i < 30; ++i) {
            Student student;
            student.setId(studentId);
            student.setName(surnames[QRandomGenerator::global()->bounded(surnames.size())] +
                           names[QRandomGenerator::global()->bounded(names.size())]);
            student.setClassId(classIdx + 1);
            student.setStudentNo(QString("2024%1%2").arg(classIdx + 1, 2, 10, QChar('0'))
                                                     .arg(i + 1, 2, 10, QChar('0')));
            m_students.append(student);

            // 为每个学生生成10次考试成绩
            for (int examIdx = 0; examIdx < 10; ++examIdx) {
                ScoreRecord record;
                record.setId(m_scores.size() + 1);
                record.setStudentId(studentId);
                record.setSubject("思想政治");
                record.setScore(randomScore());
                record.setDate(QDate::currentDate().addDays(-examIdx * 7));
                record.setKnowledgePoint(m_knowledgePointNames[QRandomGenerator::global()->bounded(m_knowledgePointNames.size())]);
                record.setExamType(static_cast<ScoreRecord::ExamType>(QRandomGenerator::global()->bounded(4)));
                m_scores.append(record);
            }

            studentId++;
        }
    }
}

double MockDataSource::randomScore()
{
    // 生成正态分布的成绩，均值75，标准差15
    double u1 = QRandomGenerator::global()->generateDouble();
    double u2 = QRandomGenerator::global()->generateDouble();
    double z = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
    double score = 75.0 + z * 15.0;
    return qBound(30.0, score, 100.0);
}

double MockDataSource::randomMasteryRate()
{
    return 40.0 + QRandomGenerator::global()->generateDouble() * 55.0;
}

QVector<Student> MockDataSource::getStudentList(int classId)
{
    if (classId < 0) {
        return m_students;
    }
    QVector<Student> result;
    for (const auto &student : m_students) {
        if (student.classId() == classId) {
            result.append(student);
        }
    }
    return result;
}

Student MockDataSource::getStudent(int studentId)
{
    for (const auto &student : m_students) {
        if (student.id() == studentId) {
            return student;
        }
    }
    return Student();
}

QVector<CourseClass> MockDataSource::getClassList()
{
    return m_classes;
}

CourseClass MockDataSource::getClass(int classId)
{
    for (const auto &cls : m_classes) {
        if (cls.id() == classId) {
            return cls;
        }
    }
    return CourseClass();
}

QVector<ScoreRecord> MockDataSource::getStudentScores(int studentId,
                                                       const QDate &startDate,
                                                       const QDate &endDate)
{
    QVector<ScoreRecord> result;
    for (const auto &record : m_scores) {
        if (record.studentId() != studentId) continue;
        if (startDate.isValid() && record.date() < startDate) continue;
        if (endDate.isValid() && record.date() > endDate) continue;
        result.append(record);
    }
    // 按日期排序
    std::sort(result.begin(), result.end(), [](const ScoreRecord &a, const ScoreRecord &b) {
        return a.date() < b.date();
    });
    return result;
}

QVector<ScoreRecord> MockDataSource::getClassScores(int classId,
                                                     const QDate &startDate,
                                                     const QDate &endDate)
{
    QVector<ScoreRecord> result;
    QSet<int> classStudentIds;
    for (const auto &student : m_students) {
        if (student.classId() == classId) {
            classStudentIds.insert(student.id());
        }
    }

    for (const auto &record : m_scores) {
        if (!classStudentIds.contains(record.studentId())) continue;
        if (startDate.isValid() && record.date() < startDate) continue;
        if (endDate.isValid() && record.date() > endDate) continue;
        result.append(record);
    }
    return result;
}

QVector<KnowledgePoint> MockDataSource::getStudentKnowledgePoints(int studentId)
{
    Q_UNUSED(studentId)
    QVector<KnowledgePoint> result;
    for (const QString &name : m_knowledgePointNames) {
        KnowledgePoint kp(name, randomMasteryRate());
        kp.setQuestionCount(QRandomGenerator::global()->bounded(5, 20));
        kp.setCategory("思想政治");
        result.append(kp);
    }
    return result;
}

QVector<KnowledgePoint> MockDataSource::getClassKnowledgePoints(int classId)
{
    Q_UNUSED(classId)
    QVector<KnowledgePoint> result;
    for (const QString &name : m_knowledgePointNames) {
        KnowledgePoint kp(name, randomMasteryRate());
        kp.setQuestionCount(QRandomGenerator::global()->bounded(20, 100));
        kp.setCategory("思想政治");
        result.append(kp);
    }
    // 按掌握率排序（从低到高，便于显示薄弱知识点）
    std::sort(result.begin(), result.end(), [](const KnowledgePoint &a, const KnowledgePoint &b) {
        return a.masteryRate() < b.masteryRate();
    });
    return result;
}

ClassStatistics MockDataSource::getClassStatistics(int classId)
{
    auto scores = getClassScores(classId);
    ClassStatistics stats;
    stats.setClassId(classId);

    if (scores.isEmpty()) {
        return stats;
    }

    // 计算最近一次考试的统计数据
    QDate latestDate;
    for (const auto &record : scores) {
        if (!latestDate.isValid() || record.date() > latestDate) {
            latestDate = record.date();
        }
    }

    QVector<double> latestScores;
    for (const auto &record : scores) {
        if (record.date() == latestDate) {
            latestScores.append(record.score());
        }
    }

    if (latestScores.isEmpty()) {
        return stats;
    }

    stats.setTotalStudents(latestScores.size());

    double sum = 0, highest = 0, lowest = 100;
    int excellent = 0, good = 0, pass = 0, fail = 0;

    for (double score : latestScores) {
        sum += score;
        highest = qMax(highest, score);
        lowest = qMin(lowest, score);

        if (score >= 90) excellent++;
        else if (score >= 80) good++;
        else if (score >= 60) pass++;
        else fail++;
    }

    stats.setAverageScore(sum / latestScores.size());
    stats.setHighestScore(highest);
    stats.setLowestScore(lowest);
    stats.setExcellentCount(excellent);
    stats.setGoodCount(good);
    stats.setPassCount(pass);
    stats.setFailCount(fail);

    return stats;
}

QVector<QPair<Student, double>> MockDataSource::getClassRanking(int classId)
{
    QVector<QPair<Student, double>> result;
    auto students = getStudentList(classId);

    for (const auto &student : students) {
        auto scores = getStudentScores(student.id());
        double avgScore = 0;
        if (!scores.isEmpty()) {
            double sum = 0;
            for (const auto &record : scores) {
                sum += record.score();
            }
            avgScore = sum / scores.size();
        }
        result.append(qMakePair(student, avgScore));
    }

    // 按成绩降序排序
    std::sort(result.begin(), result.end(),
              [](const QPair<Student, double> &a, const QPair<Student, double> &b) {
                  return a.second > b.second;
              });

    return result;
}

void MockDataSource::refreshData()
{
    generateMockData();
}
