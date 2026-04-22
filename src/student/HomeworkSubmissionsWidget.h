#ifndef HOMEWORKSUBMISSIONSWIDGET_H
#define HOMEWORKSUBMISSIONSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "HomeworkManager.h"
#include "ClassManager.h"

class HomeworkSubmissionsWidget : public QWidget
{
    Q_OBJECT

public:
    HomeworkSubmissionsWidget(const HomeworkManager::AssignmentInfo &assignment,
                               const QList<ClassManager::MemberInfo> &members,
                               QWidget *parent = nullptr);

signals:
    void backRequested();

private:
    void setupUI();
    void loadSubmissions();
    QWidget* createStudentRow(const QString &name, const QString &email,
                               const HomeworkManager::SubmissionInfo *submission);
    void showGradeDialog(const HomeworkManager::SubmissionInfo &submission);

    HomeworkManager::AssignmentInfo m_assignment;
    QList<ClassManager::MemberInfo> m_members;
    QVBoxLayout *m_listLayout = nullptr;
};

#endif
