#ifndef PAPERSERVICE_H
#define PAPERSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

class NetworkRetryHelper;
class FailedTaskTracker;

// 试卷数据结构
struct Paper {
    QString id;
    QString title;
    QString subject;
    QString grade;
    int totalScore = 100;
    int duration = 90;  // 分钟
    QString paperType;
    QString description;
    QString createdBy;
    QDateTime createdAt;
    QDateTime updatedAt;

    static Paper fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
};

// 题目数据结构（扩展版，含试卷关联）
struct PaperQuestion {
    QString id;
    QString paperId;
    QString questionType;  // single_choice, multi_choice, true_false, short_answer, essay
    QString difficulty;    // easy, medium, hard
    QString stem;          // 题干
    QString material;      // 材料内容（材料论述题专用，支持 HTML 格式含图片/表格）
    QStringList subQuestions;  // 小问列表（材料论述题专用）
    QStringList subAnswers;    // 小问答案列表（材料论述题专用）
    QStringList options;   // 选项
    QString answer;        // 答案
    QString explanation;   // 解析
    int score = 5;
    int orderNum = 0;
    QStringList tags;
    QDateTime createdAt;

    // 新增字段
    QString visibility = "private"; // public / private
    QString subject;
    QString grade;
    QString chapter;
    QStringList knowledgePoints;

    static PaperQuestion fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
};

// 题目检索条件
struct QuestionSearchCriteria {
    QString questionType;
    QString difficulty;
    QStringList tags;
    QString keyword;  // 题干关键词

    // 新增筛选条件
    QString visibility;         // "public" / "private" / "all"
    QString subject;
    QString grade;
    QString chapter;
    QStringList knowledgePoints;

    // 分页
    int offset = 0;           // 分页偏移
    int limit = 30;            // 每页数量
};

class PaperService : public QObject
{
    Q_OBJECT

public:
    explicit PaperService(QObject *parent = nullptr);
    ~PaperService();

    // 设置认证令牌（登录后获取）
    void setAccessToken(const QString &token);

    // ===== 试卷操作 =====
    void createPaper(const Paper &paper);
    void getPapers();
    void getPaperById(const QString &paperId);
    void updatePaper(const Paper &paper);
    void deletePaper(const QString &paperId);

    // ===== 题目操作 =====
    void addQuestion(const PaperQuestion &question);
    void addQuestions(const QList<PaperQuestion> &questions);  // 批量添加
    void getQuestionsByPaperId(const QString &paperId);
    void getQuestionById(const QString &questionId);
    void updateQuestion(const PaperQuestion &question);
    void deleteQuestion(const QString &questionId);

    // ===== 题目检索 =====
    void searchQuestions(const QuestionSearchCriteria &criteria);

signals:
    // 试卷相关信号
    void paperCreated(const Paper &paper);
    void papersLoaded(const QList<Paper> &papers);
    void paperLoaded(const Paper &paper);
    void paperUpdated(const Paper &paper);
    void paperDeleted(const QString &paperId);
    void paperError(const QString &operation, const QString &error);

    // 题目相关信号
    void questionAdded(const PaperQuestion &question);
    void questionsAdded(int count);
    void questionsLoaded(const QList<PaperQuestion> &questions);
    void questionLoaded(const PaperQuestion &question);
    void questionUpdated(const PaperQuestion &question);
    void questionDeleted(const QString &questionId);
    void questionError(const QString &operation, const QString &error);

    // 检索结果
    void searchCompleted(const QList<PaperQuestion> &results);
    void searchCompletedWithTotal(const QList<PaperQuestion> &results, int total);

    // 重试通知
    void requestRetrying(int attempt, int maxRetries);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_accessToken;
    NetworkRetryHelper *m_retryHelper;
    FailedTaskTracker *m_failedTaskTracker;

    // 请求类型标识
    enum class RequestType {
        CreatePaper,
        GetPapers,
        GetPaperById,
        UpdatePaper,
        DeletePaper,
        AddQuestion,
        AddQuestions,
        GetQuestionsByPaper,
        GetQuestionById,
        UpdateQuestion,
        DeleteQuestion,
        SearchQuestions
    };

    // 发送请求
    void sendRequest(const QString &endpoint, RequestType type, 
                    const QJsonDocument &data = QJsonDocument(), 
                    const QString &method = "GET");

    // 处理响应
    void handleResponse(QNetworkReply *reply, RequestType type);

    // 解析数据
    Paper parsePaper(const QJsonObject &json);
    PaperQuestion parseQuestion(const QJsonObject &json);
    QString parseError(const QJsonDocument &doc);
};

#endif // PAPERSERVICE_H
