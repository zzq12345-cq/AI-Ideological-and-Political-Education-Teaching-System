#include "QuestionRepository.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QDebug>

// 从JSON转换Question
Question Question::fromJson(const QJsonObject &json)
{
    Question q;

    q.id = json["id"].toString();
    QString typeStr = json["type"].toString();
    if (typeStr == "single") {
        q.type = QuestionType::SingleChoice;
    } else if (typeStr == "multi") {
        q.type = QuestionType::MultiChoice;
    } else if (typeStr == "truefalse") {
        q.type = QuestionType::TrueFalse;
    } else if (typeStr == "short") {
        q.type = QuestionType::ShortAnswer;
    }

    QString diffStr = json["difficulty"].toString();
    if (diffStr == "easy") {
        q.difficulty = Difficulty::Easy;
    } else if (diffStr == "medium") {
        q.difficulty = Difficulty::Medium;
    } else if (diffStr == "hard") {
        q.difficulty = Difficulty::Hard;
    }

    q.stem = json["stem"].toString();

    // 处理选项
    QJsonArray optionsArray = json["options"].toArray();
    for (const QJsonValue &val : optionsArray) {
        q.options.append(val.toString());
    }

    q.answer = json["answer"].toString();
    q.explain = json["explain"].toString();

    // 处理标签
    QJsonArray tagsArray = json["tags"].toArray();
    for (const QJsonValue &val : tagsArray) {
        q.tags.append(val.toString());
    }

    return q;
}

QuestionRepository::QuestionRepository(QObject *parent)
    : QObject(parent)
{
}

// 加载题库数据
bool QuestionRepository::loadQuestions(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误:" << error.errorString();
        return false;
    }

    if (!doc.isArray()) {
        qWarning() << "JSON不是数组格式";
        return false;
    }

    QJsonArray array = doc.array();
    m_questions.clear();

    for (const QJsonValue &val : array) {
        if (val.isObject()) {
            m_questions.append(Question::fromJson(val.toObject()));
        }
    }

    m_filteredQuestions = m_questions;
    emit dataLoaded(m_questions.count());
    emit filterChanged(m_filteredQuestions.count());

    qInfo() << "成功加载" << m_questions.count() << "道题目";
    return true;
}

// 根据筛选条件获取题目
QList<Question> QuestionRepository::getFilteredQuestions(const FilterCriteria &criteria)
{
    QList<Question> filtered;

    for (const Question &q : m_questions) {
        // 这里可以实现更复杂的筛选逻辑
        // 目前先返回所有题目
        filtered.append(q);
    }

    m_filteredQuestions = filtered;
    emit filterChanged(filtered.count());
    return filtered;
}

// 导航到下一题
Question QuestionRepository::nextQuestion()
{
    if (m_currentIndex < m_filteredQuestions.size() - 1) {
        m_currentIndex++;
        emit currentChanged(m_currentIndex);
        emit currentQuestionChanged();
    }
    return currentQuestion();
}

// 导航到上一题
Question QuestionRepository::previousQuestion()
{
    if (m_currentIndex > 0) {
        m_currentIndex--;
        emit currentChanged(m_currentIndex);
        emit currentQuestionChanged();
    }
    return currentQuestion();
}

// 获取当前题目
Question QuestionRepository::currentQuestion() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_filteredQuestions.size()) {
        return m_filteredQuestions[m_currentIndex];
    }
    return Question();
}

// 设置是否显示答案
void QuestionRepository::setShowAnswer(bool show)
{
    if (m_showAnswer != show) {
        m_showAnswer = show;
        emit currentQuestionChanged();
    }
}

// 设置当前索引
void QuestionRepository::setCurrentIndex(int index)
{
    if (index >= 0 && index < m_filteredQuestions.size()) {
        m_currentIndex = index;
        emit currentChanged(m_currentIndex);
        emit currentQuestionChanged();
    }
}
