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

    // 现代风格弹窗辅助方法（供其他模块复用）
    static bool showModernConfirm(QWidget *parent, const QString &title, const QString &message);
    static void showModernInfo(QWidget *parent, const QString &title, const QString &message);

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

    void setActiveNavButton(int index);

    ClassInfo m_classInfo;
    QLabel *m_codeLabel;
    QLabel *m_classNameLabel;
    QLabel *m_codeCountLabel;
    QLabel *m_memberCountLabel;
    QVBoxLayout *m_memberLayout;
    QWidget *m_memberSection;
    QStackedWidget *m_rightStack;
    QList<ClassManager::MemberInfo> m_cachedMembers;
    QList<QPushButton*> m_navButtons;

    // 分页相关
    int m_currentPage = 1;
    const int m_itemsPerPage = 10;
    QLabel *m_pageLabel = nullptr;
    QPushButton *m_prevPageBtn = nullptr;
    QPushButton *m_nextPageBtn = nullptr;
    void renderCurrentPage();
};

#endif
