#ifndef QUESTIONBANKWINDOW_H
#define QUESTIONBANKWINDOW_H

#include <QList>
#include <QWidget>

class QAbstractButton;
class QButtonGroup;
class QComboBox;
class QFrame;
class QLabel;
class QProgressBar;
class QPushButton;

class QuestionBankWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionBankWindow(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

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
    QWidget *createQuestionCard();
    QWidget *createOptionItem(const QString &key, const QString &text);
    QFrame *createAnswerSection();
    QFrame *createAnalysisSection();
    QWidget *createActionRow();
    QWidget *createTagRow();

    void connectFilterCombo(QComboBox *combo, const QString &labelText);
    void connectFilterButton(QButtonGroup *group, const QString &labelText);
    void updateProgress(int delta);
    void loadStyleSheet();
    void refreshOptionFrame(QWidget *frame, bool hovered);

    QList<QComboBox *> m_filterCombos;
    QList<QButtonGroup *> m_filterGroups;
    QList<QWidget *> m_optionFrames;

    QButtonGroup *m_optionGroup = nullptr;
    QLabel *m_progressValueLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QPushButton *m_generateButton = nullptr;
    QFrame *m_answerSection = nullptr;
    QFrame *m_analysisSection = nullptr;

    int m_currentQuestion = 1;
    int m_totalQuestions = 12;
};

#endif // QUESTIONBANKWINDOW_H
