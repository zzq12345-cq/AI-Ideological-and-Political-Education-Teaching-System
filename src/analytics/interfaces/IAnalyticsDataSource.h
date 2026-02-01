#ifndef IANALYTICSDATASOURCE_H
#define IANALYTICSDATASOURCE_H

#include <QVector>
#include <QDate>
#include "../models/Student.h"
#include "../models/CourseClass.h"
#include "../models/ScoreRecord.h"
#include "../models/KnowledgePoint.h"
#include "../models/ClassStatistics.h"

/**
 * @brief 数据源抽象接口
 *
 * 定义数据获取的标准接口，支持Mock和真实数据源
 * 老王说：接口就是个约定，实现类爱怎么搞怎么搞
 */
class IAnalyticsDataSource
{
public:
    virtual ~IAnalyticsDataSource() = default;

    // 学生相关
    virtual QVector<Student> getStudentList(int classId = -1) = 0;
    virtual Student getStudent(int studentId) = 0;

    // 班级相关
    virtual QVector<CourseClass> getClassList() = 0;
    virtual CourseClass getClass(int classId) = 0;

    // 成绩记录
    virtual QVector<ScoreRecord> getStudentScores(int studentId,
                                                   const QDate &startDate = QDate(),
                                                   const QDate &endDate = QDate()) = 0;
    virtual QVector<ScoreRecord> getClassScores(int classId,
                                                 const QDate &startDate = QDate(),
                                                 const QDate &endDate = QDate()) = 0;

    // 知识点掌握度
    virtual QVector<KnowledgePoint> getStudentKnowledgePoints(int studentId) = 0;
    virtual QVector<KnowledgePoint> getClassKnowledgePoints(int classId) = 0;

    // 统计数据
    virtual ClassStatistics getClassStatistics(int classId) = 0;

    // 排名数据 (返回按成绩排序的学生列表)
    virtual QVector<QPair<Student, double>> getClassRanking(int classId) = 0;
};

#endif // IANALYTICSDATASOURCE_H
