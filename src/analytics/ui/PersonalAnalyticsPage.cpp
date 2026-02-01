#include "PersonalAnalyticsPage.h"
#include "../datasources/MockDataSource.h"
#include "../models/Student.h"
#include "../models/ScoreRecord.h"
#include "../models/KnowledgePoint.h"
#include "../../services/DifyService.h"
#include "../../shared/StyleConfig.h"
#include "RadarChartWidget.h"

#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

PersonalAnalyticsPage::PersonalAnalyticsPage(QWidget *parent)
    : QWidget(parent)
    , m_dataSource(nullptr)
    , m_difyService(nullptr)
    , m_currentStudentId(-1)
    , m_currentClassId(-1)
    , m_isGenerating(false)
    , m_trendChartView(nullptr)
    , m_scoreSeries(nullptr)
    , m_classAvgSeries(nullptr)
    , m_radarWidget(nullptr)
{
    setupUI();
    setupStyles();
}

PersonalAnalyticsPage::~PersonalAnalyticsPage()
{
}

void PersonalAnalyticsPage::setDataSource(MockDataSource *dataSource)
{
    m_dataSource = dataSource;
    if (m_dataSource) {
        // 加载班级列表
        auto classes = m_dataSource->getClassList();
        m_classCombo->clear();
        for (const auto &cls : classes) {
            m_classCombo->addItem(cls.name(), cls.id());
        }
        if (!classes.isEmpty()) {
            m_currentClassId = classes.first().id();
            updateStudentList();
        }
    }
}

void PersonalAnalyticsPage::setDifyService(DifyService *service)
{
    if (m_difyService) {
        disconnect(m_difyService, nullptr, this, nullptr);
    }
    m_difyService = service;
    if (m_difyService) {
        connect(m_difyService, &DifyService::streamChunkReceived,
                this, &PersonalAnalyticsPage::onAIStreamChunk);
        connect(m_difyService, &DifyService::requestFinished,
                this, &PersonalAnalyticsPage::onAIRequestFinished);
    }
}

void PersonalAnalyticsPage::refresh()
{
    updateCharts();
    updateRadarChart();
}

void PersonalAnalyticsPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(24, 20, 24, 24);
    m_mainLayout->setSpacing(20);

    createHeader();
    createScoreTrendChart();
    createKnowledgeRadarChart();
    createAIAdviceArea();

    m_mainLayout->addStretch();
}

void PersonalAnalyticsPage::setupStyles()
{
    setStyleSheet(QString(
        "PersonalAnalyticsPage {"
        "    background: transparent;"
        "}"
    ));
}

void PersonalAnalyticsPage::createHeader()
{
    m_headerFrame = new QFrame();
    m_headerFrame->setObjectName("personalHeader");
    m_headerFrame->setFixedHeight(60);
    m_headerFrame->setStyleSheet(QString(
        "QFrame#personalHeader {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    headerLayout->setSpacing(16);

    QLabel *titleLabel = new QLabel("个人学情分析");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    // 班级选择器
    QLabel *classLabel = new QLabel("班级:");
    classLabel->setStyleSheet("color: #718096; font-size: 14px; background: transparent;");
    m_classCombo = new QComboBox();
    m_classCombo->setFixedWidth(140);
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
            this, &PersonalAnalyticsPage::onClassChanged);

    // 学生选择器
    QLabel *studentLabel = new QLabel("学生:");
    studentLabel->setStyleSheet("color: #718096; font-size: 14px; background: transparent;");
    m_studentCombo = new QComboBox();
    m_studentCombo->setFixedWidth(160);
    m_studentCombo->setStyleSheet(m_classCombo->styleSheet());
    connect(m_studentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PersonalAnalyticsPage::onStudentChanged);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(classLabel);
    headerLayout->addWidget(m_classCombo);
    headerLayout->addWidget(studentLabel);
    headerLayout->addWidget(m_studentCombo);

    m_mainLayout->addWidget(m_headerFrame);
}

void PersonalAnalyticsPage::createScoreTrendChart()
{
    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("trendChartFrame");
    chartFrame->setMinimumHeight(280);
    chartFrame->setStyleSheet(QString(
        "QFrame#trendChartFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *layout = new QVBoxLayout(chartFrame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    // 标题
    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon(":/icons/resources/icons/analytics.svg").pixmap(16, 16));
    iconLabel->setStyleSheet("background: transparent;");
    QLabel *titleLabel = new QLabel("成绩趋势");
    titleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    titleRow->addWidget(iconLabel);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();

    // 创建折线图
    m_scoreSeries = new QLineSeries();
    m_scoreSeries->setName("个人成绩");
    m_scoreSeries->setColor(QColor(StyleConfig::PATRIOTIC_RED));
    QPen pen1(QColor(StyleConfig::PATRIOTIC_RED), 2);
    m_scoreSeries->setPen(pen1);

    m_classAvgSeries = new QLineSeries();
    m_classAvgSeries->setName("班级平均");
    m_classAvgSeries->setColor(QColor("#718096"));
    QPen pen2(QColor("#718096"), 2, Qt::DashLine);
    m_classAvgSeries->setPen(pen2);

    QChart *chart = new QChart();
    chart->addSeries(m_scoreSeries);
    chart->addSeries(m_classAvgSeries);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));

    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0, 9);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("考试次数");
    axisX->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    chart->addAxis(axisX, Qt::AlignBottom);
    m_scoreSeries->attachAxis(axisX);
    m_classAvgSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(40, 100);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("分数");
    axisY->setLabelsColor(QColor(StyleConfig::TEXT_SECONDARY));
    axisY->setGridLineVisible(true);
    axisY->setGridLineColor(QColor("#E2E8F0"));
    chart->addAxis(axisY, Qt::AlignLeft);
    m_scoreSeries->attachAxis(axisY);
    m_classAvgSeries->attachAxis(axisY);

    m_trendChartView = new QChartView(chart);
    m_trendChartView->setRenderHint(QPainter::Antialiasing);
    m_trendChartView->setStyleSheet("background: transparent; border: none;");

    layout->addLayout(titleRow);
    layout->addWidget(m_trendChartView);

    m_mainLayout->addWidget(chartFrame);
}

void PersonalAnalyticsPage::createKnowledgeRadarChart()
{
    QFrame *radarFrame = new QFrame();
    radarFrame->setObjectName("radarChartFrame");
    radarFrame->setMinimumHeight(300);
    radarFrame->setStyleSheet(QString(
        "QFrame#radarChartFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *layout = new QVBoxLayout(radarFrame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QHBoxLayout *radarTitleRow = new QHBoxLayout();
    QLabel *radarIcon = new QLabel();
    radarIcon->setPixmap(QIcon(":/icons/resources/icons/award.svg").pixmap(16, 16));
    radarIcon->setStyleSheet("background: transparent;");
    QLabel *titleLabel = new QLabel("知识点掌握度");
    titleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    radarTitleRow->addWidget(radarIcon);
    radarTitleRow->addWidget(titleLabel);
    radarTitleRow->addStretch();

    // 使用自定义雷达图组件
    m_radarWidget = new RadarChartWidget();
    m_radarWidget->setMinimumHeight(250);

    layout->addLayout(radarTitleRow);
    layout->addWidget(m_radarWidget);

    m_mainLayout->addWidget(radarFrame);
}

void PersonalAnalyticsPage::createAIAdviceArea()
{
    m_adviceFrame = new QFrame();
    m_adviceFrame->setObjectName("adviceFrame");
    m_adviceFrame->setMinimumHeight(150);
    m_adviceFrame->setStyleSheet(QString(
        "QFrame#adviceFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    QVBoxLayout *layout = new QVBoxLayout(m_adviceFrame);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    // 标题行
    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *aiIcon = new QLabel();
    aiIcon->setPixmap(QIcon(":/icons/resources/icons/robot.svg").pixmap(16, 16));
    aiIcon->setStyleSheet("background: transparent;");
    QLabel *titleLabel = new QLabel("AI 个性化学习建议");
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
            this, &PersonalAnalyticsPage::onGenerateAdviceClicked);

    titleRow->addWidget(aiIcon);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(m_generateAdviceBtn);

    // 建议内容
    m_adviceContent = new QLabel("选择学生后，点击「生成建议」获取AI个性化学习建议。");
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

void PersonalAnalyticsPage::onClassChanged(int index)
{
    if (index < 0 || !m_dataSource) return;
    m_currentClassId = m_classCombo->itemData(index).toInt();
    updateStudentList();
}

void PersonalAnalyticsPage::onStudentChanged(int index)
{
    if (index < 0 || !m_dataSource) return;
    m_currentStudentId = m_studentCombo->itemData(index).toInt();
    emit studentChanged(m_currentStudentId);
    updateCharts();
    updateRadarChart();
}

void PersonalAnalyticsPage::updateStudentList()
{
    if (!m_dataSource) return;

    m_studentCombo->blockSignals(true);
    m_studentCombo->clear();

    auto students = m_dataSource->getStudentList(m_currentClassId);
    for (const auto &student : students) {
        m_studentCombo->addItem(student.displayName(), student.id());
    }

    m_studentCombo->blockSignals(false);

    if (!students.isEmpty()) {
        m_currentStudentId = students.first().id();
        updateCharts();
        updateRadarChart();
    }
}

void PersonalAnalyticsPage::updateCharts()
{
    if (!m_dataSource || m_currentStudentId < 0) return;

    // 获取学生成绩
    auto scores = m_dataSource->getStudentScores(m_currentStudentId);
    m_scoreSeries->clear();
    for (int i = 0; i < scores.size() && i < 10; ++i) {
        m_scoreSeries->append(i, scores[i].score());
    }

    // 获取班级平均成绩（简化处理）
    m_classAvgSeries->clear();
    auto classScores = m_dataSource->getClassScores(m_currentClassId);
    double avgSum = 0;
    int avgCount = 0;
    for (const auto &record : classScores) {
        avgSum += record.score();
        avgCount++;
    }
    double classAvg = avgCount > 0 ? avgSum / avgCount : 70;
    for (int i = 0; i < 10; ++i) {
        m_classAvgSeries->append(i, classAvg + (QRandomGenerator::global()->generateDouble() - 0.5) * 5);
    }
}

void PersonalAnalyticsPage::updateRadarChart()
{
    if (!m_dataSource || m_currentStudentId < 0) return;

    auto kps = m_dataSource->getStudentKnowledgePoints(m_currentStudentId);
    RadarChartWidget *radar = qobject_cast<RadarChartWidget*>(m_radarWidget);
    if (radar) {
        QVector<QPair<QString, double>> data;
        for (const auto &kp : kps) {
            data.append(qMakePair(kp.name(), kp.masteryRate()));
        }
        radar->setData(data);
    }
}

void PersonalAnalyticsPage::onGenerateAdviceClicked()
{
    if (!m_difyService || m_currentStudentId < 0 || m_isGenerating) return;

    m_isGenerating = true;
    m_currentAdvice.clear();
    m_generateAdviceBtn->setEnabled(false);
    m_generateAdviceBtn->setText("生成中...");
    m_adviceContent->setText("正在分析学生数据，生成个性化建议...");

    // 构建提示词
    auto student = m_dataSource->getStudent(m_currentStudentId);
    auto scores = m_dataSource->getStudentScores(m_currentStudentId);
    auto kps = m_dataSource->getStudentKnowledgePoints(m_currentStudentId);

    QString prompt = QString(
        "请为以下学生生成个性化学习建议：\n\n"
        "学生：%1\n"
        "最近成绩：%2分\n"
        "薄弱知识点：%3\n\n"
        "请给出3-5条具体、可操作的学习建议，帮助学生提高思政课成绩。"
    ).arg(student.name())
     .arg(scores.isEmpty() ? 0 : scores.last().score())
     .arg(kps.isEmpty() ? "无" : kps.first().name());

    m_difyService->sendMessage(prompt);
}

void PersonalAnalyticsPage::onAIStreamChunk(const QString &chunk)
{
    m_currentAdvice += chunk;
    m_adviceContent->setText(m_currentAdvice);
}

void PersonalAnalyticsPage::onAIRequestFinished()
{
    m_isGenerating = false;
    m_generateAdviceBtn->setEnabled(true);
    m_generateAdviceBtn->setText("重新生成");
}
