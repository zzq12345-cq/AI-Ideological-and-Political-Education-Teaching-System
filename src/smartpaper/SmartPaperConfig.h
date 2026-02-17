#ifndef SMARTPAPERCONFIG_H
#define SMARTPAPERCONFIG_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include "../services/PaperService.h"

/**
 * @brief 单个题型配置 - 指定题型需要多少题、每题多少分
 */
struct QuestionTypeSpec {
    QString questionType;      // "single_choice", "multi_choice", "true_false", "short_answer", "essay"
    int count = 0;             // 需要的题数
    int scorePerQuestion = 0;  // 每题分值
};

/**
 * @brief 智能组卷完整配置 - 教师设定的约束条件
 */
struct SmartPaperConfig {
    // 试卷元信息
    QString title;
    QString subject;
    QString grade;
    int duration = 90;              // 考试时长（分钟）
    int targetTotalScore = 100;     // 目标总分

    // 题型分布
    QList<QuestionTypeSpec> typeSpecs;

    // 难度比例（归一化，如 3:5:2 表示简单:中等:困难）
    int easyRatio = 3;
    int mediumRatio = 5;
    int hardRatio = 2;

    // 章节/知识点覆盖
    QStringList chapters;
    QStringList knowledgePoints;

    // 排除条件（不选这些题目ID）
    QStringList excludeQuestionIds;

    // 计算配置的实际总分
    int computedTotalScore() const {
        int total = 0;
        for (const auto &spec : typeSpecs) {
            total += spec.count * spec.scorePerQuestion;
        }
        return total;
    }

    // 计算配置的实际总题数
    int computedTotalCount() const {
        int total = 0;
        for (const auto &spec : typeSpecs) {
            total += spec.count;
        }
        return total;
    }
};

/**
 * @brief 单题选择理由 - 记录每道题被选中的原因
 */
struct QuestionSelectionReason {
    QString questionId;
    int coverageScore = 0;        // 知识点/章节覆盖得分
    int difficultyMatchScore = 0; // 难度匹配得分
    int diversityScore = 0;       // 多样性得分（随机扰动）
    QString summary;              // 一句话理由
};

/**
 * @brief 智能组卷结果 - 算法输出
 */
struct SmartPaperResult {
    QList<PaperQuestion> selectedQuestions;                   // 选中的题目
    QMap<QString, QList<PaperQuestion>> candidatePool;       // 题型 -> 替换候选题目池
    // 统计信息
    int totalScore = 0;
    QMap<QString, int> typeCount;         // 题型 -> 数量
    QMap<QString, int> difficultyCount;   // 难度 -> 数量
    QStringList coveredChapters;          // 已覆盖章节
    QStringList warnings;                 // 警告信息（题量不足等）
    bool success = false;

    // 组卷解释
    QList<QuestionSelectionReason> selectionReasons;  // 逐题选择理由
    QStringList coveredKnowledgePoints;               // 已覆盖知识点列表
    double knowledgePointCoverage = 0.0;              // 知识点覆盖率 (0.0~1.0)
};

#endif // SMARTPAPERCONFIG_H
