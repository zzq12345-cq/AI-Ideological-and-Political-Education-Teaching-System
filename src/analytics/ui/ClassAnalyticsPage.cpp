#include "ClassAnalyticsPage.h"
#include "../datasources/MockDataSource.h"
#include "../models/Student.h"
#include "../models/ClassStatistics.h"
#include "../models/KnowledgePoint.h"
#include "../../services/DifyService.h"
#include "../../shared/StyleConfig.h"

#include <QDebug>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>
#include <QtCharts/QChart>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QHorizontalBarSeries>

ClassAnalyticsPage::ClassAnalyticsPage(QWidget *parent)
    : QWidget(parent)
    , m_dataSource(nullptr)
    , m_difyService(nullptr)
    , m_currentClassId(-1)
    , m_isGenerating(false)
    , m_distributionChartView(nullptr)
    , m_gradeBarSet(nullptr)
    , m_rankingTable(nullptr)
    , m_weakPointsChartView(nullptr)
{
    setupUI();
    setupStyles();
}

ClassAnalyticsPage::~ClassAnalyticsPage()
{
}

void ClassAnalyticsPage::setDataSource(MockDataSource *dataSource)
{
    m_dataSource = dataSource;
    if (m_dataSource) {
        auto classes = m_dataSource->getClassList();
        m_classCombo->clear();
        for (const auto &cls : classes) {
            m_classCombo->addItem(cls.name(), cls.id());
        }
        if (!classes.isEmpty()) {
            m_currentClassId = classes.first().id();
            updateCharts();
            updateRankingTable();
            updateWeakPointsChart();
        }
    }
}

void ClassAnalyticsPage::setDifyService(DifyService *service)
{
    if (m_difyService) {
        disconnect(m_difyService, nullptr, this, nullptr);
    }
    m_difyService = service;
    if (m_difyService) {
        connect(m_difyService, &DifyService::streamChunkReceived,
                this, &ClassAnalyticsPage::onAIStreamChunk);
        connect(m_difyService, &DifyService::requestFinished,
                this, &ClassAnalyticsPage::onAIRequestFinished);
    }
}

void ClassAnalyticsPage::refresh()
{
    updateCharts();
    updateRankingTable();
    updateWeakPointsChart();
}

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>

void ClassAnalyticsPage::setupUI()
{
    // 创建一个中心部件和布局，放进 ScrollArea
    QWidget *scrollContent = new QWidget();
    scrollContent->setObjectName("scrollContent");
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(32, 32, 32, 32);
    contentLayout->setSpacing(32);

    // 真正的布局容器应该是 contentLayout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    createHeader();
    contentLayout->addWidget(m_headerFrame);

    // 第一行：成绩分布 + 排名表格
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(32);

    createDistributionChart();
    createRankingTable();

    QFrame *distFrame = qobject_cast<QFrame*>(m_distributionChartView->parentWidget());
    QFrame *rankFrame = qobject_cast<QFrame*>(m_rankingTable->parentWidget());
    if (distFrame) topRow->addWidget(distFrame, 1);
    if (rankFrame) topRow->addWidget(rankFrame, 1);

    contentLayout->addLayout(topRow);

    // 第二行：薄弱知识点
    QFrame *weakFrame = createWeakPointsChart();
    contentLayout->addWidget(weakFrame);

    // 第三行：AI 智能助手
    createAIAdviceArea();
    contentLayout->addWidget(m_adviceFrame);

    contentLayout->addStretch();

    // 包装进 ScrollArea
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

    m_mainLayout->addWidget(scrollArea);
}

void ClassAnalyticsPage::setupStyles()
{
    setStyleSheet("ClassAnalyticsPage { background: transparent; }");
}

void ClassAnalyticsPage::createHeader()
{
    m_headerFrame = new QFrame();
    m_headerFrame->setObjectName("classHeader");
    m_headerFrame->setFixedHeight(60);
    m_headerFrame->setStyleSheet(QString(
        "QFrame#classHeader {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #E2E8F0;"
        "}"
    ));

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    headerLayout->setSpacing(16);

    QLabel *titleLabel = new QLabel("班级学情综合分析");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 700; color: #1E293B; background: transparent;"
    ));

    QLabel *classLabel = new QLabel("选择班级:");
    classLabel->setStyleSheet("color: #64748B; font-size: 14px; background: transparent;");
    m_classCombo = new QComboBox();
    m_classCombo->setFixedWidth(160);
    m_classCombo->setStyleSheet(
        "QComboBox {"
        "    background: white;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 8px;"
        "    padding: 6px 12px;"
        "    font-size: 14px;"
        "}"
        "QComboBox::drop-down { border: none; width: 20px; }"
    );
    connect(m_classCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClassAnalyticsPage::onClassChanged);

    m_exportBtn = new QPushButton("导出数据报告");
    m_exportBtn->setFixedHeight(34);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #64748B;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 8px;"
        "    padding: 0 16px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: #F8FAFC; border-color: #CBD5E0; }"
    );
    connect(m_exportBtn, &QPushButton::clicked, this, &ClassAnalyticsPage::onExportClicked);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(classLabel);
    headerLayout->addWidget(m_classCombo);
    headerLayout->addWidget(m_exportBtn);

    m_mainLayout->addWidget(m_headerFrame);
}

void ClassAnalyticsPage::createDistributionChart()
{
    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("distChartFrame");
    chartFrame->setMinimumHeight(280);
    chartFrame->setStyleSheet(QString(
        "QFrame#distChartFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #E2E8F0;"
        "}"
    ));

    QVBoxLayout *layout = new QVBoxLayout(chartFrame);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    QWidget *titleContainer = new QWidget();
    QHBoxLayout *distTitleRow = new QHBoxLayout(titleContainer);
    distTitleRow->setContentsMargins(0, 0, 0, 0);

    QLabel *iconBg = new QLabel();
    iconBg->setFixedSize(32, 32);
    iconBg->setStyleSheet(QString(
        "background-color: #EEF2FF; border-radius: 8px;"
    ));
    iconBg->setAlignment(Qt::AlignCenter);

    QLabel *distIcon = new QLabel(iconBg);
    distIcon->setPixmap(QIcon(":/icons/resources/icons/menu.svg").pixmap(18, 18));
    distIcon->setStyleSheet("background: transparent; color: #6366F1;");

    QHBoxLayout *iconLayout = new QHBoxLayout(iconBg);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(distIcon, 0, Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("班级成绩分布");
    titleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: #1E293B; background: transparent;"
    ));

    distTitleRow->addWidget(iconBg);
    distTitleRow->addSpacing(8);
    distTitleRow->addWidget(titleLabel);
    distTitleRow->addStretch();

    m_gradeBarSet = new QBarSet("人数");
    *m_gradeBarSet << 0 << 0 << 0 << 0;
    m_gradeBarSet->setColor(QColor(StyleConfig::PRIMARY_INDIGO));

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(m_gradeBarSet);
    barSeries->setLabelsVisible(true);
    barSeries->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    QChart *chart = new QChart();
    chart->addSeries(barSeries);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(false);
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(QStringList() << "优秀" << "良好" << "及格" << "不及格");
    axisX->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 20);
    axisY->setLabelFormat("%d");
    axisY->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    m_distributionChartView = new QChartView(chart);
    m_distributionChartView->setRenderHint(QPainter::Antialiasing);
    m_distributionChartView->setStyleSheet("background: transparent; border: none;");

    layout->addWidget(titleContainer);
    layout->addWidget(m_distributionChartView);
}

void ClassAnalyticsPage::createRankingTable()
{
    QFrame *tableFrame = new QFrame();
    tableFrame->setObjectName("rankingFrame");
    tableFrame->setMinimumHeight(280);
    tableFrame->setStyleSheet(QString(
        "QFrame#rankingFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #E2E8F0;"
        "}"
    ));

    QVBoxLayout *layout = new QVBoxLayout(tableFrame);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    QWidget *titleContainer = new QWidget();
    QHBoxLayout *rankTitleRow = new QHBoxLayout(titleContainer);
    rankTitleRow->setContentsMargins(0, 0, 0, 0);

    QLabel *iconBg = new QLabel();
    iconBg->setFixedSize(32, 32);
    iconBg->setStyleSheet(QString(
        "background-color: #FFFBEB; border-radius: 8px;"
    ));
    iconBg->setAlignment(Qt::AlignCenter);

    QLabel *rankIcon = new QLabel(iconBg);
    rankIcon->setPixmap(QIcon(":/icons/resources/icons/award.svg").pixmap(18, 18));
    rankIcon->setStyleSheet("background: transparent;");

    QHBoxLayout *iconLayout = new QHBoxLayout(iconBg);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(rankIcon, 0, Qt::AlignCenter);

    QLabel *rankTitleLabel = new QLabel("班级优秀生光荣榜");
    rankTitleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: #1E293B; background: transparent;"
    ));

    rankTitleRow->addWidget(iconBg);
    rankTitleRow->addSpacing(8);
    rankTitleRow->addWidget(rankTitleLabel);
    rankTitleRow->addStretch();

    m_rankingTable = new QTableWidget();
    m_rankingTable->setColumnCount(4);
    m_rankingTable->setHorizontalHeaderLabels({"排名", "姓名", "学号", "综合分"});
    m_rankingTable->horizontalHeader()->setStretchLastSection(true);
    m_rankingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_rankingTable->verticalHeader()->setVisible(false);
    m_rankingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_rankingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_rankingTable->setAlternatingRowColors(true);
    m_rankingTable->setStyleSheet(
        "QTableWidget {"
        "    background: white;"
        "    border: 1px solid #F1F5F9;"
        "    border-radius: 12px;"
        "    gridline-color: #F8FAFC;"
        "    font-size: 14px;"
        "}"
        "QTableWidget::item {"
        "    padding: 12px;"
        "    border-bottom: 1px solid #F1F5F9;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #EEF2FF;"
        "    color: #4F46E5;"
        "}"
        "QHeaderView::section {"
        "    background: #F8FAFC;"
        "    color: #64748B;"
        "    font-weight: 700;"
        "    padding: 12px;"
        "    border: none;"
        "    border-bottom: 2px solid #F1F5F9;"
        "}"
    );

    layout->addWidget(titleContainer);
    layout->addWidget(m_rankingTable);
}

QFrame* ClassAnalyticsPage::createWeakPointsChart()
{
    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("weakPointsFrame");
    chartFrame->setMinimumHeight(320);
    chartFrame->setStyleSheet(QString(
        "QFrame#weakPointsFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #E2E8F0;"
        "}"
    ));

    QVBoxLayout *layout = new QVBoxLayout(chartFrame);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    QWidget *titleContainer = new QWidget();
    QHBoxLayout *weakTitleRow = new QHBoxLayout(titleContainer);
    weakTitleRow->setContentsMargins(0, 0, 0, 0);

    QLabel *iconBg = new QLabel();
    iconBg->setFixedSize(32, 32);
    iconBg->setStyleSheet(QString(
        "background-color: #FFF7ED; border-radius: 8px;"
    ));
    iconBg->setAlignment(Qt::AlignCenter);

    QLabel *weakIcon = new QLabel(iconBg);
    weakIcon->setPixmap(QIcon(":/icons/resources/icons/alert.svg").pixmap(18, 18));
    weakIcon->setStyleSheet("background: transparent;");

    QHBoxLayout *iconLayout = new QHBoxLayout(iconBg);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(weakIcon, 0, Qt::AlignCenter);

    QLabel *weakTitleLabel = new QLabel("重点关注薄弱知识点");
    weakTitleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: #EA580C; background: transparent;"
    ));

    weakTitleRow->addWidget(iconBg);
    weakTitleRow->addSpacing(8);
    weakTitleRow->addWidget(weakTitleLabel);
    weakTitleRow->addStretch();

    QChart *chart = new QChart();
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(false);
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(140, 0, 40, 10));

    m_weakPointsChartView = new QChartView(chart);
    m_weakPointsChartView->setRenderHint(QPainter::Antialiasing);
    m_weakPointsChartView->setStyleSheet("background: transparent; border: none;");

    layout->addWidget(titleContainer);
    layout->addWidget(m_weakPointsChartView);

    return chartFrame;
}

void ClassAnalyticsPage::createAIAdviceArea()
{
    m_adviceFrame = new QFrame();
    m_adviceFrame->setObjectName("classAdviceFrame");
    m_adviceFrame->setMinimumHeight(150);
    m_adviceFrame->setStyleSheet(QString(
        "QFrame#classAdviceFrame {"
        "    background-color: #F0F7FF;"
        "    border-radius: 12px;"
        "    border: 1px solid #E0E7FF;"
        "}"
    ));

    QVBoxLayout *layout = new QVBoxLayout(m_adviceFrame);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(20);

    QWidget *titleContainer = new QWidget();
    QHBoxLayout *titleRow = new QHBoxLayout(titleContainer);
    titleRow->setContentsMargins(0, 0, 0, 0);

    QLabel *iconBg = new QLabel();
    iconBg->setFixedSize(36, 36);
    iconBg->setStyleSheet(QString(
        "background-color: #E0E7FF; border-radius: 8px;"
    ));
    iconBg->setAlignment(Qt::AlignCenter);

    QLabel *aiIcon = new QLabel(iconBg);
    aiIcon->setPixmap(QIcon(":/icons/resources/icons/robot.svg").pixmap(20, 20));
    aiIcon->setStyleSheet("background: transparent; color: #4F46E5;");

    QHBoxLayout *iconLayout = new QHBoxLayout(iconBg);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(aiIcon, 0, Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("AI 智能教学分析助手");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 700; color: #1E293B; background: transparent;"
    ));

    m_generateAdviceBtn = new QPushButton("开启智能诊断");
    m_generateAdviceBtn->setFixedHeight(40);
    m_generateAdviceBtn->setCursor(Qt::PointingHandCursor);
    m_generateAdviceBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: #FFFFFF;"
        "    color: #4F46E5;"
        "    border: 1px solid #C7D2FE;"
        "    border-radius: 8px;"
        "    padding: 0 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #F8FAFC; border-color: #A5B4FC; }"
        "QPushButton:pressed { background-color: #F1F5F9; }"
        "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }"
    ));

    titleRow->addWidget(iconBg);
    titleRow->addSpacing(12);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(m_generateAdviceBtn);

    m_adviceContent = new QLabel("点击「开启智能诊断」，AI 将为您生成针对性的班级学情改进方案。");
    m_adviceContent->setWordWrap(true);
    m_adviceContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_adviceContent->setStyleSheet(QString(
        "color: #475569; font-size: 15px; line-height: 1.6; "
        "background-color: transparent; padding: 0;"
    ));

    layout->addWidget(titleContainer);
    layout->addWidget(m_adviceContent);
    layout->addStretch();

    m_mainLayout->addWidget(m_adviceFrame);
}

void ClassAnalyticsPage::onClassChanged(int index)
{
    if (index < 0 || !m_dataSource) return;
    m_currentClassId = m_classCombo->itemData(index).toInt();
    emit classChanged(m_currentClassId);
    updateCharts();
    updateRankingTable();
    updateWeakPointsChart();
}

void ClassAnalyticsPage::updateCharts()
{
    if (!m_dataSource || m_currentClassId < 0) return;

    auto stats = m_dataSource->getClassStatistics(m_currentClassId);
    m_gradeBarSet->replace(0, stats.excellentCount());
    m_gradeBarSet->replace(1, stats.goodCount());
    m_gradeBarSet->replace(2, stats.passCount());
    m_gradeBarSet->replace(3, stats.failCount());
}

void ClassAnalyticsPage::updateRankingTable()
{
    if (!m_dataSource || m_currentClassId < 0) return;

    auto ranking = m_dataSource->getClassRanking(m_currentClassId);
    int showCount = qMin(ranking.size(), 10);

    m_rankingTable->setRowCount(showCount);
    for (int i = 0; i < showCount; ++i) {
        const auto &pair = ranking[i];
        m_rankingTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_rankingTable->setItem(i, 1, new QTableWidgetItem(pair.first.name()));
        m_rankingTable->setItem(i, 2, new QTableWidgetItem(pair.first.studentNo()));
        m_rankingTable->setItem(i, 3, new QTableWidgetItem(QString::number(pair.second, 'f', 1)));

        if (i < 3) {
            QString rankColor;
            if (i == 0) rankColor = StyleConfig::RANK_GOLD;
            else if (i == 1) rankColor = StyleConfig::RANK_SILVER;
            else rankColor = StyleConfig::RANK_BRONZE;

            for (int col = 0; col < 4; ++col) {
                QTableWidgetItem *item = m_rankingTable->item(i, col);
                if (item) {
                    item->setBackground(QColor(rankColor).lighter(190));
                    item->setForeground(QColor(rankColor).darker(110));
                    if (col == 1 || col == 3) {
                        QFont font = item->font();
                        font.setBold(true);
                        item->setFont(font);
                    }
                }
            }
        } else {
            for (int col = 0; col < 4; ++col) {
                QTableWidgetItem *item = m_rankingTable->item(i, col);
                if (item) {
                    item->setForeground(QColor(StyleConfig::SLATE_TEXT));
                }
            }
        }
    }
}

void ClassAnalyticsPage::updateWeakPointsChart()
{
    if (!m_dataSource || m_currentClassId < 0) return;

    auto kps = m_dataSource->getClassKnowledgePoints(m_currentClassId);

    QChart *chart = m_weakPointsChartView->chart();
    chart->removeAllSeries();

    for (auto axis : chart->axes()) {
        chart->removeAxis(axis);
    }

    int showCount = qMin(kps.size(), 5);
    QBarSet *barSet = new QBarSet("掌握率");

    QLinearGradient gradient(0, 0, 1, 0);
    gradient.setColorAt(0.0, QColor("#FDBA74"));
    gradient.setColorAt(1.0, QColor("#F97316"));
    gradient.setCoordinateMode(QGradient::ObjectMode);
    barSet->setBrush(QBrush(gradient));
    barSet->setPen(Qt::NoPen);

    QStringList categories;
    for (int i = 0; i < showCount; ++i) {
        *barSet << kps[i].masteryRate();
        QString name = kps[i].name();
        if (name.length() > 10) name = name.left(9) + "...";
        categories << name;
    }

    QHorizontalBarSeries *series = new QHorizontalBarSeries();
    series->append(barSet);
    series->setLabelsVisible(true);
    series->setLabelsFormat("@value%");
    series->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);
    chart->addSeries(series);

    QBarCategoryAxis *axisY = new QBarCategoryAxis();
    axisY->append(categories);
    axisY->setLabelsColor(QColor("#64748B"));
    axisY->setGridLineVisible(false);
    axisY->setLineVisible(false);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0, 100);
    axisX->setLabelFormat("%d%%");
    axisX->setLabelsColor(QColor("#94A3B8"));
    axisX->setGridLineColor(QColor("#F1F5F9"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    chart->setMargins(QMargins(140, 0, 40, 10));
}

void ClassAnalyticsPage::onGenerateAdviceClicked()
{
    if (!m_difyService || m_currentClassId < 0 || m_isGenerating) return;

    m_isGenerating = true;
    m_currentAdvice.clear();
    m_generateAdviceBtn->setEnabled(false);
    m_generateAdviceBtn->setText("诊断中...");
    m_adviceContent->setText("正在利用 AI 深度分析班级数据，请稍候...");

    auto stats = m_dataSource->getClassStatistics(m_currentClassId);
    auto kps = m_dataSource->getClassKnowledgePoints(m_currentClassId);
    auto cls = m_dataSource->getClass(m_currentClassId);

    QString weakPoints;
    for (int i = 0; i < qMin(kps.size(), 3); ++i) {
        weakPoints += kps[i].name() + "(" + QString::number(kps[i].masteryRate(), 'f', 1) + "%)、";
    }

    QString prompt = QString(
        "请为以下班级生成教学改进建议：\n\n"
        "班级：%1\n"
        "学生人数：%2\n"
        "平均分：%3\n"
        "及格率：%4%\n"
        "薄弱知识点：%5\n\n"
        "请给出3-5条针对性的教学改进建议。"
    ).arg(cls.name())
     .arg(stats.totalStudents())
     .arg(QString::number(stats.averageScore(), 'f', 1))
     .arg(QString::number(stats.passRate(), 'f', 1))
     .arg(weakPoints);

    m_difyService->sendMessage(prompt);
}

void ClassAnalyticsPage::onAIStreamChunk(const QString &chunk)
{
    m_currentAdvice += chunk;
    m_adviceContent->setText(m_currentAdvice);
}

void ClassAnalyticsPage::onAIRequestFinished()
{
    m_isGenerating = false;
    m_generateAdviceBtn->setEnabled(true);
    m_generateAdviceBtn->setText("重新诊断");
}

void ClassAnalyticsPage::onExportClicked()
{
    QMessageBox::information(this, "提示", "班级报告导出功能开发中...");
}
