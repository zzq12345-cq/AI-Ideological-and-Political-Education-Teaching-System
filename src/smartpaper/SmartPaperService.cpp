#include "SmartPaperService.h"
#include "../services/PaperService.h"
#include <QSet>
#include <QDebug>
#include <QtMath>

SmartPaperService::SmartPaperService(PaperService *paperService, QObject *parent)
    : QObject(parent)
    , m_paperService(paperService)
{
    connect(m_paperService, &PaperService::searchCompleted,
            this, &SmartPaperService::onSearchCompleted);
}

void SmartPaperService::generate(const SmartPaperConfig &config)
{
    if (m_isGenerating) {
        qWarning() << "[SmartPaperService] 已有组卷任务在进行中，忽略重复请求";
        return;
    }

    m_config = config;
    m_result = SmartPaperResult();
    m_rawCandidates.clear();
    m_searchQueue.clear();
    m_isGenerating = true;
    m_completedSearches = 0;

    // 检查配置是否有效
    if (config.typeSpecs.isEmpty()) {
        m_isGenerating = false;
        emit generationFailed("未配置任何题型，无法组卷");
        return;
    }

    // 构建搜索队列
    for (const auto &spec : config.typeSpecs) {
        if (spec.count > 0) {
            m_searchQueue.enqueue(spec);
        }
    }
    m_totalSearches = m_searchQueue.size();

    if (m_totalSearches == 0) {
        m_isGenerating = false;
        emit generationFailed("所有题型的题数都为0，无法组卷");
        return;
    }

    emit progressUpdated(0, "正在初始化组卷...");

    // 开始串行搜索
    startNextSearch();
}

void SmartPaperService::startNextSearch()
{
    if (m_searchQueue.isEmpty()) {
        // 所有搜索完成，开始选题
        emit progressUpdated(60, "正在执行智能选题算法...");
        runGreedySelection();
        return;
    }

    QuestionTypeSpec spec = m_searchQueue.dequeue();
    m_currentSearchType = spec.questionType;

    int progress = static_cast<int>(
        (static_cast<double>(m_completedSearches) / m_totalSearches) * 60.0
    );
    emit progressUpdated(progress,
        QString("正在搜索 %1 题目...").arg(m_currentSearchType));

    // 构造搜索条件：只限定学科、年级、题型，让本地算法在更大候选池中优选
    QuestionSearchCriteria criteria;
    criteria.subject = m_config.subject;
    criteria.grade = m_config.grade;
    criteria.questionType = m_currentSearchType;
    criteria.visibility = "public";  // 从公共题库选题

    m_paperService->searchQuestions(criteria);
}

void SmartPaperService::onSearchCompleted(const QList<PaperQuestion> &results)
{
    if (!m_isGenerating) {
        return;  // 不是智能组卷发起的搜索，忽略
    }

    qDebug() << "[SmartPaperService] 搜索完成，题型:" << m_currentSearchType
             << "结果数:" << results.size();

    // 排除指定的题目ID
    QList<PaperQuestion> filtered;
    for (const auto &q : results) {
        if (!m_config.excludeQuestionIds.contains(q.id)) {
            filtered.append(q);
        }
    }

    m_rawCandidates[m_currentSearchType] = filtered;
    m_completedSearches++;

    // 检查题量是否不足
    for (const auto &spec : m_config.typeSpecs) {
        if (spec.questionType == m_currentSearchType && filtered.size() < spec.count) {
            m_result.warnings.append(
                QString("%1 题不足：需要 %2 题，仅找到 %3 题")
                    .arg(m_currentSearchType)
                    .arg(spec.count)
                    .arg(filtered.size())
            );
        }
    }

    // 继续下一个搜索
    startNextSearch();
}

void SmartPaperService::runGreedySelection()
{
    int totalRatio = m_config.easyRatio + m_config.mediumRatio + m_config.hardRatio;
    if (totalRatio <= 0) totalRatio = 10;  // 防御性处理

    QSet<QString> globalCoveredChapters;
    QSet<QString> globalCoveredKnowledgePoints;

    // 对每个题型独立执行贪心选题
    for (const auto &spec : m_config.typeSpecs) {
        if (spec.count <= 0) continue;

        QList<PaperQuestion> candidates = m_rawCandidates.value(spec.questionType);
        if (candidates.isEmpty()) continue;

        // Step 1: 按难度比例计算各难度需要的数量
        int easyCount = qRound(static_cast<double>(spec.count) * m_config.easyRatio / totalRatio);
        int hardCount = qRound(static_cast<double>(spec.count) * m_config.hardRatio / totalRatio);
        int mediumCount = spec.count - easyCount - hardCount;
        // 确保不出现负数
        if (mediumCount < 0) {
            mediumCount = 0;
            easyCount = qMin(easyCount, spec.count);
            hardCount = spec.count - easyCount;
        }

        // Step 2: 候选题按难度分桶
        QMap<QString, QList<PaperQuestion>> buckets;
        for (const auto &q : candidates) {
            QString diff = q.difficulty.toLower();
            if (diff != "easy" && diff != "medium" && diff != "hard") {
                diff = "medium";  // 未知难度归入中等
            }
            buckets[diff].append(q);
        }

        QMap<QString, int> needed;
        needed["easy"] = easyCount;
        needed["medium"] = mediumCount;
        needed["hard"] = hardCount;

        QList<PaperQuestion> selected;
        QList<PaperQuestion> remaining;

        // Step 3: 每桶内打分排序，取 top-N
        for (const QString &diff : {"easy", "medium", "hard"}) {
            int need = needed[diff];
            QList<PaperQuestion> &bucket = buckets[diff];

            // 对桶内候选题打分
            QVector<QPair<int, int>> scores;  // (分数, 索引)
            for (int i = 0; i < bucket.size(); ++i) {
                int score = scoreCandidate(bucket[i], globalCoveredChapters, globalCoveredKnowledgePoints);
                scores.append({score, i});
            }

            // 按分数降序排序
            std::sort(scores.begin(), scores.end(),
                      [](const QPair<int, int> &a, const QPair<int, int> &b) {
                          return a.first > b.first;
                      });

            int taken = 0;
            for (const auto &pair : scores) {
                if (taken >= need) {
                    remaining.append(bucket[pair.second]);
                } else {
                    const PaperQuestion &q = bucket[pair.second];
                    selected.append(q);
                    // 更新全局覆盖集
                    if (!q.chapter.isEmpty()) globalCoveredChapters.insert(q.chapter);
                    for (const auto &kp : q.knowledgePoints) {
                        globalCoveredKnowledgePoints.insert(kp);
                    }
                    taken++;
                }
            }

            // Step 4: 题量不足时从相邻难度补充
            if (taken < need) {
                int shortfall = need - taken;
                // 记录 warning（已在搜索阶段记录过总量不足，这里记录难度不足）
                m_result.warnings.append(
                    QString("%1 的 %2 难度题不足：需要 %3 题，仅找到 %4 题，将从其他难度补充")
                        .arg(spec.questionType, diff)
                        .arg(need)
                        .arg(taken)
                );
                // 将不足的数量分配到其他难度
                if (diff == "easy") {
                    needed["medium"] += shortfall;
                } else if (diff == "hard") {
                    needed["medium"] += shortfall;
                } else {
                    // medium不足，先从easy补，不够再从hard补
                    needed["easy"] += shortfall / 2;
                    needed["hard"] += shortfall - shortfall / 2;
                }
            }
        }

        // 设置分值和排序号
        for (int i = 0; i < selected.size(); ++i) {
            selected[i].score = spec.scorePerQuestion;
        }

        m_result.selectedQuestions.append(selected);

        // Step 5: 构建候选替换池
        m_result.candidatePool[spec.questionType] = remaining;
    }

    // 设置排序号
    for (int i = 0; i < m_result.selectedQuestions.size(); ++i) {
        m_result.selectedQuestions[i].orderNum = i + 1;
    }

    emit progressUpdated(90, "正在生成统计信息...");

    // 构建统计
    buildStatistics();

    m_result.success = true;
    m_isGenerating = false;

    emit progressUpdated(100, "组卷完成！");
    emit generationCompleted(m_result);
}

int SmartPaperService::scoreCandidate(const PaperQuestion &question,
                                       const QSet<QString> &coveredChapters,
                                       const QSet<QString> &coveredKnowledgePoints) const
{
    int score = 0;

    // 知识点新覆盖度: +40
    for (const auto &kp : question.knowledgePoints) {
        if (!coveredKnowledgePoints.contains(kp)) {
            score += 40;
            break;  // 有一个新知识点就够了
        }
    }

    // 章节新覆盖度: +30
    if (!question.chapter.isEmpty() && !coveredChapters.contains(question.chapter)) {
        score += 30;
    }

    // 章节匹配度: +20（在用户指定章节列表中）
    if (!m_config.chapters.isEmpty() && !question.chapter.isEmpty()) {
        if (m_config.chapters.contains(question.chapter)) {
            score += 20;
        }
    }

    // 随机扰动: +0~10（避免每次结果相同）
    score += QRandomGenerator::global()->bounded(11);

    return score;
}

void SmartPaperService::buildStatistics()
{
    m_result.totalScore = 0;
    m_result.typeCount.clear();
    m_result.difficultyCount.clear();
    m_result.coveredChapters.clear();

    QSet<QString> chapters;

    for (const auto &q : m_result.selectedQuestions) {
        m_result.totalScore += q.score;
        m_result.typeCount[q.questionType]++;
        m_result.difficultyCount[q.difficulty]++;
        if (!q.chapter.isEmpty()) {
            chapters.insert(q.chapter);
        }
    }

    m_result.coveredChapters = chapters.values();

    // 检查未覆盖的章节
    if (!m_config.chapters.isEmpty()) {
        for (const auto &ch : m_config.chapters) {
            if (!chapters.contains(ch)) {
                m_result.warnings.append(
                    QString("章节「%1」未被覆盖").arg(ch)
                );
            }
        }
    }
}

bool SmartPaperService::swapQuestion(const QString &oldId, const QString &questionType)
{
    // 找到要替换的题目
    int oldIndex = -1;
    PaperQuestion oldQuestion;
    for (int i = 0; i < m_result.selectedQuestions.size(); ++i) {
        if (m_result.selectedQuestions[i].id == oldId) {
            oldIndex = i;
            oldQuestion = m_result.selectedQuestions[i];
            break;
        }
    }

    if (oldIndex < 0) {
        qWarning() << "[SmartPaperService] 未找到要替换的题目:" << oldId;
        return false;
    }

    QList<PaperQuestion> &pool = m_result.candidatePool[questionType];
    if (pool.isEmpty()) {
        qWarning() << "[SmartPaperService] 候选池已空，无法换题";
        return false;
    }

    // 优先找同难度的替换题
    int replaceIndex = -1;
    for (int i = 0; i < pool.size(); ++i) {
        if (pool[i].difficulty == oldQuestion.difficulty) {
            replaceIndex = i;
            break;
        }
    }
    // 找不到同难度的就随便来一个
    if (replaceIndex < 0) {
        replaceIndex = 0;
    }

    PaperQuestion newQuestion = pool.takeAt(replaceIndex);
    newQuestion.score = oldQuestion.score;
    newQuestion.orderNum = oldQuestion.orderNum;

    // 替换
    m_result.selectedQuestions[oldIndex] = newQuestion;

    // 旧题放回候选池
    pool.append(oldQuestion);

    // 重新统计
    buildStatistics();

    return true;
}

SmartPaperResult SmartPaperService::currentResult() const
{
    return m_result;
}
