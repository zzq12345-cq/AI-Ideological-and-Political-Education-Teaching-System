#ifndef STUDENTCLASSDETAILWIDGET_H
#define STUDENTCLASSDETAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QNetworkAccessManager>
#include "ClassInfo.h"
#include "ClassManager.h"
#include "AttendanceManager.h"
#include "HomeworkManager.h"

class StudentClassDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StudentClassDetailWidget(const ClassInfo &info, QWidget *parent = nullptr);

signals:
    void backRequested();

private:
    void setupUI();
    QWidget* createActionButtons();
    QWidget* createMemberPanel();
    QWidget* createAttendancePanel();
    QWidget* createHomeworkPanel();
    void showMaterials();

    void loadMembers();
    void loadAttendance();
    void loadHomework();
    void showSubmissionDetailDialog(const HomeworkManager::SubmissionInfo &sub);
    void showResubmitDialog(const QString &submissionId);

    ClassInfo m_classInfo;
    QString m_studentEmail;
    QString m_studentName;
    QStackedWidget *m_rightStack;
    QVBoxLayout *m_memberLayout;
    QVBoxLayout *m_attendanceLayout;
    QVBoxLayout *m_homeworkLayout;
    QNetworkAccessManager *m_networkManagerForUpload;
};

#endif
