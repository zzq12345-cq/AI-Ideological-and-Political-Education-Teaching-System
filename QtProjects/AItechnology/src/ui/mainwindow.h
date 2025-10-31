#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QStyle>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, const QString &role, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateDateTime();
    void onLogoutClicked();
    void onProfileClicked();
    void onDashboardClicked();
    void onCoursesClicked();
    void onStudentsClicked();
    void onAssignmentsClicked();
    void onAnalyticsClicked();
    void onSettingsClicked();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupStyles();
    void createSidebar();
    void createHeader();
    void createContentArea();
    void createDashboardContent();
    void createCoursesContent();
    void createStudentsContent();
    void createAssignmentsContent();
    void createAnalyticsContent();
    void clearContent();
    void resetNavButtons();
    QFrame* createStatCard(const QString &title, const QString &value, const QString &icon, const QString &color);

    // 主要组件
    QWidget *m_centralWidget;
    QSplitter *m_splitter;
    QFrame *m_sidebar;
    QFrame *m_mainContent;
    QFrame *m_header;
    QScrollArea *m_contentArea;
    QWidget *m_contentWidget;

    // 侧边栏组件
    QLabel *m_userAvatar;
    QLabel *m_userName;
    QLabel *m_userRole;
    QPushButton *m_dashboardBtn;
    QPushButton *m_coursesBtn;
    QPushButton *m_studentsBtn;
    QPushButton *m_assignmentsBtn;
    QPushButton *m_analyticsBtn;
    QPushButton *m_settingsBtn;

    // 头部组件
    QLabel *m_logoLabel;
    QLineEdit *m_searchEdit;
    QPushButton *m_notificationBtn;
    QPushButton *m_profileBtn;

    // 状态栏组件
    QLabel *m_timeLabel;
    QLabel *m_statusLabel;

    // 用户信息
    QString m_username;
    QString m_role;

    // 当前页面
    QString m_currentPage;
};

#endif // MAINWINDOW_H