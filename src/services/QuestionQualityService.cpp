#include "QuestionQualityService.h"
#include "DifyService.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>
#include <QTimer>
#include <algorithm>

// ==================== 标签规范化映射 ====================
QMap<QString, QString> QuestionQualityService::s_tagNormalizationMap;

void QuestionQualityService::initTagMap()
{
    if (!s_tagNormalizationMap.isEmpty()) return;

    // 同义词 → 标准标签
    s_tagNormalizationMap = {
        // 思政核心概念规范化
        {"马克思主义", "马克思主义"},
        {"马克思", "马克思主义"},
        {"唯物主义", "唯物主义"},
        {"唯物论", "唯物主义"},
        {"辩证法", "唯物辩证法"},
        {"辩证唯物主义", "唯物辩证法"},
        {"唯物辩证", "唯物辩证法"},
        {"社会主义核心价值", "社会主义核心价值观"},
        {"核心价值观", "社会主义核心价值观"},
        {"价值观", "社会主义核心价值观"},
        {"中国特色社会主义", "中国特色社会主义"},
        {"中国特色", "中国特色社会主义"},
        {"宪法", "宪法"},
        {"法治", "法治"},
        {"依法治国", "法治"},
        {"道德", "道德"},
        {"品德", "道德"},
        {"公民道德", "道德"},
        {"爱国", "爱国主义"},
        {"爱国主义", "爱国主义"},
        {"民族精神", "民族精神"},
        {"人权", "人权"},
        {"权利", "公民权利"},
        {"义务", "公民义务"},
        {"生态文明", "生态文明"},
        {"环保", "生态文明"},
        {"环境保护", "生态文明"},
        {"创新", "创新"},
        {"科技创新", "创新"},
        {"改革开放", "改革开放"},
        {"改革", "改革开放"},
    };
}

// ==================== 构造 ====================

QuestionQualityService::QuestionQualityService(PaperService *paperService,
                                                 DifyService *difyService,
                                                 QObject *parent)
    : QObject(parent)
    , m_paperService(paperService)
    , m_difyService(difyService)
{
    initTagMap();

    // 连接 DifyService 的流式响应完成信号
    connect(m_difyService, &DifyService::messageReceived,
            this, [this](const QString &response) {
        if (!m_currentOperation.isEmpty()) {
            processAIResponse(response, m_currentOperation, m_currentQuestionId);
        }
    });

    connect(m_difyService, &DifyService::errorOccurred,
            this, [this](const QString &error) {
        if (!m_currentOperation.isEmpty()) {
            emit errorOccurred(m_currentOperation, error);
            m_currentOperation.clear();
            // 批量模式下继续下一个
            if (!m_batchState.queue.isEmpty()) {
                m_batchState.failCount++;
                processNextBatchItem();
            }
        }
    });
}

// ==================== 文本相似度算法 ====================

QSet<QString> QuestionQualityService::extractBigrams(const QString &text) const
{
    QSet<QString> bigrams;
    QString cleaned = text.simplified().remove(' ');
    for (int i = 0; i < cleaned.length() - 1; ++i) {
        bigrams.insert(cleaned.mid(i, 2));
    }
    return bigrams;
}

double QuestionQualityService::jaccardBigram(const QString &textA, const QString &textB) const
{
    QSet<QString> bigramsA = extractBigrams(textA);
    QSet<QString> bigramsB = extractBigrams(textB);

    if (bigramsA.isEmpty() && bigramsB.isEmpty()) return 1.0;
    if (bigramsA.isEmpty() || bigramsB.isEmpty()) return 0.0;

    QSet<QString> intersection = bigramsA;
    intersection.intersect(bigramsB);

    QSet<QString> unionSet = bigramsA;
    unionSet.unite(bigramsB);

    return static_cast<double>(intersection.size()) / unionSet.size();
}

int QuestionQualityService::editDistance(const QString &a, const QString &b) const
{
    int m = a.length();
    int n = b.length();

    // 对超长文本截断，避免 O(m*n) 爆内存
    const int MAX_LEN = 500;
    QString sa = a.left(MAX_LEN);
    QString sb = b.left(MAX_LEN);
    m = sa.length();
    n = sb.length();

    QVector<int> prev(n + 1), curr(n + 1);
    for (int j = 0; j <= n; ++j) prev[j] = j;

    for (int i = 1; i <= m; ++i) {
        curr[0] = i;
        for (int j = 1; j <= n; ++j) {
            int cost = (sa[i - 1] == sb[j - 1]) ? 0 : 1;
            curr[j] = std::min({prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost});
        }
        std::swap(prev, curr);
    }
    return prev[n];
}

double QuestionQualityService::computeSimilarity(const QString &textA, const QString &textB) const
{
    // 混合指标: 0.6 * Jaccard + 0.4 * (1 - 编辑距离/maxLen)
    double jaccard = jaccardBigram(textA, textB);

    int maxLen = qMax(textA.length(), textB.length());
    double editSim = (maxLen == 0) ? 1.0 : (1.0 - static_cast<double>(editDistance(textA, textB)) / maxLen);

    return 0.6 * jaccard + 0.4 * editSim;
}

// ==================== Level 1: 文本去重 ====================

QList<QuestionQualityService::DuplicateCandidate>
QuestionQualityService::findSimilarQuestions(const PaperQuestion &question,
                                              const QList<PaperQuestion> &pool,
                                              double threshold)
{
    QList<DuplicateCandidate> candidates;

    for (const auto &q : pool) {
        if (q.id == question.id) continue;  // 跳过自己

        double sim = computeSimilarity(question.stem, q.stem);
        if (sim >= threshold) {
            candidates.append({q.id, q.stem, sim});
        }
    }

    // 按相似度降序
    std::sort(candidates.begin(), candidates.end(),
              [](const DuplicateCandidate &a, const DuplicateCandidate &b) {
        return a.similarity > b.similarity;
    });

    return candidates;
}

// ==================== Level 2: AI 语义去重 ====================

void QuestionQualityService::checkSemanticDuplicate(const PaperQuestion &newQuestion,
                                                      const QList<DuplicateCandidate> &candidates)
{
    if (candidates.isEmpty()) {
        emit duplicateCheckCompleted(newQuestion.id, false, {});
        return;
    }

    // 取相似度最高的候选进行 AI 判断
    const auto &top = candidates.first();

    QString prompt = QString(
        "判断以下两道题是否语义重复（仅问法不同但考查相同知识点和能力）：\n"
        "题目A: %1\n"
        "题目B: %2\n"
        "请回答：DUPLICATE 或 NOT_DUPLICATE，并简要说明理由。"
    ).arg(newQuestion.stem, top.stem);

    sendAIRequest(prompt, "semantic_duplicate", newQuestion.id);
}

// ==================== 全库扫描 ====================

void QuestionQualityService::scanDuplicatesInLibrary()
{
    if (m_isScanning) {
        qWarning() << "[QuestionQualityService] 扫描已在进行中";
        return;
    }

    m_isScanning = true;
    m_scanDuplicates.clear();

    // 成功回调
    auto *successConn = new QMetaObject::Connection;
    auto *errorConn = new QMetaObject::Connection;

    *successConn = connect(m_paperService, &PaperService::searchCompleted,
                    this, [this, successConn, errorConn](const QList<PaperQuestion> &questions) {
        disconnect(*successConn);
        disconnect(*errorConn);
        delete successConn;
        delete errorConn;

        int total = questions.size();
        qDebug() << "[QuestionQualityService] 全库扫描，共" << total << "题";

        if (total == 0) {
            m_isScanning = false;
            emit libraryScanCompleted(m_scanDuplicates);
            return;
        }

        // 保存题目列表，异步分块比对
        m_scanQuestions = questions;
        m_scanIndex = 0;
        processScanChunk();
    });

    // 错误回调 — 重置状态，通知上层
    *errorConn = connect(m_paperService, &PaperService::questionError,
                    this, [this, successConn, errorConn](const QString &, const QString &err) {
        disconnect(*successConn);
        disconnect(*errorConn);
        delete successConn;
        delete errorConn;

        m_isScanning = false;
        emit errorOccurred("scan_duplicates", err);
    });

    // 搜索所有公共题目
    QuestionSearchCriteria criteria;
    criteria.visibility = "public";
    m_paperService->searchQuestions(criteria);
}

void QuestionQualityService::processScanChunk()
{
    int total = m_scanQuestions.size();
    // 每次处理一批外层迭代，然后让出事件循环
    const int CHUNK_SIZE = 5;
    int end = qMin(m_scanIndex + CHUNK_SIZE, total);

    for (int i = m_scanIndex; i < end; ++i) {
        emit libraryScanProgress(i + 1, total);

        for (int j = i + 1; j < total; ++j) {
            double sim = computeSimilarity(m_scanQuestions[i].stem, m_scanQuestions[j].stem);
            if (sim >= 0.7) {
                m_scanDuplicates.append({m_scanQuestions[i].id, m_scanQuestions[j].id});
            }
        }
    }

    m_scanIndex = end;

    if (m_scanIndex < total) {
        // 让出事件循环，UI 得以刷新
        QTimer::singleShot(0, this, &QuestionQualityService::processScanChunk);
    } else {
        // 扫描完成
        m_scanQuestions.clear();
        m_isScanning = false;
        emit libraryScanCompleted(m_scanDuplicates);
    }
}

// ==================== 标签规范化 ====================

QStringList QuestionQualityService::normalizeTags(const QStringList &rawTags)
{
    QSet<QString> normalized;
    for (const auto &tag : rawTags) {
        QString trimmed = tag.trimmed();
        if (trimmed.isEmpty()) continue;

        // 查映射表
        QString mapped = s_tagNormalizationMap.value(trimmed, trimmed);
        normalized.insert(mapped);
    }
    return normalized.values();
}

// ==================== AI 知识点标注 ====================

void QuestionQualityService::annotateKnowledgePoints(const PaperQuestion &question)
{
    QString optionsStr;
    for (int i = 0; i < question.options.size(); ++i) {
        optionsStr += QString("%1. %2\n").arg(QChar('A' + i)).arg(question.options[i]);
    }

    QString prompt = QString(
        "分析以下思政题目，标注其涉及的知识点（返回 JSON 数组）：\n"
        "题型: %1\n"
        "题干: %2\n"
        "选项: %3\n"
        "答案: %4\n"
        "请返回格式: {\"knowledgePoints\": [\"知识点1\", \"知识点2\"]}"
    ).arg(question.questionType, question.stem, optionsStr, question.answer);

    sendAIRequest(prompt, "annotate_knowledge", question.id);
}

void QuestionQualityService::batchAnnotateKnowledgePoints(const QList<PaperQuestion> &questions)
{
    m_batchState = BatchState{"annotate_knowledge", questions, 0, 0, static_cast<int>(questions.size())};
    processNextBatchItem();
}

// ==================== AI 难度评估 ====================

void QuestionQualityService::estimateDifficulty(const PaperQuestion &question)
{
    QString prompt = QString(
        "评估以下题目的难度等级（easy/medium/hard）：\n"
        "题型: %1\n"
        "题干: %2\n"
        "答案: %3\n"
        "解析: %4\n"
        "请返回格式: {\"difficulty\": \"medium\", \"confidence\": 0.85, \"reason\": \"...\"}"
    ).arg(question.questionType, question.stem, question.answer, question.explanation);

    sendAIRequest(prompt, "estimate_difficulty", question.id);
}

void QuestionQualityService::batchEstimateDifficulty(const QList<PaperQuestion> &questions)
{
    m_batchState = BatchState{"estimate_difficulty", questions, 0, 0, static_cast<int>(questions.size())};
    processNextBatchItem();
}

// ==================== AI 请求 ====================

void QuestionQualityService::sendAIRequest(const QString &prompt,
                                            const QString &operation,
                                            const QString &questionId)
{
    m_currentOperation = operation;
    m_currentQuestionId = questionId;

    // 使用独立对话，不复用用户的 conversationId
    m_difyService->clearConversation();
    m_difyService->sendMessage(prompt);
}

void QuestionQualityService::processAIResponse(const QString &response,
                                                 const QString &operation,
                                                 const QString &questionId)
{
    m_currentOperation.clear();

    if (operation == "semantic_duplicate") {
        bool isDuplicate = response.contains("DUPLICATE") && !response.contains("NOT_DUPLICATE");
        emit duplicateCheckCompleted(questionId, isDuplicate, {});

    } else if (operation == "annotate_knowledge") {
        // 解析 JSON
        QStringList knowledgePoints;
        int jsonStart = response.indexOf('{');
        int jsonEnd = response.lastIndexOf('}');
        if (jsonStart >= 0 && jsonEnd > jsonStart) {
            QString jsonStr = response.mid(jsonStart, jsonEnd - jsonStart + 1);
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            if (doc.isObject()) {
                QJsonArray arr = doc.object()["knowledgePoints"].toArray();
                for (const auto &val : arr) {
                    knowledgePoints.append(val.toString());
                }
            }
        }
        // 如果解析失败，尝试简单文本提取
        if (knowledgePoints.isEmpty()) {
            qWarning() << "[QuestionQualityService] JSON 解析失败，尝试文本提取";
        }

        emit knowledgePointsAnnotated(questionId, knowledgePoints);

    } else if (operation == "estimate_difficulty") {
        QString difficulty = "medium";
        double confidence = 0.5;

        int jsonStart = response.indexOf('{');
        int jsonEnd = response.lastIndexOf('}');
        if (jsonStart >= 0 && jsonEnd > jsonStart) {
            QString jsonStr = response.mid(jsonStart, jsonEnd - jsonStart + 1);
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                difficulty = obj["difficulty"].toString("medium");
                confidence = obj["confidence"].toDouble(0.5);
            }
        }

        emit difficultyEstimated(questionId, difficulty, confidence);
    }

    // 批量模式下继续
    if (!m_batchState.queue.isEmpty()) {
        m_batchState.successCount++;
        processNextBatchItem();
    }
}

void QuestionQualityService::processNextBatchItem()
{
    if (m_batchState.queue.isEmpty()) {
        emit batchCompleted(m_batchState.operation,
                           m_batchState.successCount,
                           m_batchState.failCount);
        return;
    }

    int current = m_batchState.totalCount - m_batchState.queue.size() + 1;
    emit batchProgress(current, m_batchState.totalCount, m_batchState.operation);

    PaperQuestion question = m_batchState.queue.takeFirst();

    if (m_batchState.operation == "annotate_knowledge") {
        annotateKnowledgePoints(question);
    } else if (m_batchState.operation == "estimate_difficulty") {
        estimateDifficulty(question);
    }
}
