#include "AIQuestionGenWidget.h"
#include "../ui/ChatWidget.h"
#include "../config/AppConfig.h"
#include "../config/embedded_keys.h"
#include "../utils/NetworkRequestFactory.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslConfiguration>

namespace {
// 绿色教育风格配色（与 SmartPaperWidget / QuestionBankWindow 统一）
const QString ACCENT_GREEN = "#2E7D32";
const QString ACCENT_GREEN_HOVER = "#1B5E20";
const QString ACCENT_GREEN_LIGHT = "#E8F5E9";
const QString ACCENT_RED = "#D32F2F";
const QString ACCENT_RED_HOVER = "#B71C1C";
const QString ACCENT_BLUE = "#1565C0";
const QString ACCENT_BLUE_LIGHT = "#E3F2FD";
const QString TEXT_SECONDARY = "#6B7280";
const QString BORDER_SUBTLE = "#E5E7EB";
const QString BG_PAGE = "#F8F9FA";
}

AIQuestionGenWidget::AIQuestionGenWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupZhipuService();
    showWelcome();
}

AIQuestionGenWidget::~AIQuestionGenWidget()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void AIQuestionGenWidget::setSavingToBank(bool saving)
{
    m_isSavingToBank = saving;
    if (!m_saveBtn) {
        return;
    }

    m_saveBtn->setEnabled(!saving && !m_lastAIResponse.trimmed().isEmpty());
    m_saveBtn->setText(saving ? "保存中..." : "💾 保存到题库");
}

void AIQuestionGenWidget::showSaveSuccessMessage(int savedCount, bool directInsert)
{
    setSavingToBank(false);
    QMessageBox::information(
        this,
        "保存成功",
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

void AIQuestionGenWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setStyleSheet(QString("AIQuestionGenWidget { background-color: %1; }").arg(BG_PAGE));

    // ===================== 聊天区域 =====================
    m_chatWidget = new ChatWidget(this);
    m_chatWidget->setMarkdownEnabled(true);
    m_chatWidget->setPlaceholderText("输入出题需求，例如：帮我出5道关于宪法的选择题...");
    mainLayout->addWidget(m_chatWidget, 1);

    // 连接用户消息信号
    connect(m_chatWidget, &ChatWidget::messageSent,
            this, &AIQuestionGenWidget::onUserMessageSent);

    // ===================== 底部操作栏 =====================
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
    m_newChatBtn = new QPushButton("🔄 新对话");
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
        "QPushButton:hover {"
        "    background-color: #F5F5F5;"
        "}"
    ).arg(TEXT_SECONDARY, BORDER_SUBTLE));
    m_newChatBtn->setCursor(Qt::PointingHandCursor);
    connect(m_newChatBtn, &QPushButton::clicked,
            this, &AIQuestionGenWidget::onNewConversation);

    barLayout->addWidget(m_newChatBtn);
    barLayout->addStretch();

    // 保存到题库按钮
    m_saveBtn = new QPushButton("💾 保存到题库");
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
    m_saveBtn->setEnabled(false);  // 初始禁用，等 AI 生成完再启用
    connect(m_saveBtn, &QPushButton::clicked,
            this, &AIQuestionGenWidget::onSaveToBank);

    barLayout->addWidget(m_saveBtn);

    mainLayout->addWidget(m_bottomBar);
}

void AIQuestionGenWidget::setupZhipuService()
{
    m_networkManager = new QNetworkAccessManager(this);

    // 从 AppConfig 读取智谱 API Key 和 Base URL
    m_apiKey = AppConfig::get("ZHIPU_QUESTION_API_KEY");
    if (m_apiKey.isEmpty()) {
        m_apiKey = AppConfig::get("ZHIPU_API_KEY", EmbeddedKeys::ZHIPU_API_KEY);
    }
    m_baseUrl = AppConfig::get("ZHIPU_BASE_URL", EmbeddedKeys::ZHIPU_BASE_URL);

    qDebug() << "[AIQuestionGen] 智谱 API 配置:"
             << "Key长度:" << m_apiKey.length()
             << "Base URL:" << m_baseUrl
             << "Model:" << MODEL_NAME;

    // 初始化对话历史，添加系统提示
    m_conversationHistory.clear();
    m_conversationHistory.append({QStringLiteral("system"), systemPrompt()});
}

void AIQuestionGenWidget::showWelcome()
{
    // AI 欢迎消息
    const QString welcome =
        "你好！我是 **AI 出题助手** 💡\n\n"
        "我可以帮你生成各类思政试题。请告诉我你的出题需求，例如：\n\n"
        "- 帮我出 **5道关于宪法** 的选择题\n"
        "- 生成一道 **中等难度** 的材料论述题\n"
        "- 围绕 **\"法治与道德\"** 出一套综合练习\n"
        "- 出 **3道判断说理题**，主题：青少年法律意识\n\n"
        "直接输入需求，或点击下方快捷按钮开始 👇";

    m_chatWidget->addMessage(welcome, false);

    // 快捷回复按钮
    m_chatWidget->addQuickReplyOptions({
        "帮我出5道选择题，主题：宪法",
        "生成一道材料论述题，难度中等",
        "围绕'法治'出综合练习（选择+判断+论述）",
        "出3道判断说理题，主题：青少年法律意识"
    });
}

// ===================== 智谱 API 调用 =====================

const QString AIQuestionGenWidget::systemPrompt()
{
    return QStringLiteral(
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
}

void AIQuestionGenWidget::sendToZhipu(const QString &userMessage)
{
    if (m_apiKey.isEmpty()) {
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
    request.setTransferTimeout(120000); // 2 分钟超时

    // SSL 配置
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
    body["stream"] = true;  // 启用流式响应
    body["temperature"] = 0.7;
    body["max_tokens"] = 4096;

    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    qDebug() << "[AIQuestionGen] 发送请求到智谱 API:"
             << "model:" << MODEL_NAME
             << "messages:" << messages.size()
             << "body size:" << data.size();

    // 发送请求
    m_sseBuffer.clear();
    m_currentReply = m_networkManager->post(request, data);

    if (!m_currentReply) {
        m_chatWidget->updateLastAIMessage("⚠️ 出题失败：网络请求创建失败\n\n请稍后重试。");
        m_isGenerating = false;
        m_chatWidget->setInputEnabled(true);
        m_chatWidget->hideTypingIndicator();
        return;
    }

    // 处理 SSL 错误
    connect(m_currentReply, &QNetworkReply::sslErrors,
            this, [this](const QList<QSslError> &errors) {
        if (m_currentReply) {
            NetworkRequestFactory::handleSslErrors(m_currentReply, errors, "[AIQuestionGen]");
        }
    });

    // 流式数据到达
    connect(m_currentReply, &QNetworkReply::readyRead,
            this, [this]() {
        if (!m_currentReply) return;
        QByteArray newData = m_currentReply->readAll();
        processSSEData(newData);
    });

    // 请求完成
    connect(m_currentReply, &QNetworkReply::finished,
            this, [this]() {
        QNetworkReply *reply = m_currentReply;
        m_currentReply = nullptr;

        if (!reply) return;
        reply->deleteLater();

        m_isGenerating = false;
        m_chatWidget->setInputEnabled(true);
        m_chatWidget->hideTypingIndicator();
        m_chatWidget->collapseThinking();

        if (reply->error() != QNetworkReply::NoError && m_lastAIResponse.isEmpty()) {
            QString errMsg;
            int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (httpStatus == 401) {
                errMsg = "⚠️ 出题失败：API Key 无效\n\n请检查 ZHIPU_API_KEY 配置。";
            } else if (httpStatus == 429) {
                errMsg = "⚠️ 出题失败：请求频率超限\n\n请稍后重试。";
            } else {
                errMsg = QString("⚠️ 出题失败：%1 (HTTP %2)\n\n请稍后重试。")
                    .arg(reply->errorString()).arg(httpStatus);
            }
            m_chatWidget->updateLastAIMessage(errMsg);
            qWarning() << "[AIQuestionGen] 请求失败:" << reply->errorString()
                       << "HTTP:" << httpStatus;
        } else {
            // 将 AI 回复添加到对话历史
            if (!m_lastAIResponse.isEmpty()) {
                m_conversationHistory.append({QStringLiteral("assistant"), m_lastAIResponse});
            }
            // 启用保存按钮
            if (!m_lastAIResponse.trimmed().isEmpty() && !m_isSavingToBank) {
                m_saveBtn->setEnabled(true);
            }
            qDebug() << "[AIQuestionGen] AI 回复完成，长度:" << m_lastAIResponse.length()
                     << "对话轮数:" << m_conversationHistory.size();
        }
    });
}

void AIQuestionGenWidget::processSSEData(const QByteArray &data)
{
    m_sseBuffer.append(data);

    // SSE 格式：data: {...}\n\n
    while (true) {
        int endIdx = m_sseBuffer.indexOf("\n\n");
        if (endIdx < 0) {
            // 也尝试 \r\n\r\n
            endIdx = m_sseBuffer.indexOf("\r\n\r\n");
            if (endIdx < 0) break;
            // 取出一个完整事件
            QByteArray event = m_sseBuffer.left(endIdx);
            m_sseBuffer.remove(0, endIdx + 4);
            // 解析事件
            for (const QByteArray &line : event.split('\n')) {
                QByteArray trimmed = line.trimmed();
                if (trimmed.startsWith("data: ")) {
                    QByteArray jsonStr = trimmed.mid(6); // 去掉 "data: "
                    if (jsonStr == "[DONE]") {
                        return; // 流结束
                    }
                    QJsonParseError parseError;
                    QJsonDocument doc = QJsonDocument::fromJson(jsonStr, &parseError);
                    if (parseError.error == QJsonParseError::NoError) {
                        QJsonObject obj = doc.object();
                        QJsonArray choices = obj["choices"].toArray();
                        if (!choices.isEmpty()) {
                            QJsonObject delta = choices[0].toObject()["delta"].toObject();
                            QString content = delta["content"].toString();
                            if (!content.isEmpty()) {
                                m_lastAIResponse += content;
                                m_chatWidget->updateLastAIMessage(m_lastAIResponse);
                            }
                        }
                    }
                }
            }
            continue;
        }

        // 取出一个完整事件
        QByteArray event = m_sseBuffer.left(endIdx);
        m_sseBuffer.remove(0, endIdx + 2);

        // 解析 SSE 事件的每一行
        for (const QByteArray &line : event.split('\n')) {
            QByteArray trimmed = line.trimmed();
            if (trimmed.startsWith("data: ")) {
                QByteArray jsonStr = trimmed.mid(6);
                if (jsonStr == "[DONE]") {
                    return; // 流结束
                }

                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(jsonStr, &parseError);
                if (parseError.error == QJsonParseError::NoError) {
                    QJsonObject obj = doc.object();
                    QJsonArray choices = obj["choices"].toArray();
                    if (!choices.isEmpty()) {
                        QJsonObject delta = choices[0].toObject()["delta"].toObject();
                        QString content = delta["content"].toString();
                        if (!content.isEmpty()) {
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
    if (message.trimmed().isEmpty() || m_isGenerating) {
        return;
    }

    qDebug() << "[AIQuestionGen] 用户消息:" << message.left(50);

    m_isGenerating = true;
    m_hasPendingAIPlaceholder = true;
    m_lastAIResponse.clear();
    m_saveBtn->setEnabled(false);
    m_chatWidget->setInputEnabled(false);

    // 创建空的 AI 消息气泡用于流式填充
    m_chatWidget->addMessage("", false);
    m_chatWidget->showTypingIndicator();

    // 发送到智谱 API
    sendToZhipu(message);
}

// ===================== 操作按钮 =====================

void AIQuestionGenWidget::onNewConversation()
{
    // 中止正在进行的请求
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }

    m_chatWidget->clearMessages();
    m_lastAIResponse.clear();
    m_saveBtn->setEnabled(false);
    m_isGenerating = false;
    m_chatWidget->setInputEnabled(true);

    // 重置对话历史（保留系统提示）
    m_conversationHistory.clear();
    m_conversationHistory.append({QStringLiteral("system"), systemPrompt()});

    showWelcome();
    qDebug() << "[AIQuestionGen] 新对话已创建";
}

void AIQuestionGenWidget::onSaveToBank()
{
    if (m_lastAIResponse.trimmed().isEmpty() || m_isSavingToBank) {
        return;
    }

    setSavingToBank(true);
    emit saveRequested(m_lastAIResponse);
    qDebug() << "[AIQuestionGen] 开始保存 AI 生成内容";
}
