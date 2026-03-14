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

#include <QTabWidget>
#include <QHash>
#include <QDateTime>
#include <QSet>

QT_BEGIN_NAMESPACE
namespace Ui { class ModernMainWindow; }
QT_END_NAMESPACE

// 前向声明
class SimpleLoginWindow;
class AIPreparationWidget;
class QuestionBankWindow;
class DifyService;
class AIChatDialog;
class ChatHistoryWidget;
class PPTXGenerator;
class HotspotTrackingWidget;
class HotspotService;
class DataAnalyticsWidget;
class NotificationService;
class NotificationWidget;
class NotificationBadge;
class AttendanceWidget;  // 考勤管理组件
class LessonPlanEditor;
class PPTPreviewPage;
class ModernMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ModernMainWindow(const QString &userRole = "教师", const QString &username = "王老师", const QString &userId = "", QWidget *parent = nullptr);
    ~ModernMainWindow();

private slots:
    void onTeacherCenterClicked();
    void onNewsTrackingClicked();     // 时政新闻
    void onAIPreparationClicked();
    void onResourceManagementClicked();
    void onLearningAnalysisClicked();
    void onAttendanceClicked();       // 考勤管理
    void onSettingsClicked();
    void onHelpClicked();
    void onQuickPreparationClicked();
    void onStartClassClicked();
    void onEnterClassClicked();

    // 通知相关槽函数
    void onNotificationBtnClicked();
    void onUnreadCountChanged(int count);

    // AI 对话相关槽函数
    void onSendChatMessage();
    void onAIResponseReceived(const QString &response);
    void onAIStreamChunk(const QString &chunk);
    void onAIThinkingChunk(const QString &thought);
    void onAIError(const QString &error);
    void onAIRequestStarted();
    void onAIRequestFinished();
    void onHistoryDeleteRequested(const QString &conversationId);
    void onPPTPreviewBackRequested();
    void onPPTPreviewDownloadRequested();

private:
    enum class AIHistoryType {
        Chat,
        LessonPlan,
        AnalyticsReport,
        CaseAnalysis
    };

    struct AIHistoryEntry {
        QString conversationId;
        AIHistoryType type = AIHistoryType::Chat;
        QString title;
        QDateTime updatedAt;
        QString previewText;
        QString lessonContent;
        QString localFilePath;
    };

    void initUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupStyles();
    void resetAllSidebarButtons();  // 重置所有侧边栏按钮为普通状态
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
    QString historyHeaderTitle(AIHistoryType type) const;
    QString historyNewButtonText(AIHistoryType type) const;
    QString formatHistoryTime(const QDateTime &time) const;
    void refreshHistorySidebar();
    void setActiveHistoryType(AIHistoryType type);
    void resetConversationForType(AIHistoryType type);
    void handleHistorySelection(const QString &conversationId);
    void upsertHistoryEntry(const AIHistoryEntry &entry);
    void loadHistoryEntries();
    void saveHistoryEntries() const;
    void loadDeletedHistoryIds();
    void saveDeletedHistoryIds() const;
    void recordLessonPlanHistory(const QString &conversationId, const QString &title, const QString &content);
    void recordAnalyticsHistory(const QString &conversationId, const QString &title, const QString &content);
    void recordCaseAnalysisHistory(const QString &conversationId, const QString &title);
    void showPPTPreviewPage(const QString &pptPath, const QString &title);
    bool savePPTToUserLocation(const QString &sourcePath, const QString &title);

    // 创建指标项组件 - 紧凑单行信息
    QWidget* createMetricItem(const QString& name,
                              const QString& value,
                              const QString& color,
                              const QString& tooltip,
                              const QString& changeText,
                              int trendDirection);

    // 核心UI组件
    QWidget *centralWidget = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    QHBoxLayout *contentLayout = nullptr;

    // 侧边栏
    QFrame *sidebar = nullptr;
    QVBoxLayout *sidebarLayout = nullptr;
    QFrame *profileWidget = nullptr;
    QLabel *m_avatarLabel = nullptr;      // 头像显示（显示姓氏首字）
    QLabel *m_userNameLabel = nullptr;    // 用户名显示

    // 侧边栏导航按钮
    QPushButton *teacherCenterBtn = nullptr;      // 教师中心
    QPushButton *newsTrackingBtn = nullptr;       // 时政新闻
    QPushButton *aiPreparationBtn = nullptr;      // AI智能备课
    QPushButton *resourceManagementBtn = nullptr; // 资源库管理
    QPushButton *learningAnalysisBtn = nullptr;   // 学情与教评
    QPushButton *attendanceBtn = nullptr;         // 考勤管理
    QPushButton *dataAnalysisBtn = nullptr;       // 数据分析报告 (新)

    // 底部菜单
    QPushButton *settingsBtn = nullptr;           // 系统设置
    QPushButton *helpBtn = nullptr;               // 帮助中心
    QPushButton *logoutBtn = nullptr;             // 退出登录

    // 主内容区域
    QStackedWidget *contentStack = nullptr;
    QWidget *dashboardWidget = nullptr;
    QScrollArea *dashboardScrollArea = nullptr;
    AIPreparationWidget *aiPreparationWidget = nullptr;
    PPTPreviewPage *m_pptPreviewPage = nullptr;

    // 试题库相关组件
    QuestionBankWindow *questionBankWindow = nullptr;

    // 时政新闻相关组件
    HotspotTrackingWidget *m_hotspotWidget = nullptr;
    HotspotService *m_hotspotService = nullptr;

    // 数据分析报告组件
    DataAnalyticsWidget *m_dataAnalyticsWidget = nullptr;

    // 考勤管理组件
    AttendanceWidget *m_attendanceWidget = nullptr;

    // 通知系统组件
    NotificationService *m_notificationService = nullptr;
    NotificationWidget *m_notificationWidget = nullptr;

    // 顶部工具栏 (Header)
    QFrame *headerWidget = nullptr;
    QHBoxLayout *headerLayout = nullptr;
    QLabel *titleLabel = nullptr;
    QLineEdit *searchInput = nullptr;
    QPushButton *notificationBtn = nullptr;
    NotificationBadge *m_notificationBadge = nullptr;  // 通知小红点

    // 仪表板内容组件
    QLabel *welcomeLabel = nullptr;
    QLabel *subtitleLabel = nullptr;

    // 核心功能卡片
    QFrame *welcomeCard = nullptr;
    QFrame *quickAccessCard = nullptr;
    
    // 移除旧版组件变量
    // QFrame *coreFeaturesFrame; ...

    // AI 对话组件
    DifyService *m_difyService = nullptr;
    AIChatDialog *m_chatDialog = nullptr;  // AI 对话框（备用）
    class ChatWidget *m_bubbleChatWidget = nullptr;  // 气泡样式聊天组件（主面板用）
    ChatHistoryWidget *m_chatHistoryWidget = nullptr;  // 历史记录侧边栏
    QTabWidget *m_aiTabWidget = nullptr;           // AI智能备课标签页容器
    LessonPlanEditor *m_lessonPlanEditor = nullptr; // 教案编辑器
    QWidget *m_chatContainer = nullptr;  // 聊天容器
    QStackedWidget *m_sidebarStack = nullptr;  // 侧边栏切换栈
    QFrame *m_chatWidget = nullptr;        // 输入框容器
    QLineEdit *m_chatInput = nullptr;
    QPushButton *m_sendBtn = nullptr;
    QString m_currentAIResponse;  // 累积流式响应
    QTimer *m_streamUpdateTimer = nullptr;  // 流式更新节流定时器
    bool m_streamUpdatePending = false;   // 是否有待处理的更新
    PPTXGenerator *m_pptxGenerator = nullptr;  // PPTX 生成器

    // PPT 模拟生成相关
    QTimer *m_pptSimulationTimer = nullptr;     // PPT 模拟思考定时器
    int m_pptSimulationStep = 0;          // 当前模拟步骤
    QString m_pendingPPTPath;         // 待提供的 PPT 文件路径
    QString m_pptTopic;               // 用户请求的 PPT 主题
    bool m_isPptQuestionFlowActive = false;
    int m_pptQuestionStep = 0;
    QString m_pendingPptRequest;
    QString m_previewPPTPath;         // 当前预览的 PPT 文件路径
    QString m_previewPPTTitle;        // 当前预览的 PPT 标题
    void startPPTQuestionFlow(const QString &userMessage); // PPT 伪提问流程
    void askNextPPTQuestion();
    void handlePPTQuestionAnswer(const QString &answer);
    void resetPPTQuestionFlow();
    void startPPTSimulation(const QString &userMessage);  // 开始 PPT 模拟生成
    void onPPTSimulationStep();       // PPT 模拟步骤处理
    bool isPPTGenerationRequest(const QString &message);  // 检测是否是 PPT 生成请求
    QString extractPPTTopic(const QString &message) const; // 从用户消息中提取主题

    // 欢迎面板（首页显示，对话后隐藏）
    QWidget *m_welcomePanel = nullptr;
    QWidget *m_welcomeInputWidget = nullptr;  // 欢迎页面底部输入框
    QStackedWidget *m_mainStack = nullptr;    // 主内容切换栈
    bool m_isConversationStarted = false;   // 是否已开始对话
    AIHistoryType m_activeHistoryType = AIHistoryType::Chat;
    AIHistoryType m_pendingHistoryType = AIHistoryType::Chat;
    QString m_pendingHistoryTitle;
    QString m_pendingLessonPlanContent;
    QString m_pendingAnalyticsContent;
    QString m_pendingCasePrompt;
    QHash<QString, AIHistoryEntry> m_historyEntries;
    QSet<QString> m_deletedHistoryIds;
    QString m_selectedHistoryConversationId;

    // 数据
    QString currentUserRole;
    QString currentUsername;
    QString currentUserId;

    // 菜单动作
    QAction *profileAction = nullptr;
    QAction *settingsAction = nullptr;
    QAction *helpAction = nullptr;
    QAction *aboutAction = nullptr;
    QAction *logoutAction = nullptr;
};

#endif // MODERNMAINWINDOW_H
