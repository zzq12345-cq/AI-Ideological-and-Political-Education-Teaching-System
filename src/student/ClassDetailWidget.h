#ifndef CLASSDETAILWIDGET_H
#define CLASSDETAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include "ClassInfo.h"
#include "ClassManager.h"
#include "AttendanceManager.h"
#include "MaterialManager.h"

class ClassDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClassDetailWidget(const ClassInfo &info, QWidget *parent = nullptr);

    void setClassInfo(const ClassInfo &info);

signals:
    void backRequested();

private:
    void setupUI();
    QWidget* createCodeSection();
    QWidget* createActionButtons();
    QWidget* createMemberList();
    void updateMemberList(const QList<ClassManager::MemberInfo> &members);
    void showAttendanceActive(const AttendanceManager::SessionInfo &session);
    void showAttendanceResult(const QString &sessionId);
    void showClassSettings();
    void showHomeworkList();
    void showMaterials();
    void showMemberList();

    ClassInfo m_classInfo;
    QLabel *m_codeLabel;
    QLabel *m_classNameLabel;
    QLabel *m_codeCountLabel;
    QLabel *m_memberCountLabel;
    QVBoxLayout *m_memberLayout;
    QWidget *m_memberSection;
    QStackedWidget *m_rightStack;
    QList<ClassManager::MemberInfo> m_cachedMembers;
};

#endif
