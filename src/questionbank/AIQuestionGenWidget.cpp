#include "AIQuestionGenWidget.h"
#include "CurriculumData.h"
#include "../ui/ChatWidget.h"
#include "../config/AiConfig.h"
#include "../config/AppConfig.h"
#include "../config/embedded_keys.h"
#include "../utils/NetworkRequestFactory.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QListView>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMessageBox>
#include "../shared/ModernDialogHelper.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslConfiguration>
#include <QUuid>
#include <QSettings>
#include <QDateTime>
#include <QRegularExpression>

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
    setupAiService();

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
    ModernDialogHelper::info(
        this, "保存成功",
        directInsert
            ? QString("AI 生成的试题已通过工作流直接入库（共 %1 题）。").arg(savedCount)
            : QString("AI 生成的试题已保存到题库（共 %1 题）。").arg(savedCount)
    );
}

void AIQuestionGenWidget::showSaveErrorMessage(const QString &error)
{
    setSavingToBank(false);
    ModernDialogHelper::warning(this, "保存失败", error);
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

    // ====== 操作按钮栏（仅在 AI 回复完成后显示） ======
    m_bottomBar = new QFrame(this);
    m_bottomBar->setObjectName("aiGenActionBar");
    m_bottomBar->setStyleSheet("QFrame#aiGenActionBar { background-color: transparent; border: none; }");
    m_bottomBar->setFixedHeight(44);
    m_bottomBar->setVisible(false); // 默认隐藏

    auto *barLayout = new QHBoxLayout(m_bottomBar);
    barLayout->setContentsMargins(20, 0, 20, 8);
    barLayout->setSpacing(12);

    barLayout->addStretch();

    // 导出试卷按钮
    m_exportBtn = new QPushButton(" 导出试卷");
    m_exportBtn->setIcon(QIcon(":/icons/resources/icons/export-white.svg"));
    m_exportBtn->setIconSize(QSize(16, 16));
    m_exportBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %1, stop:1 #1976D2);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 6px 16px;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %2, stop:1 #1565C0); }"
    ).arg(ACCENT_BLUE, ACCENT_BLUE_HOVER));
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        if (m_exportAvailable && !m_lastAIResponse.trimmed().isEmpty()) {
            emit exportRequested(m_lastAIResponse);
        }
    });
    barLayout->addWidget(m_exportBtn);

    // 保存到题库按钮
    m_saveBtn = new QPushButton(" 保存到题库");
    m_saveBtn->setIcon(QIcon(":/icons/resources/icons/save-white.svg"));
    m_saveBtn->setIconSize(QSize(16, 16));
    m_saveBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %1, stop:1 #388E3C);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 6px 16px;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %2, stop:1 #2E7D32); }"
    ).arg(ACCENT_GREEN, ACCENT_GREEN_HOVER));
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveBtn, &QPushButton::clicked, this, &AIQuestionGenWidget::onSaveToBank);
    barLayout->addWidget(m_saveBtn);

    mainLayout->addWidget(m_bottomBar);
}

void AIQuestionGenWidget::setupCurriculumBar(QVBoxLayout *mainLayout)
{
    m_curriculumBar = new QFrame(this);
    m_curriculumBar->setObjectName("aiCurriculumBar");
    m_curriculumBar->setStyleSheet(
        "QFrame#aiCurriculumBar {"
        "    background-color: transparent;"
        "}"
        "QComboBox {"
        "    background-color: rgba(255,255,255,0.18);"
        "    border: 1px solid rgba(255,255,255,0.3);"
        "    border-radius: 12px;"
        "    padding: 2px 10px;"
        "    min-height: 20px;"
        "    color: white;"
        "    font-size: 11px;"
        "}"
        "QComboBox:hover { background-color: rgba(255,255,255,0.25); border-color: rgba(255,255,255,0.5); }"
        "QComboBox:focus { background-color: rgba(255,255,255,0.25); border-color: white; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "QComboBox::down-arrow { image: url(:/QtTheme/icon/arrow_drop_down/#ffffff.svg); width: 14px; height: 14px; }"
        "QComboBox QAbstractItemView {"
        "    background-color: white;"
        "    color: #111827;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "    outline: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "    color: #111827;"
        "    min-height: 30px;"
        "    padding-left: 8px;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "    background-color: #E8F5E9;"
        "    color: #2E7D32;"
        "}"
    );

    auto *layout = new QHBoxLayout(m_curriculumBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_gradeCombo = new QComboBox(m_curriculumBar);
    m_gradeCombo->setView(new QListView()); // 强制不使用原生菜单以支持样式
    m_gradeCombo->addItem("不限年级");
    for (const auto &grade : CurriculumData::gradeSemesters()) {
        m_gradeCombo->addItem(grade);
    }
    layout->addWidget(m_gradeCombo);

    m_chapterCombo = new QComboBox(m_curriculumBar);
    m_chapterCombo->setView(new QListView()); // 强制不使用原生菜单以支持样式
    layout->addWidget(m_chapterCombo);

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
    // 不再添加到 mainLayout，而是通过 curriculumFilterWidget() 供外部使用
}

void AIQuestionGenWidget::setupAiService()
{
    m_networkManager = new QNetworkAccessManager(this);

    m_apiKey = AiConfig::apiKey();
    m_baseUrl = AiConfig::baseUrl();

    qDebug() << "[AIQuestionGen] MiniMax API 配置:"
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

    QString welcome = "**AI 出题助手** 已就绪。请直接输入需求，或点击下方选项开始 👇";

    if (!gradeSemester.isEmpty() && !chapter.isEmpty()) {
        welcome += QString(
            "\n\n**当前范围**：%1 · %2\n"
            "**考点提示**：%3"
        ).arg(
            gradeSemester,
            chapter,
            knowledgePoints.isEmpty() ? QStringLiteral("本单元核心内容") : knowledgePoints.join("、")
        );
        m_chatWidget->setPlaceholderText(
            QString("为%1 %2出3道选择题...").arg(gradeSemester, chapter));
    } else if (!gradeSemester.isEmpty()) {
        welcome += QString("\n\n**当前范围**：%1").arg(gradeSemester);
        m_chatWidget->setPlaceholderText(
            QString("围绕%1核心出5道题...").arg(gradeSemester));
    } else {
        m_chatWidget->setPlaceholderText("帮我出5道关于宪法的选择题...");
    }

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
    // 从首条用户消息智能提取精炼标题
    for (const auto &msg : m_conversationHistory) {
        if (msg.role != "user") continue;

        const QString &text = msg.content;

        // 如果原始消息本身就很短（≤8字），直接用
        if (text.length() <= 8) {
            return text.trimmed();
        }

        // ---- 智能提取关键信息 ----
        QString topic;      // 主题
        QString qType;      // 题型
        QString count;       // 数量
        QString grade;       // 年级

        // 1. 提取主题：匹配 "主题：XXX" 或 "关于XXX" 或 '引号内容'
        static const QRegularExpression topicRe(
            R"(主题[：:]\s*(.{1,8})|关于[""「]?(.{1,8})[""」]?|围绕[''「](.{1,6})[''」])");
        QRegularExpressionMatch topicMatch = topicRe.match(text);
        if (topicMatch.hasMatch()) {
            topic = topicMatch.captured(1);
            if (topic.isEmpty()) topic = topicMatch.captured(2);
            if (topic.isEmpty()) topic = topicMatch.captured(3);
            topic = topic.trimmed();
            // 清理尾部标点
            topic.remove(QRegularExpression(R"([。，,.、\s]+$)"));
        }

        // 2. 提取题型
        static const QRegularExpression typeRe(
            R"((选择题|判断题|填空题|简答题|论述题|材料分析题|材料论述题|判断说理题|辨析题|综合练习|考卷|试卷|期中考卷|期末考卷|模拟卷))");
        QRegularExpressionMatch typeMatch = typeRe.match(text);
        if (typeMatch.hasMatch()) {
            qType = typeMatch.captured(1);
        }

        // 3. 提取数量
        static const QRegularExpression countRe(
            R"((\d+)\s*(?:道|题|份|个|条))");
        QRegularExpressionMatch countMatch = countRe.match(text);
        if (countMatch.hasMatch()) {
            count = countMatch.captured(1);
        }

        // 4. 提取年级
        static const QRegularExpression gradeRe(
            R"((初[一二三]|高[一二三]|[一二三四五六七八九]年级|小学|初中|高中))");
        QRegularExpressionMatch gradeMatch = gradeRe.match(text);
        if (gradeMatch.hasMatch()) {
            grade = gradeMatch.captured(1);
        }

        // ---- 组装精炼标题 ----
        QStringList parts;

        // 年级放在最前面
        if (!grade.isEmpty()) {
            parts << grade;
        }

        // 主题
        if (!topic.isEmpty()) {
            parts << topic;
        }

        // 题型 + 数量
        if (!qType.isEmpty()) {
            if (!count.isEmpty()) {
                parts << QString("%1×%2").arg(qType, count);
            } else {
                parts << qType;
            }
        } else if (!count.isEmpty()) {
            parts << QString("%1题").arg(count);
        }

        // 如果成功提取到了信息，用 · 连接
        if (!parts.isEmpty()) {
            return parts.join(" · ");
        }

        // 兜底：截取前 12 个字
        QString fallback = text.left(12).trimmed();
        if (fallback.length() < text.length()) {
            fallback += "…";
        }
        return fallback;
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

    if (m_bottomBar) {
        m_bottomBar->setVisible(hasResponse && !m_isGenerating);
    }

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

// ===================== MiniMax API 调用 =====================

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

void AIQuestionGenWidget::sendToMiniMax(const QString &userMessage)
{
    if (m_apiKey.isEmpty()) {
        ensureAssistantMessagePlaceholder();
        m_chatWidget->updateLastAIMessage("⚠️ 出题失败：API Key 未设置\n\n请在 .env.local 中配置 MINIMAX_API_KEY。");
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
                errMsg = "⚠️ 出题失败：API Key 无效\n\n请检查 MINIMAX_API_KEY 配置。";
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
                // 过滤 <think>...</think> 思考内容后再保存
                static const QRegularExpression thinkRe(
                    QStringLiteral("<think>[\\s\\S]*?</think>\\s*"),
                    QRegularExpression::MultilineOption);
                m_lastAIResponse.remove(thinkRe);
                m_lastAIResponse = m_lastAIResponse.trimmed();
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

    // 辅助：过滤 <think>...</think> 标签（支持流式中间状态）
    auto stripThinkTags = [](const QString &text) -> QString {
        QString result = text;
        // 移除已完成的 <think>...</think> 块
        static const QRegularExpression thinkRe(
            QStringLiteral("<think>[\\s\\S]*?</think>\\s*"),
            QRegularExpression::MultilineOption);
        result.remove(thinkRe);
        // 移除流式中尚未闭合的 <think>... 块
        static const QRegularExpression thinkOpenRe(
            QStringLiteral("<think>[\\s\\S]*$"),
            QRegularExpression::MultilineOption);
        result.remove(thinkOpenRe);
        return result.trimmed();
    };

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
                                QString display = stripThinkTags(m_lastAIResponse);
                                if (!display.isEmpty()) {
                                    m_chatWidget->updateLastAIMessage(display);
                                }
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
                            QString display = stripThinkTags(m_lastAIResponse);
                            if (!display.isEmpty()) {
                                m_chatWidget->updateLastAIMessage(display);
                            }
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

    sendToMiniMax(message);
}

// ===================== 操作按钮 =====================

void AIQuestionGenWidget::onSaveToBank()
{
    if (m_lastAIResponse.trimmed().isEmpty() || m_isSavingToBank) return;

    setSavingToBank(true);
    emit saveRequested(m_lastAIResponse);
    qDebug() << "[AIQuestionGen] 开始保存 AI 生成内容";
}
