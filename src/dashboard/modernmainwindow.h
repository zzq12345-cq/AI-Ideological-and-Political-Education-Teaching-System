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
#include <QIcon>
#include <QStyle>
#include <QStackedWidget>
#include <QFrame>
#include <QLineEdit>
#include <QAction>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QQuickWidget>
#include <QQmlContext>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class ModernMainWindow; }
QT_END_NAMESPACE

// 前向声明
class SimpleLoginWindow;
class AIPreparationWidget;
class QuestionRepository;
class QuestionBankWindow;
class DifyService;
class AIChatDialog;
class ChatHistoryWidget;
class PPTXGenerator;
class HotspotTrackingWidget;
class HotspotService;

class ModernMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ModernMainWindow(const QString &userRole = "教师", const QString &username = "王老师", QWidget *parent = nullptr);
    ~ModernMainWindow();

private slots:
    void onTeacherCenterClicked();
    void onAIPreparationClicked();
    void onResourceManagementClicked();
    void onLearningAnalysisClicked();
    void onHotspotTrackingClicked();
    void onSettingsClicked();
    void onHelpClicked();
    void onQuickPreparationClicked();
    void onStartClassClicked();
    void onEnterClassClicked();

    // AI 对话相关槽函数
    void onSendChatMessage();
    void onAIResponseReceived(const QString &response);
    void onAIStreamChunk(const QString &chunk);
    void onAIThinkingChunk(const QString &thought);
    void onAIError(const QString &error);
    void onAIRequestStarted();
    void onAIRequestFinished();

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
    void applySidebarIcons();
    QIcon loadSidebarIcon(const QString &themeName, QStyle::StandardPixmap fallback) const;
    
    // 新版 UI 组件创建方法
    void createWelcomeCard();       // 欢迎卡片
    void createQuickAccessCard();   // 快捷入口卡片
    void createAIChatWidget();      // AI 对话组件
    void appendChatMessage(const QString &sender, const QString &message, bool isUser);
    void swapToHistorySidebar();    // 切换到历史记录侧边栏
    void swapToNavSidebar();        // 切换回导航侧边栏

    // 创建指标项组件 - 紧凑单行信息
    QWidget* createMetricItem(const QString& name,
                              const QString& value,
                              const QString& color,
                              const QString& tooltip,
                              const QString& changeText,
                              int trendDirection);

    // 核心UI组件
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *contentLayout;

    // 侧边栏
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QFrame *profileWidget;

    // 侧边栏导航按钮
    QPushButton *teacherCenterBtn;      // 教师中心
    QPushButton *aiPreparationBtn;      // AI智能备课
    QPushButton *resourceManagementBtn; // 资源库管理
    QPushButton *learningAnalysisBtn;   // 学情与教评
    QPushButton *hotspotTrackingBtn;    // 政治热点追踪
    QPushButton *dataAnalysisBtn;       // 数据分析报告 (新)
    
    // 底部菜单
    QPushButton *settingsBtn;           // 系统设置
    QPushButton *helpBtn;               // 帮助中心

    // 主内容区域
    QStackedWidget *contentStack;
    QWidget *dashboardWidget;
    QScrollArea *dashboardScrollArea;
    AIPreparationWidget *aiPreparationWidget;

    // 试题库相关组件
    QuestionBankWindow *questionBankWindow;
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

    // 核心功能卡片
    QFrame *welcomeCard;
    QFrame *quickAccessCard;
    
    // 移除旧版组件变量
    // QFrame *coreFeaturesFrame; ...

    // AI 对话组件
    DifyService *m_difyService;
    AIChatDialog *m_chatDialog;  // AI 对话框（备用）
    class ChatWidget *m_bubbleChatWidget;  // 气泡样式聊天组件（主面板用）
    ChatHistoryWidget *m_chatHistoryWidget;  // 历史记录侧边栏
    QWidget *m_chatContainer;  // 聊天容器
    QStackedWidget *m_sidebarStack;  // 侧边栏切换栈
    QFrame *m_chatWidget;        // 输入框容器
    QLineEdit *m_chatInput;
    QPushButton *m_sendBtn;
    QString m_currentAIResponse;  // 累积流式响应
    PPTXGenerator *m_pptxGenerator;  // PPTX 生成器
    HotspotTrackingWidget *m_hotspotWidget;  // 热点追踪组件
    HotspotService *m_hotspotService;  // 热点服务
    
    // 欢迎面板（首页显示，对话后隐藏）
    QWidget *m_welcomePanel;
    QWidget *m_welcomeInputWidget;  // 欢迎页面底部输入框
    QStackedWidget *m_mainStack;    // 主内容切换栈
    bool m_isConversationStarted;   // 是否已开始对话

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
