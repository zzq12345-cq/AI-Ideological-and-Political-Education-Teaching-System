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
#include <QVBoxLayout>
#include <QPainter>

QuestionBankWindow::QuestionBankWindow(QWidget *parent)
    : QWidget(parent)
    , m_paperService(new PaperService(this))
    , m_questionParser(new QuestionParserService(this))
{
    setObjectName("questionBankWindow");

    QFont baseFont("Lexend", 12);
    baseFont.setStyleHint(QFont::SansSerif);
    baseFont.setStyleStrategy(QFont::PreferAntialias);
    setFont(baseFont);

    setupLayout();
    loadStyleSheet();

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

    // ====== Header 悬浮卡片 ======
    auto *headerWrapper = new QWidget(pageContainer);
    headerWrapper->setObjectName("headerWrapper");
    headerWrapper->setStyleSheet("QWidget#headerWrapper { background: transparent; }");
    auto *headerWrapperLayout = new QVBoxLayout(headerWrapper);
    headerWrapperLayout->setContentsMargins(20, 16, 20, 0);
    headerWrapperLayout->setSpacing(0);
    headerWrapperLayout->addWidget(buildHeader());

    pageLayout->addWidget(headerWrapper);

    // ====== 内容区域：AI出题 / 智能组卷 ======
    m_modeStack = new QStackedWidget();

    // page 0: AI 出题
    m_aiQuestionGenWidget = new AIQuestionGenWidget(this);
    m_modeStack->addWidget(m_aiQuestionGenWidget);

    // page 1: 智能组卷
    m_smartPaperWidget = new SmartPaperWidget(this);
    m_modeStack->addWidget(m_smartPaperWidget);

    pageLayout->addWidget(m_modeStack, 1);

    rootLayout->addWidget(pageContainer);

    // ====== 试题篮悬浮组件 ======
    m_basketWidget = new QuestionBasketWidget(this);
    m_basketWidget->setParent(this);
    m_basketWidget->raise();
    m_basketWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    // 连接试题篮信号
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
    header->setFixedHeight(120);

    // 绿色渐变 + 玻璃质感（教育成长风格）
    header->setStyleSheet(
        "QFrame#pageHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #2E7D32, stop:0.3 #388E3C, stop:0.7 #43A047, stop:1 #1B5E20);"
        "    border-radius: 20px;"
        "    border: 1px solid rgba(255,255,255,0.15);"
        "}"
    );

    QGraphicsDropShadowEffect *headerShadow = new QGraphicsDropShadowEffect(header);
    headerShadow->setBlurRadius(20);
    headerShadow->setOffset(0, 6);
    headerShadow->setColor(QColor(46, 125, 50, 80));
    header->setGraphicsEffect(headerShadow);

    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(32, 20, 32, 20);
    layout->setSpacing(20);

    // 返回按钮
    auto *backButton = new QPushButton(header);
    backButton->setObjectName("backButton");
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setFixedHeight(40);
    backButton->setMinimumWidth(110);
    backButton->setIcon(QIcon(":/QtTheme/icon/chevron_left/#e0e0e0.svg"));
    backButton->setIconSize(QSize(18, 18));
    backButton->setText(QStringLiteral("< 返回"));
    backButton->setStyleSheet(
        "QPushButton#backButton {"
        "    background: rgba(255,255,255,0.15);"
        "    border: 1px solid rgba(255,255,255,0.25);"
        "    border-radius: 20px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    color: white;"
        "}"
        "QPushButton#backButton:hover {"
        "    background: rgba(255,255,255,0.25);"
        "    border: 1px solid rgba(255,255,255,0.4);"
        "}"
        "QPushButton#backButton:pressed {"
        "    background: rgba(255,255,255,0.1);"
        "}"
    );
    connect(backButton, &QPushButton::clicked, this, &QuestionBankWindow::backRequested);

    // 装饰图标
    QLabel *iconDecor = new QLabel(header);
    iconDecor->setFixedSize(52, 52);
    iconDecor->setStyleSheet(
        "background: rgba(255,255,255,0.15);"
        "border-radius: 14px;"
        "border: 1px solid rgba(255,255,255,0.2);"
    );
    iconDecor->setAlignment(Qt::AlignCenter);
    QPixmap bookIcon(":/icons/resources/icons/book.svg");
    if (!bookIcon.isNull()) {
        QPixmap whiteIcon(bookIcon.size());
        whiteIcon.fill(Qt::transparent);
        QPainter painter(&whiteIcon);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawPixmap(0, 0, bookIcon);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(whiteIcon.rect(), Qt::white);
        painter.end();
        iconDecor->setPixmap(whiteIcon.scaled(26, 26, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 标题区域
    auto *titleWrapper = new QWidget(header);
    titleWrapper->setStyleSheet("background: transparent;");
    auto *titleLayout = new QVBoxLayout(titleWrapper);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(6);

    m_headerTitle = new QLabel(QStringLiteral("AI 智能备课 · 试题中心"), titleWrapper);
    m_headerTitle->setObjectName("pageTitle");
    m_headerTitle->setStyleSheet(
        "font-size: 26px; font-weight: 800; color: white; background: transparent;"
        "letter-spacing: 2px;"
    );

    m_headerSubtitle = new QLabel(QStringLiteral("◆ AI 出题  ◆ 智能组卷  ◆ 一键导出"), titleWrapper);
    m_headerSubtitle->setObjectName("pageSubtitle");
    m_headerSubtitle->setStyleSheet(
        "font-size: 13px; color: rgba(255,255,255,0.8); background: transparent;"
        "font-weight: 500; letter-spacing: 1px;"
    );

    titleLayout->addWidget(m_headerTitle);
    titleLayout->addWidget(m_headerSubtitle);

    // 右侧分段控制
    auto *rightControls = new QWidget(header);
    rightControls->setStyleSheet("background: transparent;");
    auto *rightLayout = new QVBoxLayout(rightControls);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    // 分段切换按钮
    auto *tabRow = new QHBoxLayout();
    tabRow->setSpacing(0);

    const QString TAB_ACTIVE_STYLE =
        "QPushButton { background: rgba(255,255,255,0.9); color: #2E7D32; "
        "border: none; padding: 8px 20px; font-size: 13px; font-weight: 700; "
        "border-radius: %1; }";
    const QString TAB_NORMAL_STYLE =
        "QPushButton { background: rgba(255,255,255,0.12); color: rgba(255,255,255,0.85); "
        "border: none; padding: 8px 20px; font-size: 13px; font-weight: 500; "
        "border-radius: %1; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }";

    m_aiGenTabBtn = new QPushButton("AI 出题");
    m_aiGenTabBtn->setCursor(Qt::PointingHandCursor);
    m_aiGenTabBtn->setStyleSheet(TAB_ACTIVE_STYLE.arg("16px 0 0 16px"));

    m_smartPaperTabBtn = new QPushButton("智能组卷");
    m_smartPaperTabBtn->setCursor(Qt::PointingHandCursor);
    m_smartPaperTabBtn->setStyleSheet(TAB_NORMAL_STYLE.arg("0 16px 16px 0"));

    connect(m_aiGenTabBtn, &QPushButton::clicked, this, [this]() { switchMode(0); });
    connect(m_smartPaperTabBtn, &QPushButton::clicked, this, [this]() { switchMode(1); });

    // 质量检查按钮
    auto *qualityCheckBtn = new QPushButton("质量检查");
    qualityCheckBtn->setCursor(Qt::PointingHandCursor);
    qualityCheckBtn->setStyleSheet(
        "QPushButton { background: rgba(255,255,255,0.12); color: rgba(255,255,255,0.85); "
        "border: 1px solid rgba(255,255,255,0.25); padding: 8px 16px; font-size: 12px; "
        "font-weight: 500; border-radius: 16px; margin-left: 12px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }"
    );
    connect(qualityCheckBtn, &QPushButton::clicked, this, [this]() {
        auto *difyService = new DifyService(this);
        difyService->setApiKey(qEnvironmentVariable("DIFY_API_KEY"));
        // 质量检查需要 PaperService，但现在不再持有。创建临时实例
        auto *paperService = new PaperService(this);
        auto *qualityService = new QuestionQualityService(paperService, difyService, this);
        QualityCheckDialog dialog(qualityService, paperService, this);
        dialog.exec();
        qualityService->deleteLater();
        difyService->deleteLater();
        paperService->deleteLater();
    });

    tabRow->addStretch();
    tabRow->addWidget(m_aiGenTabBtn);
    tabRow->addWidget(m_smartPaperTabBtn);
    tabRow->addWidget(qualityCheckBtn);
    rightLayout->addLayout(tabRow);

    layout->addWidget(backButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(iconDecor, 0, Qt::AlignVCenter);
    layout->addWidget(titleWrapper, 1, Qt::AlignVCenter);
    layout->addWidget(rightControls, 0, Qt::AlignRight | Qt::AlignVCenter);

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

    // 将试题篮组件定位到右下角
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
        "border: none; padding: 8px 20px; font-size: 13px; font-weight: 700; "
        "border-radius: %1; }";
    const QString TAB_NORMAL =
        "QPushButton { background: rgba(255,255,255,0.12); color: rgba(255,255,255,0.85); "
        "border: none; padding: 8px 20px; font-size: 13px; font-weight: 500; "
        "border-radius: %1; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }";

    if (mode == 0) {
        // AI 出题模式
        if (m_aiGenTabBtn) m_aiGenTabBtn->setStyleSheet(TAB_ACTIVE.arg("16px 0 0 16px"));
        if (m_smartPaperTabBtn) m_smartPaperTabBtn->setStyleSheet(TAB_NORMAL.arg("0 16px 16px 0"));
        if (m_headerTitle) m_headerTitle->setText("AI 智能备课 · AI 出题");
        if (m_headerSubtitle) m_headerSubtitle->setText("◆ 对话出题  ◆ 智能生成  ◆ 一键保存");
        if (m_basketWidget) m_basketWidget->setVisible(false);
    } else {
        // 智能组卷模式
        if (m_aiGenTabBtn) m_aiGenTabBtn->setStyleSheet(TAB_NORMAL.arg("16px 0 0 16px"));
        if (m_smartPaperTabBtn) m_smartPaperTabBtn->setStyleSheet(TAB_ACTIVE.arg("0 16px 16px 0"));
        if (m_headerTitle) m_headerTitle->setText("AI 智能备课 · 智能组卷");
        if (m_headerSubtitle) m_headerSubtitle->setText("◆ 智能选题  ◆ 约束优化  ◆ 一键成卷");
        if (m_basketWidget) m_basketWidget->setVisible(true);
    }
}

void QuestionBankWindow::onSaveGeneratedQuestionsRequested(const QString &content)
{
    if (m_isSavingGeneratedQuestions) {
        return;
    }

    if (!m_aiQuestionGenWidget || !m_questionParser) {
        return;
    }

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
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) {
        return;
    }

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
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) {
        return;
    }

    m_isSavingGeneratedQuestions = false;
    m_aiQuestionGenWidget->showSaveSuccessMessage(count, false);
}

void QuestionBankWindow::onGeneratedQuestionsSaveError(const QString &, const QString &error)
{
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) {
        return;
    }

    m_isSavingGeneratedQuestions = false;
    m_aiQuestionGenWidget->showSaveErrorMessage(error);
}

void QuestionBankWindow::onGeneratedQuestionsParseError(const QString &error)
{
    if (!m_isSavingGeneratedQuestions || !m_aiQuestionGenWidget) {
        return;
    }

    m_isSavingGeneratedQuestions = false;
    m_aiQuestionGenWidget->showSaveErrorMessage(error);
}
