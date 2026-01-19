#include "QuestionBasket.h"
#include <QDebug>
#include <algorithm>

QuestionBasket *QuestionBasket::s_instance = nullptr;

QuestionBasket *QuestionBasket::instance()
{
    if (!s_instance) {
        s_instance = new QuestionBasket();
    }
    return s_instance;
}

QuestionBasket::QuestionBasket(QObject *parent)
    : QObject(parent)
{
}

bool QuestionBasket::addQuestion(const PaperQuestion &question)
{
    if (question.id.isEmpty()) {
        qWarning() << "[QuestionBasket] Cannot add question without ID";
        return false;
    }

    if (contains(question.id)) {
        qDebug() << "[QuestionBasket] Question already in basket:" << question.id;
        return false;
    }

    m_questions.append(question);
    m_scores[question.id] = question.score > 0 ? question.score : 5;  // 默认5分

    qDebug() << "[QuestionBasket] Added question:" << question.id
             << "Total:" << m_questions.size();

    emit questionAdded(question);
    emit countChanged(m_questions.size());
    return true;
}

bool QuestionBasket::removeQuestion(const QString &questionId)
{
    for (int i = 0; i < m_questions.size(); ++i) {
        if (m_questions[i].id == questionId) {
            m_questions.removeAt(i);
            m_scores.remove(questionId);

            qDebug() << "[QuestionBasket] Removed question:" << questionId
                     << "Remaining:" << m_questions.size();

            emit questionRemoved(questionId);
            emit countChanged(m_questions.size());
            return true;
        }
    }
    return false;
}

bool QuestionBasket::contains(const QString &questionId) const
{
    for (const PaperQuestion &q : m_questions) {
        if (q.id == questionId) {
            return true;
        }
    }
    return false;
}

void QuestionBasket::clear()
{
    m_questions.clear();
    m_scores.clear();

    qDebug() << "[QuestionBasket] Cleared all questions";

    emit cleared();
    emit countChanged(0);
}

QList<PaperQuestion> QuestionBasket::questions() const
{
    return m_questions;
}

int QuestionBasket::count() const
{
    return m_questions.size();
}

QMap<QString, QList<PaperQuestion>> QuestionBasket::groupedByType() const
{
    QMap<QString, QList<PaperQuestion>> grouped;
    for (const PaperQuestion &q : m_questions) {
        grouped[q.questionType].append(q);
    }
    return grouped;
}

QMap<QString, int> QuestionBasket::countByType() const
{
    QMap<QString, int> counts;
    for (const PaperQuestion &q : m_questions) {
        counts[q.questionType]++;
    }
    return counts;
}

int QuestionBasket::totalScore() const
{
    int total = 0;
    for (const PaperQuestion &q : m_questions) {
        total += m_scores.value(q.id, 5);
    }
    return total;
}

void QuestionBasket::setQuestionScore(const QString &questionId, int score)
{
    if (contains(questionId)) {
        m_scores[questionId] = score;
    }
}

int QuestionBasket::questionScore(const QString &questionId) const
{
    return m_scores.value(questionId, 5);
}

void QuestionBasket::moveQuestion(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= m_questions.size() ||
        toIndex < 0 || toIndex >= m_questions.size() ||
        fromIndex == toIndex) {
        return;
    }

    m_questions.move(fromIndex, toIndex);
    qDebug() << "[QuestionBasket] Moved question from" << fromIndex << "to" << toIndex;
}
