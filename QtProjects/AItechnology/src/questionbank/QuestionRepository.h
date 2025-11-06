#pragma once

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QMap>

// 题目类型枚举
enum class QuestionType {
    SingleChoice,  // 单选题
    MultiChoice,   // 多选题
    TrueFalse,     // 判断题
    ShortAnswer    // 简答题
};

// 难度枚举
enum class Difficulty {
    Easy,     // 简单
    Medium,   // 中等
    Hard      // 困难
};

// 题目数据结构
struct Question {
    QString id;
    QuestionType type;
    Difficulty difficulty;
    QString stem;          // 题干
    QStringList options;   // 选项
    QString answer;        // 答案
    QString explain;       // 解析
    QStringList tags;      // 标签

    // 从JSON转换
    static Question fromJson(const QJsonObject &json);
};

// 筛选条件结构
struct FilterCriteria {
    QStringList courses;       // 课程范围
    QStringList versions;      // 教材版本
    QStringList grades;        // 年级学期
    QStringList chapters;      // 章节
    QStringList paperTypes;    // 试卷类型
    QStringList questionTypes; // 题目题型
    QStringList difficulties;  // 题目难度
};

// 题库仓库类
class QuestionRepository : public QObject {
    Q_OBJECT

public:
    explicit QuestionRepository(QObject *parent = nullptr);

    // 加载题库数据
    Q_INVOKABLE bool loadQuestions(const QString &filePath);

    // 根据筛选条件获取题目
    Q_INVOKABLE QList<Question> getFilteredQuestions(const FilterCriteria &criteria);

    // 题目列表
    QList<Question> questions() const { return m_questions; }

    // 当前进度
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentChanged)
    int currentIndex() const { return m_currentIndex; }

    Q_PROPERTY(int totalCount READ totalCount NOTIFY filterChanged)
    int totalCount() const { return m_filteredQuestions.size(); }

    // 导航方法
    Q_INVOKABLE Question nextQuestion();
    Q_INVOKABLE Question previousQuestion();

    // 当前题目属性
    Q_PROPERTY(Question currentQuestion READ currentQuestion NOTIFY currentQuestionChanged)
    Question currentQuestion() const;

    // 状态管理
    Q_INVOKABLE void setShowAnswer(bool show);
    Q_INVOKABLE bool isShowAnswer() const { return m_showAnswer; }

    // 设置当前索引（筛选后）
    Q_INVOKABLE void setCurrentIndex(int index);

    // 获取已筛选的题目
    QList<Question> filteredQuestions() const { return m_filteredQuestions; }

signals:
    void dataLoaded(int count);
    void filterChanged(int count);
    void currentChanged(int index);
    void currentQuestionChanged();

private:
    QList<Question> m_questions;
    QList<Question> m_filteredQuestions;
    int m_currentIndex = 0;
    bool m_showAnswer = false;
};
