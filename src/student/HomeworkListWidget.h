#ifndef HOMEWORKLISTWIDGET_H
#define HOMEWORKLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "HomeworkManager.h"

class HomeworkListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomeworkListWidget(const QString &classId, QWidget *parent = nullptr);

signals:
    void createRequested();
    void viewSubmissions(const HomeworkManager::AssignmentInfo &assignment);
    void backRequested();

private:
    void setupUI();
    void refreshList();
    QWidget* createAssignmentCard(const HomeworkManager::AssignmentInfo &info);

    QString m_classId;
    QVBoxLayout *m_listLayout = nullptr;
};

#endif
