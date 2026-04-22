#ifndef MYCLASSWIDGET_H
#define MYCLASSWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include "ClassInfo.h"

class ClassDetailWidget;
class StudentClassDetailWidget;

class MyClassWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyClassWidget(bool isTeacher, const QString &userEmail, QWidget *parent = nullptr);

    void reloadClasses();

private:
    void setupUI();
    QWidget* createClassCard(const ClassInfo &info);
    QWidget* createEmptyState();

    // 教师
    void onCreateClicked();
    void openClassDetail(const ClassInfo &info);
    void closeClassDetail();

    // 学生
    void onJoinClicked();
    void onSignAttendance();
    void onSignResult(bool success, const QString &message);
    void openStudentClassDetail(const ClassInfo &info);

    void refreshCards(const QList<ClassInfo> &classes);

    bool m_isTeacher;
    QString m_userEmail;

    QGridLayout *m_gridLayout;
    QWidget *m_gridContainer;
    QStackedWidget *m_stack;
    QWidget *m_listPage;
    ClassDetailWidget *m_detailWidget = nullptr;
    StudentClassDetailWidget *m_studentDetailWidget = nullptr;
    QList<ClassInfo> m_classes;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif
