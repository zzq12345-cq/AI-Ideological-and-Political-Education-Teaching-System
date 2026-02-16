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
const QString kPrimaryColor = "#D32F2F";          // 主色红
const QString kPrimaryLight = "#FFEBEE";          // 浅红背景
const QString kAccentColor = "#FF8F00";           // 聚焦态 - 琥珀橙
const QString kAccentLight = "#FFF3E0";           // 聚焦背景
const QString kErrorColor = "#D32F2F";            // 错误红
const QString kErrorLight = "#FFCDD2";            // 错误背景
const QString kTextPrimary = "#1A1A2E";           // 深邃文字
const QString kTextSecondary = "#6B7280";         // 次要文字
const QString kBorderColor = "#E0E0E0";
const QString kBackgroundLight = "#F8F9FC";

void applyShadow(QWidget *target, qreal blurRadius, const QPointF &offset, const QColor &color)
{
    if (!target) return;
    auto *shadow = new QGraphicsDropShadowEffect(target);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(offset);
    shadow->setColor(color);
    target->setGraphicsEffect(shadow);
}

// ==================== 现代风格样式表 ====================
QString getDialogStyleSheet()
{
    return R"(
/* ========== 对话框整体 - 干净简洁背景 ========== */
QDialog {
    background: #F5F5F5;
    font-family: "PingFang SC", "Noto Sans SC", "Microsoft YaHei", "Source Han Sans SC", "Helvetica Neue", Arial, sans-serif;
}

/* ========== 输入框 - 现代扁平风格 ========== */
QLineEdit {
    background: rgba(255,255,255,0.95);
    border: 2px solid rgba(0,0,0,0.06);
    border-radius: 14px;
    padding: 0 20px;
    font-size: 15px;
    font-weight: 500;
    color: #1A1A2E;
    selection-background-color: #FFCC80;
}

QLineEdit:hover {
    background: rgba(255,255,255,1);
    border-color: rgba(211,47,47,0.15);
}

QLineEdit:focus {
    background: #FFFFFF;
    border: 2px solid #D32F2F;
}

QLineEdit[isError="true"] {
    border: 2px solid #EF4444;
    background: #FEF2F2;
}

QLineEdit::placeholder {
    color: #9CA3AF;
    font-weight: 400;
}

/* ========== 表格 - 现代卡片风格 ========== */
QTableWidget {
    background: #FFFFFF;
    border: 1px solid #E0E0E0;
    border-radius: 12px;
    outline: none;
    gridline-color: transparent;
    selection-background-color: transparent;
}

QTableWidget:focus {
    border: 1px solid #E0E0E0;
    outline: none;
}

QTableWidget::item {
    padding: 14px 10px;
    border: none;
    border-bottom: 1px solid #F0F0F0;
    background: transparent;
    outline: none;
}

QTableWidget::item:selected {
    background: #FFF8F8;
    color: #1A1A2E;
    border: none;
    outline: none;
}

QTableWidget::item:focus {
    background: #FFF8F8;
    border: none;
    outline: none;
}

QTableWidget::item:hover {
    background: #FAFAFA;
}

/* 表头 - 简约透明风格 */
QHeaderView::section {
    background: transparent;
    color: #6B7280;
    font-size: 11px;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.8px;
    padding: 14px 10px;
    border: none;
    border-bottom: 1px solid rgba(0,0,0,0.06);
}

/* 精致滚动条 */
QScrollBar:vertical {
    width: 6px;
    background: transparent;
    margin: 0;
}

QScrollBar::handle:vertical {
    background: #D1D5DB;
    border-radius: 3px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background: #9CA3AF;
}

QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
    height: 0;
    background: transparent;
}

/* ========== 分值输入 - 低调灰色风格 ========== */
QSpinBox {
    background: #F5F7FA;
    border: none;
    border-radius: 6px;
    padding: 6px 10px;
    font-size: 14px;
    font-weight: 500;
    color: #303133;
    min-width: 50px;
    outline: none;
}

QSpinBox:hover {
    background: #EBEEF5;
}

QSpinBox:focus {
    background: #FFFFFF;
    border: 1px solid #E4E7ED;
    outline: none;
}

QSpinBox::up-button,
QSpinBox::down-button {
    width: 0;
    border: none;
}

/* ========== 主要按钮 - 教育蓝色调 ========== */
QPushButton#primaryButton {
    background: #409EFF;
    color: #FFFFFF;
    border: none;
    border-radius: 8px;
    font-size: 14px;
    font-weight: 500;
    padding: 0 24px;
}

QPushButton#primaryButton:hover {
    background: #66B1FF;
}

QPushButton#primaryButton:pressed {
    background: #3A8EE6;
}

QPushButton#primaryButton:disabled {
    background: #E5E7EB;
    color: #9CA3AF;
}

/* 次要按钮 - 文字按钮风格 */
QPushButton#secondaryButton {
    background: transparent;
    color: #606266;
    border: none;
    border-radius: 8px;
    font-size: 13px;
    font-weight: 400;
    padding: 0 20px;
}

QPushButton#secondaryButton:hover {
    background: #F5F7FA;
    color: #409EFF;
}

QPushButton#secondaryButton:pressed {
    background: #EBEEF5;
}

/* 行内操作按钮 - 可见但低调 */
QPushButton#inlineActionButton {
    background: transparent;
    border: none;
    border-radius: 4px;
    padding: 0;
    font-size: 12px;
    font-weight: 400;
    color: #909399;
}

QPushButton#inlineActionButton:hover {
    background: #ECF5FF;
    color: #409EFF;
}

QPushButton#inlineDeleteButton {
    background: transparent;
    border: none;
    border-radius: 4px;
    padding: 0;
    font-size: 12px;
    font-weight: 400;
    color: #909399;
}

QPushButton#inlineDeleteButton:hover {
    background: #FEF0F0;
    color: #F56C6C;
}
)";
}

}  // namespace

PaperComposerDialog::PaperComposerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("组卷预览");
    setMinimumSize(900, 680);
    resize(980, 780);

    // 应用全局样式
    setStyleSheet(getDialogStyleSheet());

    setupUI();
    populateTable();
    updateSummary();
}

void PaperComposerDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(0);

    // ==================== 顶部标题栏（现代毛玻璃风格） ====================
    auto *headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");
    headerFrame->setStyleSheet(
        "QFrame#headerFrame {"
        "  background: #FFFFFF;"
        "  border-radius: 8px;"
        "  border: 1px solid #EBEEF5;"
        "}"
    );

    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 16, 20, 16);
    headerLayout->setSpacing(16);

    // 左侧：标题和统计信息
    auto *titleGroup = new QWidget(headerFrame);
    auto *titleGroupLayout = new QVBoxLayout(titleGroup);
    titleGroupLayout->setContentsMargins(0, 0, 0, 0);
    titleGroupLayout->setSpacing(4);

    // 标题输入行
    auto *titleRow = new QWidget(titleGroup);
    auto *titleRowLayout = new QHBoxLayout(titleRow);
    titleRowLayout->setContentsMargins(0, 0, 0, 0);
    titleRowLayout->setSpacing(12);

    auto *titleIcon = new QLabel(titleRow);
    QSvgRenderer clipboardRenderer(QString(":/icons/resources/icons/clipboard.svg"));
    if (clipboardRenderer.isValid()) {
        QPixmap clipboardPixmap(20, 20);
        clipboardPixmap.fill(Qt::transparent);
        QPainter clipboardPainter(&clipboardPixmap);
        clipboardRenderer.render(&clipboardPainter);
        titleIcon->setPixmap(clipboardPixmap);
    }
    titleIcon->setStyleSheet("QLabel { background: transparent; }");

    m_titleEdit = new QLineEdit(titleRow);
    m_titleEdit->setPlaceholderText("输入试卷标题...");
    m_titleEdit->setText("思政课程单元测试");
    m_titleEdit->setFixedHeight(44);
    m_titleEdit->setMinimumWidth(400);

    titleRowLayout->addWidget(titleIcon);
    titleRowLayout->addWidget(m_titleEdit);
    titleRowLayout->addStretch();

    // 统计信息 - 合并为一行低调灰色文字
    m_summaryLabel = new QLabel("共 0 题，合计 0 分", titleGroup);
    m_summaryLabel->setStyleSheet("QLabel { color: #909399; font-size: 13px; font-weight: 400; padding-left: 32px; }");

    titleGroupLayout->addWidget(titleRow);
    titleGroupLayout->addWidget(m_summaryLabel);

    headerLayout->addWidget(titleGroup);
    headerLayout->addStretch();

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
    m_questionTable->setColumnWidth(4, 76);  // 行内操作列

    // 行高 - 更宽松
    m_questionTable->verticalHeader()->setDefaultSectionSize(64);

    mainLayout->addWidget(m_questionTable, 1);
    mainLayout->addSpacing(24);

    // ==================== 底部操作栏 ====================
    auto *footerFrame = new QFrame(this);
    auto *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(12);

    // 左侧提示 - 更精致的样式
    auto *tipLabel = new QLabel("点击表格内按钮可调整顺序或移除试题", footerFrame);
    tipLabel->setStyleSheet("QLabel { color: #9CA3AF; font-size: 12px; font-weight: 400; }");

    // 右侧按钮
    auto *cancelButton = new QPushButton("取消", footerFrame);
    cancelButton->setObjectName("secondaryButton");
    cancelButton->setFixedSize(80, 40);
    cancelButton->setCursor(Qt::PointingHandCursor);

    m_exportButton = new QPushButton("导出试卷", footerFrame);
    m_exportButton->setObjectName("primaryButton");
    m_exportButton->setFixedSize(120, 40);
    m_exportButton->setCursor(Qt::PointingHandCursor);

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

        // 题型 - 彩色药丸标签
        QString typeName = typeNames.value(q.questionType, q.questionType);

        // 创建题型标签 Widget
        auto *typeWidget = new QWidget(m_questionTable);
        auto *typeLayout = new QHBoxLayout(typeWidget);
        typeLayout->setContentsMargins(0, 0, 0, 0);
        typeLayout->setAlignment(Qt::AlignCenter);

        auto *typeLabel = new QLabel(typeName, typeWidget);
        typeLabel->setAlignment(Qt::AlignCenter);
        typeLabel->setFixedHeight(24);

        // 根据题型设置不同颜色 - 低饱和度胶囊风格
        QString typeStyle;
        if (q.questionType == "single_choice" || q.questionType == "multi_choice") {
            // 选择题 - 淡蓝色
            typeStyle = "background: #ECF5FF; color: #409EFF; border-radius: 12px; padding: 4px 14px; font-size: 11px; font-weight: 500;";
        } else if (q.questionType == "material_essay") {
            // 材料题 - 淡紫色
            typeStyle = "background: #F4F4F5; color: #909399; border-radius: 12px; padding: 4px 14px; font-size: 11px; font-weight: 500;";
        } else if (q.questionType == "short_answer" || q.questionType == "essay") {
            // 简答/论述题 - 淡绿色
            typeStyle = "background: #F0F9EB; color: #67C23A; border-radius: 12px; padding: 4px 14px; font-size: 11px; font-weight: 500;";
        } else if (q.questionType == "fill_blank") {
            // 填空题 - 淡橙色
            typeStyle = "background: #FDF6EC; color: #E6A23C; border-radius: 12px; padding: 4px 14px; font-size: 11px; font-weight: 500;";
        } else {
            // 其他题型 - 中性灰
            typeStyle = "background: #F5F7FA; color: #606266; border-radius: 12px; padding: 4px 14px; font-size: 11px; font-weight: 500;";
        }
        typeLabel->setStyleSheet(typeStyle);

        typeLayout->addWidget(typeLabel);
        m_questionTable->setCellWidget(i, 1, typeWidget);

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

        // 行内操作按钮组 - 更加低调
        auto *actionWidget = new QWidget(m_questionTable);
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(0, 0, 8, 0);
        actionLayout->setSpacing(0);

        // 上移按钮
        auto *upBtn = new QPushButton("∧", actionWidget);
        upBtn->setObjectName("inlineActionButton");
        upBtn->setFixedSize(22, 22);
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
        auto *downBtn = new QPushButton("∨", actionWidget);
        downBtn->setObjectName("inlineActionButton");
        downBtn->setFixedSize(22, 22);
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
        delBtn->setFixedSize(22, 22);
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

    m_summaryLabel->setText(QString("共 %1 题，合计 %2 分").arg(count).arg(totalScore));

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

    // 导出成功后清空试题篮
    QuestionBasket::instance()->clear();

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
            font-family: "PingFang SC", "Noto Sans SC", "Microsoft YaHei", "Helvetica Neue", Arial, sans-serif;
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
