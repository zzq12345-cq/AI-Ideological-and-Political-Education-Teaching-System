#include "QualityCheckDialog.h"
#include "../services/QuestionQualityService.h"
#include "../services/PaperService.h"
#include <QGraphicsDropShadowEffect>
#include <QStackedWidget>
#include <QScrollArea>
#include <QTimer>

namespace {
    const QString ACCENT = "#D32F2F";
    const QString BG = "#F5F6FA";
}

QualityCheckDialog::QualityCheckDialog(QuestionQualityService *qualityService,
                                         PaperService *paperService,
                                         QWidget *parent)
    : QDialog(parent)
    , m_qualityService(qualityService)
    , m_paperService(paperService)
{
    setWindowTitle("题库质量检查");
    setMinimumSize(580, 480);
    resize(620, 520);
    setStyleSheet(QString("QDialog { background-color: %1; }").arg(BG));
    initUI();

    connect(m_qualityService, &QuestionQualityService::libraryScanProgress,
            this, &QualityCheckDialog::onScanProgress);
    connect(m_qualityService, &QuestionQualityService::libraryScanCompleted,
            this, &QualityCheckDialog::onScanCompleted);
    connect(m_qualityService, &QuestionQualityService::errorOccurred,
            this, [this](const QString &, const QString &err) { onScanError(err); });

    // 监听 PaperService 错误
    connect(m_paperService, &PaperService::questionError,
            this, [this](const QString &, const QString &err) { onScanError(err); });
}

void QualityCheckDialog::initUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    // ── 标题区 ──
    auto *titleRow = new QHBoxLayout();
    auto *iconLabel = new QLabel();
    iconLabel->setFixedSize(40, 40);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(
        "QLabel { background-color: #FFEBEE; border-radius: 12px; "
        "font-size: 20px; }");
    iconLabel->setText("Q");  // 简单占位符
    titleRow->addWidget(iconLabel);

    auto *titleGroup = new QVBoxLayout();
    titleGroup->setSpacing(2);
    auto *titleLabel = new QLabel("题库质量检查");
    titleLabel->setStyleSheet(
        "QLabel { font-size: 18px; font-weight: 700; color: #1A1A2E; }");
    titleGroup->addWidget(titleLabel);
    auto *subtitleLabel = new QLabel("扫描公共题库，检测疑似重复题目");
    subtitleLabel->setStyleSheet(
        "QLabel { font-size: 12px; color: #9CA3AF; }");
    titleGroup->addWidget(subtitleLabel);
    titleRow->addLayout(titleGroup, 1);
    root->addLayout(titleRow);

    // ── 内容卡片 ──
    auto *card = new QFrame();
    card->setStyleSheet(
        "QFrame#qualityCard { background-color: white; border-radius: 16px; "
        "border: 1px solid #E5E7EB; }");
    card->setObjectName("qualityCard");

    auto *cardShadow = new QGraphicsDropShadowEffect(card);
    cardShadow->setBlurRadius(20);
    cardShadow->setOffset(0, 4);
    cardShadow->setColor(QColor(0, 0, 0, 15));
    card->setGraphicsEffect(cardShadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(16);

    // === 空闲态 ===
    m_idleView = new QWidget();
    auto *idleLayout = new QVBoxLayout(m_idleView);
    idleLayout->setContentsMargins(0, 20, 0, 20);
    idleLayout->setAlignment(Qt::AlignCenter);

    auto *idleIcon = new QLabel();
    idleIcon->setAlignment(Qt::AlignCenter);
    idleIcon->setStyleSheet(
        "QLabel { font-size: 48px; color: #E5E7EB; }");
    idleIcon->setText("~");
    idleLayout->addWidget(idleIcon);

    auto *idleText = new QLabel("点击下方按钮开始扫描题库中的重复题目");
    idleText->setAlignment(Qt::AlignCenter);
    idleText->setStyleSheet(
        "QLabel { font-size: 13px; color: #9CA3AF; margin-top: 8px; }");
    idleLayout->addWidget(idleText);
    cardLayout->addWidget(m_idleView);

    // === 扫描中态 ===
    m_scanningView = new QWidget();
    auto *scanLayout = new QVBoxLayout(m_scanningView);
    scanLayout->setContentsMargins(0, 20, 0, 20);
    scanLayout->setAlignment(Qt::AlignCenter);

    m_progressLabel = new QLabel("正在加载题库...");
    m_progressLabel->setAlignment(Qt::AlignCenter);
    m_progressLabel->setStyleSheet(
        "QLabel { font-size: 14px; font-weight: 600; color: #374151; }");
    scanLayout->addWidget(m_progressLabel);

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #F3F4F6; border-radius: 3px; border: none; }"
        "QProgressBar::chunk { background-color: " + ACCENT + "; border-radius: 3px; }");
    scanLayout->addWidget(m_progressBar);

    m_scanningView->setVisible(false);
    cardLayout->addWidget(m_scanningView);

    // === 结果态 ===
    m_resultView = new QWidget();
    auto *resultLayout = new QVBoxLayout(m_resultView);
    resultLayout->setContentsMargins(0, 0, 0, 0);
    resultLayout->setSpacing(12);

    m_resultSummary = new QLabel();
    m_resultSummary->setWordWrap(true);
    m_resultSummary->setStyleSheet(
        "QLabel { font-size: 14px; font-weight: 600; padding: 12px 16px; "
        "border-radius: 10px; }");
    resultLayout->addWidget(m_resultSummary);

    // 重复列表滚动区
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto *listWidget = new QWidget();
    m_resultListLayout = new QVBoxLayout(listWidget);
    m_resultListLayout->setContentsMargins(0, 0, 0, 0);
    m_resultListLayout->setSpacing(8);
    m_resultListLayout->addStretch();
    scrollArea->setWidget(listWidget);
    resultLayout->addWidget(scrollArea, 1);

    m_resultView->setVisible(false);
    cardLayout->addWidget(m_resultView, 1);

    root->addWidget(card, 1);

    // ── 底部按钮 ──
    m_scanBtn = new QPushButton("开始扫描");
    m_scanBtn->setFixedHeight(44);
    m_scanBtn->setCursor(Qt::PointingHandCursor);
    m_scanBtn->setStyleSheet(
        "QPushButton { background-color: " + ACCENT + "; color: white; "
        "border: none; border-radius: 12px; font-size: 14px; font-weight: 700; "
        "padding: 0 40px; }"
        "QPushButton:hover { background-color: #B71C1C; }"
        "QPushButton:disabled { background-color: #E5E7EB; color: #9CA3AF; }");
    connect(m_scanBtn, &QPushButton::clicked, this, &QualityCheckDialog::onStartScan);
    root->addWidget(m_scanBtn);
}

void QualityCheckDialog::onStartScan()
{
    m_scanBtn->setEnabled(false);
    m_scanBtn->setText("扫描中...");

    m_idleView->setVisible(false);
    m_resultView->setVisible(false);
    m_scanningView->setVisible(true);
    m_progressBar->setValue(0);
    m_progressLabel->setText("正在加载题库...");

    m_qualityService->scanDuplicatesInLibrary();
}

void QualityCheckDialog::onScanProgress(int current, int total)
{
    if (total > 0) {
        m_progressBar->setValue(qRound(100.0 * current / total));
    }
    m_progressLabel->setText(
        QString("正在比对 %1 / %2 题...").arg(current).arg(total));
}

void QualityCheckDialog::onScanCompleted(const QList<QPair<QString,QString>> &duplicatePairs)
{
    m_scanBtn->setEnabled(true);
    m_scanBtn->setText("重新扫描");

    m_scanningView->setVisible(false);
    m_resultView->setVisible(true);

    // 清空旧结果
    QLayoutItem *item;
    while ((item = m_resultListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    if (duplicatePairs.isEmpty()) {
        m_resultSummary->setText("未发现疑似重复题目，题库质量良好");
        m_resultSummary->setStyleSheet(
            "QLabel { font-size: 14px; font-weight: 600; padding: 12px 16px; "
            "border-radius: 10px; background-color: #E8F5E9; color: #2E7D32; }");
    } else {
        m_resultSummary->setText(
            QString("发现 %1 对疑似重复题目").arg(duplicatePairs.size()));
        m_resultSummary->setStyleSheet(
            "QLabel { font-size: 14px; font-weight: 600; padding: 12px 16px; "
            "border-radius: 10px; background-color: #FFF3E0; color: #E65100; }");

        int idx = 1;
        for (const auto &pair : duplicatePairs) {
            auto *row = new QFrame();
            row->setStyleSheet(
                "QFrame { background-color: #FAFAFA; border: 1px solid #F0F0F0; "
                "border-radius: 8px; }");
            auto *rowLayout = new QVBoxLayout(row);
            rowLayout->setContentsMargins(14, 10, 14, 10);
            rowLayout->setSpacing(4);

            auto *pairTitle = new QLabel(QString("第 %1 对").arg(idx++));
            pairTitle->setStyleSheet(
                "QLabel { font-size: 11px; font-weight: 600; color: #9CA3AF; }");
            rowLayout->addWidget(pairTitle);

            auto *idA = new QLabel(QString("A: %1").arg(pair.first));
            idA->setStyleSheet("QLabel { font-size: 12px; color: #374151; }");
            idA->setTextInteractionFlags(Qt::TextSelectableByMouse);
            rowLayout->addWidget(idA);

            auto *idB = new QLabel(QString("B: %1").arg(pair.second));
            idB->setStyleSheet("QLabel { font-size: 12px; color: #374151; }");
            idB->setTextInteractionFlags(Qt::TextSelectableByMouse);
            rowLayout->addWidget(idB);

            m_resultListLayout->addWidget(row);
        }
    }

    m_resultListLayout->addStretch();
}

void QualityCheckDialog::onScanError(const QString &error)
{
    m_scanBtn->setEnabled(true);
    m_scanBtn->setText("重新扫描");

    m_scanningView->setVisible(false);
    m_resultView->setVisible(true);

    // 清空旧结果
    QLayoutItem *item;
    while ((item = m_resultListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    m_resultSummary->setText(QString("扫描失败: %1").arg(error));
    m_resultSummary->setStyleSheet(
        "QLabel { font-size: 14px; font-weight: 600; padding: 12px 16px; "
        "border-radius: 10px; background-color: #FFEBEE; color: #C62828; }");
    m_resultListLayout->addStretch();
}
