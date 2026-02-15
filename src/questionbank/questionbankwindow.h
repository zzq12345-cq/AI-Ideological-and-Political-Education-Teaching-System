#ifndef QUESTIONBANKWINDOW_H
#define QUESTIONBANKWINDOW_H

#include <QList>
#include <QMap>
#include <QWidget>
#include "../services/PaperService.h"

class QAbstractButton;
class QButtonGroup;
class QComboBox;
class QFrame;
class QLabel;
class QProgressBar;
class QPushButton;
class QResizeEvent;
class QScrollArea;
class QStackedWidget;
class QVBoxLayout;
class QuestionBasketWidget;
class SmartPaperWidget;

// 材料论述题解析结果
struct MaterialEssayParsed {
    QString material;           // 材料内容
    QStringList subQuestions;   // 小问列表
    QStringList subAnswers;     // 小问答案
    QString stem;               // 题目要求（如"阅读材料，回答下列问题"）
};

class QuestionBankWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionBankWindow(QWidget *parent = nullptr);

public slots:
    void loadQuestions(const QString &questionType = QString());  // 从 Supabase 加载题目，可选按题型筛选
    void onSearchClicked();  // 搜索按钮点击

signals:
    void backRequested();  // 请求返回主界面

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onQuestionsReceived(const QList<PaperQuestion> &questions);
    void onQuestionsError(const QString &type, const QString &error);
    void onQuestionTypeChanged();  // 题型筛选变化
    void onAddToBasket(const PaperQuestion &question);  // 添加试题到篮子
    void onRemoveFromBasket(const QString &questionId);  // 从篮子移除试题
    void onComposePaper();  // 打开组卷对话框
    void updateAddToBasketButton(const QString &questionId, bool inBasket);  // 更新按钮状态
    void switchMode(int mode);  // 切换题库浏览/智能组卷模式

private:
    void setupLayout();
    QWidget *buildHeader();
    QWidget *buildBody();
    QWidget *buildSidebar();
    QWidget *buildContentArea();
    QWidget *createComboField(const QString &labelText, const QStringList &options);
    QWidget *createFilterButtons(const QString &labelText,
                                 const QStringList &options,
                                 const QString &groupId,
                                 int columns = 2);
    QButtonGroup *createFilterButtonsWithGroup(const QString &labelText,
                                               const QStringList &options,
                                               const QString &groupId,
                                               int columns,
                                               QVBoxLayout *parentLayout);  // 返回按钮组引用
    QWidget *createQuestionCard(const PaperQuestion &question, int index);
    QWidget *createOptionItem(const QString &key, const QString &text);
    QFrame *createAnswerSection(const QString &answer);
    QFrame *createAnalysisSection(const QString &explanation);
    QWidget *createActionRow();
    QWidget *createTagRow(const QStringList &tags);
    
    void clearQuestionCards();
    void displayQuestions(const QList<PaperQuestion> &questions);
    QString getSelectedQuestionType();  // 获取当前选中的题型
    MaterialEssayParsed parseMaterialEssay(const PaperQuestion &question);  // 智能解析材料论述题

    void connectFilterCombo(QComboBox *combo, const QString &labelText);
    void connectFilterButton(QButtonGroup *group, const QString &labelText);
    void updateProgress(int delta);
    void loadStyleSheet();
    void refreshOptionFrame(QWidget *frame, bool hovered);

    // UI 组件
    QList<QComboBox *> m_filterCombos;
    QList<QButtonGroup *> m_filterGroups;
    QList<QWidget *> m_optionFrames;

    QButtonGroup *m_optionGroup = nullptr;
    QButtonGroup *m_questionTypeGroup = nullptr;  // 题型筛选按钮组
    QLabel *m_progressValueLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QFrame *m_answerSection = nullptr;
    QFrame *m_analysisSection = nullptr;
    
    // 动态题目相关
    PaperService *m_paperService = nullptr;
    QVBoxLayout *m_questionListLayout = nullptr;
    QScrollArea *m_questionScrollArea = nullptr;
    QList<PaperQuestion> m_questions;
    QLabel *m_statusLabel = nullptr;
    
    // 筛选条件
    QComboBox *m_subjectCombo = nullptr;
    QComboBox *m_gradeCombo = nullptr;
    QComboBox *m_difficultyCombo = nullptr;
    QComboBox *m_typeCombo = nullptr;

    int m_currentQuestion = 1;
    int m_totalQuestions = 0;

    // 试题篮相关
    QuestionBasketWidget *m_basketWidget = nullptr;
    QMap<QString, QPushButton *> m_addToBasketButtons;  // questionId -> button

    // 模式切换（题库浏览 / 智能组卷）
    QStackedWidget *m_modeStack = nullptr;
    SmartPaperWidget *m_smartPaperWidget = nullptr;
    QPushButton *m_browseTabBtn = nullptr;
    QPushButton *m_smartPaperTabBtn = nullptr;
    QLabel *m_headerTitle = nullptr;
    QLabel *m_headerSubtitle = nullptr;
};

#endif // QUESTIONBANKWINDOW_H


