#ifndef MODERNMAINWINDOW_H
#define MODERNMAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QScrollArea>
#include <QStackedWidget>
#include <QFrame>
#include <QLineEdit>
#include <QAction>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QQuickWidget>
#include <QQmlContext>

QT_BEGIN_NAMESPACE
namespace Ui { class ModernMainWindow; }
QT_END_NAMESPACE

// 前向声明
class SimpleLoginWindow;
class AIPreparationWidget;
class QuestionRepository;

class ModernMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ModernMainWindow(const QString &userRole = "教师", const QString &username = "王老师", QWidget *parent = nullptr);
    ~ModernMainWindow();

private slots:
    void onTeacherCenterClicked();
    void onContentAnalysisClicked();
    void onAIPreparationClicked();
    void onResourceManagementClicked();
    void onLearningAnalysisClicked();
    void onDataReportClicked();
    void onSettingsClicked();
    void onHelpClicked();
    void onQuickPreparationClicked();
    void onStartClassClicked();
    void onEnterClassClicked();

private:
    void initUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupStyles();
    void createDashboard();
    void createSidebarProfile();
    void createHeaderWidget();
    void createQuickActions();
    void createCoreFeatures();
    void createRecentCourses();
    void createLearningAnalytics();
    void createRecentActivities();
    void applyPatrioticRedTheme();

    // 创建指标项组件 - 紧凑单行信息
    QWidget* createMetricItem(const QString& name, const QString& value, const QString& color, const QString& tooltip);

    // 核心UI组件
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *contentLayout;

    // 侧边栏
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QFrame *profileWidget;

    // 侧边栏导航按钮
    QPushButton *teacherCenterBtn;
    QPushButton *contentAnalysisBtn;
    QPushButton *aiPreparationBtn;
    QPushButton *resourceManagementBtn;
    QPushButton *learningAnalysisBtn;
    QPushButton *dataReportBtn;
    QPushButton *settingsBtn;
    QPushButton *helpBtn;

    // 主内容区域
    QStackedWidget *contentStack;
    QWidget *dashboardWidget;
    QScrollArea *dashboardScrollArea;
    AIPreparationWidget *aiPreparationWidget;

    // 试题库相关组件
    QQuickWidget *questionBankQuickWidget;
    QuestionRepository *questionRepository;

    // 顶部工具栏 (Header)
    QFrame *headerWidget;
    QHBoxLayout *headerLayout;
    QLabel *titleLabel;
    QLineEdit *searchInput;
    QPushButton *notificationBtn;
    QPushButton *headerProfileBtn;

    // 仪表板内容组件
    QLabel *welcomeLabel;
    QLabel *subtitleLabel;
    QFrame *quickActionsFrame;
    QPushButton *quickPreparationBtn;
    QPushButton *startClassBtn;

    // 核心功能卡片
    QFrame *coreFeaturesFrame;
    QGridLayout *coreFeaturesLayout;
    QPushButton *psychologyCard;
    QPushButton *editDocumentCard;
    QPushButton *slideshowCard;
    QPushButton *folderOpenCard;

    // 近期课程和学情分析
    QFrame *coursesAnalyticsFrame;
    QGridLayout *coursesAnalyticsLayout;
    QFrame *recentCoursesFrame;
    QFrame *learningAnalyticsFrame;
    QPushButton *enterClassBtn;

    // 近期活动
    QFrame *recentActivitiesFrame;
    QVBoxLayout *recentActivitiesLayout;

    // 新布局：双行网格
    QFrame *analyticsRowFrame;
    QWidget *chartsContainer;

    // 数据
    QString currentUserRole;
    QString currentUsername;

    // 菜单动作
    QAction *profileAction;
    QAction *settingsAction;
    QAction *helpAction;
    QAction *aboutAction;
    QAction *logoutAction;
};

#endif // MODERNMAINWINDOW_H