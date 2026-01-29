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
class DataAnalyticsWidget;

class ModernMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ModernMainWindow(const QString &userRole = "教师", const QString &username = "王老师", QWidget *parent = nullptr);
    ~ModernMainWindow();

private slots:
    void onTeacherCenterClicked();
    void onNewsTrackingClicked();     // 时政新闻
    void onAIPreparationClicked();
    void onResourceManagementClicked();
    void onLearningAnalysisClicked();
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
    QLabel *m_avatarLabel;      // 头像显示（显示姓氏首字）
    QLabel *m_userNameLabel;    // 用户名显示

    // 侧边栏导航按钮
    QPushButton *teacherCenterBtn;      // 教师中心
    QPushButton *newsTrackingBtn;       // 时政新闻
    QPushButton *aiPreparationBtn;      // AI智能备课
    QPushButton *resourceManagementBtn; // 资源库管理
    QPushButton *learningAnalysisBtn;   // 学情与教评
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

    // 时政新闻相关组件
    HotspotTrackingWidget *m_hotspotWidget;
    HotspotService *m_hotspotService;

    // 数据分析报告组件
    DataAnalyticsWidget *m_dataAnalyticsWidget;

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
    DifyService *m_difyService = nullptr;
    AIChatDialog *m_chatDialog = nullptr;  // AI 对话框（备用）
    class ChatWidget *m_bubbleChatWidget = nullptr;  // 气泡样式聊天组件（主面板用）
    ChatHistoryWidget *m_chatHistoryWidget = nullptr;  // 历史记录侧边栏
    QWidget *m_chatContainer;  // 聊天容器
    QStackedWidget *m_sidebarStack;  // 侧边栏切换栈
    QFrame *m_chatWidget;        // 输入框容器
    QLineEdit *m_chatInput;
    QPushButton *m_sendBtn;
    QString m_currentAIResponse;  // 累积流式响应
    QTimer *m_streamUpdateTimer;  // 流式更新节流定时器
    bool m_streamUpdatePending;   // 是否有待处理的更新
    PPTXGenerator *m_pptxGenerator;  // PPTX 生成器

    // PPT 模拟生成相关
    QTimer *m_pptSimulationTimer;     // PPT 模拟思考定时器
    int m_pptSimulationStep;          // 当前模拟步骤
    QString m_pendingPPTPath;         // 待提供的 PPT 文件路径
    int m_pptQuestionStep;            // PPT 问答阶段（0=未开始，1-3=问问题，4=生成中）
    QStringList m_pptUserAnswers;     // 用户的回答记录
    QTimer *m_pptTypingTimer;         // 打字效果定时器
    QString m_pptTypingText;          // 待打字的完整文本
    int m_pptTypingIndex;             // 当前打字位置
    void startPPTSimulation(const QString &userMessage);  // 开始 PPT 模拟生成
    void onPPTSimulationStep();       // PPT 模拟步骤处理
    bool isPPTGenerationRequest(const QString &message);  // 检测是否是 PPT 生成请求
    void handlePPTConversation(const QString &message);   // 处理 PPT 问答对话
    void typeMessageWithEffect(const QString &text);      // 带打字效果的消息显示
    void onPPTTypingStep();           // 打字效果定时器回调

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
