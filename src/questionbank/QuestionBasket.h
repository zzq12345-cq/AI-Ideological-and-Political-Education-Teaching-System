#ifndef QUESTIONBASKET_H
#define QUESTIONBASKET_H

#include <QObject>
#include <QList>
#include <QMap>
#include "../services/PaperService.h"

/**
 * @brief 试题篮 - 管理用户选择的试题
 *
 * 单例模式，用于在组卷过程中临时存储用户选择的试题
 * 支持添加、移除、清空操作，并发送相应信号通知UI更新
 */
class QuestionBasket : public QObject
{
    Q_OBJECT

public:
    static QuestionBasket *instance();

    // 添加试题到篮子
    bool addQuestion(const PaperQuestion &question);

    // 从篮子移除试题
    bool removeQuestion(const QString &questionId);

    // 检查试题是否已在篮子中
    bool contains(const QString &questionId) const;

    // 清空篮子
    void clear();

    // 获取所有已选试题
    QList<PaperQuestion> questions() const;

    // 获取已选试题数量
    int count() const;

    // 获取按题型分组的试题
    QMap<QString, QList<PaperQuestion>> groupedByType() const;

    // 获取按题型统计的数量
    QMap<QString, int> countByType() const;

    // 计算总分（如果有分值的话）
    int totalScore() const;

    // 设置试题分值
    void setQuestionScore(const QString &questionId, int score);

    // 获取试题分值
    int questionScore(const QString &questionId) const;

    // 调整试题顺序
    void moveQuestion(int fromIndex, int toIndex);

signals:
    // 试题添加时发出
    void questionAdded(const PaperQuestion &question);

    // 试题移除时发出
    void questionRemoved(const QString &questionId);

    // 篮子清空时发出
    void cleared();

    // 数量变化时发出
    void countChanged(int newCount);

private:
    explicit QuestionBasket(QObject *parent = nullptr);
    ~QuestionBasket() override = default;

    static QuestionBasket *s_instance;

    QList<PaperQuestion> m_questions;
    QMap<QString, int> m_scores;  // questionId -> score
};

#endif // QUESTIONBASKET_H
