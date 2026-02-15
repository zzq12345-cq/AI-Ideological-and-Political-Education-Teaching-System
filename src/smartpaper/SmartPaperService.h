#ifndef SMARTPAPERSERVICE_H
#define SMARTPAPERSERVICE_H

#include <QObject>
#include <QQueue>
#include <QRandomGenerator>
#include "SmartPaperConfig.h"

class PaperService;

/**
 * @brief 智能组卷算法服务
 *
 * 从云端题库中按约束条件自动选题，支持换题和统计。
 * 核心算法：分阶段贪心 + 约束满足。
 * 由于 PaperService::searchCompleted 信号无请求标识，必须串行搜索。
 */
class SmartPaperService : public QObject
{
    Q_OBJECT

public:
    explicit SmartPaperService(PaperService *paperService, QObject *parent = nullptr);

    /**
     * @brief 根据配置生成试卷（异步，结果通过信号返回）
     */
    void generate(const SmartPaperConfig &config);

    /**
     * @brief 换一题 - 用候选池中的题目替换当前题目
     * @param oldId 要替换的题目ID
     * @param questionType 题型（用于定位候选池）
     * @return 是否替换成功
     */
    bool swapQuestion(const QString &oldId, const QString &questionType);

    /**
     * @brief 获取当前组卷结果
     */
    SmartPaperResult currentResult() const;

signals:
    void generationCompleted(const SmartPaperResult &result);
    void progressUpdated(int percent, const QString &message);
    void generationFailed(const QString &error);

private slots:
    void onSearchCompleted(const QList<PaperQuestion> &results);

private:
    // 串行搜索管理
    void startNextSearch();

    // 贪心选题算法
    void runGreedySelection();

    // 对候选题打分
    int scoreCandidate(const PaperQuestion &question,
                       const QSet<QString> &coveredChapters,
                       const QSet<QString> &coveredKnowledgePoints) const;

    // 构建统计信息
    void buildStatistics();

    PaperService *m_paperService;
    SmartPaperConfig m_config;
    SmartPaperResult m_result;

    // 串行搜索状态
    QQueue<QuestionTypeSpec> m_searchQueue;        // 待搜索的题型队列
    QString m_currentSearchType;                    // 当前正在搜索的题型
    QMap<QString, QList<PaperQuestion>> m_rawCandidates;  // 题型 -> 原始候选题
    bool m_isGenerating = false;
    int m_totalSearches = 0;                        // 总搜索数（用于进度计算）
    int m_completedSearches = 0;                    // 已完成搜索数
};

#endif // SMARTPAPERSERVICE_H
