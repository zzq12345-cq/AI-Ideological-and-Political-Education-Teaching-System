#ifndef AIQUESTIONGENWIDGET_H
#define AIQUESTIONGENWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>

class ChatWidget;
class QVBoxLayout;
#include <QFrame>
class QPushButton;
class QComboBox;
class PaperService;

/**
 * @brief AI 对话式出题组件
 *
 * 直接调用 MiniMax API（OpenAI 兼容格式），通过自然语言对话生成试题。
 * 支持流式 SSE 响应、多轮对话上下文、历史持久化和 DOCX 导出。
 */
class AIQuestionGenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AIQuestionGenWidget(QWidget *parent = nullptr);
    ~AIQuestionGenWidget();

    // 保存按钮状态
    void setSavingToBank(bool saving);
    void showSaveSuccessMessage(int savedCount, bool directInsert);
    void showSaveErrorMessage(const QString &error);

    // ===== 对话管理（公开接口）=====

    /// 开始新对话：清空当前对话，生成新 UUID
    void startNewConversation();

    /// 加载指定历史对话：从 QSettings 恢复消息 + 渲染到 ChatWidget
    void loadConversation(const QString &id);

    /// 当前会话 ID
    QString currentConversationId() const { return m_conversationId; }

    /// 当前对话标题（从首条用户消息提取前 20 字）
    QString currentConversationTitle() const;

    /// 最后一次 AI 回复（供外部解析导出）
    QString lastAIResponse() const { return m_lastAIResponse; }

    /// 删除指定会话的持久化数据
    void deleteConversation(const QString &id);

    /// 设置 DOCX 导出是否可用
    void setExportAvailable(bool available, const QString &reason = QString());

    /// 获取用于放入 Header 的课标筛选组件
    QWidget* curriculumFilterWidget() const { return m_curriculumBar; }

signals:
    /// 请求将 AI 生成的内容保存到题库
    void saveRequested(const QString &content);

    /// 请求导出试卷（content = AI 原始回复 Markdown）
    void exportRequested(const QString &content);

    /// 对话更新通知（AI 回复完成后发出）
    void conversationUpdated(const QString &id, const QString &title);

private slots:
    void onUserMessageSent(const QString &message);
    void onSaveToBank();

private:
    void setupUI();
    void setupAiService();
    void showWelcome();
    void setupCurriculumBar(QVBoxLayout *mainLayout);
    void refreshChapterOptions(bool preserveSelection = false);
    void applyCurriculumContextChange();
    QString selectedGradeSemester() const;
    QString selectedChapter() const;
    QStringList selectedKnowledgePoints() const;
    QString currentSystemPrompt() const;
    void ensureAssistantMessagePlaceholder();

    // MiniMax API 调用（流式 SSE）
    void sendToMiniMax(const QString &userMessage);
    void processSSEData(const QByteArray &data);

    // 持久化
    void saveCurrentConversation();
    void cancelCurrentReply();
    void updateActionButtons();

    // UI 组件
    ChatWidget *m_chatWidget = nullptr;
    QFrame *m_curriculumBar = nullptr;
    QComboBox *m_gradeCombo = nullptr;
    QComboBox *m_chapterCombo = nullptr;
    QFrame *m_bottomBar = nullptr;
    QPushButton *m_newChatBtn = nullptr;
    QPushButton *m_saveBtn = nullptr;
    QPushButton *m_exportBtn = nullptr;

    // 网络
    QNetworkAccessManager *m_networkManager = nullptr;
    QPointer<QNetworkReply> m_currentReply;
    QString m_apiKey;
    QString m_baseUrl;

    // 对话历史（OpenAI 格式 messages）
    struct ChatMessage {
        QString role;    // "system" / "user" / "assistant"
        QString content;
    };
    QList<ChatMessage> m_conversationHistory;
    QString m_conversationId;   // 当前会话 UUID

    // 状态
    bool m_isGenerating = false;
    bool m_hasPendingAIPlaceholder = false;
    bool m_isSavingToBank = false;
    bool m_exportAvailable = true;
    QString m_lastAIResponse;       // 最后一次完整 AI 回复（用于保存/导出）
    QString m_exportUnavailableReason;
    QByteArray m_sseBuffer;         // SSE 数据缓冲区
    bool m_isUpdatingCurriculumUi = false;

    static QString buildSystemPrompt(const QString &gradeSemester,
                                     const QString &chapter,
                                     const QStringList &knowledgePoints);
};

#endif // AIQUESTIONGENWIDGET_H
