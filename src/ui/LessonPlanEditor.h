#ifndef LESSONPLANEDITOR_H
#define LESSONPLANEDITOR_H

#include <QMap>

#include <QWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QTimer>
#include <QSettings>

class DifyService;
class MarkdownRenderer;

/**
 * @brief 教案编辑器组件
 *
 * 所见即所得的富文本教案编辑器，支持：
 * - 课程目录级联选择（年级→学期→单元→课时）
 * - AI一键生成教案
 * - 富文本格式化工具栏
 * - 保存功能
 */
class LessonPlanEditor : public QWidget
{
    Q_OBJECT

public:
    explicit LessonPlanEditor(QWidget *parent = nullptr);
    ~LessonPlanEditor();

    // 获取当前教案内容（HTML格式）
    QString getContent() const;

    // 设置教案内容
    void setContent(const QString &html);

    // 获取当前选择的课时标题
    QString getCurrentLessonTitle() const;

    // 清空编辑器
    void clear();

    void setDifyService(DifyService *service);
    void setConversationId(const QString &conversationId);
    QString conversationId() const;
    bool isGenerating() const { return m_isGenerating; }

signals:
    // 内容变化信号
    void contentChanged();

    // 保存请求信号
    void saveRequested(const QString &lessonTitle, const QString &content);

    // AI生成开始/完成信号
    void aiGenerationStarted();
    void aiGenerationFinished();

private slots:
    // 课程选择级联更新
    void onGradeChanged(int index);
    void onSemesterChanged(int index);
    void onUnitChanged(int index);
    void onLessonChanged(int index);

    // AI生成教案
    void onAIGenerateClicked();
    void onAIStreamChunk(const QString &chunk);
    void onAIFinished();
    void onAIError(const QString &error);

    // 保存
    void onSaveClicked();

    // 格式化工具栏
    void onBoldClicked();
    void onItalicClicked();
    void onHeading1Clicked();
    void onHeading2Clicked();
    void onHeading3Clicked();
    void onBulletListClicked();
    void onNumberedListClicked();

    // 内容变化
    void onTextChanged();

private:
    void initUI();
    void initCourseSelector();
    void initToolbar();
    void initEditor();
    void initStatusBar();
    void connectSignals();
    void loadCurriculum();
    void updateWordCount();
    QString buildAIPrompt() const;
    void flushPendingAIText(bool flushAll = false);

    // 教案结构化章节
    struct LessonPlanSections {
        QString knowledgeSkills;   // 知识与技能
        QString processMethod;     // 过程与方法
        QString emotionValues;     // 情感态度与价值观
        QString keyPoints;         // 重点
        QString difficulties;      // 难点
        QString introduction;      // 导入新课
        QString mainTeaching;      // 新课讲授
        QString classExercise;     // 课堂练习
        QString classSummary;      // 课堂小结
        QString boardDesign;       // 板书设计
        QString homework;          // 作业布置
        QString reflection;        // 教学反思
    };

    // 解析 Markdown 教案为结构化章节
    LessonPlanSections parseLessonPlanSections(const QString &markdown) const;

    // 将纯文本列表项转为 HTML 列表
    QString textToHtmlList(const QString &text) const;

    // 保存辅助函数
    bool saveToPdf(const QString &filePath, const QString &title);
    bool saveToWord(const QString &filePath, const QString &title, const QString &content);
    QString buildHtmlDocument(const QString &title, const QString &content);
    QString buildStructuredHtml(const QString &title, const LessonPlanSections &sections, bool forPrint = false) const;

    // UI组件 - 课程选择区
    QComboBox *m_gradeCombo;
    QComboBox *m_semesterCombo;
    QComboBox *m_unitCombo;
    QComboBox *m_lessonCombo;
    QPushButton *m_aiGenerateBtn;
    QPushButton *m_saveBtn;

    // UI组件 - 工具栏
    QToolBar *m_toolbar;
    QPushButton *m_boldBtn;
    QPushButton *m_italicBtn;
    QPushButton *m_heading1Btn;
    QPushButton *m_heading2Btn;
    QPushButton *m_heading3Btn;
    QPushButton *m_bulletListBtn;
    QPushButton *m_numberedListBtn;
    QPushButton *m_undoBtn;
    QPushButton *m_redoBtn;

    // UI组件 - 编辑区
    QTextEdit *m_editor;

    // UI组件 - 状态栏
    QLabel *m_wordCountLabel;
    QLabel *m_statusLabel;

    // 服务
    DifyService *m_difyService;
    MarkdownRenderer *m_markdownRenderer;

    // 状态
    bool m_isGenerating;
    bool m_isModified;
    QString m_accumulatedMarkdown;  // AI生成时累积的Markdown内容
    QString m_pendingMarkdown;      // 等待平滑显示的AI文本
    QString m_currentConversationId;

    // 自动保存
    QTimer *m_autoSaveTimer;
    QTimer *m_streamRenderTimer;
    void autoSave();
    void checkAndRestoreDraft();
    void clearAutoSaveDraft();
};

#endif // LESSONPLANEDITOR_H
