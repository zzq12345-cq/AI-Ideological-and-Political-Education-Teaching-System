#include "questionbankwindow.h"
#include "AIQuestionGenWidget.h"
#include "PaperComposerDialog.h"
#include "QuestionBasket.h"
#include "QuestionBasketWidget.h"
#include "QualityCheckDialog.h"
#include "../config/AppConfig.h"
#include "../shared/StyleConfig.h"
#include "../smartpaper/SmartPaperWidget.h"
#include "../services/DifyService.h"
#include "../services/PaperService.h"
#include "../services/QuestionParserService.h"
#include "../services/QuestionQualityService.h"
#include "../services/DocxGenerator.h"
#include "../ui/ChatHistoryWidget.h"

#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QPainter>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include "../shared/ModernDialogHelper.h"

QuestionBankWindow::QuestionBankWindow(QWidget *parent)
    : QWidget(parent)
    , m_paperService(new PaperService(this))
    , m_questionParser(new QuestionParserService(this))
    , m_docxGenerator(new DocxGenerator(this))
{
    setObjectName("questionBankWindow");

    QFont baseFont("Lexend", 12);
    baseFont.setStyleHint(QFont::SansSerif);
    baseFont.setStyleStrategy(QFont::PreferAntialias);
    setFont(baseFont);

    setupLayout();
    loadStyleSheet();

    // 保存链路的解析器配置
    QString parserApiKey = AppConfig::get("PARSER_API_KEY");
    if (parserApiKey.isEmpty()) {
        parserApiKey = AppConfig::get("DIFY_API_KEY");
    }
    if (parserApiKey.isEmpty()) {
        parserApiKey = AppConfig::get("DIFY_QUESTION_GEN_API_KEY");
    }
    m_questionParser->setApiKey(parserApiKey);

    const QString parserBaseUrl = AppConfig::get("PARSER_API_BASE_URL");
    if (!parserBaseUrl.isEmpty()) {
        m_questionParser->setBaseUrl(parserBaseUrl);
    }

    // 导出链路现在使用本地 Markdown 解析，不再依赖 Dify API
    m_aiQuestionGenWidget->setExportAvailable(true);

    // ====== 保存链路信号 ======
    connect(m_aiQuestionGenWidget, &AIQuestionGenWidget::saveRequested,
            this, &QuestionBankWindow::onSaveGeneratedQuestionsRequested);
    connect(m_questionParser, &QuestionParserService::parseCompleted,
            this, &QuestionBankWindow::onGeneratedQuestionsParsed);
    connect(m_questionParser, &QuestionParserService::errorOccurred,
            this, &QuestionBankWindow::onGeneratedQuestionsParseError);
    connect(m_paperService, &PaperService::questionsAdded,
            this, &QuestionBankWindow::onGeneratedQuestionsSaved);
    connect(m_paperService, &PaperService::questionError,
            this, &QuestionBankWindow::onGeneratedQuestionsSaveError);

    // ====== 导出链路信号（本地解析，无需异步回调）======
    connect(m_aiQuestionGenWidget, &AIQuestionGenWidget::exportRequested,
            this, &QuestionBankWindow::onExportToDocx);

    // ====== 历史侧边栏信号 ======
    connect(m_questionHistoryWidget, &ChatHistoryWidget::newChatRequested,
            m_aiQuestionGenWidget, &AIQuestionGenWidget::startNewConversation);
    connect(m_questionHistoryWidget, &ChatHistoryWidget::historyItemSelected,
            this, &QuestionBankWindow::onQuestionHistorySelected);
    connect(m_questionHistoryWidget, &ChatHistoryWidget::historyDeleteRequested,
            this, &QuestionBankWindow::onQuestionHistoryDeleted);
    // ← 返回按钮 → 回主页面
    connect(m_questionHistoryWidget, &ChatHistoryWidget::backRequested,
            this, &QuestionBankWindow::backRequested);

    // 对话更新 → 刷新历史列表
    connect(m_aiQuestionGenWidget, &AIQuestionGenWidget::conversationUpdated,
            this, [this](const QString &, const QString &) {
        refreshQuestionHistory();
    });

    // 新建对话后也刷新
    connect(m_questionHistoryWidget, &ChatHistoryWidget::newChatRequested,
            this, &QuestionBankWindow::refreshQuestionHistory);

    // 初始加载历史
    refreshQuestionHistory();
}

void QuestionBankWindow::setupLayout()
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *pageContainer = new QWidget(this);
    pageContainer->setObjectName("pageContainer");
    pageContainer->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    auto *pageLayout = new QVBoxLayout(pageContainer);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // 提前创建内容组件，以便 header 能获取它们的子组件
    m_aiQuestionGenWidget = new AIQuestionGenWidget(this);
    m_smartPaperWidget = new SmartPaperWidget(this);

    // ====== Header 悬浮卡片 ======
    auto *headerWrapper = new QWidget(pageContainer);
    headerWrapper->setObjectName("headerWrapper");
    headerWrapper->setStyleSheet("QWidget#headerWrapper { background: transparent; }");
    auto *headerWrapperLayout = new QVBoxLayout(headerWrapper);
    headerWrapperLayout->setContentsMargins(12, 8, 12, 4);
    headerWrapperLayout->setSpacing(0);
    headerWrapperLayout->addWidget(buildHeader());

    pageLayout->addWidget(headerWrapper);

    // ====== 内容区域：AI出题 / 智能组卷 ======
    m_modeStack = new QStackedWidget();

    // page 0: AI 出题（历史侧边栏已迁移到全局 m_sidebarStack）
    m_aiGenPageWrapper = new QWidget();
    auto *aiGenLayout = new QHBoxLayout(m_aiGenPageWrapper);
    aiGenLayout->setContentsMargins(0, 0, 0, 0);
    aiGenLayout->setSpacing(0);

    // 创建历史侧边栏（不再放入此处的布局，由 ModernMainWindow 统一管理）
    m_questionHistoryWidget = new ChatHistoryWidget();
    m_questionHistoryWidget->setHeaderTitle("出题历史");
    m_questionHistoryWidget->setNewButtonText("➕ 新建出题");
    m_questionHistoryWidget->setMinimumWidth(240);
    m_questionHistoryWidget->setMaximumWidth(400);
    // 适配试题库的绿色主题
    m_questionHistoryWidget->setThemeColors("#2E7D32", "#1B5E20", "#E8F5E9", "#A5D6A7");

    // 右侧聊天区域（直接填满整个内容区）
    aiGenLayout->addWidget(m_aiQuestionGenWidget);

    m_modeStack->addWidget(m_aiGenPageWrapper);

    // page 1: 智能组卷
    m_modeStack->addWidget(m_smartPaperWidget);

    pageLayout->addWidget(m_modeStack, 1);

    rootLayout->addWidget(pageContainer);

    // ====== 试题篮悬浮组件 ======
    m_basketWidget = new QuestionBasketWidget(this);
    m_basketWidget->setParent(this);
    m_basketWidget->raise();
    m_basketWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    connect(m_basketWidget, &QuestionBasketWidget::composePaperRequested,
            this, &QuestionBankWindow::onComposePaper);
    connect(m_basketWidget, &QuestionBasketWidget::sizeChanged,
            this, [this]() {
                if (m_basketWidget) {
                    const int margin = 24;
                    int x = width() - m_basketWidget->width() - margin;
                    int y = height() - m_basketWidget->height() - margin;
                    m_basketWidget->move(x, y);
                }
            });

    switchMode(0);
}

QWidget *QuestionBankWindow::buildHeader()
{
    auto *header = new QFrame(this);
    header->setObjectName("pageHeader");
    header->setFixedHeight(56);

    header->setStyleSheet(
        "QFrame#pageHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #2E7D32, stop:1 #43A047);"
        "    border-radius: 14px;"
        "}"
    );

    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(12);

    // 返回按钮（精简）
    auto *backButton = new QPushButton("← 返回", header);
    backButton->setObjectName("backButton");
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(
        "QPushButton#backButton {"
        "    background: transparent;"
        "    border: none;"
        "    padding: 4px 8px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "    color: rgba(255,255,255,0.8);"
        "}"
        "QPushButton#backButton:hover {"
        "    color: white;"
        "}"
    );
    connect(backButton, &QPushButton::clicked, this, &QuestionBankWindow::backRequested);

    // 标题
    m_headerTitle = new QLabel(QStringLiteral("AI 出题中心"), header);
    m_headerTitle->setObjectName("pageTitle");
    m_headerTitle->setStyleSheet(
        "font-size: 16px; font-weight: 700; color: white; background: transparent;"
    );

    // 隐藏副标题（不再使用，但保留成员变量避免空指针）
    m_headerSubtitle = new QLabel("", header);
    m_headerSubtitle->hide();

    // 右侧：标签页切换
    const QString TAB_ACTIVE_STYLE =
        "QPushButton { background: rgba(255,255,255,0.9); color: #2E7D32; "
        "border: none; padding: 5px 16px; font-size: 12px; font-weight: 700; "
        "border-radius: %1; }";
    const QString TAB_NORMAL_STYLE =
        "QPushButton { background: rgba(255,255,255,0.12); color: rgba(255,255,255,0.85); "
        "border: none; padding: 5px 16px; font-size: 12px; font-weight: 500; "
        "border-radius: %1; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }";

    m_aiGenTabBtn = new QPushButton("AI 出题");
    m_aiGenTabBtn->setCursor(Qt::PointingHandCursor);
    m_aiGenTabBtn->setStyleSheet(TAB_ACTIVE_STYLE.arg("10px 0 0 10px"));

    m_smartPaperTabBtn = new QPushButton("智能组卷");
    m_smartPaperTabBtn->setCursor(Qt::PointingHandCursor);
    m_smartPaperTabBtn->setStyleSheet(TAB_NORMAL_STYLE.arg("0 10px 10px 0"));

    connect(m_aiGenTabBtn, &QPushButton::clicked, this, [this]() { switchMode(0); });
    connect(m_smartPaperTabBtn, &QPushButton::clicked, this, [this]() { switchMode(1); });

    // 质量检查按钮
    auto *qualityCheckBtn = new QPushButton("质量检查");
    qualityCheckBtn->setCursor(Qt::PointingHandCursor);
    qualityCheckBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.12); color: rgba(255,255,255,0.85); "
        "border: 1px solid rgba(255,255,255,0.25); padding: 5px 12px; font-size: 11px; "
        "font-weight: 500; border-radius: 10px; margin-left: 8px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }"
    );
    connect(qualityCheckBtn, &QPushButton::clicked, this, [this]() {
        auto *difyService = new DifyService(this);
        difyService->setApiKey(AppConfig::get(QStringLiteral("DIFY_API_KEY")));
        difyService->setBaseUrl(AppConfig::get(QStringLiteral("DIFY_API_BASE_URL"),
                                               QStringLiteral("https://api.dify.ai/v1")));
        auto *paperService = new PaperService(this);
        auto *qualityService = new QuestionQualityService(paperService, difyService, this);
        QualityCheckDialog dialog(qualityService, paperService, this);
        dialog.exec();
        qualityService->deleteLater();
        difyService->deleteLater();
        paperService->deleteLater();
    });

    layout->addWidget(backButton);
    layout->addWidget(m_headerTitle, 1);

    // 将 AI 侧边栏的下拉筛选组件嵌入到 Header 中
    if (m_aiQuestionGenWidget) {
        QWidget *filterWidget = m_aiQuestionGenWidget->curriculumFilterWidget();
        if (filterWidget) {
            layout->addWidget(filterWidget);
            // 添加一些间距，与右侧选项卡隔开
            layout->addSpacing(16);
        }
    }

    layout->addWidget(m_aiGenTabBtn);
    layout->addWidget(m_smartPaperTabBtn);
    layout->addWidget(qualityCheckBtn);

    return header;
}

void QuestionBankWindow::loadStyleSheet()
{
    QFile file(QStringLiteral(":/styles/resources/styles/question_bank.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to load question bank stylesheet from :/styles/resources/styles/question_bank.qss";

        QFile fallbackFile(QStringLiteral(":/styles/question_bank.qss"));
        if (!fallbackFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Unable to load question bank stylesheet from fallback path";
            return;
        }

        setStyleSheet(QString::fromUtf8(fallbackFile.readAll()));
        qDebug() << "Loaded question bank stylesheet from fallback path";
        return;
    }

    setStyleSheet(QString::fromUtf8(file.readAll()));
    qDebug() << "Loaded question bank stylesheet from primary path";
}

bool QuestionBankWindow::eventFilter(QObject *watched, QEvent *event)
{
    return QWidget::eventFilter(watched, event);
}

void QuestionBankWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (m_basketWidget) {
        const int margin = 24;
        int x = width() - m_basketWidget->width() - margin;
        int y = height() - m_basketWidget->height() - margin;
        m_basketWidget->move(x, y);
    }
}

void QuestionBankWindow::onComposePaper()
{
    int count = QuestionBasket::instance()->count();
    if (count == 0) {
        qDebug() << "[QuestionBankWindow] Basket is empty, cannot compose paper";
        return;
    }

    qDebug() << "[QuestionBankWindow] Opening paper composer with" << count << "questions";

    PaperComposerDialog dialog(this);
    connect(&dialog, &PaperComposerDialog::paperExported, this, [this](const QString &filePath) {
        qDebug() << "[QuestionBankWindow] Paper exported to:" << filePath;
        QuestionBasket::instance()->clear();
        if (m_basketWidget) {
            m_basketWidget->refresh();
        }
    });
    dialog.exec();
}

void QuestionBankWindow::switchMode(int mode)
{
    if (!m_modeStack) return;
    m_modeStack->setCurrentIndex(mode);

    const QString TAB_ACTIVE =
        "QPushButton { background: rgba(255,255,255,0.9); color: #2E7D32; "
        "border: none; padding: 5px 16px; font-size: 12px; font-weight: 700; "
        "border-radius: %1; }";
    const QString TAB_NORMAL =
        "QPushButton { background: rgba(255,255,255,0.12); color: rgba(255,255,255,0.85); "
        "border: none; padding: 5px 16px; font-size: 12px; font-weight: 500; "
        "border-radius: %1; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }";

    if (mode == 0) {
        if (m_aiGenTabBtn) m_aiGenTabBtn->setStyleSheet(TAB_ACTIVE.arg("10px 0 0 10px"));
        if (m_smartPaperTabBtn) m_smartPaperTabBtn->setStyleSheet(TAB_NORMAL.arg("0 10px 10px 0"));
        if (m_basketWidget) m_basketWidget->setVisible(false);
    } else {
        if (m_aiGenTabBtn) m_aiGenTabBtn->setStyleSheet(TAB_NORMAL.arg("10px 0 0 10px"));
        if (m_smartPaperTabBtn) m_smartPaperTabBtn->setStyleSheet(TAB_ACTIVE.arg("0 10px 10px 0"));
        if (m_basketWidget) m_basketWidget->setVisible(true);
    }
}

// ===================== AI 出题历史管理 =====================

void QuestionBankWindow::refreshQuestionHistory()
{
    if (!m_questionHistoryWidget) return;

    m_questionHistoryWidget->clearHistory();

    QSettings settings;
    QByteArray indexData = settings.value("questionGen/index").toByteArray();
    QJsonArray index = QJsonDocument::fromJson(indexData).array();

    for (const QJsonValue &val : index) {
        QJsonObject entry = val.toObject();
        QString id = entry["id"].toString();
        QString title = entry["title"].toString();
        QString updatedAt = entry["updatedAt"].toString();

        // 格式化时间
        QDateTime dt = QDateTime::fromString(updatedAt, Qt::ISODate);
        QString timeStr;
        if (dt.isValid()) {
            QDate today = QDate::currentDate();
            if (dt.date() == today) {
                timeStr = dt.toString("HH:mm");
            } else if (dt.date() == today.addDays(-1)) {
                timeStr = "昨天 " + dt.toString("HH:mm");
            } else {
                timeStr = dt.toString("MM-dd HH:mm");
            }
        }

        m_questionHistoryWidget->addHistoryItem(id, title, timeStr);
    }

    // 选中当前对话
    if (m_aiQuestionGenWidget) {
        QString currentId = m_aiQuestionGenWidget->currentConversationId();
        if (!currentId.isEmpty()) {
            m_questionHistoryWidget->selectItem(currentId);
        }
    }

    qDebug() << "[QuestionBankWindow] 历史列表已刷新，共" << index.size() << "条";
}

void QuestionBankWindow::onQuestionHistorySelected(const QString &id)
{
    if (!m_aiQuestionGenWidget) return;

    // 如果选中的就是当前对话，忽略
    if (id == m_aiQuestionGenWidget->currentConversationId()) return;

    m_aiQuestionGenWidget->loadConversation(id);
    qDebug() << "[QuestionBankWindow] 切换到历史对话:" << id;
}

void QuestionBankWindow::onQuestionHistoryDeleted(const QString &id)
{
    if (!m_aiQuestionGenWidget) return;

    m_aiQuestionGenWidget->deleteConversation(id);

    // 从侧边栏移除
    if (m_questionHistoryWidget) {
        m_questionHistoryWidget->removeHistoryItem(id);
    }

    // 如果删的是当前对话，重建新对话
    if (id == m_aiQuestionGenWidget->currentConversationId()) {
        m_aiQuestionGenWidget->startNewConversation();
    }

    qDebug() << "[QuestionBankWindow] 已删除历史对话:" << id;
}

// ===================== 导出 DOCX =====================

void QuestionBankWindow::onExportToDocx(const QString &content)
{
    if (content.trimmed().isEmpty() || m_isExporting) return;

    m_isExporting = true;

    qDebug() << "[QuestionBankWindow] 开始导出 DOCX（Markdown 直接转换）";

    // 智能生成试卷标题（而不是直接用用户的提示词）
    QString title = "道德与法治 专项练习";
    if (m_aiQuestionGenWidget) {
        QString convTitle = m_aiQuestionGenWidget->currentConversationTitle();
        if (!convTitle.isEmpty() && convTitle != "新对话") {
            // 从提示词中提取主题关键词
            // 例如 "帮我出5道选择题，主题：宪法" → 提取 "宪法"
            static const QRegularExpression topicRe(
                R"(主题[：:]\s*(.+)|关于[""「]?(.+?)[""」]|(?:出|生成).+?(?:关于|主题)\s*[：:]?\s*(.+))"
            );
            QRegularExpressionMatch topicMatch = topicRe.match(convTitle);
            if (topicMatch.hasMatch()) {
                QString topic = topicMatch.captured(1);
                if (topic.isEmpty()) topic = topicMatch.captured(2);
                if (topic.isEmpty()) topic = topicMatch.captured(3);
                topic = topic.trimmed();
                // 清理末尾标点
                topic.remove(QRegularExpression(R"([。，,.\s]+$)"));
                if (!topic.isEmpty()) {
                    title = QString("道德与法治 · %1 专项练习").arg(topic);
                }
            }
        }
    }

    // 弹出文件选择对话框
    QString defaultName = "AI出题_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".docx";
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出试卷",
        QDir::homePath() + "/Desktop/" + defaultName,
        "Word 文档 (*.docx)");

    if (filePath.isEmpty()) {
        m_isExporting = false;
        return;
    }

    // 直接从 Markdown 生成 DOCX，无需解析为 PaperQuestion
    bool success = m_docxGenerator->generateFromMarkdown(filePath, title, content);

    m_isExporting = false;

    if (success) {
        if (ModernDialogHelper::confirm(
                this, "导出成功",
                QString("试卷已导出到：\n%1\n\n是否打开文件？").arg(filePath))) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    } else {
        ModernDialogHelper::warning(this, "导出失败",
            "DOCX 文件生成失败：" + m_docxGenerator->lastError());
    }
}

// ===================== 保存到题库链路（原有逻辑） =====================

void QuestionBankWindow::onSaveGeneratedQuestionsRequested(const QString &content)
{
    if (m_isSavingGeneratedQuestions) return;
    if (!m_aiQuestionGenWidget || !m_questionParser) return;

    if (content.trimmed().isEmpty()) {
        m_aiQuestionGenWidget->showSaveErrorMessage("没有可保存的题目内容。");
        return;
    }

    if (!m_questionParser->isConfigured()) {
        m_aiQuestionGenWidget->showSaveErrorMessage("未配置题目解析服务的 API Key，无法保存到题库。");
        return;
    }

    m_isSavingGeneratedQuestions = true;
    m_aiQuestionGenWidget->setSavingToBank(true);
    m_questionParser->parseDocument(content, "道德与法治");
}

void QuestionBankWindow::onGeneratedQuestionsParsed(const QList<PaperQuestion> &questions)
{
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) return;

    if (questions.isEmpty()) {
        m_isSavingGeneratedQuestions = false;
        m_aiQuestionGenWidget->showSaveErrorMessage("AI 内容未能解析为有效题目，请调整格式后重试。");
        return;
    }

    const bool directInsert = questions.first().stem.startsWith("已由工作流插入");
    if (directInsert) {
        m_isSavingGeneratedQuestions = false;
        m_aiQuestionGenWidget->showSaveSuccessMessage(questions.size(), true);
        return;
    }

    m_paperService->addQuestions(questions);
}

void QuestionBankWindow::onGeneratedQuestionsSaved(int count)
{
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) return;

    m_isSavingGeneratedQuestions = false;
    m_aiQuestionGenWidget->showSaveSuccessMessage(count, false);
}

void QuestionBankWindow::onGeneratedQuestionsSaveError(const QString &, const QString &error)
{
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) return;

    m_isSavingGeneratedQuestions = false;
    m_aiQuestionGenWidget->showSaveErrorMessage(error);
}

void QuestionBankWindow::onGeneratedQuestionsParseError(const QString &error)
{
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) return;

    m_isSavingGeneratedQuestions = false;
    m_aiQuestionGenWidget->showSaveErrorMessage(error);
}
