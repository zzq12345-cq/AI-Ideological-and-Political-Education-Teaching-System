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
    // 学习班级分析页面的布局方式，用 ScrollArea 包装避免重叠
    QWidget *scrollContent = new QWidget();
    scrollContent->setObjectName("scrollContent");
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(32, 32, 32, 32);
    contentLayout->setSpacing(32);

    // 主布局只放 ScrollArea
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    createHeader();
    contentLayout->addWidget(m_headerFrame);

    createScoreTrendChart();
    // 通过 parentWidget 获取趋势图的 Frame
    QFrame *trendFrame = qobject_cast<QFrame*>(m_trendChartView->parentWidget());
    if (trendFrame) contentLayout->addWidget(trendFrame);

    createKnowledgeRadarChart();
    // 通过 parentWidget 获取雷达图的 Frame
    QFrame *radarFrame = qobject_cast<QFrame*>(m_radarWidget->parentWidget());
    if (radarFrame) contentLayout->addWidget(radarFrame);

    createAIAdviceArea();
    contentLayout->addWidget(m_adviceFrame);

    contentLayout->addStretch();

    // 包装进 ScrollArea，再也不会重叠了
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

    m_mainLayout->addWidget(scrollArea);
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
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 #FFFFFF);"
        "    border-radius: 16px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED_TINT, StyleConfig::BORDER_LIGHT));

    // 给Header加点阴影，显得高级
    QGraphicsDropShadowEffect *headerShadow = new QGraphicsDropShadowEffect(this);
    headerShadow->setBlurRadius(20);
    headerShadow->setColor(QColor(0, 0, 0, 20));
    headerShadow->setOffset(0, 4);
    m_headerFrame->setGraphicsEffect(headerShadow);

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
    // 不在这里添加到布局，由 setupUI 统一管理
}

void PersonalAnalyticsPage::createScoreTrendChart()
{
    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("trendChartFrame");
    chartFrame->setMinimumHeight(280);
    chartFrame->setStyleSheet(QString(
        "QFrame#trendChartFrame {"
        "    background-color: white;"
        "    border-radius: 16px;"
        "    border: 1px solid %1;"
        "}"
    ).arg(StyleConfig::BORDER_LIGHT));

    // 阴影是必须的
    QGraphicsDropShadowEffect *chartShadow = new QGraphicsDropShadowEffect(this);
    chartShadow->setBlurRadius(25);
    chartShadow->setColor(QColor(0, 0, 0, 15));
    chartShadow->setOffset(0, 6);
    chartFrame->setGraphicsEffect(chartShadow);

    QVBoxLayout *layout = new QVBoxLayout(chartFrame);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    // 标题行容器
    QWidget *titleContainer = new QWidget();
    QHBoxLayout *titleRow = new QHBoxLayout(titleContainer);
    titleRow->setContentsMargins(0, 0, 0, 0);

    // 图标装饰背景
    QLabel *iconBg = new QLabel();
    iconBg->setFixedSize(32, 32);
    iconBg->setStyleSheet(QString(
        "background-color: %1; border-radius: 8px;"
    ).arg(StyleConfig::PATRIOTIC_RED_TINT));
    iconBg->setAlignment(Qt::AlignCenter);

    QLabel *iconLabel = new QLabel(iconBg);
    iconLabel->setPixmap(QIcon(":/icons/resources/icons/analytics.svg").pixmap(18, 18));
    iconLabel->setStyleSheet("background: transparent;");

    // 把iconLabel放进iconBg的正中心
    QHBoxLayout *iconLayout = new QHBoxLayout(iconBg);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(iconLabel, 0, Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("成绩趋势分析");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 800; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    titleRow->addWidget(iconBg);
    titleRow->addSpacing(8);
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

    layout->addWidget(titleContainer);
    layout->addWidget(m_trendChartView);
    // 不在这里添加到布局，由 setupUI 统一管理
}

void PersonalAnalyticsPage::createKnowledgeRadarChart()
{
    QFrame *radarFrame = new QFrame();
    radarFrame->setObjectName("radarChartFrame");
    radarFrame->setMinimumHeight(300);
    radarFrame->setStyleSheet(QString(
        "QFrame#radarChartFrame {"
        "    background-color: white;"
        "    border-radius: 16px;"
        "    border: 1px solid %1;"
        "}"
    ).arg(StyleConfig::BORDER_LIGHT));

    QGraphicsDropShadowEffect *radarShadow = new QGraphicsDropShadowEffect(this);
    radarShadow->setBlurRadius(25);
    radarShadow->setColor(QColor(0, 0, 0, 15));
    radarShadow->setOffset(0, 6);
    radarFrame->setGraphicsEffect(radarShadow);

    QVBoxLayout *layout = new QVBoxLayout(radarFrame);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    QWidget *titleContainer = new QWidget();
    QHBoxLayout *radarTitleRow = new QHBoxLayout(titleContainer);
    radarTitleRow->setContentsMargins(0, 0, 0, 0);

    QLabel *iconBg = new QLabel();
    iconBg->setFixedSize(32, 32);
    iconBg->setStyleSheet(QString(
        "background-color: %1; border-radius: 8px;"
    ).arg(StyleConfig::PATRIOTIC_RED_TINT));
    iconBg->setAlignment(Qt::AlignCenter);

    QLabel *radarIcon = new QLabel(iconBg);
    radarIcon->setPixmap(QIcon(":/icons/resources/icons/award.svg").pixmap(18, 18));
    radarIcon->setStyleSheet("background: transparent;");

    QHBoxLayout *iconLayout = new QHBoxLayout(iconBg);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(radarIcon, 0, Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("知识掌握多维分析");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 800; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    radarTitleRow->addWidget(iconBg);
    radarTitleRow->addSpacing(8);
    radarTitleRow->addWidget(titleLabel);
    radarTitleRow->addStretch();

    // 使用自定义雷达图组件
    m_radarWidget = new RadarChartWidget();
    m_radarWidget->setMinimumHeight(250);

    layout->addWidget(titleContainer);
    layout->addWidget(m_radarWidget);
    // 不在这里添加到布局，由 setupUI 统一管理
}

void PersonalAnalyticsPage::createAIAdviceArea()
{
    m_adviceFrame = new QFrame();
    m_adviceFrame->setObjectName("adviceFrame");
    m_adviceFrame->setMinimumHeight(150);
    m_adviceFrame->setStyleSheet(QString(
        "QFrame#adviceFrame {"
        "    background-color: #F8FAFC;"
        "    border-radius: 16px;"
        "    border: 1px solid %1;"
        "}"
    ).arg(StyleConfig::BORDER_LIGHT));

    QGraphicsDropShadowEffect *adviceShadow = new QGraphicsDropShadowEffect(this);
    adviceShadow->setBlurRadius(20);
    adviceShadow->setColor(QColor(0, 0, 0, 10));
    adviceShadow->setOffset(0, 4);
    m_adviceFrame->setGraphicsEffect(adviceShadow);

    QVBoxLayout *layout = new QVBoxLayout(m_adviceFrame);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    // 标题行
    QWidget *titleContainer = new QWidget();
    QHBoxLayout *titleRow = new QHBoxLayout(titleContainer);
    titleRow->setContentsMargins(0, 0, 0, 0);

    QLabel *iconBg = new QLabel();
    iconBg->setFixedSize(32, 32);
    iconBg->setStyleSheet(QString(
        "background-color: #EBF8FF; border-radius: 8px;"
    ));
    iconBg->setAlignment(Qt::AlignCenter);

    QLabel *aiIcon = new QLabel(iconBg);
    aiIcon->setPixmap(QIcon(":/icons/resources/icons/robot.svg").pixmap(18, 18));
    aiIcon->setStyleSheet("background: transparent;");

    QHBoxLayout *iconLayout = new QHBoxLayout(iconBg);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(aiIcon, 0, Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("AI 个性化学情诊断");
    titleLabel->setStyleSheet(QString(
        "font-size: 18px; font-weight: 800; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    m_generateAdviceBtn = new QPushButton("开始分析");
    m_generateAdviceBtn->setFixedHeight(36);
    m_generateAdviceBtn->setCursor(Qt::PointingHandCursor);
    m_generateAdviceBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 #EF5350);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 18px;"
        "    padding: 0 24px;"
        "    font-size: 14px;"
        "    font-weight: 700;"
        "}"
        "QPushButton:hover { background: %2; }"
        "QPushButton:pressed { background: %3; }"
        "QPushButton:disabled { background: #CBD5E0; }"
    ).arg(StyleConfig::PATRIOTIC_RED, StyleConfig::PATRIOTIC_RED_DARK, StyleConfig::PATRIOTIC_RED_DARK));

    titleRow->addWidget(iconBg);
    titleRow->addSpacing(8);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(m_generateAdviceBtn);

    // 建议内容展示区（加个半透明背景显得更专业）
    m_adviceContent = new QLabel("请点击右侧「开始分析」按钮，AI 将根据您的学习轨迹、薄弱环节为您提供 1 对 1 教学辅导建议。");
    m_adviceContent->setWordWrap(true);
    m_adviceContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_adviceContent->setStyleSheet(QString(
        "color: %1; font-size: 15px; line-height: 1.8; background-color: rgba(255, 255, 255, 0.6); "
        "border-radius: 12px; padding: 16px; border: 1px dashed #CBD5E0;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    layout->addWidget(titleContainer);
    layout->addWidget(m_adviceContent);
    layout->addStretch();
    // 不在这里添加到布局，由 setupUI 统一管理
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
