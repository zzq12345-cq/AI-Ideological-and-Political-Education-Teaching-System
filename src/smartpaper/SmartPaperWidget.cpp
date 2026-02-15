#include "SmartPaperWidget.h"
#include "../services/PaperService.h"
#include "../questionbank/QuestionBasket.h"
#include "../questionbank/PaperComposerDialog.h"

#include <QScrollArea>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QGraphicsDropShadowEffect>

// 样式常量 — 精致教育风格
namespace {
    // 主色板
    const QString ACCENT_RED = "#D32F2F";
    const QString ACCENT_RED_HOVER = "#B71C1C";
    const QString ACCENT_RED_LIGHT = "#FFEBEE";
    const QString ACCENT_BLUE = "#1565C0";
    const QString ACCENT_BLUE_LIGHT = "#E3F2FD";
    const QString SUCCESS_GREEN = "#2E7D32";
    const QString SUCCESS_GREEN_LIGHT = "#E8F5E9";
    const QString WARNING_AMBER = "#E65100";
    const QString WARNING_AMBER_LIGHT = "#FFF3E0";

    // 中性色
    const QString BG_PAGE = "#F8F9FA";
    const QString CARD_BG = "#FFFFFF";
    const QString TEXT_PRIMARY = "#1A1A2E";
    const QString TEXT_SECONDARY = "#6B7280";
    const QString TEXT_HINT = "#9CA3AF";
    const QString BORDER_SUBTLE = "#E5E7EB";
    const QString BORDER_FOCUS = "#D32F2F";
    const QString DIVIDER = "#F3F4F6";

    // 卡片样式
    const QString REFINED_CARD = R"(
        QFrame#configCard, QFrame#resultStatsCard {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 16px;
        }
    )";

    // 输入框样式 — 精致圆角 + 焦点高亮
    const QString INPUT_STYLE = R"(
        QLineEdit, QComboBox, QSpinBox {
            background-color: #F9FAFB;
            border: 1.5px solid #E5E7EB;
            border-radius: 10px;
            padding: 10px 14px;
            font-size: 14px;
            color: #1A1A2E;
            selection-background-color: #FFEBEE;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
            border-color: #D32F2F;
            background-color: #FFFFFF;
            border-width: 2px;
        }
        QLineEdit:hover, QComboBox:hover, QSpinBox:hover {
            border-color: #D1D5DB;
            background-color: #FFFFFF;
        }
        QComboBox::drop-down {
            border: none;
            padding-right: 10px;
            width: 24px;
        }
        QComboBox::down-arrow {
            image: none;
            border: none;
        }
        QSpinBox::up-button, QSpinBox::down-button {
            width: 20px;
            border: none;
            border-radius: 4px;
        }
        QSpinBox::up-button:hover, QSpinBox::down-button:hover {
            background-color: #F3F4F6;
        }
    )";

    // 主操作按钮
    const QString PRIMARY_BTN_STYLE = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #D32F2F, stop:1 #E53935);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 48px;
            font-size: 15px;
            font-weight: 700;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #B71C1C, stop:1 #C62828);
        }
        QPushButton:pressed {
            background-color: #B71C1C;
        }
        QPushButton:disabled {
            background-color: #E5E7EB;
            color: #9CA3AF;
        }
    )";

    // 底部操作按钮 — %1=背景色, %2=hover色
    const QString ACTION_BTN_STYLE = R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 11px 28px;
            font-size: 13px;
            font-weight: 600;
            letter-spacing: 0.3px;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:pressed {
            background-color: %2;
            padding-top: 12px;
        }
        QPushButton:disabled {
            background-color: #E5E7EB;
            color: #9CA3AF;
        }
    )";

    // 删除按钮
    const QString REMOVE_BTN_STYLE = R"(
        QPushButton {
            background-color: transparent;
            color: #EF4444;
            border: 1.5px solid #FECACA;
            border-radius: 8px;
            padding: 4px 10px;
            font-size: 12px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #FEF2F2;
            border-color: #EF4444;
        }
    )";

    // 换一题按钮
    const QString SWAP_BTN_STYLE = R"(
        QPushButton {
            background-color: transparent;
            color: #1565C0;
            border: 1.5px solid #BBDEFB;
            border-radius: 8px;
            padding: 4px 14px;
            font-size: 12px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #E3F2FD;
            border-color: #1565C0;
        }
        QPushButton:disabled {
            color: #D1D5DB;
            border-color: #E5E7EB;
        }
    )";

    // 难度胶囊
    const QString DIFFICULTY_CAPSULE_STYLE = R"(
        QLabel {
            background-color: %1;
            color: white;
            border-radius: 10px;
            padding: 3px 12px;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 0.5px;
        }
    )";

    // 虚线添加按钮
    const QString ADD_TYPE_BTN_STYLE = R"(
        QPushButton {
            background: transparent;
            color: #D32F2F;
            border: 1.5px dashed #FFCDD2;
            border-radius: 10px;
            padding: 10px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #FFF5F5;
            border-color: #D32F2F;
        }
    )";

    // 节标题样式
    const QString SECTION_HEADER_STYLE = R"(
        QLabel {
            font-size: 15px;
            font-weight: 700;
            color: #1A1A2E;
            padding-left: 12px;
            border-left: 3px solid #D32F2F;
        }
    )";

    // 别名 — 统一引用（避免命名不一致导致编译失败）
    const QString PATRIOTIC_RED = ACCENT_RED;
    const QString WARNING_YELLOW = "#F59E0B";
    const QString BORDER_COLOR = BORDER_SUBTLE;
    const QString ERROR_RED = "#EF4444";
    const QString CARD_STYLE = REFINED_CARD;
    const QString CARD_WHITE = CARD_BG;
}

SmartPaperWidget::SmartPaperWidget(QWidget *parent)
    : QWidget(parent)
{
    // 创建 PaperService（后续可以改为注入）
    m_paperService = new PaperService(this);
    m_smartPaperService = new SmartPaperService(m_paperService, this);

    // 连接 SmartPaperService 信号
    connect(m_smartPaperService, &SmartPaperService::generationCompleted,
            this, &SmartPaperWidget::onGenerationCompleted);
    connect(m_smartPaperService, &SmartPaperService::progressUpdated,
            this, &SmartPaperWidget::onProgressUpdated);
    connect(m_smartPaperService, &SmartPaperService::generationFailed,
            this, &SmartPaperWidget::onGenerationFailed);

    // 连接 PaperService 信号（保存到云端）
    connect(m_paperService, &PaperService::paperCreated,
            this, &SmartPaperWidget::onPaperCreated);

    initUI();
}

SmartPaperWidget::~SmartPaperWidget() = default;

void SmartPaperWidget::initUI()
{
    // 外层滚动区域
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: " + BG_PAGE + "; border: none; }");

    auto *scrollContent = new QWidget();
    auto *mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setContentsMargins(32, 32, 32, 32);
    mainLayout->setSpacing(24);

    setupConfigCard(mainLayout);
    setupStatusArea(mainLayout);
    setupResultPreview(mainLayout);
    setupBottomActions(mainLayout);

    mainLayout->addStretch();
    scrollArea->setWidget(scrollContent);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scrollArea);

    // 初始状态
    setState(AssemblyState::Idle);
}

void SmartPaperWidget::setupTitleSection(QVBoxLayout *)
{
    // 标题由 QuestionBankWindow 的 header 提供，此处不再需要
}

void SmartPaperWidget::setupConfigCard(QVBoxLayout *mainLayout)
{
    auto *card = new QFrame();
    card->setObjectName("configCard");
    card->setStyleSheet(REFINED_CARD + "\n" + INPUT_STYLE);

    // 精致投影
    auto *cardShadow = new QGraphicsDropShadowEffect(card);
    cardShadow->setBlurRadius(24);
    cardShadow->setOffset(0, 4);
    cardShadow->setColor(QColor(0, 0, 0, 25));
    card->setGraphicsEffect(cardShadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(20);

    // ==================== 基本信息行 ====================
    auto *basicRow = new QHBoxLayout();
    basicRow->setSpacing(16);

    auto labelStyle = [](const QString &text) {
        auto *lbl = new QLabel(text);
        lbl->setStyleSheet("QLabel { font-size: 12px; font-weight: 600; color: #6B7280; letter-spacing: 0.3px; margin-bottom: 2px; }");
        return lbl;
    };

    // 试卷标题
    auto *titleGroup = new QVBoxLayout();
    titleGroup->addWidget(labelStyle("试卷标题"));
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("例：2024年春季学期思政期中考试");
    m_titleEdit->setMinimumHeight(40);
    titleGroup->addWidget(m_titleEdit);
    basicRow->addLayout(titleGroup, 2);

    // 学科
    auto *subjectGroup = new QVBoxLayout();
    subjectGroup->addWidget(labelStyle("学科"));
    m_subjectCombo = new QComboBox();
    m_subjectCombo->addItems({"道德与法治"});
    m_subjectCombo->setMinimumHeight(40);
    subjectGroup->addWidget(m_subjectCombo);
    basicRow->addLayout(subjectGroup, 1);

    // 年级
    auto *gradeGroup = new QVBoxLayout();
    gradeGroup->addWidget(labelStyle("年级"));
    m_gradeCombo = new QComboBox();
    m_gradeCombo->addItems({"七年级", "八年级", "九年级"});
    m_gradeCombo->setMinimumHeight(40);
    gradeGroup->addWidget(m_gradeCombo);
    basicRow->addLayout(gradeGroup, 1);

    // 时长
    auto *durationGroup = new QVBoxLayout();
    durationGroup->addWidget(labelStyle("时长(分钟)"));
    m_durationCombo = new QComboBox();
    m_durationCombo->addItems({"45", "60", "90", "120", "150"});
    m_durationCombo->setCurrentText("90");
    m_durationCombo->setMinimumHeight(40);
    durationGroup->addWidget(m_durationCombo);
    basicRow->addLayout(durationGroup, 1);

    cardLayout->addLayout(basicRow);

    // ==================== 分隔线 ====================
    auto *sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("QFrame { color: " + BORDER_SUBTLE + "; }");
    cardLayout->addWidget(sep1);

    // ==================== 题型分布表 ====================
    auto *typeHeader = new QLabel("题型分布");
    typeHeader->setStyleSheet(SECTION_HEADER_STYLE);
    cardLayout->addWidget(typeHeader);

    // 题型表头行
    auto *headerRow = new QWidget();
    auto *headerRowLayout = new QHBoxLayout(headerRow);
    headerRowLayout->setContentsMargins(0, 0, 0, 0);
    headerRowLayout->setSpacing(12);
    auto makeHeaderLabel = [](const QString &text, int stretch) {
        auto *lbl = new QLabel(text);
        lbl->setStyleSheet("QLabel { font-size: 12px; font-weight: 600; color: #9CA3AF; letter-spacing: 0.5px; }");
        return lbl;
    };
    headerRowLayout->addWidget(makeHeaderLabel("题型", 2), 2);
    headerRowLayout->addWidget(makeHeaderLabel("数量", 1), 1);
    headerRowLayout->addWidget(makeHeaderLabel("分值", 1), 1);
    headerRowLayout->addWidget(makeHeaderLabel("小计", 0), 0);
    auto *placeholderLabel = new QLabel("");
    placeholderLabel->setFixedWidth(50);
    headerRowLayout->addWidget(placeholderLabel);
    cardLayout->addWidget(headerRow);

    m_typeRowsLayout = new QVBoxLayout();
    m_typeRowsLayout->setSpacing(8);
    cardLayout->addLayout(m_typeRowsLayout);

    // 默认添加两行
    onAddTypeRow();
    onAddTypeRow();

    // 添加题型按钮
    auto *addTypeBtn = new QPushButton("+ 添加题型");
    addTypeBtn->setStyleSheet(ADD_TYPE_BTN_STYLE);
    connect(addTypeBtn, &QPushButton::clicked, this, &SmartPaperWidget::onAddTypeRow);
    cardLayout->addWidget(addTypeBtn);

    // 总分汇总
    m_totalScoreLabel = new QLabel("总分: 0 分 | 总题数: 0 题");
    m_totalScoreLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + TEXT_PRIMARY + "; }");
    cardLayout->addWidget(m_totalScoreLabel);
    updateTotalScoreLabel();  // 用已添加的题型行刷新总分显示

    // ==================== 分隔线 ====================
    auto *sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("QFrame { color: " + BORDER_SUBTLE + "; }");
    cardLayout->addWidget(sep2);

    // ==================== 难度比例 ====================
    auto *diffHeader = new QLabel("难度比例");
    diffHeader->setStyleSheet(SECTION_HEADER_STYLE);
    cardLayout->addWidget(diffHeader);

    auto *diffRow = new QHBoxLayout();
    diffRow->setSpacing(12);

    auto createDiffSpin = [&](const QString &label, int defaultVal) -> QSpinBox * {
        auto *group = new QVBoxLayout();
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("QLabel { font-size: 12px; font-weight: 600; color: #6B7280; }");
        group->addWidget(lbl);
        auto *spin = new QSpinBox();
        spin->setRange(0, 10);
        spin->setValue(defaultVal);
        spin->setMinimumHeight(40);
        group->addWidget(spin);
        diffRow->addLayout(group);
        return spin;
    };

    m_easyRatioSpin = createDiffSpin("简单", 3);
    auto *colonLabel1 = new QLabel(":");
    colonLabel1->setAlignment(Qt::AlignCenter);
    colonLabel1->setFixedWidth(20);
    diffRow->addWidget(colonLabel1);

    m_mediumRatioSpin = createDiffSpin("中等", 5);
    auto *colonLabel2 = new QLabel(":");
    colonLabel2->setAlignment(Qt::AlignCenter);
    colonLabel2->setFixedWidth(20);
    diffRow->addWidget(colonLabel2);

    m_hardRatioSpin = createDiffSpin("困难", 2);
    diffRow->addStretch();

    cardLayout->addLayout(diffRow);

    // ==================== 开始组卷按钮 ====================
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    m_generateBtn = new QPushButton("开始组卷");
    m_generateBtn->setStyleSheet(PRIMARY_BTN_STYLE);
    m_generateBtn->setMinimumWidth(200);
    m_generateBtn->setCursor(Qt::PointingHandCursor);
    connect(m_generateBtn, &QPushButton::clicked, this, &SmartPaperWidget::onGenerateClicked);
    btnRow->addWidget(m_generateBtn);
    btnRow->addStretch();
    cardLayout->addLayout(btnRow);

    mainLayout->addWidget(card);

    // 初始校验
    updateGenerateButton();
}

QWidget *SmartPaperWidget::createTypeRow(int index)
{
    auto *row = new QWidget();
    row->setStyleSheet(
        QString("QWidget { background-color: %1; border-radius: 10px; padding: 6px 12px; }")
            .arg(index % 2 == 0 ? "#FFFFFF" : "#F8F9FB"));
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(4, 4, 4, 4);
    rowLayout->setSpacing(12);

    // 题型选择 — 展示中文名，userData 存原始值
    auto *typeCombo = new QComboBox();
    typeCombo->setObjectName("typeCombo");
    typeCombo->addItem("选择题", "single_choice");
    typeCombo->addItem("多选题", "multi_choice");
    typeCombo->addItem("判断题", "true_false");
    typeCombo->addItem("简答题", "short_answer");
    typeCombo->addItem("论述题", "essay");
    typeCombo->addItem("材料分析题", "material_analysis");
    // 设置默认选项
    if (index == 0) typeCombo->setCurrentIndex(0);  // 选择题
    else if (index == 1) typeCombo->setCurrentIndex(2);  // 判断题
    typeCombo->setMinimumHeight(32);
    typeCombo->setMinimumWidth(140);
    rowLayout->addWidget(typeCombo, 2);

    // 数量
    auto *countSpin = new QSpinBox();
    countSpin->setObjectName("countSpin");
    countSpin->setRange(1, 50);
    countSpin->setValue(index == 0 ? 10 : 5);
    countSpin->setSuffix(" 题");
    countSpin->setMinimumHeight(32);
    rowLayout->addWidget(countSpin, 1);

    // 每题分值
    auto *scoreSpin = new QSpinBox();
    scoreSpin->setObjectName("scoreSpin");
    scoreSpin->setRange(1, 50);
    scoreSpin->setValue(index == 0 ? 2 : 4);
    scoreSpin->setSuffix(" 分/题");
    scoreSpin->setMinimumHeight(32);
    rowLayout->addWidget(scoreSpin, 1);

    // 小计
    auto *subtotalLabel = new QLabel();
    subtotalLabel->setObjectName("subtotalLabel");
    subtotalLabel->setMinimumWidth(80);
    subtotalLabel->setAlignment(Qt::AlignCenter);
    subtotalLabel->setStyleSheet("QLabel { font-weight: 600; color: " + PATRIOTIC_RED + "; }");
    rowLayout->addWidget(subtotalLabel);

    // 删除按钮
    auto *removeBtn = new QPushButton("删除");
    removeBtn->setStyleSheet(REMOVE_BTN_STYLE);
    removeBtn->setCursor(Qt::PointingHandCursor);
    rowLayout->addWidget(removeBtn);

    // 更新小计
    auto updateSubtotal = [countSpin, scoreSpin, subtotalLabel, this]() {
        int sub = countSpin->value() * scoreSpin->value();
        subtotalLabel->setText(QString("= %1 分").arg(sub));
        updateTotalScoreLabel();
    };
    connect(countSpin, QOverload<int>::of(&QSpinBox::valueChanged), row, updateSubtotal);
    connect(scoreSpin, QOverload<int>::of(&QSpinBox::valueChanged), row, updateSubtotal);
    updateSubtotal();

    // 删除按钮连接
    connect(removeBtn, &QPushButton::clicked, this, [this, row]() {
        int idx = m_typeRows.indexOf(row);
        if (idx >= 0) {
            onRemoveTypeRow(idx);
        }
    });

    return row;
}

void SmartPaperWidget::onAddTypeRow()
{
    int index = m_typeRows.size();
    auto *row = createTypeRow(index);
    m_typeRows.append(row);
    m_typeRowsLayout->addWidget(row);
    updateGenerateButton();
}

void SmartPaperWidget::onRemoveTypeRow(int index)
{
    if (index < 0 || index >= m_typeRows.size()) return;
    auto *row = m_typeRows.takeAt(index);
    m_typeRowsLayout->removeWidget(row);
    row->deleteLater();
    updateTotalScoreLabel();
    updateGenerateButton();
}

void SmartPaperWidget::updateTotalScoreLabel()
{
    if (!m_totalScoreLabel) return;  // 初始化阶段尚未创建
    int totalScore = 0;
    int totalCount = 0;
    for (auto *row : m_typeRows) {
        auto *countSpin = row->findChild<QSpinBox *>("countSpin");
        auto *scoreSpin = row->findChild<QSpinBox *>("scoreSpin");
        if (countSpin && scoreSpin) {
            totalCount += countSpin->value();
            totalScore += countSpin->value() * scoreSpin->value();
        }
    }

    m_totalScoreLabel->setText(
        QString("总分: %1 分 | 总题数: %2 题").arg(totalScore).arg(totalCount));

    // 不匹配100分时标红提示
    if (totalScore != 100) {
        m_totalScoreLabel->setStyleSheet(
            "QLabel { font-size: 14px; font-weight: 600; color: " + WARNING_YELLOW + "; }");
    } else {
        m_totalScoreLabel->setStyleSheet(
            "QLabel { font-size: 14px; font-weight: 600; color: " + SUCCESS_GREEN + "; }");
    }
}

void SmartPaperWidget::updateGenerateButton()
{
    if (!m_generateBtn) return;  // 初始化阶段尚未创建
    bool hasTypeRows = !m_typeRows.isEmpty();
    m_generateBtn->setEnabled(hasTypeRows);
    m_generateBtn->setToolTip(hasTypeRows ? "" : "请至少添加一个题型");
}

void SmartPaperWidget::setupStatusArea(QVBoxLayout *mainLayout)
{
    m_statusStack = new QStackedWidget();
    m_statusStack->setFixedHeight(60);

    // 页面0: 空（Idle状态不显示）
    m_statusStack->addWidget(new QWidget());

    // 页面1: Generating - 进度条 + 状态文字
    auto *generatingWidget = new QWidget();
    auto *genLayout = new QVBoxLayout(generatingWidget);
    genLayout->setContentsMargins(0, 0, 0, 0);
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #F3F4F6; border-radius: 4px; border: none; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 " + ACCENT_RED + ", stop:1 #E53935); border-radius: 4px; }");
    m_statusLabel = new QLabel("正在组卷...");
    m_statusLabel->setStyleSheet("QLabel { font-size: 13px; color: " + TEXT_SECONDARY + "; }");
    genLayout->addWidget(m_progressBar);
    genLayout->addWidget(m_statusLabel);
    m_statusStack->addWidget(generatingWidget);

    // 页面2: Failed - 错误信息 + 重试按钮
    auto *failedWidget = new QWidget();
    auto *failLayout = new QHBoxLayout(failedWidget);
    failLayout->setContentsMargins(0, 0, 0, 0);
    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("QLabel { font-size: 13px; color: " + ERROR_RED + "; }");
    m_retryBtn = new QPushButton("重试");
    m_retryBtn->setStyleSheet(ACTION_BTN_STYLE.arg(ACCENT_RED, ACCENT_RED_HOVER));
    m_retryBtn->setCursor(Qt::PointingHandCursor);
    connect(m_retryBtn, &QPushButton::clicked, this, &SmartPaperWidget::onGenerateClicked);
    failLayout->addWidget(m_errorLabel, 1);
    failLayout->addWidget(m_retryBtn);
    m_statusStack->addWidget(failedWidget);

    mainLayout->addWidget(m_statusStack);
}

void SmartPaperWidget::setupResultPreview(QVBoxLayout *mainLayout)
{
    m_resultWidget = new QWidget();
    m_resultLayout = new QVBoxLayout(m_resultWidget);
    m_resultLayout->setContentsMargins(0, 0, 0, 0);
    m_resultLayout->setSpacing(16);
    m_resultWidget->setVisible(false);
    mainLayout->addWidget(m_resultWidget);
}

void SmartPaperWidget::setupBottomActions(QVBoxLayout *mainLayout)
{
    m_bottomActions = new QWidget();
    auto *actionsLayout = new QHBoxLayout(m_bottomActions);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(12);

    auto *regenerateBtn = new QPushButton("重新组卷");
    regenerateBtn->setStyleSheet(ACTION_BTN_STYLE.arg("#757575", "#616161"));
    regenerateBtn->setCursor(Qt::PointingHandCursor);
    connect(regenerateBtn, &QPushButton::clicked, this, &SmartPaperWidget::onRegenerate);

    auto *saveBtn = new QPushButton("保存到云端");
    saveBtn->setStyleSheet(ACTION_BTN_STYLE.arg("#1976D2", "#1565C0"));
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &SmartPaperWidget::onSaveToCloud);

    auto *importBtn = new QPushButton("导入试题篮");
    importBtn->setStyleSheet(ACTION_BTN_STYLE.arg(SUCCESS_GREEN, "#1B5E20"));
    importBtn->setCursor(Qt::PointingHandCursor);
    connect(importBtn, &QPushButton::clicked, this, &SmartPaperWidget::onImportToBasket);

    auto *editBtn = new QPushButton("编辑导出");
    editBtn->setStyleSheet(ACTION_BTN_STYLE.arg(ACCENT_RED, ACCENT_RED_HOVER));
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &SmartPaperWidget::onEditExport);

    actionsLayout->addWidget(regenerateBtn);
    actionsLayout->addStretch();
    actionsLayout->addWidget(saveBtn);
    actionsLayout->addWidget(importBtn);
    actionsLayout->addWidget(editBtn);

    m_bottomActions->setVisible(false);
    mainLayout->addWidget(m_bottomActions);
}

void SmartPaperWidget::setState(AssemblyState state)
{
    m_state = state;
    switch (state) {
    case AssemblyState::Idle:
        m_statusStack->setCurrentIndex(0);
        m_resultWidget->setVisible(false);
        m_bottomActions->setVisible(false);
        m_generateBtn->setEnabled(!m_typeRows.isEmpty());
        break;
    case AssemblyState::Generating:
        m_statusStack->setCurrentIndex(1);
        m_resultWidget->setVisible(false);
        m_bottomActions->setVisible(false);
        m_generateBtn->setEnabled(false);
        break;
    case AssemblyState::Success:
        m_statusStack->setCurrentIndex(0);
        m_resultWidget->setVisible(true);
        m_bottomActions->setVisible(true);
        m_generateBtn->setEnabled(true);
        break;
    case AssemblyState::Failed:
        m_statusStack->setCurrentIndex(2);
        m_resultWidget->setVisible(false);
        m_bottomActions->setVisible(false);
        m_generateBtn->setEnabled(true);
        break;
    }
}

void SmartPaperWidget::onGenerateClicked()
{
    // 收集配置
    m_config = SmartPaperConfig();
    m_config.title = m_titleEdit->text().trimmed();
    if (m_config.title.isEmpty()) {
        m_config.title = "智能组卷试卷";
    }
    m_config.subject = m_subjectCombo->currentText();
    m_config.grade = m_gradeCombo->currentText();
    m_config.duration = m_durationCombo->currentText().toInt();
    m_config.easyRatio = m_easyRatioSpin->value();
    m_config.mediumRatio = m_mediumRatioSpin->value();
    m_config.hardRatio = m_hardRatioSpin->value();

    // 收集题型配置
    for (auto *row : m_typeRows) {
        auto *typeCombo = row->findChild<QComboBox *>("typeCombo");
        auto *countSpin = row->findChild<QSpinBox *>("countSpin");
        auto *scoreSpin = row->findChild<QSpinBox *>("scoreSpin");
        if (typeCombo && countSpin && scoreSpin) {
            QuestionTypeSpec spec;
            // 从 userData 获取原始题型值（如 single_choice）
            spec.questionType = typeCombo->currentData().toString();
            spec.count = countSpin->value();
            spec.scorePerQuestion = scoreSpin->value();
            m_config.typeSpecs.append(spec);
        }
    }

    m_config.targetTotalScore = m_config.computedTotalScore();

    setState(AssemblyState::Generating);
    clearResultPreview();

    // 发起组卷
    m_smartPaperService->generate(m_config);
}

void SmartPaperWidget::onGenerationCompleted(const SmartPaperResult &result)
{
    m_currentResult = result;
    setState(AssemblyState::Success);
    buildResultPreview();
}

void SmartPaperWidget::onProgressUpdated(int percent, const QString &message)
{
    m_progressBar->setValue(percent);
    m_statusLabel->setText(message);
}

void SmartPaperWidget::onGenerationFailed(const QString &error)
{
    m_errorLabel->setText(error);
    setState(AssemblyState::Failed);
}

void SmartPaperWidget::clearResultPreview()
{
    // 清除结果预览区的所有子控件
    QLayoutItem *item;
    while ((item = m_resultLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void SmartPaperWidget::buildResultPreview()
{
    clearResultPreview();

    // ==================== 统计卡片行 ====================
    auto *statsRow = new QFrame();
    statsRow->setObjectName("resultStatsCard");
    statsRow->setStyleSheet(REFINED_CARD);

    // 统计卡片投影
    auto *statsShadow = new QGraphicsDropShadowEffect(statsRow);
    statsShadow->setBlurRadius(20);
    statsShadow->setOffset(0, 3);
    statsShadow->setColor(QColor(0, 0, 0, 20));
    statsRow->setGraphicsEffect(statsShadow);

    auto *statsLayout = new QHBoxLayout(statsRow);
    statsLayout->setContentsMargins(20, 16, 20, 16);
    statsLayout->setSpacing(24);

    // 总题数
    auto *countWidget = new QVBoxLayout();
    auto *countValue = new QLabel(QString::number(m_currentResult.selectedQuestions.size()));
    countValue->setStyleSheet("QLabel { font-size: 28px; font-weight: 700; color: " + PATRIOTIC_RED + "; }");
    countValue->setAlignment(Qt::AlignCenter);
    auto *countLabel = new QLabel("总题数");
    countLabel->setStyleSheet("QLabel { font-size: 12px; color: " + TEXT_SECONDARY + "; }");
    countLabel->setAlignment(Qt::AlignCenter);
    countWidget->addWidget(countValue);
    countWidget->addWidget(countLabel);
    statsLayout->addLayout(countWidget);

    // 分隔线
    auto *vSep1 = new QFrame();
    vSep1->setFrameShape(QFrame::VLine);
    vSep1->setStyleSheet("QFrame { color: " + BORDER_COLOR + "; }");
    statsLayout->addWidget(vSep1);

    // 总分
    auto *scoreWidget = new QVBoxLayout();
    auto *scoreValue = new QLabel(QString::number(m_currentResult.totalScore));
    scoreValue->setStyleSheet("QLabel { font-size: 28px; font-weight: 700; color: " + PATRIOTIC_RED + "; }");
    scoreValue->setAlignment(Qt::AlignCenter);
    auto *scoreLabel = new QLabel("总分");
    scoreLabel->setStyleSheet("QLabel { font-size: 12px; color: " + TEXT_SECONDARY + "; }");
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreWidget->addWidget(scoreValue);
    scoreWidget->addWidget(scoreLabel);
    statsLayout->addLayout(scoreWidget);

    // 分隔线
    auto *vSep2 = new QFrame();
    vSep2->setFrameShape(QFrame::VLine);
    vSep2->setStyleSheet("QFrame { color: " + BORDER_COLOR + "; }");
    statsLayout->addWidget(vSep2);

    // 难度分布
    auto *diffWidget = new QVBoxLayout();
    auto *diffLabel = new QLabel("难度分布");
    diffLabel->setStyleSheet("QLabel { font-size: 12px; color: " + TEXT_SECONDARY + "; }");
    diffLabel->setAlignment(Qt::AlignCenter);
    diffWidget->addWidget(diffLabel);
    auto *diffRow = new QHBoxLayout();
    diffRow->setSpacing(8);
    for (const QString &diff : {"easy", "medium", "hard"}) {
        int count = m_currentResult.difficultyCount.value(diff, 0);
        if (count > 0) {
            auto *capsule = new QLabel(QString("%1: %2").arg(difficultyDisplayName(diff)).arg(count));
            capsule->setStyleSheet(DIFFICULTY_CAPSULE_STYLE.arg(difficultyColor(diff).name()));
            diffRow->addWidget(capsule);
        }
    }
    diffWidget->addLayout(diffRow);
    statsLayout->addLayout(diffWidget);

    // 分隔线
    auto *vSep3 = new QFrame();
    vSep3->setFrameShape(QFrame::VLine);
    vSep3->setStyleSheet("QFrame { color: " + BORDER_COLOR + "; }");
    statsLayout->addWidget(vSep3);

    // 章节覆盖
    auto *chapterWidget = new QVBoxLayout();
    auto *chapterValue = new QLabel(QString::number(m_currentResult.coveredChapters.size()));
    chapterValue->setStyleSheet("QLabel { font-size: 28px; font-weight: 700; color: #1976D2; }");
    chapterValue->setAlignment(Qt::AlignCenter);
    auto *chapterLabel = new QLabel("覆盖章节");
    chapterLabel->setStyleSheet("QLabel { font-size: 12px; color: " + TEXT_SECONDARY + "; }");
    chapterLabel->setAlignment(Qt::AlignCenter);
    chapterWidget->addWidget(chapterValue);
    chapterWidget->addWidget(chapterLabel);
    statsLayout->addLayout(chapterWidget);

    statsLayout->addStretch();
    m_resultLayout->addWidget(statsRow);

    // ==================== 警告提示区 ====================
    if (!m_currentResult.warnings.isEmpty()) {
        auto *warningCard = new QFrame();
        warningCard->setStyleSheet(
            "QFrame { background-color: #FFF8E1; border: 1px solid #FFE082; border-radius: 12px; }");
        auto *warningLayout = new QVBoxLayout(warningCard);
        warningLayout->setContentsMargins(16, 12, 16, 12);

        auto *warningHeader = new QLabel("注意");
        warningHeader->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: #F57F17; }");
        warningLayout->addWidget(warningHeader);

        for (const auto &w : m_currentResult.warnings) {
            auto *wLabel = new QLabel("- " + w);
            wLabel->setStyleSheet("QLabel { font-size: 13px; color: #795548; }");
            wLabel->setWordWrap(true);
            warningLayout->addWidget(wLabel);
        }
        m_resultLayout->addWidget(warningCard);
    }

    // ==================== 题目列表（按题型分组） ====================
    // 按题型分组
    QMap<QString, QList<QPair<int, PaperQuestion>>> groupedQuestions;
    for (int i = 0; i < m_currentResult.selectedQuestions.size(); ++i) {
        const auto &q = m_currentResult.selectedQuestions[i];
        groupedQuestions[q.questionType].append({i, q});
    }

    // 题型中文序号
    QStringList chineseNumbers = {"一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
    int groupIndex = 0;

    for (auto it = groupedQuestions.constBegin(); it != groupedQuestions.constEnd(); ++it, ++groupIndex) {
        const QString &type = it.key();
        const auto &questions = it.value();

        // 查找对应的题型配置
        int scorePerQ = 0;
        for (const auto &spec : m_config.typeSpecs) {
            if (spec.questionType == type) {
                scorePerQ = spec.scorePerQuestion;
                break;
            }
        }

        // 组标题
        QString groupNum = groupIndex < chineseNumbers.size() ? chineseNumbers[groupIndex] : QString::number(groupIndex + 1);
        auto *groupTitle = new QLabel(
            QString("%1、%2 (%3题 × %4分 = %5分)")
                .arg(groupNum)
                .arg(questionTypeDisplayName(type))
                .arg(questions.size())
                .arg(scorePerQ)
                .arg(questions.size() * scorePerQ)
        );
        groupTitle->setStyleSheet(
            "QLabel { font-size: 16px; font-weight: 600; color: " + TEXT_PRIMARY + "; "
            "padding: 8px 0; }");
        m_resultLayout->addWidget(groupTitle);

        // 每道题
        int questionNum = 1;
        for (const auto &pair : questions) {
            const PaperQuestion &q = pair.second;

            auto *questionRow = new QFrame();
            questionRow->setStyleSheet(
                "QFrame { background-color: " + CARD_WHITE + "; border: 1px solid " + BORDER_COLOR + "; "
                "border-radius: 10px; }"
                "QFrame:hover { background-color: #F9FAFB; border-color: #D1D5DB; }");

            // 轻微投影
            auto *qShadow = new QGraphicsDropShadowEffect(questionRow);
            qShadow->setBlurRadius(8);
            qShadow->setOffset(0, 1);
            qShadow->setColor(QColor(0, 0, 0, 10));
            questionRow->setGraphicsEffect(qShadow);

            auto *qLayout = new QHBoxLayout(questionRow);
            qLayout->setContentsMargins(16, 10, 16, 10);
            qLayout->setSpacing(12);

            // 序号
            auto *numLabel = new QLabel(QString::number(questionNum++));
            numLabel->setFixedWidth(30);
            numLabel->setAlignment(Qt::AlignCenter);
            numLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + TEXT_SECONDARY + "; }");
            qLayout->addWidget(numLabel);

            // 难度胶囊
            auto *diffCapsule = new QLabel(difficultyDisplayName(q.difficulty));
            diffCapsule->setStyleSheet(DIFFICULTY_CAPSULE_STYLE.arg(difficultyColor(q.difficulty).name()));
            diffCapsule->setFixedWidth(50);
            diffCapsule->setAlignment(Qt::AlignCenter);
            qLayout->addWidget(diffCapsule);

            // 题干预览
            QString stemPreview = q.stem;
            if (stemPreview.length() > 80) {
                stemPreview = stemPreview.left(80) + "...";
            }
            auto *stemLabel = new QLabel(stemPreview);
            stemLabel->setStyleSheet("QLabel { font-size: 13px; color: " + TEXT_PRIMARY + "; }");
            stemLabel->setWordWrap(true);
            qLayout->addWidget(stemLabel, 1);

            // 换一题按钮
            auto *swapBtn = new QPushButton("换一题");
            swapBtn->setStyleSheet(SWAP_BTN_STYLE);
            swapBtn->setCursor(Qt::PointingHandCursor);
            // 检查候选池是否还有题
            bool hasCandidate = !m_currentResult.candidatePool.value(type).isEmpty();
            swapBtn->setEnabled(hasCandidate);
            if (!hasCandidate) {
                swapBtn->setToolTip("候选池已空，无法换题");
            }
            connect(swapBtn, &QPushButton::clicked, this, [this, qId = q.id, qType = type]() {
                onSwapQuestion(qId, qType);
            });
            qLayout->addWidget(swapBtn);

            m_resultLayout->addWidget(questionRow);
        }
    }
}

void SmartPaperWidget::onSwapQuestion(const QString &questionId, const QString &questionType)
{
    if (m_smartPaperService->swapQuestion(questionId, questionType)) {
        m_currentResult = m_smartPaperService->currentResult();
        buildResultPreview();
    } else {
        QMessageBox::information(this, "提示", "候选池已空，无法换题");
    }
}

void SmartPaperWidget::onRegenerate()
{
    setState(AssemblyState::Idle);
    clearResultPreview();
}

void SmartPaperWidget::onSaveToCloud()
{
    // 构建 Paper 对象
    Paper paper;
    paper.title = m_config.title;
    paper.subject = m_config.subject;
    paper.grade = m_config.grade;
    paper.totalScore = m_currentResult.totalScore;
    paper.duration = m_config.duration;
    paper.paperType = "智能组卷";

    // 保存待发送的题目列表
    m_pendingSaveQuestions = m_currentResult.selectedQuestions;

    // 创建试卷
    m_paperService->createPaper(paper);
}

void SmartPaperWidget::onPaperCreated(const Paper &paper)
{
    if (m_pendingSaveQuestions.isEmpty()) return;

    // 设置每题的 paperId 和 orderNum
    for (int i = 0; i < m_pendingSaveQuestions.size(); ++i) {
        m_pendingSaveQuestions[i].paperId = paper.id;
        m_pendingSaveQuestions[i].orderNum = i + 1;
    }

    // 批量添加题目
    m_paperService->addQuestions(m_pendingSaveQuestions);
    m_pendingSaveQuestions.clear();

    QMessageBox::information(this, "保存成功",
        QString("试卷「%1」已保存到云端（共 %2 题）")
            .arg(paper.title)
            .arg(m_currentResult.selectedQuestions.size()));
}

void SmartPaperWidget::onImportToBasket()
{
    QuestionBasket *basket = QuestionBasket::instance();
    basket->clear();

    for (const auto &q : m_currentResult.selectedQuestions) {
        basket->addQuestion(q);
        basket->setQuestionScore(q.id, q.score);
    }

    QMessageBox::information(this, "导入成功",
        QString("已将 %1 道题导入试题篮").arg(m_currentResult.selectedQuestions.size()));
}

void SmartPaperWidget::onEditExport()
{
    // 先导入试题篮
    onImportToBasket();

    // 打开 PaperComposerDialog
    PaperComposerDialog dialog(this);
    dialog.exec();
}

QString SmartPaperWidget::questionTypeDisplayName(const QString &type) const
{
    static const QMap<QString, QString> names = {
        {"single_choice", "选择题"},
        {"multi_choice", "多选题"},
        {"true_false", "判断题"},
        {"short_answer", "简答题"},
        {"essay", "论述题"},
        {"material_analysis", "材料分析题"},
    };
    return names.value(type, type);
}

QString SmartPaperWidget::difficultyDisplayName(const QString &diff) const
{
    static const QMap<QString, QString> names = {
        {"easy", "简单"},
        {"medium", "中等"},
        {"hard", "困难"},
    };
    return names.value(diff.toLower(), diff);
}

QColor SmartPaperWidget::difficultyColor(const QString &diff) const
{
    QString d = diff.toLower();
    if (d == "easy") return QColor("#4CAF50");
    if (d == "hard") return QColor("#F44336");
    return QColor("#FF9800");  // medium 或其他
}
