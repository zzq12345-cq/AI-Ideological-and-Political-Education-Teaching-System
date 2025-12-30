#ifndef QUESTIONBANKWINDOW_H
#define QUESTIONBANKWINDOW_H

#include <QList>
#include <QWidget>
#include "../services/PaperService.h"

class QAbstractButton;
class QButtonGroup;
class QComboBox;
class QFrame;
class QLabel;
class QProgressBar;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

class QuestionBankWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionBankWindow(QWidget *parent = nullptr);

public slots:
    void loadQuestions(const QString &questionType = QString());  // 从 Supabase 加载题目，可选按题型筛选
    void onSearchClicked();  // 搜索按钮点击

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onQuestionsReceived(const QList<PaperQuestion> &questions);
    void onQuestionsError(const QString &type, const QString &error);
    void onQuestionTypeChanged();  // 题型筛选变化

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
    QPushButton *m_generateButton = nullptr;
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
};

#endif // QUESTIONBANKWINDOW_H


