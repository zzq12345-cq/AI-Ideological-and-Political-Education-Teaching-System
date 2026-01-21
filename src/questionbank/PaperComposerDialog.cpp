#include "PaperComposerDialog.h"
#include "QuestionBasket.h"
#include "../services/DocxGenerator.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollArea>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTextStream>
#include <QUrl>
#include <QSvgRenderer>
#include <QPainter>

namespace {

// ==================== 核心样式常量 ====================
const QString kPrimaryColor = "#B71C1C";        // 党建红
const QString kPrimaryLight = "#FFEBEE";        // 浅红背景
const QString kAccentColor = "#FF8F00";         // 聚焦态 - 琥珀橙
const QString kAccentLight = "#FFF3E0";         // 聚焦背景
const QString kErrorColor = "#D32F2F";          // 错误红
const QString kErrorLight = "#FFCDD2";          // 错误背景
const QString kTextPrimary = "#212121";
const QString kTextSecondary = "#757575";
const QString kBorderColor = "#E0E0E0";
const QString kBackgroundLight = "#FAFAFA";

void applyShadow(QWidget *target, qreal blurRadius, const QPointF &offset, const QColor &color)
{
    if (!target) return;
    auto *shadow = new QGraphicsDropShadowEffect(target);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(offset);
    shadow->setColor(color);
    target->setGraphicsEffect(shadow);
}

// ==================== 完整的 QSS 样式表 ====================
QString getDialogStyleSheet()
{
    return R"(
/* ========== 对话框整体 ========== */
QDialog {
    background: #FAFAFA;
    font-family: "Microsoft YaHei", "PingFang SC", sans-serif;
}

/* ========== QLineEdit 输入框 ========== */
QLineEdit {
    background: #FFFFFF;
    border: 1.5px solid #E0E0E0;
    border-radius: 8px;
    padding: 0 16px;
    font-size: 14px;
    color: #212121;
    selection-background-color: #FFCC80;
}

QLineEdit:hover {
    border-color: #BDBDBD;
}

/* 聚焦态 - 使用琥珀橙，避免红色焦虑感 */
QLineEdit:focus {
    border: 2px solid #FF8F00;
    background: #FFFBF5;
    padding: 0 15px; /* 补偿边框增粗 */
}

/* 错误态 - 通过自定义属性触发 */
QLineEdit[isError="true"] {
    border: 2px solid #D32F2F;
    background: #FFF5F5;
    padding: 0 15px;
}

QLineEdit::placeholder {
    color: #9E9E9E;
}

/* ========== QTableWidget 表格 ========== */
QTableWidget {
    background: #FFFFFF;
    border: 1px solid #E0E0E0;
    border-radius: 12px;
    gridline-color: #F5F5F5;
    outline: 0; /* 移除虚线框 */
}

QTableWidget::item {
    padding: 12px 8px;
    border-bottom: 1px solid #F5F5F5;
    outline: 0;
}

QTableWidget::item:selected {
    background: #FFF3E0;
    color: #212121;
}

QTableWidget::item:hover {
    background: #FAFAFA;
}

/* 表头样式 - 增加高度和视觉层次 */
QHeaderView::section {
    background: linear-gradient(to bottom, #FAFAFA, #F5F5F5);
    color: #424242;
    font-size: 13px;
    font-weight: 600;
    padding: 14px 8px;
    border: none;
    border-bottom: 2px solid #E0E0E0;
    border-right: 1px solid #EEEEEE;
}

QHeaderView::section:last {
    border-right: none;
}

/* 滚动条美化 */
QTableWidget QScrollBar:vertical {
    width: 8px;
    background: transparent;
    margin: 4px 0;
}

QTableWidget QScrollBar::handle:vertical {
    background: #BDBDBD;
    border-radius: 4px;
    min-height: 30px;
}

QTableWidget QScrollBar::handle:vertical:hover {
    background: #9E9E9E;
}

QTableWidget QScrollBar::add-line:vertical,
QTableWidget QScrollBar::sub-line:vertical {
    height: 0;
}

/* ========== QSpinBox 数字输入 - 表格内扁平化 ========== */
QSpinBox {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 6px;
    padding: 4px 8px;
    font-size: 14px;
    font-weight: 600;
    color: #B71C1C;
    min-width: 50px;
}

QSpinBox:hover {
    background: #FFF3E0;
    border-color: #FFE0B2;
}

QSpinBox:focus {
    background: #FFFFFF;
    border: 1.5px solid #FF8F00;
}

/* 隐藏原生箭头按钮 */
QSpinBox::up-button,
QSpinBox::down-button {
    width: 0;
    border: none;
}

/* ========== QPushButton 按钮系统 ========== */
/* 主要操作按钮 */
QPushButton#primaryButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #C62828, stop:1 #B71C1C);
    color: #FFFFFF;
    border: none;
    border-radius: 8px;
    font-size: 14px;
    font-weight: 600;
    padding: 0 28px;
}

QPushButton#primaryButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #D32F2F, stop:1 #C62828);
}

QPushButton#primaryButton:pressed {
    background: #9A0007;
}

QPushButton#primaryButton:disabled {
    background: #BDBDBD;
    color: #757575;
}

/* 次要操作按钮 */
QPushButton#secondaryButton {
    background: #FFFFFF;
    color: #424242;
    border: 1.5px solid #E0E0E0;
    border-radius: 8px;
    font-size: 13px;
    font-weight: 500;
    padding: 0 20px;
}

QPushButton#secondaryButton:hover {
    background: #F5F5F5;
    border-color: #BDBDBD;
}

QPushButton#secondaryButton:pressed {
    background: #EEEEEE;
}

/* 行内操作按钮（表格内） */
QPushButton#inlineActionButton {
    background: transparent;
    border: none;
    border-radius: 4px;
    padding: 4px 8px;
    font-size: 12px;
    color: #757575;
}

QPushButton#inlineActionButton:hover {
    background: #FFEBEE;
    color: #B71C1C;
}

QPushButton#inlineDeleteButton {
    background: transparent;
    border: none;
    border-radius: 4px;
    padding: 4px;
    font-size: 16px;
    font-weight: bold;
    color: #BDBDBD;
}

QPushButton#inlineDeleteButton:hover {
    background: #FFEBEE;
    color: #D32F2F;
}

/* ========== 徽章样式 ========== */
QLabel#badgeLabel {
    background: #FFEBEE;
    color: #B71C1C;
    font-size: 12px;
    font-weight: 600;
    padding: 4px 12px;
    border-radius: 12px;
}

QLabel#scoreBadge {
    background: #B71C1C;
    color: #FFFFFF;
    font-size: 13px;
    font-weight: 700;
    padding: 6px 14px;
    border-radius: 14px;
}
)";
}

}  // namespace

PaperComposerDialog::PaperComposerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("组卷预览");
    setMinimumSize(860, 640);
    resize(920, 720);

    // 应用全局样式
    setStyleSheet(getDialogStyleSheet());

    setupUI();
    populateTable();
    updateSummary();
}

void PaperComposerDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(0);

    // ==================== 顶部标题栏（整合标题 + 统计徽章） ====================
    auto *headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");
    headerFrame->setStyleSheet(
        "QFrame#headerFrame {"
        "  background: #FFFFFF;"
        "  border-radius: 12px;"
        "  border: 1px solid #EEEEEE;"
        "}"
    );
    applyShadow(headerFrame, 8, QPointF(0, 2), QColor(0, 0, 0, 15));

    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 16, 20, 16);
    headerLayout->setSpacing(16);

    // 左侧：标题输入
    auto *titleGroup = new QWidget(headerFrame);
    auto *titleGroupLayout = new QHBoxLayout(titleGroup);
    titleGroupLayout->setContentsMargins(0, 0, 0, 0);
    titleGroupLayout->setSpacing(12);

    auto *titleIcon = new QLabel(titleGroup);
    QSvgRenderer clipboardRenderer(QString(":/icons/resources/icons/clipboard.svg"));
    if (clipboardRenderer.isValid()) {
        QPixmap clipboardPixmap(20, 20);
        clipboardPixmap.fill(Qt::transparent);
        QPainter clipboardPainter(&clipboardPixmap);
        clipboardRenderer.render(&clipboardPainter);
        titleIcon->setPixmap(clipboardPixmap);
    }
    titleIcon->setStyleSheet("QLabel { background: transparent; }");

    m_titleEdit = new QLineEdit(titleGroup);
    m_titleEdit->setPlaceholderText("请输入试卷标题...");
    m_titleEdit->setText("思政课程单元测试");
    m_titleEdit->setFixedHeight(44);
    m_titleEdit->setMinimumWidth(320);

    titleGroupLayout->addWidget(titleIcon);
    titleGroupLayout->addWidget(m_titleEdit);

    // 右侧：统计徽章（整合到标题栏）
    m_questionCountLabel = new QLabel("0 题", headerFrame);
    m_questionCountLabel->setObjectName("badgeLabel");

    m_totalScoreLabel = new QLabel("0 分", headerFrame);
    m_totalScoreLabel->setObjectName("scoreBadge");

    headerLayout->addWidget(titleGroup);
    headerLayout->addStretch();
    headerLayout->addWidget(m_questionCountLabel);
    headerLayout->addWidget(m_totalScoreLabel);

    mainLayout->addWidget(headerFrame);
    mainLayout->addSpacing(16);

    // ==================== 试题表格（行内操作，去掉外部按钮） ====================
    m_questionTable = new QTableWidget(this);
    m_questionTable->setColumnCount(5);
    m_questionTable->setHorizontalHeaderLabels({"序号", "题型", "题目内容", "分值", ""});
    m_questionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_questionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_questionTable->setAlternatingRowColors(false);
    m_questionTable->setShowGrid(false);
    m_questionTable->verticalHeader()->setVisible(false);
    m_questionTable->horizontalHeader()->setHighlightSections(false);
    m_questionTable->setFocusPolicy(Qt::NoFocus);

    // 列宽设置
    m_questionTable->setColumnWidth(0, 60);
    m_questionTable->setColumnWidth(1, 90);
    m_questionTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_questionTable->setColumnWidth(3, 80);
    m_questionTable->setColumnWidth(4, 100);  // 行内操作列

    // 行高
    m_questionTable->verticalHeader()->setDefaultSectionSize(56);

    mainLayout->addWidget(m_questionTable, 1);
    mainLayout->addSpacing(20);

    // ==================== 底部操作栏 ====================
    auto *footerFrame = new QFrame(this);
    auto *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(12);

    // 左侧提示
    auto *tipLabel = new QLabel(" 点击表格内按钮可调整顺序或移除试题", footerFrame);
    tipLabel->setStyleSheet("QLabel { color: #9E9E9E; font-size: 12px; }");

    // 右侧按钮
    auto *cancelButton = new QPushButton("取消", footerFrame);
    cancelButton->setObjectName("secondaryButton");
    cancelButton->setFixedSize(100, 44);
    cancelButton->setCursor(Qt::PointingHandCursor);

    m_exportButton = new QPushButton("导出试卷", footerFrame);
    m_exportButton->setObjectName("primaryButton");
    m_exportButton->setFixedSize(140, 44);
    m_exportButton->setCursor(Qt::PointingHandCursor);
    applyShadow(m_exportButton, 12, QPointF(0, 4), QColor(183, 28, 28, 80));

    footerLayout->addWidget(tipLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(cancelButton);
    footerLayout->addWidget(m_exportButton);

    mainLayout->addWidget(footerFrame);

    // 信号连接
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_exportButton, &QPushButton::clicked, this, &PaperComposerDialog::onExportPaper);

    // 标题输入变化时清除错误状态
    connect(m_titleEdit, &QLineEdit::textChanged, this, [this]() {
        m_titleEdit->setProperty("isError", false);
        m_titleEdit->style()->unpolish(m_titleEdit);
        m_titleEdit->style()->polish(m_titleEdit);
    });
}

void PaperComposerDialog::populateTable()
{
    m_questionTable->setRowCount(0);

    const QList<PaperQuestion> &questions = QuestionBasket::instance()->questions();

    static const QMap<QString, QString> typeNames = {
        {"single_choice", "选择题"},
        {"fill_blank", "填空题"},
        {"true_false", "判断题"},
        {"material_essay", "材料题"},
        {"short_answer", "简答题"},
        {"essay", "论述题"}
    };

    for (int i = 0; i < questions.size(); ++i) {
        const PaperQuestion &q = questions[i];
        m_questionTable->insertRow(i);

        // 序号 - 居中显示
        auto *indexItem = new QTableWidgetItem(QString::number(i + 1));
        indexItem->setTextAlignment(Qt::AlignCenter);
        indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable);
        indexItem->setForeground(QColor("#757575"));
        m_questionTable->setItem(i, 0, indexItem);

        // 题型 - 带颜色标签效果
        QString typeName = typeNames.value(q.questionType, q.questionType);
        auto *typeItem = new QTableWidgetItem(typeName);
        typeItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        typeItem->setForeground(QColor("#B71C1C"));
        typeItem->setFont(QFont("Microsoft YaHei", 10, QFont::Medium));
        m_questionTable->setItem(i, 1, typeItem);

        // 题目预览
        QString preview = q.stem.left(45);
        preview.remove(QRegularExpression("<[^>]*>"));
        if (q.stem.length() > 45) {
            preview += "...";
        }
        auto *stemItem = new QTableWidgetItem(preview);
        stemItem->setFlags(stemItem->flags() & ~Qt::ItemIsEditable);
        stemItem->setData(Qt::UserRole, q.id);
        stemItem->setForeground(QColor("#424242"));
        m_questionTable->setItem(i, 2, stemItem);

        // 分值 - 使用 QSpinBox
        auto *scoreWidget = new QSpinBox(m_questionTable);
        scoreWidget->setRange(1, 50);
        scoreWidget->setValue(QuestionBasket::instance()->questionScore(q.id));
        scoreWidget->setAlignment(Qt::AlignCenter);
        scoreWidget->setButtonSymbols(QAbstractSpinBox::NoButtons);
        int row = i;
        connect(scoreWidget, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this, row](int value) {
                    onScoreChanged(row, value);
                });
        m_questionTable->setCellWidget(i, 3, scoreWidget);

        // 行内操作按钮组
        auto *actionWidget = new QWidget(m_questionTable);
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 0, 4, 0);
        actionLayout->setSpacing(2);

        // 上移按钮
        auto *upBtn = new QPushButton("↑", actionWidget);
        upBtn->setObjectName("inlineActionButton");
        upBtn->setFixedSize(28, 28);
        upBtn->setCursor(Qt::PointingHandCursor);
        upBtn->setToolTip("上移");
        int currentRow = i;
        connect(upBtn, &QPushButton::clicked, this, [this, currentRow]() {
            if (currentRow > 0) {
                QuestionBasket::instance()->moveQuestion(currentRow, currentRow - 1);
                populateTable();
                updateSummary();
            }
        });

        // 下移按钮
        auto *downBtn = new QPushButton("↓", actionWidget);
        downBtn->setObjectName("inlineActionButton");
        downBtn->setFixedSize(28, 28);
        downBtn->setCursor(Qt::PointingHandCursor);
        downBtn->setToolTip("下移");
        connect(downBtn, &QPushButton::clicked, this, [this, currentRow]() {
            if (currentRow < m_questionTable->rowCount() - 1) {
                QuestionBasket::instance()->moveQuestion(currentRow, currentRow + 1);
                populateTable();
                updateSummary();
            }
        });

        // 删除按钮
        auto *delBtn = new QPushButton("×", actionWidget);
        delBtn->setObjectName("inlineDeleteButton");
        delBtn->setFixedSize(28, 28);
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setToolTip("移除");
        QString questionId = q.id;
        connect(delBtn, &QPushButton::clicked, this, [this, questionId]() {
            QuestionBasket::instance()->removeQuestion(questionId);
            populateTable();
            updateSummary();
        });

        actionLayout->addWidget(upBtn);
        actionLayout->addWidget(downBtn);
        actionLayout->addWidget(delBtn);

        m_questionTable->setCellWidget(i, 4, actionWidget);
    }
}

void PaperComposerDialog::updateSummary()
{
    int count = QuestionBasket::instance()->count();
    int totalScore = QuestionBasket::instance()->totalScore();

    m_questionCountLabel->setText(QString("%1 题").arg(count));
    m_totalScoreLabel->setText(QString("%1 分").arg(totalScore));

    m_exportButton->setEnabled(count > 0);
}

void PaperComposerDialog::onRefreshList()
{
    populateTable();
    updateSummary();
}

void PaperComposerDialog::onMoveUp()
{
    // 已移至行内按钮，此方法保留兼容性
}

void PaperComposerDialog::onMoveDown()
{
    // 已移至行内按钮，此方法保留兼容性
}

void PaperComposerDialog::onRemoveSelected()
{
    // 已移至行内按钮，此方法保留兼容性
}

void PaperComposerDialog::onScoreChanged(int row, int score)
{
    if (row < 0 || row >= m_questionTable->rowCount()) return;

    auto *stemItem = m_questionTable->item(row, 2);
    if (!stemItem) return;

    QString questionId = stemItem->data(Qt::UserRole).toString();
    QuestionBasket::instance()->setQuestionScore(questionId, score);
    updateSummary();
}

void PaperComposerDialog::onExportPaper()
{
    // ========== 错误校验逻辑 ==========
    QString title = m_titleEdit->text().trimmed();

    if (title.isEmpty()) {
        // 触发错误态样式
        m_titleEdit->setProperty("isError", true);
        m_titleEdit->style()->unpolish(m_titleEdit);
        m_titleEdit->style()->polish(m_titleEdit);
        m_titleEdit->setFocus();

        // 可选：显示提示
        QMessageBox::warning(this, "提示", "请输入试卷标题");
        return;
    }

    // 清除错误态
    m_titleEdit->setProperty("isError", false);
    m_titleEdit->style()->unpolish(m_titleEdit);
    m_titleEdit->style()->polish(m_titleEdit);

    // 默认保存到桌面
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString defaultFileName = QString("%1/%2.docx").arg(desktopPath).arg(title);

    // 导出文件
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出试卷",
        defaultFileName,
        "Word 文档 (*.docx);;所有文件 (*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    // 确保文件名以 .docx 结尾
    if (!fileName.endsWith(".docx", Qt::CaseInsensitive)) {
        fileName += ".docx";
    }

    // 获取试题列表
    const QList<PaperQuestion> &questions = QuestionBasket::instance()->questions();

    if (questions.isEmpty()) {
        QMessageBox::warning(this, "导出失败", "试题篮为空，请先添加题目");
        return;
    }

    // 使用 DocxGenerator 生成 Word 文档
    DocxGenerator generator;
    bool success = generator.generatePaper(fileName, title, questions);

    if (!success) {
        QMessageBox::warning(this, "导出失败", generator.lastError());
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "导出成功",
        QString("试卷已导出到:\n%1\n\n是否立即打开?").arg(fileName),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }

    emit paperExported(fileName);
    accept();
}

QString PaperComposerDialog::generatePaperHtml()
{
    QString title = m_titleEdit->text();
    const QList<PaperQuestion> &questions = QuestionBasket::instance()->questions();

    static const QMap<QString, QString> typeNames = {
        {"single_choice", "选择题"},
        {"fill_blank", "填空题"},
        {"true_false", "判断题"},
        {"material_essay", "材料论述题"},
        {"short_answer", "简答题"},
        {"essay", "论述题"}
    };

    QString html = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" + title + R"(</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: "Microsoft YaHei", "PingFang SC", sans-serif;
            font-size: 14px;
            line-height: 1.8;
            color: #212121;
            padding: 48px;
            max-width: 800px;
            margin: 0 auto;
            background: #fff;
        }
        h1 {
            text-align: center;
            font-size: 26px;
            margin-bottom: 12px;
            color: #B71C1C;
            font-weight: 700;
        }
        .summary {
            text-align: center;
            color: #757575;
            margin-bottom: 36px;
            padding-bottom: 24px;
            border-bottom: 2px solid #FFEBEE;
            font-size: 14px;
        }
        .summary span {
            display: inline-block;
            background: #FFEBEE;
            color: #B71C1C;
            padding: 4px 16px;
            border-radius: 16px;
            margin: 0 8px;
            font-weight: 600;
        }
        .section-title {
            font-size: 16px;
            font-weight: 700;
            color: #B71C1C;
            margin: 28px 0 16px;
            padding: 12px 16px;
            background: linear-gradient(90deg, #FFEBEE, transparent);
            border-left: 4px solid #B71C1C;
            border-radius: 0 8px 8px 0;
        }
        .question {
            margin-bottom: 24px;
            padding: 20px;
            background: #FAFAFA;
            border-radius: 12px;
            border: 1px solid #F5F5F5;
        }
        .question-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 12px;
        }
        .question-number {
            font-weight: 700;
            color: #B71C1C;
            font-size: 15px;
        }
        .question-score {
            font-size: 12px;
            color: #fff;
            background: #B71C1C;
            padding: 4px 12px;
            border-radius: 12px;
            font-weight: 600;
        }
        .question-stem {
            margin-bottom: 12px;
            line-height: 1.8;
        }
        .options {
            padding-left: 8px;
        }
        .option {
            margin-bottom: 8px;
            padding: 10px 16px;
            background: #fff;
            border-radius: 8px;
            border: 1px solid #EEEEEE;
        }
        .material {
            background: #fff;
            padding: 16px;
            border-left: 4px solid #B71C1C;
            margin-bottom: 16px;
            border-radius: 0 8px 8px 0;
            font-style: italic;
            color: #424242;
        }
        .sub-questions {
            padding-left: 16px;
        }
        .sub-question {
            margin-bottom: 12px;
            padding-left: 8px;
            border-left: 2px solid #FFCDD2;
        }
        @media print {
            body { padding: 24px; }
            .question { page-break-inside: avoid; }
        }
    </style>
</head>
<body>
    <h1>)" + title + R"(</h1>
    <div class="summary">
        <span>共 )" + QString::number(questions.size()) + R"( 道题</span>
        <span>总分 )" + QString::number(QuestionBasket::instance()->totalScore()) + R"( 分</span>
    </div>
)";

    // 按题型分组
    QMap<QString, QList<PaperQuestion>> grouped;
    for (const PaperQuestion &q : questions) {
        grouped[q.questionType].append(q);
    }

    const QStringList typeOrder = {"single_choice", "true_false", "fill_blank", "short_answer", "material_essay"};
    int globalIndex = 1;

    for (const QString &type : typeOrder) {
        if (!grouped.contains(type) || grouped[type].isEmpty()) {
            continue;
        }

        QString typeName = typeNames.value(type, type);
        html += QString("<div class=\"section-title\">%1</div>\n").arg(typeName);

        for (const PaperQuestion &q : grouped[type]) {
            int score = QuestionBasket::instance()->questionScore(q.id);
            QString stem = q.stem;

            html += "<div class=\"question\">\n";
            html += QString("<div class=\"question-header\">"
                           "<span class=\"question-number\">第 %1 题</span>"
                           "<span class=\"question-score\">%2 分</span>"
                           "</div>\n")
                    .arg(globalIndex).arg(score);

            if (type == "material_essay" && !q.material.isEmpty()) {
                html += QString("<div class=\"material\">%1</div>\n").arg(q.material);
            }

            html += QString("<div class=\"question-stem\">%1</div>\n").arg(stem);

            if (!q.options.isEmpty()) {
                html += "<div class=\"options\">\n";
                QStringList keys = {"A", "B", "C", "D", "E", "F"};
                for (int i = 0; i < q.options.size() && i < keys.size(); ++i) {
                    html += QString("<div class=\"option\">%1. %2</div>\n")
                            .arg(keys[i]).arg(q.options[i]);
                }
                html += "</div>\n";
            }

            if (!q.subQuestions.isEmpty()) {
                html += "<div class=\"sub-questions\">\n";
                for (int i = 0; i < q.subQuestions.size(); ++i) {
                    html += QString("<div class=\"sub-question\">(%1) %2</div>\n")
                            .arg(i + 1).arg(q.subQuestions[i]);
                }
                html += "</div>\n";
            }

            html += "</div>\n";
            globalIndex++;
        }

        grouped.remove(type);
    }

    for (auto it = grouped.begin(); it != grouped.end(); ++it) {
        if (it.value().isEmpty()) continue;

        QString typeName = typeNames.value(it.key(), it.key());
        html += QString("<div class=\"section-title\">%1</div>\n").arg(typeName);

        for (const PaperQuestion &q : it.value()) {
            int score = QuestionBasket::instance()->questionScore(q.id);
            html += "<div class=\"question\">\n";
            html += QString("<div class=\"question-header\">"
                           "<span class=\"question-number\">第 %1 题</span>"
                           "<span class=\"question-score\">%2 分</span>"
                           "</div>\n")
                    .arg(globalIndex).arg(score);
            html += QString("<div class=\"question-stem\">%1</div>\n").arg(q.stem);
            html += "</div>\n";
            globalIndex++;
        }
    }

    html += R"(
</body>
</html>
)";

    return html;
}
