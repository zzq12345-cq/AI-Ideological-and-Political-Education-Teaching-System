#ifndef QUESTIONBANKWINDOW_H
#define QUESTIONBANKWINDOW_H

#include <QWidget>

class QFrame;
class QLabel;
class QPushButton;
class QResizeEvent;
class QStackedWidget;
class QuestionBasketWidget;
class SmartPaperWidget;
class AIQuestionGenWidget;
class ChatHistoryWidget;
class PaperService;
class QuestionParserService;
class DocxGenerator;
struct PaperQuestion;

class QuestionBankWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionBankWindow(QWidget *parent = nullptr);

signals:
    void backRequested();  // 请求返回主界面

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onComposePaper();  // 打开组卷对话框
    void switchMode(int mode);  // 切换 AI出题 / 智能组卷
    void onSaveGeneratedQuestionsRequested(const QString &content);
    void onGeneratedQuestionsParsed(const QList<PaperQuestion> &questions);
    void onGeneratedQuestionsSaved(int count);
    void onGeneratedQuestionsSaveError(const QString &operation, const QString &error);
    void onGeneratedQuestionsParseError(const QString &error);

private:
    void setupLayout();
    QWidget *buildHeader();
    void loadStyleSheet();

    // AI 出题历史管理
    void refreshQuestionHistory();
    void onQuestionHistorySelected(const QString &id);
    void onQuestionHistoryDeleted(const QString &id);

    // 导出 DOCX（Markdown 直接转换）
    void onExportToDocx(const QString &content);

    // 模式切换（AI出题 / 智能组卷）
    QStackedWidget *m_modeStack = nullptr;
    AIQuestionGenWidget *m_aiQuestionGenWidget = nullptr;
    SmartPaperWidget *m_smartPaperWidget = nullptr;
    QPushButton *m_aiGenTabBtn = nullptr;
    QPushButton *m_smartPaperTabBtn = nullptr;
    QLabel *m_headerTitle = nullptr;
    QLabel *m_headerSubtitle = nullptr;

    // AI 出题页布局包裹层（history + chat）
    QWidget *m_aiGenPageWrapper = nullptr;
    ChatHistoryWidget *m_questionHistoryWidget = nullptr;

    // 试题篮
    QuestionBasketWidget *m_basketWidget = nullptr;

    // AI 出题保存链路
    PaperService *m_paperService = nullptr;
    QuestionParserService *m_questionParser = nullptr;
    bool m_isSavingGeneratedQuestions = false;

    // 导出链路
    DocxGenerator *m_docxGenerator = nullptr;
    bool m_isExporting = false;
};

#endif // QUESTIONBANKWINDOW_H
