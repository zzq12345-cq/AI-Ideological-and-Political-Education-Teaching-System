#include "DataAnalyticsWidget.h"
#include "AnalyticsDataService.h"
#include "../services/DifyService.h"
#include "../shared/StyleConfig.h"
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QSvgWidget>
#include <QTextEdit>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QFontDatabase>
#include <QTextOption>
#include <QTextDocument>
#include <QRegularExpression>
#include <QtCharts/QChart>

DataAnalyticsWidget::DataAnalyticsWidget(QWidget *parent)
    : QWidget(parent)
    , m_difyService(nullptr)
    , m_dataService(AnalyticsDataService::instance())
    , m_isGeneratingReport(false)
    , m_barChartView(nullptr)
    , m_lineChartView(nullptr)
    , m_gradeBarSet(nullptr)
    , m_participationSeries(nullptr)
    , m_completionSeries(nullptr)
{
    setupUI();
    setupStyles();

    // 连接数据刷新信号
    connect(m_dataService, &AnalyticsDataService::dataRefreshed,
            this, &DataAnalyticsWidget::onDataRefreshed);
}

DataAnalyticsWidget::~DataAnalyticsWidget()
{
}

void DataAnalyticsWidget::setDifyService(DifyService *service)
{
    if (m_difyService) {
        disconnect(m_difyService, nullptr, this, nullptr);
    }

    m_difyService = service;

    if (m_difyService) {
        connect(m_difyService, &DifyService::messageReceived,
                this, &DataAnalyticsWidget::onAIResponseReceived);
        connect(m_difyService, &DifyService::streamChunkReceived,
                this, &DataAnalyticsWidget::onAIStreamChunk);
        // 流式模式结束时也要更新按钮状态
        connect(m_difyService, &DifyService::requestFinished, this, [this]() {
            onAIResponseReceived(QString());
        });
    }
}

void DataAnalyticsWidget::refresh()
{
    qDebug() << "[DataAnalyticsWidget] Refreshing data...";
    m_dataService->refreshData();
}

void DataAnalyticsWidget::onDataRefreshed()
{
    updateMetricsDisplay();
    updateCharts();
}

void DataAnalyticsWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(32, 28, 32, 32);
    m_mainLayout->setSpacing(24);

    createHeader();
    createMetricsCards();
    createChartsArea();
    createAIReportArea();

    m_mainLayout->addStretch();

    // 初始化显示
    updateMetricsDisplay();
}

void DataAnalyticsWidget::setupStyles()
{
    setStyleSheet(QString(
        "DataAnalyticsWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 #FAFBFC, stop:0.5 %1, stop:1 #F0F2F5);"
        "}"
    ).arg(StyleConfig::BG_APP));
}

void DataAnalyticsWidget::createHeader()
{
    m_headerFrame = new QFrame();
    m_headerFrame->setFixedHeight(120);
    m_headerFrame->setObjectName("headerFrame");
    m_headerFrame->setStyleSheet(QString(
        "QFrame#headerFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #1E3A5F, stop:0.5 #2C5282, stop:1 #1A365D);"
        "    border-radius: %1px;"
        "    border: 1px solid rgba(255,255,255,0.1);"
        "}"
    ).arg(StyleConfig::RADIUS_XL));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(m_headerFrame);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(30, 58, 95, 60));
    m_headerFrame->setGraphicsEffect(shadow);

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(32, 20, 32, 20);
    headerLayout->setSpacing(20);

    // 左侧标题区
    QHBoxLayout *titleArea = new QHBoxLayout();
    titleArea->setSpacing(16);

    QLabel *iconContainer = new QLabel();
    iconContainer->setFixedSize(52, 52);
    iconContainer->setStyleSheet(
        "background: rgba(255,255,255,0.15);"
        "border-radius: 14px;"
        "border: 1px solid rgba(255,255,255,0.2);"
    );
    iconContainer->setAlignment(Qt::AlignCenter);
    QPixmap iconPix(":/icons/resources/icons/analytics.svg");
    if (iconPix.isNull()) {
        iconPix = QPixmap(":/icons/resources/icons/dashboard.svg");
    }
    iconContainer->setPixmap(iconPix.scaled(26, 26, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QVBoxLayout *titleTextLayout = new QVBoxLayout();
    titleTextLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel("数据分析报告");
    titleLabel->setStyleSheet(
        "font-size: 26px; font-weight: 800; color: white; background: transparent;"
        "letter-spacing: 2px;"
    );

    QLabel *subtitleLabel = new QLabel("教学数据可视化 | 学情分析 | AI 智能报告");
    subtitleLabel->setStyleSheet(
        "font-size: 13px; color: rgba(255,255,255,0.75); background: transparent;"
        "font-weight: 500;"
    );

    titleTextLayout->addWidget(titleLabel);
    titleTextLayout->addWidget(subtitleLabel);

    titleArea->addWidget(iconContainer);
    titleArea->addLayout(titleTextLayout);

    // 右侧按钮
    m_exportBtn = new QPushButton("导出报告");
    m_exportBtn->setIcon(QIcon(":/icons/resources/icons/document.svg"));
    m_exportBtn->setIconSize(QSize(16, 16));
    m_exportBtn->setFixedHeight(40);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setStyleSheet(
        "QPushButton {"
        "    background: rgba(255,255,255,0.12);"
        "    color: white;"
        "    border: 1px solid rgba(255,255,255,0.25);"
        "    border-radius: 20px;"
        "    padding: 0 24px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(255,255,255,0.22);"
        "    border-color: rgba(255,255,255,0.4);"
        "}"
    );
    connect(m_exportBtn, &QPushButton::clicked, this, &DataAnalyticsWidget::onExportClicked);

    m_refreshBtn = new QPushButton("刷新数据");
    m_refreshBtn->setIcon(QIcon(":/icons/resources/icons/dashboard.svg"));
    m_refreshBtn->setIconSize(QSize(16, 16));
    m_refreshBtn->setFixedHeight(40);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 rgba(255,255,255,0.25), stop:1 rgba(255,255,255,0.12));"
        "    color: white;"
        "    border: 1px solid rgba(255,255,255,0.3);"
        "    border-radius: 20px;"
        "    padding: 0 24px;"
        "    font-size: 13px;"
        "    font-weight: 700;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 rgba(255,255,255,0.35), stop:1 rgba(255,255,255,0.2));"
        "}"
    );
    connect(m_refreshBtn, &QPushButton::clicked, this, &DataAnalyticsWidget::onRefreshClicked);

    headerLayout->addLayout(titleArea);
    headerLayout->addStretch();
    headerLayout->addWidget(m_exportBtn);
    headerLayout->addWidget(m_refreshBtn);

    m_mainLayout->addWidget(m_headerFrame);
}

void DataAnalyticsWidget::createMetricsCards()
{
    m_metricsContainer = new QWidget();
    m_metricsContainer->setStyleSheet("background: transparent;");

    QHBoxLayout *metricsLayout = new QHBoxLayout(m_metricsContainer);
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setSpacing(20);

    // 从数据服务获取初始数据
    auto participation = m_dataService->getParticipationRate();
    auto completion = m_dataService->getCompletionRate();
    auto achievement = m_dataService->getAchievementRate();

    // 三个指标卡片
    QFrame *card1 = createMetricCard(
        ":/icons/resources/icons/user.svg",
        "课堂参与度",
        QString::number(participation.value, 'f', 1) + "%",
        QString("%1%2%").arg(participation.isPositive ? "+" : "").arg(QString::number(participation.change, 'f', 1)),
        participation.isPositive
    );
    m_participationValue = card1->findChild<QLabel*>("valueLabel");
    m_participationChange = card1->findChild<QLabel*>("changeLabel");

    QFrame *card2 = createMetricCard(
        ":/icons/resources/icons/document.svg",
        "作业完成率",
        QString::number(completion.value, 'f', 1) + "%",
        QString("%1%2%").arg(completion.isPositive ? "+" : "").arg(QString::number(completion.change, 'f', 1)),
        completion.isPositive
    );
    m_completionValue = card2->findChild<QLabel*>("valueLabel");
    m_completionChange = card2->findChild<QLabel*>("changeLabel");

    QFrame *card3 = createMetricCard(
        ":/icons/resources/icons/award.svg",
        "目标达成率",
        QString::number(achievement.value, 'f', 1) + "%",
        QString("%1%2%").arg(achievement.isPositive ? "+" : "").arg(QString::number(achievement.change, 'f', 1)),
        achievement.isPositive
    );
    m_achievementValue = card3->findChild<QLabel*>("valueLabel");
    m_achievementChange = card3->findChild<QLabel*>("changeLabel");

    metricsLayout->addWidget(card1);
    metricsLayout->addWidget(card2);
    metricsLayout->addWidget(card3);

    m_mainLayout->addWidget(m_metricsContainer);
}

QFrame* DataAnalyticsWidget::createMetricCard(const QString &iconPath,
                                               const QString &title,
                                               const QString &value,
                                               const QString &change,
                                               bool isPositive)
{
    QFrame *card = new QFrame();
    card->setObjectName("metricCard");
    card->setMinimumHeight(120);
    card->setStyleSheet(QString(
        "QFrame#metricCard {"
        "    background-color: %1;"
        "    border-radius: %2px;"
        "    border: 1px solid %3;"
        "}"
        "QFrame#metricCard:hover {"
        "    border-color: #3182CE;"
        "    background-color: #FAFCFF;"
        "}"
    ).arg(StyleConfig::BG_CARD).arg(StyleConfig::RADIUS_L).arg(StyleConfig::BORDER_LIGHT));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(12);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 15));
    card->setGraphicsEffect(shadow);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 18);
    cardLayout->setSpacing(16);

    // 左侧图标
    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(48, 48);
    iconLabel->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #EBF8FF, stop:1 #BEE3F8);"
        "border-radius: 12px;"
    );
    iconLabel->setAlignment(Qt::AlignCenter);
    QPixmap pix(iconPath);
    if (!pix.isNull()) {
        iconLabel->setPixmap(pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 右侧内容
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(6);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QString(
        "color: %1; font-size: 13px; font-weight: 500; background: transparent;"
    ).arg(StyleConfig::TEXT_SECONDARY));

    QLabel *valueLabel = new QLabel(value);
    valueLabel->setObjectName("valueLabel");
    valueLabel->setStyleSheet(QString(
        "color: %1; font-size: 28px; font-weight: 800; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    // 变化指示
    QHBoxLayout *changeLayout = new QHBoxLayout();
    changeLayout->setSpacing(4);

    QLabel *arrowLabel = new QLabel();
    arrowLabel->setFixedSize(14, 14);
    QString arrowIcon = isPositive ? ":/icons/resources/icons/arrow-up.svg" : ":/icons/resources/icons/arrow-down.svg";
    QPixmap arrowPix(arrowIcon);
    if (!arrowPix.isNull()) {
        arrowLabel->setPixmap(arrowPix.scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        arrowLabel->setText(isPositive ? "↑" : "↓");
        arrowLabel->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent;")
            .arg(isPositive ? "#38A169" : "#E53E3E"));
    }

    QLabel *changeLabel = new QLabel(change + " 较上周");
    changeLabel->setObjectName("changeLabel");
    QString changeColor = isPositive ? "#38A169" : "#E53E3E";
    changeLabel->setStyleSheet(QString(
        "color: %1; font-size: 12px; font-weight: 500; background: transparent;"
    ).arg(changeColor));

    changeLayout->addWidget(arrowLabel);
    changeLayout->addWidget(changeLabel);
    changeLayout->addStretch();

    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(valueLabel);
    contentLayout->addLayout(changeLayout);

    cardLayout->addWidget(iconLabel);
    cardLayout->addLayout(contentLayout);
    cardLayout->addStretch();

    return card;
}

void DataAnalyticsWidget::updateMetricsDisplay()
{
    auto participation = m_dataService->getParticipationRate();
    auto completion = m_dataService->getCompletionRate();
    auto achievement = m_dataService->getAchievementRate();

    if (m_participationValue) {
        m_participationValue->setText(QString::number(participation.value, 'f', 1) + "%");
    }
    if (m_completionValue) {
        m_completionValue->setText(QString::number(completion.value, 'f', 1) + "%");
    }
    if (m_achievementValue) {
        m_achievementValue->setText(QString::number(achievement.value, 'f', 1) + "%");
    }
}

void DataAnalyticsWidget::createChartsArea()
{
    m_chartsContainer = new QWidget();
    m_chartsContainer->setStyleSheet("background: transparent;");
    m_chartsContainer->setMinimumHeight(300);

    QHBoxLayout *chartsLayout = new QHBoxLayout(m_chartsContainer);
    chartsLayout->setContentsMargins(0, 0, 0, 0);
    chartsLayout->setSpacing(20);

    // ========== 左侧：成绩分布柱状图 ==========
    QFrame *distributionCard = new QFrame();
    distributionCard->setObjectName("chartCard");
    distributionCard->setStyleSheet(QString(
        "QFrame#chartCard {"
        "    background-color: %1;"
        "    border-radius: %2px;"
        "    border: 1px solid %3;"
        "}"
    ).arg(StyleConfig::BG_CARD).arg(StyleConfig::RADIUS_L).arg(StyleConfig::BORDER_LIGHT));

    QVBoxLayout *distLayout = new QVBoxLayout(distributionCard);
    distLayout->setContentsMargins(20, 16, 20, 16);
    distLayout->setSpacing(12);

    // 标题行
    QHBoxLayout *distTitleRow = new QHBoxLayout();
    QLabel *distIcon = new QLabel();
    distIcon->setPixmap(QIcon(":/icons/resources/icons/menu.svg").pixmap(18, 18));
    QLabel *distTitle = new QLabel("成绩分布");
    distTitle->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 700; background: transparent;")
        .arg(StyleConfig::TEXT_PRIMARY));
    distTitleRow->addWidget(distIcon);
    distTitleRow->addWidget(distTitle);
    distTitleRow->addStretch();

    // 创建柱状图
    auto gradeData = m_dataService->getGradeDistribution();
    m_gradeBarSet = new QBarSet("人数");
    *m_gradeBarSet << gradeData.excellent << gradeData.good << gradeData.pass << gradeData.fail;
    m_gradeBarSet->setColor(QColor("#4299E1"));

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(m_gradeBarSet);
    barSeries->setLabelsVisible(true);
    barSeries->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    QChart *barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setAnimationOptions(QChart::SeriesAnimations);
    barChart->legend()->setVisible(false);
    barChart->setBackgroundVisible(false);
    barChart->setMargins(QMargins(0, 0, 0, 0));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(QStringList() << "优秀" << "良好" << "及格" << "不及格");
    axisX->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    barChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 50);
    axisY->setLabelFormat("%d");
    axisY->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    axisY->setGridLineVisible(true);
    axisY->setGridLineColor(QColor("#E2E8F0"));
    barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    m_barChartView = new QChartView(barChart);
    m_barChartView->setRenderHint(QPainter::Antialiasing);
    m_barChartView->setMinimumHeight(220);
    m_barChartView->setStyleSheet("background: transparent; border: none;");

    distLayout->addLayout(distTitleRow);
    distLayout->addWidget(m_barChartView);

    // ========== 右侧：学习趋势折线图 ==========
    QFrame *trendCard = new QFrame();
    trendCard->setObjectName("chartCard");
    trendCard->setStyleSheet(QString(
        "QFrame#chartCard {"
        "    background-color: %1;"
        "    border-radius: %2px;"
        "    border: 1px solid %3;"
        "}"
    ).arg(StyleConfig::BG_CARD).arg(StyleConfig::RADIUS_L).arg(StyleConfig::BORDER_LIGHT));

    QVBoxLayout *trendLayout = new QVBoxLayout(trendCard);
    trendLayout->setContentsMargins(20, 16, 20, 16);
    trendLayout->setSpacing(12);

    // 标题行
    QHBoxLayout *trendTitleRow = new QHBoxLayout();
    QLabel *trendIcon = new QLabel();
    trendIcon->setPixmap(QIcon(":/icons/resources/icons/analytics.svg").pixmap(18, 18));
    QLabel *trendTitle = new QLabel("学习趋势 (近30天)");
    trendTitle->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 700; background: transparent;")
        .arg(StyleConfig::TEXT_PRIMARY));
    trendTitleRow->addWidget(trendIcon);
    trendTitleRow->addWidget(trendTitle);
    trendTitleRow->addStretch();

    // 创建折线图
    m_participationSeries = new QLineSeries();
    m_participationSeries->setName("参与度");
    m_participationSeries->setColor(QColor("#E53935"));
    QPen pen1(QColor("#E53935"), 2);
    m_participationSeries->setPen(pen1);

    m_completionSeries = new QLineSeries();
    m_completionSeries->setName("完成率");
    m_completionSeries->setColor(QColor("#4299E1"));
    QPen pen2(QColor("#4299E1"), 2);
    m_completionSeries->setPen(pen2);

    // 填充数据
    auto participationTrend = m_dataService->getParticipationTrend();
    auto completionTrend = m_dataService->getCompletionTrend();
    for (int i = 0; i < participationTrend.size(); ++i) {
        m_participationSeries->append(i, participationTrend[i].value);
        m_completionSeries->append(i, completionTrend[i].value);
    }

    QChart *lineChart = new QChart();
    lineChart->addSeries(m_participationSeries);
    lineChart->addSeries(m_completionSeries);
    lineChart->setAnimationOptions(QChart::SeriesAnimations);
    lineChart->legend()->setVisible(true);
    lineChart->legend()->setAlignment(Qt::AlignBottom);
    lineChart->setBackgroundVisible(false);
    lineChart->setMargins(QMargins(0, 0, 0, 0));

    QValueAxis *lineAxisX = new QValueAxis();
    lineAxisX->setRange(0, 29);
    lineAxisX->setLabelFormat("%d");
    lineAxisX->setTitleText("天");
    lineAxisX->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    lineChart->addAxis(lineAxisX, Qt::AlignBottom);
    m_participationSeries->attachAxis(lineAxisX);
    m_completionSeries->attachAxis(lineAxisX);

    QValueAxis *lineAxisY = new QValueAxis();
    lineAxisY->setRange(60, 100);
    lineAxisY->setLabelFormat("%d%%");
    lineAxisY->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    lineAxisY->setGridLineVisible(true);
    lineAxisY->setGridLineColor(QColor("#E2E8F0"));
    lineChart->addAxis(lineAxisY, Qt::AlignLeft);
    m_participationSeries->attachAxis(lineAxisY);
    m_completionSeries->attachAxis(lineAxisY);

    m_lineChartView = new QChartView(lineChart);
    m_lineChartView->setRenderHint(QPainter::Antialiasing);
    m_lineChartView->setMinimumHeight(220);
    m_lineChartView->setStyleSheet("background: transparent; border: none;");

    trendLayout->addLayout(trendTitleRow);
    trendLayout->addWidget(m_lineChartView);

    chartsLayout->addWidget(distributionCard, 1);
    chartsLayout->addWidget(trendCard, 1);

    m_mainLayout->addWidget(m_chartsContainer);
}

void DataAnalyticsWidget::updateCharts()
{
    // 更新柱状图
    if (m_gradeBarSet) {
        auto gradeData = m_dataService->getGradeDistribution();
        m_gradeBarSet->replace(0, gradeData.excellent);
        m_gradeBarSet->replace(1, gradeData.good);
        m_gradeBarSet->replace(2, gradeData.pass);
        m_gradeBarSet->replace(3, gradeData.fail);
    }

    // 更新折线图
    if (m_participationSeries && m_completionSeries) {
        m_participationSeries->clear();
        m_completionSeries->clear();

        auto participationTrend = m_dataService->getParticipationTrend();
        auto completionTrend = m_dataService->getCompletionTrend();
        for (int i = 0; i < participationTrend.size(); ++i) {
            m_participationSeries->append(i, participationTrend[i].value);
            m_completionSeries->append(i, completionTrend[i].value);
        }
    }
}

void DataAnalyticsWidget::createAIReportArea()
{
    m_aiReportFrame = new QFrame();
    m_aiReportFrame->setObjectName("aiReportFrame");
    m_aiReportFrame->setMinimumHeight(200);
    m_aiReportFrame->setStyleSheet(QString(
        "QFrame#aiReportFrame {"
        "    background-color: %1;"
        "    border-radius: %2px;"
        "    border: 1px solid %3;"
        "}"
    ).arg(StyleConfig::BG_CARD).arg(StyleConfig::RADIUS_L).arg(StyleConfig::BORDER_LIGHT));

    QVBoxLayout *reportLayout = new QVBoxLayout(m_aiReportFrame);
    reportLayout->setContentsMargins(24, 20, 24, 20);
    reportLayout->setSpacing(16);

    // 标题行
    QHBoxLayout *titleRow = new QHBoxLayout();

    QLabel *aiIcon = new QLabel();
    aiIcon->setFixedSize(32, 32);
    aiIcon->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #FEF2F2, stop:1 #FFF7ED);"
        "border-radius: 8px;"
    );
    aiIcon->setAlignment(Qt::AlignCenter);
    QPixmap robotPix(":/icons/resources/icons/robot.svg");
    if (!robotPix.isNull()) {
        aiIcon->setPixmap(robotPix.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QLabel *titleLabel = new QLabel("AI 智能分析报告");
    titleLabel->setStyleSheet(QString(
        "color: %1; font-size: 18px; font-weight: 700; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    m_generateReportBtn = new QPushButton("生成报告");
    m_generateReportBtn->setIcon(QIcon(":/icons/resources/icons/sparkle.svg"));
    m_generateReportBtn->setIconSize(QSize(14, 14));
    m_generateReportBtn->setFixedHeight(34);
    m_generateReportBtn->setCursor(Qt::PointingHandCursor);
    m_generateReportBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %1, stop:1 #C62828);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 17px;"
        "    padding: 0 20px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #EF4444, stop:1 #DC2626);"
        "}"
        "QPushButton:disabled {"
        "    background: #CBD5E0;"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(m_generateReportBtn, &QPushButton::clicked, this, &DataAnalyticsWidget::onGenerateReportClicked);

    QPushButton *copyBtn = new QPushButton("复制");
    copyBtn->setIcon(QIcon(":/icons/resources/icons/document.svg"));
    copyBtn->setIconSize(QSize(14, 14));
    copyBtn->setFixedHeight(34);
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: %1;"
        "    border: 1.5px solid %2;"
        "    border-radius: 17px;"
        "    padding: 0 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background-color: %3;"
        "    color: %4;"
        "}"
    ).arg(StyleConfig::TEXT_SECONDARY, StyleConfig::BORDER_LIGHT,
          StyleConfig::PATRIOTIC_RED_TINT, StyleConfig::PATRIOTIC_RED));
    connect(copyBtn, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(m_aiReportContent->text());
        QMessageBox::information(this, "提示", "报告已复制到剪贴板");
    });

    titleRow->addWidget(aiIcon);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(m_generateReportBtn);
    titleRow->addWidget(copyBtn);

    // 分割线
    QFrame *divider = new QFrame();
    divider->setFixedHeight(1);
    divider->setStyleSheet("background-color: #E2E8F0;");

    // 报告内容区
    m_aiReportContent = new QLabel();
    m_aiReportContent->setWordWrap(true);
    m_aiReportContent->setMinimumHeight(100);
    m_aiReportContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_aiReportContent->setStyleSheet(QString(
        "color: %1;"
        "font-size: 14px;"
        "line-height: 1.8;"
        "background: transparent;"
        "padding: 8px 0;"
    ).arg(StyleConfig::TEXT_SECONDARY));
    m_aiReportContent->setText(
        "点击「生成报告」按钮，AI 将根据当前教学数据自动生成分析报告，"
        "包括整体评估、薄弱环节识别、改进建议等内容。"
    );

    reportLayout->addLayout(titleRow);
    reportLayout->addWidget(divider);
    reportLayout->addWidget(m_aiReportContent);
    reportLayout->addStretch();

    m_mainLayout->addWidget(m_aiReportFrame);
}

void DataAnalyticsWidget::onRefreshClicked()
{
    qDebug() << "[DataAnalyticsWidget] Refresh clicked";
    refresh();
}

void DataAnalyticsWidget::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出数据分析报告",
        QDir::homePath() + "/数据分析报告.pdf",
        "PDF 文件 (*.pdf)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    // 使用屏幕分辨率，避免高DPI缩放问题
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QPainter painter(&printer);
    if (!painter.isActive()) {
        QMessageBox::warning(this, "导出失败", "无法创建 PDF 文件");
        return;
    }

    // 获取页面尺寸
    QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    int margin = 40;
    int contentWidth = pageWidth - 2 * margin;

    // 设置字体 - macOS 使用系统字体
    QFont titleFont("PingFang SC", 22, QFont::Bold);
    QFont subtitleFont("PingFang SC", 14, QFont::Bold);
    QFont contentFont("PingFang SC", 11);

    // 备用字体
    if (!QFontDatabase::hasFamily("PingFang SC")) {
        titleFont = QFont("Helvetica", 22, QFont::Bold);
        subtitleFont = QFont("Helvetica", 14, QFont::Bold);
        contentFont = QFont("Helvetica", 11);
    }

    int yPos = margin;
    int lineHeight = 28;
    int sectionGap = 40;

    // ========== 标题 ==========
    painter.setFont(titleFont);
    painter.setPen(QColor("#1A365D"));
    QRect titleRect(margin, yPos, contentWidth, 40);
    painter.drawText(titleRect, Qt::AlignCenter, "数据分析报告");
    yPos += 50;

    // 分割线
    painter.setPen(QPen(QColor("#E2E8F0"), 2));
    painter.drawLine(margin, yPos, pageWidth - margin, yPos);
    yPos += 20;

    // 生成时间
    painter.setFont(contentFont);
    painter.setPen(QColor("#718096"));
    QRect timeRect(margin, yPos, contentWidth, lineHeight);
    painter.drawText(timeRect, Qt::AlignCenter,
                     "生成时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    yPos += sectionGap;

    // ========== 核心指标 ==========
    painter.setFont(subtitleFont);
    painter.setPen(QColor("#2D3748"));
    painter.drawText(margin, yPos, contentWidth, lineHeight, Qt::AlignLeft, "一、核心指标");
    yPos += lineHeight + 10;

    painter.setFont(contentFont);
    painter.setPen(QColor("#4A5568"));

    auto participation = m_dataService->getParticipationRate();
    auto completion = m_dataService->getCompletionRate();
    auto achievement = m_dataService->getAchievementRate();

    QString text1 = QString("• 课堂参与度: %1% (%2%3%)")
        .arg(QString::number(participation.value, 'f', 1))
        .arg(participation.isPositive ? "+" : "")
        .arg(QString::number(participation.change, 'f', 1));
    painter.drawText(margin + 20, yPos, contentWidth, lineHeight, Qt::AlignLeft, text1);
    yPos += lineHeight;

    QString text2 = QString("• 作业完成率: %1% (%2%3%)")
        .arg(QString::number(completion.value, 'f', 1))
        .arg(completion.isPositive ? "+" : "")
        .arg(QString::number(completion.change, 'f', 1));
    painter.drawText(margin + 20, yPos, contentWidth, lineHeight, Qt::AlignLeft, text2);
    yPos += lineHeight;

    QString text3 = QString("• 目标达成率: %1% (%2%3%)")
        .arg(QString::number(achievement.value, 'f', 1))
        .arg(achievement.isPositive ? "+" : "")
        .arg(QString::number(achievement.change, 'f', 1));
    painter.drawText(margin + 20, yPos, contentWidth, lineHeight, Qt::AlignLeft, text3);
    yPos += sectionGap;

    // ========== 成绩分布 ==========
    painter.setFont(subtitleFont);
    painter.setPen(QColor("#2D3748"));
    painter.drawText(margin, yPos, contentWidth, lineHeight, Qt::AlignLeft, "二、成绩分布");
    yPos += lineHeight + 10;

    painter.setFont(contentFont);
    painter.setPen(QColor("#4A5568"));

    auto gradeData = m_dataService->getGradeDistribution();
    painter.drawText(margin + 20, yPos, contentWidth, lineHeight, Qt::AlignLeft,
                     QString("• 优秀 (90-100分): %1 人").arg(gradeData.excellent));
    yPos += lineHeight;
    painter.drawText(margin + 20, yPos, contentWidth, lineHeight, Qt::AlignLeft,
                     QString("• 良好 (80-89分): %1 人").arg(gradeData.good));
    yPos += lineHeight;
    painter.drawText(margin + 20, yPos, contentWidth, lineHeight, Qt::AlignLeft,
                     QString("• 及格 (60-79分): %1 人").arg(gradeData.pass));
    yPos += lineHeight;
    painter.drawText(margin + 20, yPos, contentWidth, lineHeight, Qt::AlignLeft,
                     QString("• 不及格 (<60分): %1 人").arg(gradeData.fail));
    yPos += sectionGap;

    // ========== AI 分析报告 ==========
    if (!m_currentAIResponse.isEmpty()) {
        painter.setFont(subtitleFont);
        painter.setPen(QColor("#2D3748"));
        painter.drawText(margin, yPos, contentWidth, lineHeight, Qt::AlignLeft, "三、AI 智能分析");
        yPos += lineHeight + 15;

        // 使用 QTextDocument 正确渲染多行文本
        QTextDocument doc;
        doc.setDefaultFont(contentFont);
        doc.setTextWidth(contentWidth - 20);
        doc.setPlainText(m_currentAIResponse);

        // 设置文档样式
        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WordWrap);
        doc.setDefaultTextOption(textOption);

        // 移动画布到正确位置并渲染
        painter.save();
        painter.translate(margin + 20, yPos);
        painter.setPen(QColor("#4A5568"));
        doc.drawContents(&painter);
        painter.restore();
    }

    painter.end();

    QMessageBox::information(this, "导出成功",
                             QString("报告已导出到:\n%1").arg(fileName));
}

void DataAnalyticsWidget::onGenerateReportClicked()
{
    if (!m_difyService) {
        QMessageBox::warning(this, "提示", "AI 服务未就绪，请稍后重试");
        return;
    }

    if (m_isGeneratingReport) {
        return;
    }

    m_isGeneratingReport = true;
    m_currentAIResponse.clear();
    m_generateReportBtn->setEnabled(false);
    m_generateReportBtn->setText("生成中...");
    m_aiReportContent->setText("正在分析数据，生成报告中...");

    // 使用真实数据构建分析提示词
    QString dataSummary = m_dataService->getDataSummary();
    QString prompt = QString(
        "请根据以下教学数据，生成一份简洁的分析报告：\n\n"
        "%1\n\n"
        "请按以下格式输出（简洁明了，每部分2-3句话）：\n"
        "【整体评估】\n"
        "【薄弱环节】\n"
        "【改进建议】"
    ).arg(dataSummary);

    m_difyService->sendMessage(prompt);
}

void DataAnalyticsWidget::onAIResponseReceived(const QString &response)
{
    Q_UNUSED(response)
    m_isGeneratingReport = false;
    m_generateReportBtn->setEnabled(true);
    m_generateReportBtn->setText("重新生成");

    // 最终清理一次 Markdown 符号
    QString cleaned = m_currentAIResponse;
    cleaned.remove(QRegularExpression("^##\\s*", QRegularExpression::MultilineOption));
    cleaned.remove(QRegularExpression("//+\\s*$"));
    cleaned.remove(QRegularExpression("\\*\\*"));
    cleaned = cleaned.trimmed();
    m_currentAIResponse = cleaned;
    m_aiReportContent->setText(cleaned);
}

void DataAnalyticsWidget::onAIStreamChunk(const QString &chunk)
{
    m_currentAIResponse += chunk;

    // 过滤 Markdown 格式符号
    QString displayText = m_currentAIResponse;
    // 去掉 ## 标题标记
    displayText.remove(QRegularExpression("^##\\s*", QRegularExpression::MultilineOption));
    // 去掉 // 注释符号
    displayText.remove(QRegularExpression("//+\\s*"));
    // 去掉 ** 加粗符号
    displayText.remove(QRegularExpression("\\*\\*"));

    m_aiReportContent->setText(displayText);
}
