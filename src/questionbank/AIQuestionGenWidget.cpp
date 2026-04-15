#include "AIQuestionGenWidget.h"
#include "CurriculumData.h"
#include "../ui/ChatWidget.h"
#include "../config/AppConfig.h"
#include "../config/embedded_keys.h"
#include "../utils/NetworkRequestFactory.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslConfiguration>
#include <QUuid>
#include <QSettings>
#include <QDateTime>

namespace {
// 绿色教育风格配色（与 SmartPaperWidget / QuestionBankWindow 统一）
const QString ACCENT_GREEN = "#2E7D32";
const QString ACCENT_GREEN_HOVER = "#1B5E20";
const QString ACCENT_BLUE = "#1565C0";
const QString ACCENT_BLUE_HOVER = "#0D47A1";
const QString TEXT_SECONDARY = "#6B7280";
const QString BORDER_SUBTLE = "#E5E7EB";
const QString BG_PAGE = "#F8F9FA";
}

AIQuestionGenWidget::AIQuestionGenWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupZhipuService();

    // 初始分配一个新会话 ID
    m_conversationId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    showWelcome();
}

AIQuestionGenWidget::~AIQuestionGenWidget()
{
    cancelCurrentReply();
}

void AIQuestionGenWidget::setSavingToBank(bool saving)
{
    m_isSavingToBank = saving;
    if (m_saveBtn) {
        m_saveBtn->setText(saving ? "保存中..." : "💾 保存到题库");
    }
    updateActionButtons();
}

void AIQuestionGenWidget::showSaveSuccessMessage(int savedCount, bool directInsert)
{
    setSavingToBank(false);
    QMessageBox::information(
        this, "保存成功",
        directInsert
            ? QString("AI 生成的试题已通过工作流直接入库（共 %1 题）。").arg(savedCount)
            : QString("AI 生成的试题已保存到题库（共 %1 题）。").arg(savedCount)
    );
}

void AIQuestionGenWidget::showSaveErrorMessage(const QString &error)
{
    setSavingToBank(false);
    QMessageBox::warning(this, "保存失败", error);
}

void AIQuestionGenWidget::setExportAvailable(bool available, const QString &reason)
{
    m_exportAvailable = available;
    m_exportUnavailableReason = reason;
    updateActionButtons();
}

// ===================== UI 构建 =====================

void AIQuestionGenWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setStyleSheet(QString("AIQuestionGenWidget { background-color: %1; }").arg(BG_PAGE));

    setupCurriculumBar(mainLayout);

    // ====== 聊天区域 ======
    m_chatWidget = new ChatWidget(this);
    m_chatWidget->setMarkdownEnabled(true);
    m_chatWidget->setPlaceholderText("输入出题需求，例如：帮我出5道关于宪法的选择题...");
    mainLayout->addWidget(m_chatWidget, 1);

    connect(m_chatWidget, &ChatWidget::messageSent,
            this, &AIQuestionGenWidget::onUserMessageSent);

    // ====== 底部操作栏 ======
    m_bottomBar = new QFrame(this);
    m_bottomBar->setObjectName("aiGenBottomBar");
    m_bottomBar->setStyleSheet(
        "QFrame#aiGenBottomBar {"
        "    background-color: #FFFFFF;"
        "    border-top: 1px solid " + BORDER_SUBTLE + ";"
        "    border-radius: 0px;"
        "}"
    );
    m_bottomBar->setFixedHeight(56);

    auto *barShadow = new QGraphicsDropShadowEffect(m_bottomBar);
    barShadow->setBlurRadius(12);
    barShadow->setOffset(0, -2);
    barShadow->setColor(QColor(0, 0, 0, 15));
    m_bottomBar->setGraphicsEffect(barShadow);

    auto *barLayout = new QHBoxLayout(m_bottomBar);
    barLayout->setContentsMargins(20, 8, 20, 8);
    barLayout->setSpacing(12);

    // 新对话按钮
    m_newChatBtn = new QPushButton(" 新对话");
    m_newChatBtn->setIcon(QIcon(":/icons/resources/icons/refresh.svg"));
    m_newChatBtn->setIconSize(QSize(18, 18));
    m_newChatBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: %1;"
        "    border: 1.5px solid %2;"
        "    border-radius: 10px;"
        "    padding: 8px 20px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #F5F5F5; }"
    ).arg(TEXT_SECONDARY, BORDER_SUBTLE));
    m_newChatBtn->setCursor(Qt::PointingHandCursor);
    connect(m_newChatBtn, &QPushButton::clicked,
            this, &AIQuestionGenWidget::startNewConversation);

    barLayout->addWidget(m_newChatBtn);
    barLayout->addStretch();

    // 导出试卷按钮
    m_exportBtn = new QPushButton(" 导出试卷");
    m_exportBtn->setIcon(QIcon(":/icons/resources/icons/export-white.svg"));
    m_exportBtn->setIconSize(QSize(18, 18));
    m_exportBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %1, stop:1 #1976D2);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 8px 24px;"
        "    font-size: 13px;"
        "    font-weight: 700;"
        "    letter-spacing: 0.3px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %2, stop:1 #1565C0);"
        "}"
        "QPushButton:disabled {"
        "    background-color: #E5E7EB;"
        "    color: #9CA3AF;"
        "}"
    ).arg(ACCENT_BLUE, ACCENT_BLUE_HOVER));
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setEnabled(false);
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        if (m_exportAvailable && !m_lastAIResponse.trimmed().isEmpty()) {
            emit exportRequested(m_lastAIResponse);
        }
    });
    barLayout->addWidget(m_exportBtn);

    // 保存到题库按钮
    m_saveBtn = new QPushButton(" 保存到题库");
    m_saveBtn->setIcon(QIcon(":/icons/resources/icons/save-white.svg"));
    m_saveBtn->setIconSize(QSize(18, 18));
    m_saveBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %1, stop:1 #388E3C);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 8px 24px;"
        "    font-size: 13px;"
        "    font-weight: 700;"
        "    letter-spacing: 0.3px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %2, stop:1 #2E7D32);"
        "}"
        "QPushButton:disabled {"
        "    background-color: #E5E7EB;"
        "    color: #9CA3AF;"
        "}"
    ).arg(ACCENT_GREEN, ACCENT_GREEN_HOVER));
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setEnabled(false);
    connect(m_saveBtn, &QPushButton::clicked,
            this, &AIQuestionGenWidget::onSaveToBank);
    barLayout->addWidget(m_saveBtn);

    mainLayout->addWidget(m_bottomBar);
}

void AIQuestionGenWidget::setupCurriculumBar(QVBoxLayout *mainLayout)
{
    m_curriculumBar = new QFrame(this);
    m_curriculumBar->setObjectName("aiCurriculumBar");
    m_curriculumBar->setStyleSheet(
        "QFrame#aiCurriculumBar {"
        "    background-color: #FFFFFF;"
        "    border-bottom: 1px solid " + BORDER_SUBTLE + ";"
        "}"
        "QComboBox {"
        "    background-color: #F9FAFB;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 10px;"
        "    padding: 8px 12px;"
        "    min-height: 20px;"
        "    color: #111827;"
        "    font-size: 13px;"
        "}"
        "QComboBox:hover { border-color: #9CA3AF; background-color: #FFFFFF; }"
        "QComboBox:focus { border-color: #2E7D32; background-color: #FFFFFF; }"
    );

    auto *layout = new QHBoxLayout(m_curriculumBar);
    layout->setContentsMargins(20, 14, 20, 14);
    layout->setSpacing(12);

    auto *scopeLabel = new QLabel("课标范围");
    scopeLabel->setStyleSheet("QLabel { font-size: 13px; font-weight: 700; color: #111827; }");
    layout->addWidget(scopeLabel);

    m_gradeCombo = new QComboBox(m_curriculumBar);
    m_gradeCombo->addItem("不限年级册别");
    for (const auto &grade : CurriculumData::gradeSemesters()) {
        m_gradeCombo->addItem(grade);
    }
    layout->addWidget(m_gradeCombo, 1);

    m_chapterCombo = new QComboBox(m_curriculumBar);
    layout->addWidget(m_chapterCombo, 2);
    layout->addStretch();

    connect(m_gradeCombo, &QComboBox::currentTextChanged, this, [this]() {
        if (m_isUpdatingCurriculumUi) {
            return;
        }
        refreshChapterOptions(false);
        applyCurriculumContextChange();
    });

    connect(m_chapterCombo, &QComboBox::currentTextChanged, this, [this]() {
        if (m_isUpdatingCurriculumUi) {
            return;
        }
        applyCurriculumContextChange();
    });

    refreshChapterOptions(false);
    mainLayout->addWidget(m_curriculumBar);
}

void AIQuestionGenWidget::setupZhipuService()
{
    m_networkManager = new QNetworkAccessManager(this);

    m_apiKey = AppConfig::get("ZHIPU_QUESTION_API_KEY");
    if (m_apiKey.isEmpty()) {
        m_apiKey = AppConfig::get("ZHIPU_API_KEY", EmbeddedKeys::ZHIPU_API_KEY);
    }
    m_baseUrl = AppConfig::get("ZHIPU_BASE_URL", EmbeddedKeys::ZHIPU_BASE_URL);

    qDebug() << "[AIQuestionGen] 智谱 API 配置:"
             << "Key长度:" << m_apiKey.length()
             << "Base URL:" << m_baseUrl
             << "Model:" << MODEL_NAME;

    // 初始化对话历史
    m_conversationHistory.clear();
    m_conversationHistory.append({QStringLiteral("system"), currentSystemPrompt()});
}

void AIQuestionGenWidget::showWelcome()
{
    const QString gradeSemester = selectedGradeSemester();
    const QString chapter = selectedChapter();
    const QStringList knowledgePoints = selectedKnowledgePoints();

    QString welcome =
        "你好！我是 **AI 出题助手** 💡\n\n"
        "我会根据你当前选择的课标范围生成更贴教材、更贴章节的思政试题。";

    if (!gradeSemester.isEmpty() && !chapter.isEmpty()) {
        welcome += QString(
            "\n\n**当前范围**：%1 · %2\n"
            "**核心考点**：%3\n"
            "**高频考法**：%4"
        ).arg(
            gradeSemester,
            chapter,
            knowledgePoints.isEmpty() ? QStringLiteral("本单元核心内容") : knowledgePoints.join("、"),
            CurriculumData::highFrequencyPatternsFor(gradeSemester, chapter).join("；")
        );
        m_chatWidget->setPlaceholderText(
            QString("例如：为%1 %2出3道材料分析题，难度中等...")
                .arg(gradeSemester, chapter));
    } else if (!gradeSemester.isEmpty()) {
        welcome += QString(
            "\n\n**当前范围**：%1\n"
            "**核心考点示例**：%2"
        ).arg(
            gradeSemester,
            knowledgePoints.isEmpty() ? QStringLiteral("请继续选择章节以缩小范围") : knowledgePoints.join("、")
        );
        m_chatWidget->setPlaceholderText(
            QString("例如：围绕%1核心内容出5道选择题...").arg(gradeSemester));
    } else {
        welcome +=
            "\n\n你可以先在上方选择 **年级册别 + 章节**，再生成更精准的题目。"
            "\n也可以直接输入需求，我会按通用课程范围出题。";
        m_chatWidget->setPlaceholderText("输入出题需求，例如：帮我出5道关于宪法的选择题...");
    }

    welcome += "\n\n直接输入需求，或点击下方快捷按钮开始 👇";
    m_chatWidget->addMessage(welcome, false);
    m_chatWidget->addQuickReplyOptions(CurriculumData::quickPromptsFor(gradeSemester, chapter));
}

void AIQuestionGenWidget::refreshChapterOptions(bool preserveSelection)
{
    if (!m_chapterCombo) {
        return;
    }

    const QString previous = preserveSelection ? selectedChapter() : QString();

    m_isUpdatingCurriculumUi = true;
    m_chapterCombo->clear();
    m_chapterCombo->addItem("不限章节");

    const QString gradeSemester = selectedGradeSemester();
    if (!gradeSemester.isEmpty()) {
        for (const auto &chapter : CurriculumData::chaptersForGradeSemester(gradeSemester)) {
            m_chapterCombo->addItem(chapter);
        }
    }

    if (!previous.isEmpty()) {
        const int index = m_chapterCombo->findText(previous);
        if (index >= 0) {
            m_chapterCombo->setCurrentIndex(index);
        }
    }
    m_isUpdatingCurriculumUi = false;
}

void AIQuestionGenWidget::applyCurriculumContextChange()
{
    if (m_isUpdatingCurriculumUi) {
        return;
    }

    const QString oldId = m_conversationId;
    const QString oldTitle = currentConversationTitle();
    bool hasUserMessage = false;
    for (const auto &msg : m_conversationHistory) {
        if (msg.role == "user") {
            hasUserMessage = true;
            break;
        }
    }

    if (hasUserMessage && !oldId.isEmpty()) {
        saveCurrentConversation();
        emit conversationUpdated(oldId, oldTitle);
    }

    startNewConversation();
}

QString AIQuestionGenWidget::selectedGradeSemester() const
{
    if (!m_gradeCombo) {
        return {};
    }
    const QString value = m_gradeCombo->currentText().trimmed();
    return value == "不限年级册别" ? QString() : value;
}

QString AIQuestionGenWidget::selectedChapter() const
{
    if (!m_chapterCombo) {
        return {};
    }
    const QString value = m_chapterCombo->currentText().trimmed();
    return value == "不限章节" ? QString() : value;
}

QStringList AIQuestionGenWidget::selectedKnowledgePoints() const
{
    return CurriculumData::promptKnowledgePointsFor(selectedGradeSemester(), selectedChapter());
}

QString AIQuestionGenWidget::currentSystemPrompt() const
{
    return buildSystemPrompt(selectedGradeSemester(), selectedChapter(), selectedKnowledgePoints());
}

void AIQuestionGenWidget::ensureAssistantMessagePlaceholder()
{
    if (!m_hasPendingAIPlaceholder) {
        return;
    }

    m_chatWidget->hideTypingIndicator();
    m_chatWidget->addMessage("", false);
    m_hasPendingAIPlaceholder = false;
}

// ===================== 对话管理（公开接口）=====================

QString AIQuestionGenWidget::currentConversationTitle() const
{
    // 从首条用户消息提取标题（前 20 字）
    for (const auto &msg : m_conversationHistory) {
        if (msg.role == "user") {
            QString title = msg.content.left(20).trimmed();
            if (title.length() < msg.content.length()) {
                title += "...";
            }
            return title;
        }
    }
    return QStringLiteral("新对话");
}

void AIQuestionGenWidget::cancelCurrentReply()
{
    if (!m_currentReply) {
        return;
    }

    QPointer<QNetworkReply> reply = m_currentReply;
    m_currentReply = nullptr;

    if (reply) {
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }
}

void AIQuestionGenWidget::updateActionButtons()
{
    const bool hasResponse = !m_lastAIResponse.trimmed().isEmpty();

    if (m_saveBtn) {
        m_saveBtn->setEnabled(hasResponse && !m_isSavingToBank);
    }

    if (m_exportBtn) {
        m_exportBtn->setEnabled(hasResponse && m_exportAvailable);
        if (!m_exportAvailable && !m_exportUnavailableReason.isEmpty()) {
            m_exportBtn->setToolTip(m_exportUnavailableReason);
        } else {
            m_exportBtn->setToolTip(QString());
        }
    }
}

void AIQuestionGenWidget::startNewConversation()
{
    cancelCurrentReply();

    // 分配新 UUID
    m_conversationId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    m_chatWidget->clearMessages();
    m_lastAIResponse.clear();
    m_sseBuffer.clear();
    m_isGenerating = false;
    m_chatWidget->setInputEnabled(true);
    updateActionButtons();

    // 重置对话历史（保留系统提示）
    m_conversationHistory.clear();
    m_conversationHistory.append({QStringLiteral("system"), currentSystemPrompt()});

    showWelcome();
    qDebug() << "[AIQuestionGen] 新对话已创建:" << m_conversationId;
}

void AIQuestionGenWidget::loadConversation(const QString &id)
{
    QSettings settings;
    QString messagesKey = QString("questionGen/messages/%1").arg(id);
    QByteArray data = settings.value(messagesKey).toByteArray();

    if (data.isEmpty()) {
        qWarning() << "[AIQuestionGen] 加载对话失败，未找到:" << id;
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[AIQuestionGen] 对话 JSON 解析失败:" << parseError.errorString();
        return;
    }

    cancelCurrentReply();

    m_conversationId = id;
    m_conversationHistory.clear();
    m_lastAIResponse.clear();
    m_sseBuffer.clear();
    m_isGenerating = false;
    m_chatWidget->setInputEnabled(true);
    m_chatWidget->clearMessages();

    // 恢复消息
    QJsonArray messages = doc.array();
    for (const QJsonValue &val : messages) {
        QJsonObject obj = val.toObject();
        QString role = obj["role"].toString();
        QString content = obj["content"].toString();
        m_conversationHistory.append({role, content});

        // 渲染到 ChatWidget（跳过 system 消息）
        if (role == "user") {
            m_chatWidget->addMessage(content, true);
        } else if (role == "assistant") {
            m_chatWidget->addMessage(content, false);
            m_lastAIResponse = content; // 记录最后一次 AI 回复
        }
    }

    updateActionButtons();

    qDebug() << "[AIQuestionGen] 对话已加载:" << id
             << "消息数:" << m_conversationHistory.size();
}

void AIQuestionGenWidget::saveCurrentConversation()
{
    if (m_conversationId.isEmpty()) return;

    // 检查是否有实质内容（至少有一条用户消息）
    bool hasUserMessage = false;
    for (const auto &msg : m_conversationHistory) {
        if (msg.role == "user") { hasUserMessage = true; break; }
    }
    if (!hasUserMessage) return;

    QSettings settings;

    // 1. 保存消息内容
    QJsonArray messages;
    for (const auto &msg : m_conversationHistory) {
        QJsonObject obj;
        obj["role"] = msg.role;
        obj["content"] = msg.content;
        messages.append(obj);
    }
    settings.setValue(
        QString("questionGen/messages/%1").arg(m_conversationId),
        QJsonDocument(messages).toJson(QJsonDocument::Compact)
    );

    // 2. 更新元数据索引
    QByteArray indexData = settings.value("questionGen/index").toByteArray();
    QJsonArray index = QJsonDocument::fromJson(indexData).array();

    QJsonObject currentEntry;
    currentEntry["id"] = m_conversationId;
    currentEntry["title"] = currentConversationTitle();
    currentEntry["updatedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // 当前会话每次更新都置顶，保证历史列表和最近使用顺序一致。
    QJsonArray newIndex;
    newIndex.append(currentEntry);
    for (const QJsonValue &v : index) {
        if (v.toObject()["id"].toString() != m_conversationId) {
            newIndex.append(v);
        }
    }
    index = newIndex;

    settings.setValue("questionGen/index", QJsonDocument(index).toJson(QJsonDocument::Compact));

    qDebug() << "[AIQuestionGen] 对话已保存:" << m_conversationId
             << "标题:" << currentConversationTitle();
}

void AIQuestionGenWidget::deleteConversation(const QString &id)
{
    QSettings settings;

    // 删除消息内容
    settings.remove(QString("questionGen/messages/%1").arg(id));

    // 从索引中移除
    QByteArray indexData = settings.value("questionGen/index").toByteArray();
    QJsonArray index = QJsonDocument::fromJson(indexData).array();
    QJsonArray newIndex;
    for (const QJsonValue &v : index) {
        if (v.toObject()["id"].toString() != id) {
            newIndex.append(v);
        }
    }
    settings.setValue("questionGen/index", QJsonDocument(newIndex).toJson(QJsonDocument::Compact));

    qDebug() << "[AIQuestionGen] 对话已删除:" << id;
}

// ===================== 智谱 API 调用 =====================

QString AIQuestionGenWidget::buildSystemPrompt(const QString &gradeSemester,
                                               const QString &chapter,
                                               const QStringList &knowledgePoints)
{
    QString prompt = QStringLiteral(
        "你是一位专业的中学《道德与法治》出题专家。你的任务是根据用户的需求生成高质量的试题。\n\n"
        "## 出题规范\n"
        "1. 所有试题必须围绕中学《道德与法治》课程内容\n"
        "2. 试题内容必须符合社会主义核心价值观\n"
        "3. 难度适合初中学生水平\n"
        "4. 每道题必须包含：题目、选项（选择题）、正确答案、解析\n\n"
        "## 输出格式\n"
        "使用清晰的 Markdown 格式输出试题，包括：\n"
        "- 题目编号和题型标注\n"
        "- 选择题的 A/B/C/D 选项\n"
        "- 【答案】标注正确答案\n"
        "- 【解析】详细解题思路\n\n"
        "## 题型支持\n"
        "- 单选题、多选题\n"
        "- 判断题、判断说理题\n"
        "- 简答题、论述题\n"
        "- 材料分析题\n\n"
        "请认真审题，确保答案准确、解析详尽。"
    );

    if (gradeSemester.isEmpty() && chapter.isEmpty()) {
        return prompt;
    }

    prompt += "\n\n## 当前课标上下文\n";
    if (!gradeSemester.isEmpty() && !chapter.isEmpty()) {
        prompt += QString("你正在为“%1 %2”出题。\n").arg(gradeSemester, chapter);
    } else {
        prompt += QString("你正在为“%1”范围内的内容出题。\n").arg(gradeSemester);
    }

    if (!knowledgePoints.isEmpty()) {
        prompt += QString("核心考点：%1。\n").arg(knowledgePoints.join("、"));
    }

    const QStringList patterns = CurriculumData::highFrequencyPatternsFor(gradeSemester, chapter);
    if (!patterns.isEmpty()) {
        prompt += QString("高频考法：%1。\n").arg(patterns.join("；"));
    }

    const QStringList types = CurriculumData::suitableTypesFor(gradeSemester, chapter);
    if (!types.isEmpty()) {
        QStringList displayTypes;
        for (const auto &type : types) {
            displayTypes.append(CurriculumData::displayQuestionType(type));
        }
        prompt += QString("优先考虑的题型：%1。\n").arg(displayTypes.join("、"));
    }

    prompt += "如果用户要求超出当前课标范围，请温和提醒并把题目拉回当前范围。";
    return prompt;
}

void AIQuestionGenWidget::sendToZhipu(const QString &userMessage)
{
    if (m_apiKey.isEmpty()) {
        ensureAssistantMessagePlaceholder();
        m_chatWidget->updateLastAIMessage("⚠️ 出题失败：API Key 未设置\n\n请在 .env.local 中配置 ZHIPU_API_KEY。");
        m_isGenerating = false;
        m_chatWidget->setInputEnabled(true);
        m_chatWidget->hideTypingIndicator();
        return;
    }

    // 添加用户消息到对话历史
    m_conversationHistory.append({QStringLiteral("user"), userMessage});

    // 构建请求
    QUrl url(m_baseUrl + "/chat/completions");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    request.setRawHeader("Content-Type", "application/json");
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(120000);

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);

    // 构建消息数组
    QJsonArray messages;
    for (const auto &msg : m_conversationHistory) {
        QJsonObject msgObj;
        msgObj["role"] = msg.role;
        msgObj["content"] = msg.content;
        messages.append(msgObj);
    }

    QJsonObject body;
    body["model"] = MODEL_NAME;
    body["messages"] = messages;
    body["stream"] = true;
    body["temperature"] = 0.7;
    body["max_tokens"] = 4096;

    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    qDebug() << "[AIQuestionGen] 发送请求:" << "messages:" << messages.size() << "body:" << data.size();

    m_sseBuffer.clear();
    cancelCurrentReply();
    m_currentReply = m_networkManager->post(request, data);

    if (!m_currentReply) {
        ensureAssistantMessagePlaceholder();
        m_chatWidget->updateLastAIMessage("⚠️ 出题失败：网络请求创建失败\n\n请稍后重试。");
        m_isGenerating = false;
        m_chatWidget->setInputEnabled(true);
        m_chatWidget->hideTypingIndicator();
        return;
    }

    connect(m_currentReply, &QNetworkReply::sslErrors,
            this, [this](const QList<QSslError> &errors) {
        if (m_currentReply) {
            NetworkRequestFactory::handleSslErrors(m_currentReply, errors, "[AIQuestionGen]");
        }
    });

    connect(m_currentReply, &QNetworkReply::readyRead,
            this, [this]() {
        if (!m_currentReply) return;
        processSSEData(m_currentReply->readAll());
    });

    connect(m_currentReply, &QNetworkReply::finished,
            this, [this]() {
        QNetworkReply *reply = m_currentReply;
        m_currentReply = nullptr;
        if (!reply) return;
        reply->deleteLater();

        m_isGenerating = false;
        m_hasPendingAIPlaceholder = false;
        m_chatWidget->setInputEnabled(true);
        m_chatWidget->hideTypingIndicator();

        if (reply->error() != QNetworkReply::NoError && m_lastAIResponse.isEmpty()) {
            int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString errMsg;
            if (httpStatus == 401) {
                errMsg = "⚠️ 出题失败：API Key 无效\n\n请检查 ZHIPU_API_KEY 配置。";
            } else if (httpStatus == 429) {
                errMsg = "⚠️ 出题失败：请求频率超限\n\n请稍后重试。";
            } else {
                errMsg = QString("⚠️ 出题失败：%1 (HTTP %2)\n\n请稍后重试。")
                    .arg(reply->errorString()).arg(httpStatus);
            }
            ensureAssistantMessagePlaceholder();
            m_chatWidget->updateLastAIMessage(errMsg);
        } else {
            // AI 回复完成 — 添加到历史、保存、通知外部
            if (!m_lastAIResponse.isEmpty()) {
                m_conversationHistory.append({QStringLiteral("assistant"), m_lastAIResponse});
            }
            updateActionButtons();

            // 自动保存对话 + 通知外部刷新历史
            saveCurrentConversation();
            emit conversationUpdated(m_conversationId, currentConversationTitle());

            qDebug() << "[AIQuestionGen] AI 回复完成，长度:" << m_lastAIResponse.length();
        }
    });
}

void AIQuestionGenWidget::processSSEData(const QByteArray &data)
{
    m_sseBuffer.append(data);

    while (true) {
        int endIdx = m_sseBuffer.indexOf("\n\n");
        if (endIdx < 0) {
            endIdx = m_sseBuffer.indexOf("\r\n\r\n");
            if (endIdx < 0) break;
            QByteArray event = m_sseBuffer.left(endIdx);
            m_sseBuffer.remove(0, endIdx + 4);
            for (const QByteArray &line : event.split('\n')) {
                QByteArray trimmed = line.trimmed();
                if (trimmed.startsWith("data: ")) {
                    QByteArray jsonStr = trimmed.mid(6);
                    if (jsonStr == "[DONE]") return;
                    QJsonParseError pe;
                    QJsonDocument doc = QJsonDocument::fromJson(jsonStr, &pe);
                    if (pe.error == QJsonParseError::NoError) {
                        QJsonArray choices = doc.object()["choices"].toArray();
                        if (!choices.isEmpty()) {
                            QString content = choices[0].toObject()["delta"].toObject()["content"].toString();
                            if (!content.isEmpty()) {
                                ensureAssistantMessagePlaceholder();
                                m_lastAIResponse += content;
                                m_chatWidget->updateLastAIMessage(m_lastAIResponse);
                            }
                        }
                    }
                }
            }
            continue;
        }

        QByteArray event = m_sseBuffer.left(endIdx);
        m_sseBuffer.remove(0, endIdx + 2);

        for (const QByteArray &line : event.split('\n')) {
            QByteArray trimmed = line.trimmed();
            if (trimmed.startsWith("data: ")) {
                QByteArray jsonStr = trimmed.mid(6);
                if (jsonStr == "[DONE]") return;
                QJsonParseError pe;
                QJsonDocument doc = QJsonDocument::fromJson(jsonStr, &pe);
                if (pe.error == QJsonParseError::NoError) {
                    QJsonArray choices = doc.object()["choices"].toArray();
                    if (!choices.isEmpty()) {
                        QString content = choices[0].toObject()["delta"].toObject()["content"].toString();
                        if (!content.isEmpty()) {
                            ensureAssistantMessagePlaceholder();
                            m_lastAIResponse += content;
                            m_chatWidget->updateLastAIMessage(m_lastAIResponse);
                        }
                    }
                }
            }
        }
    }
}

// ===================== 消息处理 =====================

void AIQuestionGenWidget::onUserMessageSent(const QString &message)
{
    if (message.trimmed().isEmpty() || m_isGenerating) return;

    qDebug() << "[AIQuestionGen] 用户消息:" << message.left(50);

    m_isGenerating = true;
    m_hasPendingAIPlaceholder = true;
    m_lastAIResponse.clear();
    updateActionButtons();
    m_chatWidget->setInputEnabled(false);

    // 先添加用户消息气泡
    m_chatWidget->addMessage(message, true);
    m_chatWidget->clearQuickReplyOptions();

    // 显示 AI 正在思考，而不是空白占位气泡
    m_chatWidget->showTypingIndicator();

    sendToZhipu(message);
}

// ===================== 操作按钮 =====================

void AIQuestionGenWidget::onSaveToBank()
{
    if (m_lastAIResponse.trimmed().isEmpty() || m_isSavingToBank) return;

    setSavingToBank(true);
    emit saveRequested(m_lastAIResponse);
    qDebug() << "[AIQuestionGen] 开始保存 AI 生成内容";
}
