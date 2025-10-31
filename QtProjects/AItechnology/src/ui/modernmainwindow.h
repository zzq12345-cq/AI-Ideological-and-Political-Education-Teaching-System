#ifndef MODERNMAINWINDOW_H
#define MODERNMAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QTabWidget>
#include <QStackedWidget>
#include <QToolBar>
#include <QAction>

QT_BEGIN_NAMESPACE
namespace Ui { class ModernMainWindow; }
QT_END_NAMESPACE

class ModernMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ModernMainWindow(const QString &userRole = "学生", const QString &username = "用户", QWidget *parent = nullptr);
    ~ModernMainWindow();

private slots:
    void onLogoutClicked();
    void onProfileClicked();
    void onSettingsClicked();
    void onDashboardClicked();
    void onCoursesClicked();
    void onAssignmentsClicked();
    void onAnalyticsClicked();
    void onMessagesClicked();
    void onHelpClicked();

private:
    void initUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupStyles();
    void createDashboard();
    void createCoursesPage();
    void createAssignmentsPage();
    void createAnalyticsPage();
    void createMessagesPage();

    // UI组件
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *contentLayout;

    // 侧边栏
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QPushButton *dashboardBtn;
    QPushButton *coursesBtn;
    QPushButton *assignmentsBtn;
    QPushButton *analyticsBtn;
    QPushButton *messagesBtn;
    QPushButton *settingsBtn;
    QPushButton *logoutBtn;

    // 主内容区域
    QStackedWidget *contentStack;
    QWidget *dashboardWidget;
    QWidget *coursesWidget;
    QWidget *assignmentsWidget;
    QWidget *analyticsWidget;
    QWidget *messagesWidget;

    // 用户信息
    QLabel *userRoleLabel;
    QLabel *usernameLabel;
    QLabel *welcomeLabel;

    // 数据
    QString currentUserRole;
    QString currentUsername;

    // 菜单和工具栏
    QToolBar *mainToolBar;

    // 动作
    QAction *profileAction;
    QAction *settingsAction;
    QAction *helpAction;
    QAction *aboutAction;
    QAction *logoutAction;
};

#endif // MODERNMAINWINDOW_H