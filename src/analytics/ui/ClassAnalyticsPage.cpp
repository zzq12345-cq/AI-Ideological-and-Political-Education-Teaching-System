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

void ClassAnalyticsPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(24, 20, 24, 24);
    m_mainLayout->setSpacing(20);

    createHeader();

    // 上部分：成绩分布 + 排名表格
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(20);

    createDistributionChart();
    createRankingTable();

    // 获取分布图和排名表的frame
    QFrame *distFrame = qobject_cast<QFrame*>(m_distributionChartView->parentWidget());
    QFrame *rankFrame = qobject_cast<QFrame*>(m_rankingTable->parentWidget());
    if (distFrame) topRow->addWidget(distFrame, 1);
    if (rankFrame) topRow->addWidget(rankFrame, 1);

    m_mainLayout->addLayout(topRow);

    createWeakPointsChart();
    createAIAdviceArea();

    m_mainLayout->addStretch();
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
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    headerLayout->setSpacing(16);

    QLabel *titleLabel = new QLabel("班级整体分析");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    QLabel *classLabel = new QLabel("班级:");
    classLabel->setStyleSheet("color: #718096; font-size: 14px; background: transparent;");
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

    m_exportBtn = new QPushButton("导出报告");
    m_exportBtn->setFixedHeight(34);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #718096;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 17px;"
        "    padding: 0 16px;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover { background: #F7FAFC; border-color: #CBD5E0; }"
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
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *layout = new QVBoxLayout(chartFrame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QHBoxLayout *distTitleRow = new QHBoxLayout();
    QLabel *distIcon = new QLabel();
    distIcon->setPixmap(QIcon(":/icons/resources/icons/menu.svg").pixmap(16, 16));
    distIcon->setStyleSheet("background: transparent;");
    QLabel *titleLabel = new QLabel("成绩分布");
    titleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    distTitleRow->addWidget(distIcon);
    distTitleRow->addWidget(titleLabel);
    distTitleRow->addStretch();

    m_gradeBarSet = new QBarSet("人数");
    *m_gradeBarSet << 0 << 0 << 0 << 0;
    m_gradeBarSet->setColor(QColor("#4299E1"));

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

    layout->addLayout(distTitleRow);
    layout->addWidget(m_distributionChartView);
}

void ClassAnalyticsPage::createRankingTable()
{
    QFrame *tableFrame = new QFrame();
    tableFrame->setObjectName("rankingFrame");
    tableFrame->setMinimumHeight(280);
    tableFrame->setStyleSheet(QString(
        "QFrame#rankingFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *layout = new QVBoxLayout(tableFrame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QHBoxLayout *rankTitleRow = new QHBoxLayout();
    QLabel *rankIcon = new QLabel();
    rankIcon->setPixmap(QIcon(":/icons/resources/icons/award.svg").pixmap(16, 16));
    rankIcon->setStyleSheet("background: transparent;");
    QLabel *rankTitleLabel = new QLabel("学生排名 (Top 10)");
    rankTitleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    rankTitleRow->addWidget(rankIcon);
    rankTitleRow->addWidget(rankTitleLabel);
    rankTitleRow->addStretch();

    m_rankingTable = new QTableWidget();
    m_rankingTable->setColumnCount(4);
    m_rankingTable->setHorizontalHeaderLabels({"排名", "姓名", "学号", "平均分"});
    m_rankingTable->horizontalHeader()->setStretchLastSection(true);
    m_rankingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_rankingTable->verticalHeader()->setVisible(false);
    m_rankingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_rankingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_rankingTable->setAlternatingRowColors(true);
    m_rankingTable->setStyleSheet(
        "QTableWidget {"
        "    background: white;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 8px;"
        "    gridline-color: #E2E8F0;"
        "}"
        "QTableWidget::item { padding: 8px; }"
        "QHeaderView::section {"
        "    background: #F7FAFC;"
        "    color: #4A5568;"
        "    font-weight: 600;"
        "    padding: 8px;"
        "    border: none;"
        "    border-bottom: 1px solid #E2E8F0;"
        "}"
    );

    layout->addLayout(rankTitleRow);
    layout->addWidget(m_rankingTable);
}

void ClassAnalyticsPage::createWeakPointsChart()
{
    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("weakPointsFrame");
    chartFrame->setMinimumHeight(200);
    chartFrame->setStyleSheet(QString(
        "QFrame#weakPointsFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *layout = new QVBoxLayout(chartFrame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QHBoxLayout *weakTitleRow = new QHBoxLayout();
    QLabel *weakIcon = new QLabel();
    weakIcon->setPixmap(QIcon(":/icons/resources/icons/alert.svg").pixmap(16, 16));
    weakIcon->setStyleSheet("background: transparent;");
    QLabel *weakTitleLabel = new QLabel("薄弱知识点分析");
    weakTitleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    weakTitleRow->addWidget(weakIcon);
    weakTitleRow->addWidget(weakTitleLabel);
    weakTitleRow->addStretch();

    // 使用水平条形图显示知识点掌握度
    QChart *chart = new QChart();
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(false);
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));

    m_weakPointsChartView = new QChartView(chart);
    m_weakPointsChartView->setRenderHint(QPainter::Antialiasing);
    m_weakPointsChartView->setStyleSheet("background: transparent; border: none;");

    layout->addLayout(weakTitleRow);
    layout->addWidget(m_weakPointsChartView);

    m_mainLayout->addWidget(chartFrame);
}

void ClassAnalyticsPage::createAIAdviceArea()
{
    m_adviceFrame = new QFrame();
    m_adviceFrame->setObjectName("classAdviceFrame");
    m_adviceFrame->setMinimumHeight(150);
    m_adviceFrame->setStyleSheet(QString(
        "QFrame#classAdviceFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *layout = new QVBoxLayout(m_adviceFrame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *aiIcon = new QLabel();
    aiIcon->setPixmap(QIcon(":/icons/resources/icons/robot.svg").pixmap(16, 16));
    aiIcon->setStyleSheet("background: transparent;");
    QLabel *titleLabel = new QLabel("AI 班级教学建议");
    titleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    m_generateAdviceBtn = new QPushButton("生成建议");
    m_generateAdviceBtn->setFixedHeight(32);
    m_generateAdviceBtn->setCursor(Qt::PointingHandCursor);
    m_generateAdviceBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 16px;"
        "    padding: 0 20px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover { background: #C62828; }"
        "QPushButton:disabled { background: #CBD5E0; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(m_generateAdviceBtn, &QPushButton::clicked,
            this, &ClassAnalyticsPage::onGenerateAdviceClicked);

    titleRow->addWidget(aiIcon);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(m_generateAdviceBtn);

    m_adviceContent = new QLabel("选择班级后，点击「生成建议」获取AI班级教学建议。");
    m_adviceContent->setWordWrap(true);
    m_adviceContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_adviceContent->setStyleSheet(QString(
        "color: %1; font-size: 14px; line-height: 1.6; background: transparent; padding: 8px 0;"
    ).arg(StyleConfig::TEXT_SECONDARY));

    layout->addLayout(titleRow);
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

        // 前三名特殊样式
        if (i < 3) {
            for (int col = 0; col < 4; ++col) {
                QTableWidgetItem *item = m_rankingTable->item(i, col);
                if (item) {
                    item->setBackground(QColor("#FFFAF0"));
                    item->setForeground(QColor("#DD6B20"));
                }
            }
        }
    }
}

void ClassAnalyticsPage::updateWeakPointsChart()
{
    if (!m_dataSource || m_currentClassId < 0) return;

    auto kps = m_dataSource->getClassKnowledgePoints(m_currentClassId);

    // 重新创建图表
    QChart *chart = m_weakPointsChartView->chart();
    chart->removeAllSeries();

    // 只显示前5个薄弱知识点
    int showCount = qMin(kps.size(), 5);
    QBarSet *barSet = new QBarSet("掌握率");
    barSet->setColor(QColor(StyleConfig::PATRIOTIC_RED));

    QStringList categories;
    for (int i = 0; i < showCount; ++i) {
        *barSet << kps[i].masteryRate();
        QString name = kps[i].name();
        if (name.length() > 8) name = name.left(7) + "...";
        categories << name;
    }

    QHorizontalBarSeries *series = new QHorizontalBarSeries();
    series->append(barSet);
    series->setLabelsVisible(true);
    series->setLabelsFormat("@value%");
    chart->addSeries(series);

    // 重新设置坐标轴
    chart->createDefaultAxes();
    if (!chart->axes(Qt::Horizontal).isEmpty()) {
        QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
        if (axisX) {
            axisX->setRange(0, 100);
            axisX->setLabelFormat("%d%%");
        }
    }
}

void ClassAnalyticsPage::onGenerateAdviceClicked()
{
    if (!m_difyService || m_currentClassId < 0 || m_isGenerating) return;

    m_isGenerating = true;
    m_currentAdvice.clear();
    m_generateAdviceBtn->setEnabled(false);
    m_generateAdviceBtn->setText("生成中...");
    m_adviceContent->setText("正在分析班级数据，生成教学建议...");

    auto stats = m_dataSource->getClassStatistics(m_currentClassId);
    auto kps = m_dataSource->getClassKnowledgePoints(m_currentClassId);
    auto cls = m_dataSource->getClass(m_currentClassId);

    QString weakPoints;
    for (int i = 0; i < qMin(kps.size(), 3); ++i) {
        weakPoints += kps[i].name() + "(" + QString::number(kps[i].masteryRate(), 'f', 0) + "%)、";
    }

    QString prompt = QString(
        "请为以下班级生成教学改进建议：\n\n"
        "班级：%1\n"
        "学生人数：%2\n"
        "平均分：%3\n"
        "及格率：%4%\n"
        "薄弱知识点：%5\n\n"
        "请给出3-5条针对性的教学改进建议，帮助提升班级整体思政课成绩。"
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
    m_generateAdviceBtn->setText("重新生成");
}

void ClassAnalyticsPage::onExportClicked()
{
    QMessageBox::information(this, "提示", "班级报告导出功能开发中...");
}
