#ifndef CURRICULUMSERVICE_H
#define CURRICULUMSERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

/**
 * @brief 课程目录服务类
 *
 * 负责加载和管理道德与法治课程目录数据
 * 为教案编辑器提供课程选择功能
 */
class CurriculumService : public QObject
{
    Q_OBJECT

public:
    // 课时信息结构体
    struct LessonInfo {
        int lessonId;
        QString lessonName;
        QStringList sections;     // 课时下的小节
        QString unitName;         // 所属单元
        QString gradeSemester;    // 年级学期 (如 "七年级上册")
    };

    // 单元信息结构体
    struct UnitInfo {
        int unitId;
        QString unitName;
        QList<LessonInfo> lessons;
    };

    // 学期信息结构体
    struct SemesterInfo {
        int semesterId;
        QString semester;         // "上册" 或 "下册"
        QList<UnitInfo> units;
    };

    // 年级信息结构体
    struct GradeInfo {
        int gradeId;
        QString gradeName;
        QList<SemesterInfo> semesters;
    };

    explicit CurriculumService(QObject *parent = nullptr);
    ~CurriculumService();

    // 单例访问
    static CurriculumService* instance();

    // 初始化加载课程数据
    bool loadCurriculum(const QString &jsonPath = ":/data/curriculum_morality_law.json");

    // 获取所有年级列表
    QStringList getGrades() const;

    // 获取指定年级的学期列表
    QStringList getSemesters(const QString &grade) const;

    // 获取指定年级学期的单元列表
    QStringList getUnits(const QString &grade, const QString &semester) const;

    // 获取指定单元的课时列表
    QStringList getLessons(const QString &grade, const QString &semester, const QString &unit) const;

    // 获取课时详情
    LessonInfo getLessonInfo(const QString &grade, const QString &semester,
                              const QString &unit, const QString &lesson) const;

    // 获取课时的小节列表
    QStringList getSections(const QString &grade, const QString &semester,
                            const QString &unit, const QString &lesson) const;

    // 构建完整的课时标题 (用于AI生成教案)
    QString buildLessonTitle(const QString &grade, const QString &semester,
                             const QString &unit, const QString &lesson) const;

    // 获取统计信息
    int getTotalGrades() const;
    int getTotalUnits() const;
    int getTotalLessons() const;

    // 搜索课时 (模糊匹配)
    QList<LessonInfo> searchLessons(const QString &keyword) const;

signals:
    void curriculumLoaded();
    void loadError(const QString &error);

private:
    void parseGrades(const QJsonArray &gradesArray);
    GradeInfo* findGrade(const QString &gradeName);
    const GradeInfo* findGrade(const QString &gradeName) const;

    QList<GradeInfo> m_grades;
    QJsonObject m_metadata;
    bool m_loaded;

    static CurriculumService* s_instance;
};

#endif // CURRICULUMSERVICE_H
