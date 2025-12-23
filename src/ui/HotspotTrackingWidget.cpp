#include "HotspotTrackingWidget.h"
#include "../services/HotspotService.h"
#include "../services/DifyService.h"
#include <QDebug>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QMessageBox>

// 样式常量
namespace {
    const QString PATRIOTIC_RED = "#e53935";
    const QString PATRIOTIC_RED_LIGHT = "#ffebee";
    const QString PRIMARY_TEXT = "#212121";
    const QString SECONDARY_TEXT = "#757575";
    const QString CARD_WHITE = "#ffffff";
    const QString BACKGROUND_LIGHT = "#fafafa";
    const QString SEPARATOR = "#e8eaf6";
}

HotspotTrackingWidget::HotspotTrackingWidget(QWidget *parent)
    : QWidget(parent)
    , m_hotspotService(nullptr)
    , m_difyService(nullptr)
{
    setupUI();
    setupStyles();
}

HotspotTrackingWidget::~HotspotTrackingWidget()
{
}

void HotspotTrackingWidget::setHotspotService(HotspotService *service)
{
    if (m_hotspotService) {
        disconnect(m_hotspotService, nullptr, this, nullptr);
    }
    
    m_hotspotService = service;
    
    if (m_hotspotService) {
        connect(m_hotspotService, &HotspotService::hotNewsUpdated,
                this, &HotspotTrackingWidget::onNewsListUpdated);
        connect(m_hotspotService, &HotspotService::loadingStateChanged,
                this, &HotspotTrackingWidget::onLoadingStateChanged);
        connect(m_hotspotService, &HotspotService::errorOccurred,
                this, [this](const QString &error) {
                    m_emptyLabel->setText("加载失败：" + error);
                    m_emptyLabel->setVisible(true);
                });
    }
}

void HotspotTrackingWidget::setDifyService(DifyService *service)
{
    m_difyService = service;
}

void HotspotTrackingWidget::refresh()
{
    if (m_hotspotService) {
        m_hotspotService->refreshHotNews(m_currentCategory);
    }
}

void HotspotTrackingWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(24, 24, 24, 24);
    m_mainLayout->setSpacing(20);
    
    createHeader();
    createCategoryFilter();
    createNewsGrid();
    
    // 加载提示
    m_loadingLabel = new QLabel("正在加载热点新闻...");
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 14px; padding: 40px;");
    m_loadingLabel->setVisible(false);
    m_mainLayout->addWidget(m_loadingLabel);
    
    // 空状态提示
    m_emptyLabel = new QLabel("暂无热点新闻");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 14px; padding: 40px;");
    m_emptyLabel->setVisible(false);
    m_mainLayout->addWidget(m_emptyLabel);
}

void HotspotTrackingWidget::setupStyles()
{
    setStyleSheet(QString(
        "HotspotTrackingWidget {"
        "    background-color: %1;"
        "}"
    ).arg(BACKGROUND_LIGHT));
}

void HotspotTrackingWidget::createHeader()
{
    m_headerFrame = new QFrame();
    m_headerFrame->setStyleSheet(QString(
        "QFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    padding: 16px;"
        "}"
    ).arg(CARD_WHITE));
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(16, 12, 16, 12);
    headerLayout->setSpacing(16);
    
    // 标题
    m_titleLabel = new QLabel("🔥 政治热点追踪");
    m_titleLabel->setStyleSheet(QString(
        "font-size: 20px; font-weight: bold; color: %1;"
    ).arg(PATRIOTIC_RED));
    
    // 搜索框
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("搜索热点关键词...");
    m_searchInput->setFixedWidth(250);
    m_searchInput->setStyleSheet(QString(
        "QLineEdit {"
        "    border: 1px solid %1;"
        "    border-radius: 18px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    background-color: %2;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %3;"
        "}"
    ).arg(SEPARATOR, BACKGROUND_LIGHT, PATRIOTIC_RED));
    
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &HotspotTrackingWidget::onSearchTextChanged);
    
    // 刷新按钮
    m_refreshBtn = new QPushButton("刷新");
    m_refreshBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 18px;"
        "    padding: 8px 20px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background-color: #c62828;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #b71c1c;"
        "}"
    ).arg(PATRIOTIC_RED));
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &HotspotTrackingWidget::onRefreshClicked);
    
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_searchInput);
    headerLayout->addWidget(m_refreshBtn);
    
    m_mainLayout->addWidget(m_headerFrame);
}

void HotspotTrackingWidget::createCategoryFilter()
{
    m_categoryFrame = new QFrame();
    QHBoxLayout *categoryLayout = new QHBoxLayout(m_categoryFrame);
    categoryLayout->setContentsMargins(0, 0, 0, 0);
    categoryLayout->setSpacing(12);
    
    m_categoryGroup = new QButtonGroup(this);
    m_categoryGroup->setExclusive(true);
    
    QStringList categories = {"全部", "国内", "国外"};
    
    for (int i = 0; i < categories.size(); ++i) {
        QPushButton *btn = new QPushButton(categories[i]);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "    background-color: %1;"
            "    color: %2;"
            "    border: 1px solid %3;"
            "    border-radius: 16px;"
            "    padding: 6px 16px;"
            "    font-size: 13px;"
            "}"
            "QPushButton:checked {"
            "    background-color: %4;"
            "    color: white;"
            "    border-color: %4;"
            "}"
            "QPushButton:hover:!checked {"
            "    background-color: %5;"
            "}"
        ).arg(CARD_WHITE, PRIMARY_TEXT, SEPARATOR, PATRIOTIC_RED, PATRIOTIC_RED_LIGHT));
        
        if (i == 0) {
            btn->setChecked(true);
        }
        
        m_categoryGroup->addButton(btn, i);
        m_categoryButtons.append(btn);
        categoryLayout->addWidget(btn);
    }
    
    categoryLayout->addStretch();
    
    connect(m_categoryGroup, &QButtonGroup::idClicked,
            this, &HotspotTrackingWidget::onCategoryChanged);
    
    m_mainLayout->addWidget(m_categoryFrame);
}

void HotspotTrackingWidget::createNewsGrid()
{
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    
    m_newsContainer = new QWidget();
    m_newsContainer->setStyleSheet("background: transparent;");
    
    m_newsGridLayout = new QGridLayout(m_newsContainer);
    m_newsGridLayout->setContentsMargins(0, 0, 0, 0);
    m_newsGridLayout->setSpacing(16);
    m_newsGridLayout->setAlignment(Qt::AlignTop);
    
    m_scrollArea->setWidget(m_newsContainer);
    m_mainLayout->addWidget(m_scrollArea, 1);
}

QWidget* HotspotTrackingWidget::createNewsCard(const NewsItem &news)
{
    QFrame *card = new QFrame();
    card->setFixedHeight(220);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(QString(
        "QFrame {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
        "QFrame:hover {"
        "    border-color: %3;"
        "}"
    ).arg(CARD_WHITE, SEPARATOR, PATRIOTIC_RED));
    
    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(12);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 20));
    card->setGraphicsEffect(shadow);
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(10);
    
    // 分类和热度标签
    QHBoxLayout *topRow = new QHBoxLayout();
    
    QLabel *categoryLabel = new QLabel(news.category);
    categoryLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 11px; "
        "padding: 3px 8px; border-radius: 10px; font-weight: 500;"
    ).arg(PATRIOTIC_RED_LIGHT, PATRIOTIC_RED));
    
    QLabel *hotLabel = new QLabel(QString("🔥 %1").arg(news.hotScore));
    hotLabel->setStyleSheet("color: #ff6b6b; font-size: 12px; font-weight: 500;");
    
    topRow->addWidget(categoryLabel);
    topRow->addStretch();
    topRow->addWidget(hotLabel);
    cardLayout->addLayout(topRow);
    
    // 标题
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setMaximumHeight(48);
    titleLabel->setStyleSheet(QString(
        "font-size: 15px; font-weight: 600; color: %1; line-height: 1.4;"
    ).arg(PRIMARY_TEXT));
    cardLayout->addWidget(titleLabel);
    
    // 摘要
    QLabel *summaryLabel = new QLabel(news.summary);
    summaryLabel->setWordWrap(true);
    summaryLabel->setMaximumHeight(40);
    summaryLabel->setStyleSheet(QString(
        "font-size: 13px; color: %1; line-height: 1.3;"
    ).arg(SECONDARY_TEXT));
    cardLayout->addWidget(summaryLabel);
    
    cardLayout->addStretch();
    
    // 底部：来源和时间
    QHBoxLayout *bottomRow = new QHBoxLayout();
    
    QLabel *sourceLabel = new QLabel(news.source);
    sourceLabel->setStyleSheet("color: #9e9e9e; font-size: 12px;");
    
    QLabel *timeLabel = new QLabel(news.publishTime.toString("MM-dd hh:mm"));
    timeLabel->setStyleSheet("color: #9e9e9e; font-size: 12px;");
    
    // 生成教学案例按钮
    QPushButton *generateBtn = new QPushButton("生成教学案例");
    generateBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: %1;"
        "    border: 1px solid %1;"
        "    border-radius: 12px;"
        "    padding: 4px 10px;"
        "    font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
    ).arg(PATRIOTIC_RED, PATRIOTIC_RED_LIGHT));
    generateBtn->setCursor(Qt::PointingHandCursor);
    
    // 存储新闻数据
    generateBtn->setProperty("newsId", news.id);
    
    connect(generateBtn, &QPushButton::clicked, this, [this, news]() {
        onGenerateTeachingClicked(news);
    });
    
    bottomRow->addWidget(sourceLabel);
    bottomRow->addWidget(timeLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(generateBtn);
    cardLayout->addLayout(bottomRow);
    
    // 点击卡片查看详情
    card->setProperty("newsId", news.id);
    card->installEventFilter(this);
    
    return card;
}

void HotspotTrackingWidget::clearNewsGrid()
{
    QLayoutItem *item;
    while ((item = m_newsGridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void HotspotTrackingWidget::onRefreshClicked()
{
    qDebug() << "[HotspotTrackingWidget] Refresh clicked";
    refresh();
}

void HotspotTrackingWidget::onSearchTextChanged(const QString &text)
{
    if (m_hotspotService) {
        if (text.trimmed().isEmpty()) {
            m_hotspotService->refreshHotNews(m_currentCategory);
        } else {
            m_hotspotService->searchNews(text);
        }
    }
}

void HotspotTrackingWidget::onCategoryChanged(int categoryIndex)
{
    QStringList categories = {"", "国内", "国外"};
    m_currentCategory = (categoryIndex > 0 && categoryIndex < categories.size()) 
                        ? categories[categoryIndex] : "";
    
    qDebug() << "[HotspotTrackingWidget] Category changed to:" << m_currentCategory;
    
    if (m_hotspotService) {
        m_hotspotService->refreshHotNews(m_currentCategory);
    }
}

void HotspotTrackingWidget::onNewsListUpdated(const QList<NewsItem> &newsList)
{
    qDebug() << "[HotspotTrackingWidget] Received" << newsList.size() << "news items";
    
    m_currentNews = newsList;
    clearNewsGrid();
    
    m_loadingLabel->setVisible(false);
    m_emptyLabel->setVisible(newsList.isEmpty());
    m_scrollArea->setVisible(!newsList.isEmpty());
    
    if (newsList.isEmpty()) {
        return;
    }
    
    // 每行显示 2 个卡片
    const int columns = 2;
    int row = 0, col = 0;
    
    for (const NewsItem &news : newsList) {
        QWidget *card = createNewsCard(news);
        m_newsGridLayout->addWidget(card, row, col);
        
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    
    // 设置列拉伸
    for (int c = 0; c < columns; ++c) {
        m_newsGridLayout->setColumnStretch(c, 1);
    }
}

void HotspotTrackingWidget::onNewsCardClicked(const NewsItem &news)
{
    showNewsDetail(news);
}

void HotspotTrackingWidget::onGenerateTeachingClicked(const NewsItem &news)
{
    qDebug() << "[HotspotTrackingWidget] Generate teaching content for:" << news.title;
    
    if (!m_hotspotService || !m_difyService) {
        QMessageBox::warning(this, "提示", "服务未就绪，请稍后重试");
        return;
    }
    
    // 显示生成中提示
    QMessageBox::information(this, "生成中", 
        "正在使用 AI 生成教学案例，请稍候...\n\n"
        "生成完成后将显示在 AI 对话区域。");
    
    m_hotspotService->generateTeachingContent(news, m_difyService);
}

void HotspotTrackingWidget::onLoadingStateChanged(bool isLoading)
{
    m_loadingLabel->setVisible(isLoading);
    m_refreshBtn->setEnabled(!isLoading);
    
    if (isLoading) {
        m_scrollArea->setVisible(false);
        m_emptyLabel->setVisible(false);
    }
}

void HotspotTrackingWidget::showNewsDetail(const NewsItem &news)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(news.title);
    dialog->setMinimumSize(600, 500);
    dialog->setStyleSheet(QString(
        "QDialog { background-color: %1; }"
    ).arg(CARD_WHITE));
    
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    
    // 标题
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet(QString(
        "font-size: 20px; font-weight: bold; color: %1;"
    ).arg(PRIMARY_TEXT));
    layout->addWidget(titleLabel);
    
    // 元信息
    QLabel *metaLabel = new QLabel(QString("%1 | %2 | 热度 %3")
        .arg(news.source)
        .arg(news.publishTime.toString("yyyy-MM-dd hh:mm"))
        .arg(news.hotScore));
    metaLabel->setStyleSheet("color: " + SECONDARY_TEXT + "; font-size: 13px;");
    layout->addWidget(metaLabel);
    
    // 内容
    QTextEdit *contentEdit = new QTextEdit();
    contentEdit->setPlainText(news.content.isEmpty() ? news.summary : news.content);
    contentEdit->setReadOnly(true);
    contentEdit->setStyleSheet(QString(
        "QTextEdit {"
        "    border: 1px solid %1;"
        "    border-radius: 8px;"
        "    padding: 12px;"
        "    font-size: 14px;"
        "    line-height: 1.6;"
        "    color: %2;"
        "}"
    ).arg(SEPARATOR, PRIMARY_TEXT));
    layout->addWidget(contentEdit, 1);
    
    // 关键词
    if (!news.keywords.isEmpty()) {
        QLabel *keywordsLabel = new QLabel("关键词：" + news.keywords.join(" | "));
        keywordsLabel->setStyleSheet(QString(
            "color: %1; font-size: 13px; padding: 8px; "
            "background-color: %2; border-radius: 6px;"
        ).arg(PATRIOTIC_RED, PATRIOTIC_RED_LIGHT));
        layout->addWidget(keywordsLabel);
    }
    
    // 按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    
    QPushButton *generateBtn = new QPushButton("生成教学案例");
    generateBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 18px;"
        "    padding: 10px 24px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #c62828; }"
    ).arg(PATRIOTIC_RED));
    
    QPushButton *closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    border-radius: 18px;"
        "    padding: 10px 24px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: %4; }"
    ).arg(CARD_WHITE, PRIMARY_TEXT, SEPARATOR, BACKGROUND_LIGHT));
    
    buttonBox->addButton(generateBtn, QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeBtn, QDialogButtonBox::RejectRole);
    
    connect(generateBtn, &QPushButton::clicked, [this, news, dialog]() {
        dialog->accept();
        onGenerateTeachingClicked(news);
    });
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    
    layout->addWidget(buttonBox);
    
    dialog->exec();
    delete dialog;
}
