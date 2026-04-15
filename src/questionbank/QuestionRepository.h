#ifndef QUESTIONREPOSITORY_H
#define QUESTIONREPOSITORY_H

#include <QObject>
#include <QString>

class QuestionRepository : public QObject
{
public:
    explicit QuestionRepository(QObject *parent = nullptr);

    void loadQuestions(const QString &path);
};

#endif // QUESTIONREPOSITORY_H
