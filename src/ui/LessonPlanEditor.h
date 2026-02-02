#ifndef LESSONPLANEDITOR_H
#define LESSONPLANEDITOR_H

#include <QWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>

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

    // 保存辅助函数
    bool saveToPdf(const QString &filePath, const QString &title);
    bool saveToWord(const QString &filePath, const QString &title, const QString &content);
    QString buildHtmlDocument(const QString &title, const QString &content);

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
    QString m_currentConversationId;
};

#endif // LESSONPLANEDITOR_H
