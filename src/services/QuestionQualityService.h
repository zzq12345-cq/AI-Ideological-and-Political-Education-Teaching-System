#ifndef QUESTIONQUALITYSERVICE_H
#define QUESTIONQUALITYSERVICE_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QSet>
#include "PaperService.h"

class DifyService;

/**
 * @brief 题库质量管线服务
 *
 * 统一入口：去重（文本+AI语义）、标签规范化、AI 知识点标注、AI 难度评估。
 */
class QuestionQualityService : public QObject
{
    Q_OBJECT

public:
    explicit QuestionQualityService(PaperService *paperService,
                                     DifyService *difyService,
                                     QObject *parent = nullptr);

    // === 去重 ===

    // 疑似重复候选
    struct DuplicateCandidate {
        QString questionId;
        QString stem;
        double similarity;  // 0.0~1.0
    };

    // Level 1: 文本相似度快速筛查（纯本地）
    QList<DuplicateCandidate> findSimilarQuestions(const PaperQuestion &question,
                                                    const QList<PaperQuestion> &pool,
                                                    double threshold = 0.7);

    // Level 2: AI 语义去重（调 Dify）
    void checkSemanticDuplicate(const PaperQuestion &newQuestion,
                                 const QList<DuplicateCandidate> &candidates);

    // 全库扫描去重
    void scanDuplicatesInLibrary();

    // === 标签规范化 ===
    QStringList normalizeTags(const QStringList &rawTags);

    // === AI 知识点标注 ===
    void annotateKnowledgePoints(const PaperQuestion &question);
    void batchAnnotateKnowledgePoints(const QList<PaperQuestion> &questions);

    // === AI 难度评估 ===
    void estimateDifficulty(const PaperQuestion &question);
    void batchEstimateDifficulty(const QList<PaperQuestion> &questions);

signals:
    // 去重
    void duplicateCheckCompleted(const QString &questionId,
                                  bool isDuplicate,
                                  const QList<DuplicateCandidate> &matches);
    void libraryScanProgress(int current, int total);
    void libraryScanCompleted(const QList<QPair<QString,QString>> &duplicatePairs);

    // AI 标注
    void knowledgePointsAnnotated(const QString &questionId,
                                   const QStringList &knowledgePoints);
    void difficultyEstimated(const QString &questionId,
                              const QString &difficulty,
                              double confidence);

    // 批量
    void batchProgress(int current, int total, const QString &operation);
    void batchCompleted(const QString &operation, int successCount, int failCount);

    void errorOccurred(const QString &operation, const QString &error);

private:
    // 文本相似度算法
    double computeSimilarity(const QString &textA, const QString &textB) const;
    double jaccardBigram(const QString &textA, const QString &textB) const;
    int editDistance(const QString &a, const QString &b) const;
    QSet<QString> extractBigrams(const QString &text) const;

    // AI 请求管理
    void sendAIRequest(const QString &prompt, const QString &operation,
                       const QString &questionId);
    void processAIResponse(const QString &response, const QString &operation,
                           const QString &questionId);

    // 批量队列
    void processNextBatchItem();

    // 异步扫描分块处理
    void processScanChunk();

    PaperService *m_paperService;
    DifyService *m_difyService;

    // 标签规范化映射表
    static QMap<QString, QString> s_tagNormalizationMap;
    static void initTagMap();

    // 批量操作状态
    struct BatchState {
        QString operation;
        QList<PaperQuestion> queue;
        int successCount = 0;
        int failCount = 0;
        int totalCount = 0;
    };
    BatchState m_batchState;

    // 全库扫描状态
    bool m_isScanning = false;
    QList<QPair<QString,QString>> m_scanDuplicates;
    QList<PaperQuestion> m_scanQuestions;  // 异步扫描用
    int m_scanIndex = 0;

    // AI 响应解析用的当前操作上下文
    QString m_currentOperation;
    QString m_currentQuestionId;
};

#endif // QUESTIONQUALITYSERVICE_H
