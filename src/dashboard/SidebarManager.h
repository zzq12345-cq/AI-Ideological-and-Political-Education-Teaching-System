#ifndef SIDEBARMANAGER_H
#define SIDEBARMANAGER_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyle>
#include <QIcon>

/**
 * @brief 侧边栏管理器
 *
 * 从 ModernMainWindow 中提取的侧边栏相关功能，
 * 负责管理导航按钮、用户头像和菜单切换。
 */
class SidebarManager : public QObject
{
    Q_OBJECT

public:
    // 导航页面索引
    enum PageIndex {
        Dashboard = 0,
        NewsTracking = 1,
        AIPreparation = 2,
        ResourceManagement = 3,
        LearningAnalysis = 4,
        DataAnalysis = 5,
        Settings = 6,
        Help = 7
    };
    Q_ENUM(PageIndex)

    explicit SidebarManager(QWidget *parentWidget, QObject *parent = nullptr);
    ~SidebarManager();

    // 获取侧边栏组件
    QFrame* sidebar() const { return m_sidebar; }

    // 设置用户信息
    void setUserInfo(const QString &username, const QString &role);

    // 高亮选中的导航按钮
    void setActiveNavigation(PageIndex index);

signals:
    // 导航请求信号
    void navigationRequested(int pageIndex);

    // 按钮点击信号
    void settingsClicked();
    void helpClicked();

private slots:
    void onTeacherCenterClicked();
    void onNewsTrackingClicked();
    void onAIPreparationClicked();
    void onResourceManagementClicked();
    void onLearningAnalysisClicked();
    void onDataAnalysisClicked();

private:
    void setupSidebar();
    void setupNavButtons();
    void setupBottomButtons();
    void createSidebarProfile();
    void applySidebarIcons();
    QIcon loadSidebarIcon(const QString &themeName, QStyle::StandardPixmap fallback) const;
    void updateButtonStyles(QPushButton *activeButton);

    // 组件
    QWidget *m_parentWidget;
    QFrame *m_sidebar;
    QVBoxLayout *m_sidebarLayout;

    // 用户信息
    QFrame *m_profileWidget;
    QLabel *m_avatarLabel;
    QLabel *m_userNameLabel;
    QString m_username;
    QString m_userRole;

    // 导航按钮
    QPushButton *m_teacherCenterBtn;
    QPushButton *m_newsTrackingBtn;
    QPushButton *m_aiPreparationBtn;
    QPushButton *m_resourceManagementBtn;
    QPushButton *m_learningAnalysisBtn;
    QPushButton *m_dataAnalysisBtn;

    // 底部按钮
    QPushButton *m_settingsBtn;
    QPushButton *m_helpBtn;

    // 当前选中按钮
    QPushButton *m_activeButton;
};

#endif // SIDEBARMANAGER_H
