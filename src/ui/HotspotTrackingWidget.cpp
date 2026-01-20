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
#include <QEvent>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>

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
    categoryLayout->setContentsMargins(0, 8, 0, 8);
    categoryLayout->setSpacing(24);

    m_categoryGroup = new QButtonGroup(this);
    m_categoryGroup->setExclusive(true);

    QStringList categories = {"全部", "国内", "国际"};

    for (int i = 0; i < categories.size(); ++i) {
        QPushButton *btn = new QPushButton(categories[i]);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        // 文字选项卡风格：选中时底部红线
        btn->setStyleSheet(
            "QPushButton {"
            "    background-color: transparent;"
            "    color: #666666;"
            "    border: none;"
            "    border-bottom: 2px solid transparent;"
            "    padding: 8px 4px;"
            "    font-size: 14px;"
            "    font-weight: 500;"
            "}"
            "QPushButton:checked {"
            "    color: #D32F2F;"
            "    border-bottom: 2px solid #D32F2F;"
            "}"
            "QPushButton:hover:!checked {"
            "    color: #333333;"
            "}"
        );

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
    // 美化滚动条：细长、圆角、浅灰色
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: #F5F5F5;"
        "    width: 6px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #CCCCCC;"
        "    min-height: 30px;"
        "    border-radius: 3px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #999999;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );
    
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
    card->setMinimumHeight(120);
    card->setMaximumHeight(180);
    card->setCursor(Qt::PointingHandCursor);
    // 极简卡片：无边框，hover 时上浮阴影
    card->setStyleSheet(QString(
        "QFrame {"
        "    background-color: %1;"
        "    border-radius: 8px;"
        "    border: 1px solid #EEEEEE;"
        "}"
        "QFrame:hover {"
        "    border-color: #E0E0E0;"
        "}"
    ).arg(CARD_WHITE));

    // 默认无阴影，hover 时添加阴影效果通过 CSS 实现
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(0);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 0));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(8);

    // 顶部：分类标签
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    QLabel *categoryLabel = new QLabel(news.category);
    categoryLabel->setStyleSheet(
        "background-color: #FFF0F0; color: #D32F2F; font-size: 11px; "
        "padding: 2px 8px; border-radius: 8px; font-weight: 500;"
    );

    // 热度标签：淡红背景 + 深红字
    QLabel *hotLabel = new QLabel(QString("🔥 %1").arg(news.hotScore));
    hotLabel->setStyleSheet(
        "background-color: #FFF0F0; color: #D32F2F; font-size: 12px; "
        "font-weight: 600; padding: 2px 6px; border-radius: 8px;"
    );

    topRow->addWidget(categoryLabel);
    topRow->addStretch();
    topRow->addWidget(hotLabel);
    cardLayout->addLayout(topRow);

    // 标题：无边框，纯文字，加粗
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setMinimumHeight(20);
    titleLabel->setMaximumHeight(48);
    titleLabel->setStyleSheet(
        "font-size: 15px; font-weight: 600; color: #333333; "
        "line-height: 1.5; background: transparent; border: none; padding: 0;"
    );
    cardLayout->addWidget(titleLabel);

    // 摘要：只在有真正不同的摘要时显示
    bool showSummary = !news.summary.isEmpty() &&
                       news.summary != news.title &&
                       !news.summary.startsWith(news.title.left(30));
    if (showSummary) {
        QLabel *summaryLabel = new QLabel(news.summary);
        summaryLabel->setWordWrap(true);
        summaryLabel->setMaximumHeight(36);
        summaryLabel->setStyleSheet(
            "font-size: 13px; color: #666666; line-height: 1.4; "
            "background: transparent; border: none; padding: 0;"
        );
        cardLayout->addWidget(summaryLabel);
    }

    cardLayout->addStretch();

    // 底部：来源、时间、生成按钮
    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(6);

    QLabel *sourceLabel = new QLabel(news.source);
    sourceLabel->setStyleSheet("color: #999999; font-size: 12px; background: transparent;");

    QLabel *separator = new QLabel("·");
    separator->setStyleSheet("color: #CCCCCC; font-size: 12px;");

    // 时间显示
    QString timeText;
    if (news.publishTime.isValid()) {
        qint64 daysDiff = news.publishTime.daysTo(QDateTime::currentDateTime());
        if (daysDiff == 0) {
            timeText = news.publishTime.toString("HH:mm");
        } else if (daysDiff == 1) {
            timeText = "昨天";
        } else if (daysDiff < 7) {
            timeText = QString("%1天前").arg(daysDiff);
        } else {
            timeText = news.publishTime.toString("MM-dd");
        }
    } else {
        timeText = "刚刚";
    }

    QLabel *timeLabel = new QLabel(timeText);
    timeLabel->setStyleSheet("color: #999999; font-size: 12px; background: transparent;");

    // 生成教学案例：弱化为文字链接风格
    QPushButton *generateBtn = new QPushButton("📚 生成案例");
    generateBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #999999;"
        "    border: none;"
        "    padding: 2px 6px;"
        "    font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "    color: #D32F2F;"
        "    background-color: #FFF0F0;"
        "    border-radius: 8px;"
        "}"
    );
    generateBtn->setCursor(Qt::PointingHandCursor);
    
    // 存储新闻数据
    generateBtn->setProperty("newsId", news.id);
    
    connect(generateBtn, &QPushButton::clicked, this, [this, news]() {
        onGenerateTeachingClicked(news);
    });
    
    bottomRow->addWidget(sourceLabel);
    bottomRow->addWidget(separator);
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
    QStringList categories = {"", "国内", "国际"};
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

bool HotspotTrackingWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QFrame *card = qobject_cast<QFrame*>(watched);
        if (card) {
            QString newsId = card->property("newsId").toString();
            // 查找对应的新闻
            for (const auto &news : m_currentNews) {
                if (news.id == newsId) {
                    // 如果有原文链接，在浏览器中打开
                    if (!news.url.isEmpty()) {
                        QDesktopServices::openUrl(QUrl(news.url));
                    } else {
                        // 否则显示详情对话框
                        showNewsDetail(news);
                    }
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
