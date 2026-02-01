#ifndef MOCKDATASOURCE_H
#define MOCKDATASOURCE_H

#include "../interfaces/IAnalyticsDataSource.h"
#include <QRandomGenerator>

/**
 * @brief 模拟数据源
 *
 * 生成模拟的学生、班级、成绩数据用于开发测试
 * 老王说：先用假数据把UI搞好，真数据以后再接
 */
class MockDataSource : public IAnalyticsDataSource
{
public:
    MockDataSource();

    // 实现接口方法
    QVector<Student> getStudentList(int classId = -1) override;
    Student getStudent(int studentId) override;

    QVector<CourseClass> getClassList() override;
    CourseClass getClass(int classId) override;

    QVector<ScoreRecord> getStudentScores(int studentId,
                                           const QDate &startDate = QDate(),
                                           const QDate &endDate = QDate()) override;
    QVector<ScoreRecord> getClassScores(int classId,
                                         const QDate &startDate = QDate(),
                                         const QDate &endDate = QDate()) override;

    QVector<KnowledgePoint> getStudentKnowledgePoints(int studentId) override;
    QVector<KnowledgePoint> getClassKnowledgePoints(int classId) override;

    ClassStatistics getClassStatistics(int classId) override;

    QVector<QPair<Student, double>> getClassRanking(int classId) override;

    // 刷新数据（重新生成随机数据）
    void refreshData();

private:
    void generateMockData();
    double randomScore();
    double randomMasteryRate();

    QVector<Student> m_students;
    QVector<CourseClass> m_classes;
    QVector<ScoreRecord> m_scores;
    QStringList m_knowledgePointNames;
};

#endif // MOCKDATASOURCE_H
