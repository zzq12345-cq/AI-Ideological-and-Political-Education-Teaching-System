#ifndef ADMINDASHBOARD_H
#define ADMINDASHBOARD_H

#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

class AdminDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit AdminDashboard(QWidget *parent = nullptr);

private:
    void setupUI();
    QWidget* createOverviewPage();
    QWidget* createSchoolPage();
    QWidget* createTeacherPage();
    QWidget* createClassPage();

    void refreshOverview();
    void refreshSchools();
    void showCreateSchoolDialog();
    void showEditSchoolDialog(const QString &id, const QString &name, int quota);
    void showInviteCodeDialog(const QString &schoolId, const QString &schoolName);
    void refreshTeachers();
    void refreshClasses();
    void showAssignTeacherDialog(const QString &email);
    void showResetPasswordDialog(const QString &email);
    void showEditClassDialog(const QString &classId, const QString &name, const QString &description, const QString &status);

    QStackedWidget *m_stack;
    // 总览
    QLabel *m_statSchools, *m_statTeachers, *m_statClasses, *m_statStudents;
    // 学校页
    QVBoxLayout *m_schoolListLayout;
    QLabel *m_schoolCountLabel;
    // 教师页
    QVBoxLayout *m_teacherListLayout;
    QLabel *m_teacherCountLabel;
    // 班级页
    QVBoxLayout *m_classListLayout;
    QLabel *m_classCountLabel;

    // 缓存学校列表供分配用
    QList<QPair<QString, QString>> m_schoolList; // id, name
};

#endif // ADMINDASHBOARD_H
