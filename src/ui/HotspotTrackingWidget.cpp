#include "HotspotTrackingWidget.h"
#include "../services/HotspotService.h"
#include "../services/DifyService.h"
#include "../shared/StyleConfig.h"
#include <QDebug>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QSvgRenderer>
#include <QEvent>

HotspotTrackingWidget::HotspotTrackingWidget(QWidget *parent)
    : QWidget(parent)
    , m_hotspotService(nullptr)
    , m_difyService(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
{
    m_imageCache.setMaxCost(50);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &HotspotTrackingWidget::onImageDownloaded);

    setupUI();
    setupStyles();
}

HotspotTrackingWidget::~HotspotTrackingWidget()
{
}

void HotspotTrackingWidget::loadImage(const QString &url, QLabel *label)
{
    if (url.isEmpty()) return;

    if (m_imageCache.contains(url)) {
        QPixmap scaled = m_imageCache[url]->scaled(
            label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 居中裁剪
        int x = (scaled.width() - label->width()) / 2;
        int y = (scaled.height() - label->height()) / 2;
        label->setPixmap(scaled.copy(x, y, label->width(), label->height()));
        return;
    }

    QNetworkRequest request{QUrl(url)};
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingImages[reply] = label;
}

void HotspotTrackingWidget::onImageDownloaded(QNetworkReply *reply)
{
    if (m_pendingImages.contains(reply)) {
        QLabel *label = m_pendingImages.take(reply);
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QPixmap pixmap;
            if (pixmap.loadFromData(data)) {
                QString url = reply->url().toString();
                m_imageCache.insert(url, new QPixmap(pixmap));
                if (label) {
                    QPixmap scaled = pixmap.scaled(
                        label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    int x = (scaled.width() - label->width()) / 2;
                    int y = (scaled.height() - label->height()) / 2;
                    label->setPixmap(scaled.copy(x, y, label->width(), label->height()));
                }
            }
        }
    }
    reply->deleteLater();
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
    m_mainLayout->setContentsMargins(24, 20, 24, 24);
    m_mainLayout->setSpacing(16);

    createHeader();
    createCategoryFilter();
    createNewsGrid();

    // 加载提示
    m_loadingLabel = new QLabel("正在获取最新热点...");
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet(
        "color: #9CA3AF; font-size: 14px; padding: 60px;"
    );
    m_loadingLabel->setVisible(false);
    m_mainLayout->addWidget(m_loadingLabel);

    // 空状态
    m_emptyLabel = new QLabel("暂无热点新闻");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(
        "color: #9CA3AF; font-size: 14px; padding: 60px;"
    );
    m_emptyLabel->setVisible(false);
    m_mainLayout->addWidget(m_emptyLabel);
}

void HotspotTrackingWidget::setupStyles()
{
    setStyleSheet(QString(
        "HotspotTrackingWidget {"
        "    background-color: %1;"
        "}"
    ).arg(StyleConfig::BG_APP));
}

void HotspotTrackingWidget::createHeader()
{
    m_headerFrame = new QFrame();
    m_headerFrame->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #DC2626, stop:1 #B91C1C);"
        "    border-radius: 16px;"
        "}"
    );

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(20, 16, 20, 16);
    headerLayout->setSpacing(16);

    // 左侧标题区
    QVBoxLayout *titleArea = new QVBoxLayout();
    titleArea->setSpacing(4);

    m_titleLabel = new QLabel("时政热点");
    m_titleLabel->setStyleSheet(
        "font-size: 22px; font-weight: 700; color: white; background: transparent;"
    );

    QLabel *subtitleLabel = new QLabel("实时追踪国内外重大新闻");
    subtitleLabel->setStyleSheet(
        "font-size: 13px; color: rgba(255,255,255,0.8); background: transparent;"
    );

    titleArea->addWidget(m_titleLabel);
    titleArea->addWidget(subtitleLabel);

    // 搜索框 - 白色半透明
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("搜索关键词...");
    m_searchInput->setFixedWidth(220);
    m_searchInput->setStyleSheet(
        "QLineEdit {"
        "    background-color: rgba(255,255,255,0.15);"
        "    border: 1px solid rgba(255,255,255,0.2);"
        "    border-radius: 20px;"
        "    padding: 10px 16px;"
        "    font-size: 14px;"
        "    color: white;"
        "}"
        "QLineEdit::placeholder {"
        "    color: rgba(255,255,255,0.6);"
        "}"
        "QLineEdit:focus {"
        "    background-color: rgba(255,255,255,0.25);"
        "    border-color: rgba(255,255,255,0.4);"
        "}"
    );

    connect(m_searchInput, &QLineEdit::textChanged,
            this, &HotspotTrackingWidget::onSearchTextChanged);

    // 刷新按钮
    m_refreshBtn = new QPushButton("刷新");
    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255,255,255,0.2);"
        "    color: white;"
        "    border: 1px solid rgba(255,255,255,0.3);"
        "    border-radius: 20px;"
        "    padding: 10px 24px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255,255,255,0.3);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(255,255,255,0.15);"
        "}"
    );
    m_refreshBtn->setCursor(Qt::PointingHandCursor);

    connect(m_refreshBtn, &QPushButton::clicked,
            this, &HotspotTrackingWidget::onRefreshClicked);

    headerLayout->addLayout(titleArea);
    headerLayout->addStretch();
    headerLayout->addWidget(m_searchInput);
    headerLayout->addWidget(m_refreshBtn);

    m_mainLayout->addWidget(m_headerFrame);
}

void HotspotTrackingWidget::createCategoryFilter()
{
    m_categoryFrame = new QFrame();
    m_categoryFrame->setStyleSheet("background: transparent;");

    QHBoxLayout *categoryLayout = new QHBoxLayout(m_categoryFrame);
    categoryLayout->setContentsMargins(0, 4, 0, 4);
    categoryLayout->setSpacing(8);

    m_categoryGroup = new QButtonGroup(this);
    m_categoryGroup->setExclusive(true);

    QStringList categories = {"全部", "国内", "国际"};

    for (int i = 0; i < categories.size(); ++i) {
        QPushButton *btn = new QPushButton(categories[i]);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(36);
        // 胶囊按钮风格
        btn->setStyleSheet(
            "QPushButton {"
            "    background-color: transparent;"
            "    color: #6B7280;"
            "    border: 1px solid #E5E7EB;"
            "    border-radius: 18px;"
            "    padding: 0 20px;"
            "    font-size: 14px;"
            "    font-weight: 500;"
            "}"
            "QPushButton:checked {"
            "    background-color: #1F2937;"
            "    color: white;"
            "    border-color: #1F2937;"
            "}"
            "QPushButton:hover:!checked {"
            "    background-color: #F3F4F6;"
            "    border-color: #D1D5DB;"
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
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;"
        "    width: 8px;"
        "    margin: 4px 2px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #D1D5DB;"
        "    min-height: 40px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #9CA3AF;"
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

// 创建头条新闻卡片 - 大图横幅样式
QWidget* HotspotTrackingWidget::createHeadlineCard(const NewsItem &news)
{
    QFrame *card = new QFrame();
    card->setFixedHeight(220);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(
        "QFrame#headlineCard {"
        "    background-color: #1F2937;"
        "    border-radius: 16px;"
        "}"
        "QFrame#headlineCard:hover {"
        "    background-color: #111827;"
        "}"
    );
    card->setObjectName("headlineCard");

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // 左侧：文字区
    QWidget *textArea = new QWidget();
    textArea->setStyleSheet("background: transparent;");
    QVBoxLayout *textLayout = new QVBoxLayout(textArea);
    textLayout->setContentsMargins(28, 24, 20, 24);
    textLayout->setSpacing(12);

    // 头条标签
    QLabel *headlineTag = new QLabel("头条");
    headlineTag->setFixedSize(48, 24);
    headlineTag->setAlignment(Qt::AlignCenter);
    headlineTag->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #F59E0B, stop:1 #D97706);"
        "color: white; font-size: 12px; font-weight: 600; border-radius: 12px;"
    );

    // 标题
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: 700; color: white; "
        "line-height: 1.4; background: transparent;"
    );

    // 摘要
    QLabel *summaryLabel = new QLabel(news.summary.left(80) + (news.summary.length() > 80 ? "..." : ""));
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet(
        "font-size: 14px; color: rgba(255,255,255,0.7); "
        "line-height: 1.5; background: transparent;"
    );

    // 底部信息
    QHBoxLayout *metaRow = new QHBoxLayout();
    metaRow->setSpacing(12);

    QLabel *sourceLabel = new QLabel(news.source);
    sourceLabel->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 13px; background: transparent;");

    QLabel *timeLabel = new QLabel(news.publishTime.isValid() ? news.publishTime.toString("MM-dd HH:mm") : "刚刚");
    timeLabel->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 13px; background: transparent;");

    QPushButton *generateBtn = new QPushButton("生成案例");
    generateBtn->setStyleSheet(
        "QPushButton {"
        "    background: rgba(255,255,255,0.1);"
        "    color: white;"
        "    border: 1px solid rgba(255,255,255,0.2);"
        "    padding: 6px 16px;"
        "    font-size: 13px;"
        "    border-radius: 14px;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(255,255,255,0.2);"
        "}"
    );
    generateBtn->setCursor(Qt::PointingHandCursor);
    connect(generateBtn, &QPushButton::clicked, this, [this, news]() {
        onGenerateTeachingClicked(news);
    });

    metaRow->addWidget(sourceLabel);
    metaRow->addWidget(timeLabel);
    metaRow->addStretch();
    metaRow->addWidget(generateBtn);

    textLayout->addWidget(headlineTag);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(summaryLabel);
    textLayout->addStretch();
    textLayout->addLayout(metaRow);

    cardLayout->addWidget(textArea, 3);

    // 右侧：图片区
    if (!news.imageUrl.isEmpty()) {
        QLabel *imageLabel = new QLabel();
        imageLabel->setFixedWidth(320);
        imageLabel->setStyleSheet(
            "background-color: #374151;"
            "border-top-right-radius: 16px;"
            "border-bottom-right-radius: 16px;"
        );
        imageLabel->setScaledContents(false);
        imageLabel->setAlignment(Qt::AlignCenter);
        loadImage(news.imageUrl, imageLabel);
        cardLayout->addWidget(imageLabel);
    }

    card->setProperty("newsId", news.id);
    card->installEventFilter(this);

    return card;
}

// 创建普通新闻卡片 - 带图片的左右布局
QWidget* HotspotTrackingWidget::createNewsCard(const NewsItem &news)
{
    QFrame *card = new QFrame();
    card->setMinimumHeight(140);
    card->setMaximumHeight(160);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #E5E7EB;"
        "}"
        "QFrame:hover {"
        "    border-color: #D1D5DB;"
        "    background-color: #FAFAFA;"
        "}"
    );

    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(8);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 15));
    card->setGraphicsEffect(shadow);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // 左侧：图片区域（只有有图才显示）
    if (!news.imageUrl.isEmpty()) {
        QLabel *imageLabel = new QLabel();
        imageLabel->setFixedSize(140, 140);
        imageLabel->setStyleSheet(
            "background-color: #F3F4F6;"
            "border-top-left-radius: 12px;"
            "border-bottom-left-radius: 12px;"
        );
        imageLabel->setAlignment(Qt::AlignCenter);
        loadImage(news.imageUrl, imageLabel);
        cardLayout->addWidget(imageLabel);
    }

    // 右侧：文字区域
    QWidget *textArea = new QWidget();
    textArea->setStyleSheet("background: transparent;");
    QVBoxLayout *textLayout = new QVBoxLayout(textArea);
    textLayout->setContentsMargins(14, 12, 14, 12);
    textLayout->setSpacing(8);

    // 顶部：分类 + 热度
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    QLabel *categoryLabel = new QLabel(news.category);
    QString catColor = news.category == "国际" ? "#3B82F6" : "#10B981";
    QString catBg = news.category == "国际" ? "#EFF6FF" : "#ECFDF5";
    categoryLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 11px; "
        "padding: 2px 8px; border-radius: 8px; font-weight: 500;"
    ).arg(catBg, catColor));

    // 热度指示器 - 根据热度显示不同颜色
    QString hotColor;
    QString hotBg;
    if (news.hotScore >= 80) {
        hotColor = "#DC2626"; hotBg = "#FEF2F2";
    } else if (news.hotScore >= 50) {
        hotColor = "#F59E0B"; hotBg = "#FFFBEB";
    } else {
        hotColor = "#6B7280"; hotBg = "#F3F4F6";
    }

    QLabel *hotLabel = new QLabel(QString::number(news.hotScore));
    hotLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 11px; "
        "font-weight: 600; padding: 2px 6px; border-radius: 8px;"
    ).arg(hotBg, hotColor));

    topRow->addWidget(categoryLabel);
    topRow->addStretch();
    topRow->addWidget(hotLabel);
    textLayout->addLayout(topRow);

    // 标题
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setMaximumHeight(44);
    titleLabel->setStyleSheet(
        "font-size: 14px; font-weight: 600; color: #1F2937; "
        "line-height: 1.4; background: transparent; border: none;"
    );
    textLayout->addWidget(titleLabel);

    textLayout->addStretch();

    // 底部：来源、时间、按钮
    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(4);

    QLabel *sourceLabel = new QLabel(news.source);
    sourceLabel->setStyleSheet("color: #9CA3AF; font-size: 11px; background: transparent;");

    QString timeText = formatTimeAgo(news.publishTime);
    QLabel *timeLabel = new QLabel(timeText);
    timeLabel->setStyleSheet("color: #9CA3AF; font-size: 11px; background: transparent;");

    QPushButton *generateBtn = new QPushButton("生成");
    generateBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #9CA3AF;"
        "    border: none;"
        "    padding: 2px 6px;"
        "    font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "    color: #DC2626;"
        "    background-color: #FEF2F2;"
        "    border-radius: 6px;"
        "}"
    );
    generateBtn->setCursor(Qt::PointingHandCursor);

    connect(generateBtn, &QPushButton::clicked, this, [this, news]() {
        onGenerateTeachingClicked(news);
    });

    bottomRow->addWidget(sourceLabel);
    bottomRow->addWidget(new QLabel("·"));
    bottomRow->addWidget(timeLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(generateBtn);
    textLayout->addLayout(bottomRow);

    cardLayout->addWidget(textArea, 1);

    card->setProperty("newsId", news.id);
    card->installEventFilter(this);

    return card;
}

QString HotspotTrackingWidget::formatTimeAgo(const QDateTime &time)
{
    if (!time.isValid()) return "刚刚";

    qint64 secs = time.secsTo(QDateTime::currentDateTime());
    if (secs < 60) return "刚刚";
    if (secs < 3600) return QString("%1分钟前").arg(secs / 60);
    if (secs < 86400) return QString("%1小时前").arg(secs / 3600);

    qint64 days = secs / 86400;
    if (days == 1) return "昨天";
    if (days < 7) return QString("%1天前").arg(days);

    return time.toString("MM-dd");
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

void HotspotTrackingWidget::performSearchDebounced()
{
    // 搜索去抖已在 onSearchTextChanged 中直接处理
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

    int row = 0;
    int col = 0;
    const int columns = computeGridColumns();

    for (int i = 0; i < newsList.size(); ++i) {
        const NewsItem &news = newsList[i];

        // 第一条作为头条，横跨全宽
        if (i == 0 && columns > 1) {
            QWidget *headline = createHeadlineCard(news);
            m_newsGridLayout->addWidget(headline, row, 0, 1, columns);
            row++;
        } else {
            QWidget *card = createNewsCard(news);
            m_newsGridLayout->addWidget(card, row, col);

            col++;
            if (col >= columns) {
                col = 0;
                row++;
            }
        }
    }

    // 设置列拉伸
    for (int c = 0; c < columns; ++c) {
        m_newsGridLayout->setColumnStretch(c, 1);
    }
}

int HotspotTrackingWidget::computeGridColumns() const
{
    int width = m_scrollArea->width();
    if (width < 600) return 1;
    if (width < 1000) return 2;
    return 3;
}

void HotspotTrackingWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    static int lastCols = -1;
    int cols = computeGridColumns();
    if (cols != lastCols) {
        lastCols = cols;
        onNewsListUpdated(m_currentNews);
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
    dialog->setMinimumSize(700, 550);
    dialog->setStyleSheet("QDialog { background-color: white; border-radius: 16px; }");

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 20);
    layout->setSpacing(0);

    // 顶部图片
    if (!news.imageUrl.isEmpty()) {
        QLabel *bigImage = new QLabel();
        bigImage->setFixedHeight(240);
        bigImage->setStyleSheet("background-color: #F3F4F6;");
        bigImage->setAlignment(Qt::AlignCenter);
        loadImage(news.imageUrl, bigImage);
        layout->addWidget(bigImage);
    }

    // 内容区
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(28, 20, 28, 20);
    contentLayout->setSpacing(12);

    // 分类标签
    QLabel *categoryLabel = new QLabel(news.category);
    QString catColor = news.category == "国际" ? "#3B82F6" : "#10B981";
    QString catBg = news.category == "国际" ? "#EFF6FF" : "#ECFDF5";
    categoryLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 12px; "
        "padding: 4px 12px; border-radius: 12px; font-weight: 500;"
    ).arg(catBg, catColor));
    categoryLabel->setFixedWidth(categoryLabel->sizeHint().width() + 24);
    contentLayout->addWidget(categoryLabel);

    // 标题
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet(
        "font-size: 22px; font-weight: 700; color: #1F2937; line-height: 1.4;"
    );
    contentLayout->addWidget(titleLabel);

    // 元信息
    QLabel *metaLabel = new QLabel(QString("%1 · %2")
        .arg(news.source)
        .arg(news.publishTime.toString("yyyy-MM-dd HH:mm")));
    metaLabel->setStyleSheet("color: #9CA3AF; font-size: 13px;");
    contentLayout->addWidget(metaLabel);

    // 分割线
    QFrame *line = new QFrame();
    line->setFixedHeight(1);
    line->setStyleSheet("background-color: #E5E7EB;");
    contentLayout->addWidget(line);

    // 正文
    QTextEdit *contentEdit = new QTextEdit();
    QString bodyText = news.content.isEmpty() ? news.summary : news.content;
    contentEdit->setHtml(QString(
        "<div style='line-height: 1.8; font-size: 15px; color: #374151;'>%1</div>"
    ).arg(bodyText.replace("\n", "<br>")));
    contentEdit->setReadOnly(true);
    contentEdit->setFrameShape(QFrame::NoFrame);
    contentEdit->setStyleSheet("background: transparent;");
    contentLayout->addWidget(contentEdit, 1);

    layout->addWidget(contentWidget, 1);

    // 底部按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(28, 0, 28, 0);
    btnLayout->setSpacing(12);

    QPushButton *closeBtn = new QPushButton("关闭");
    closeBtn->setFixedHeight(42);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #F3F4F6;"
        "    color: #374151;"
        "    border: none;"
        "    border-radius: 21px;"
        "    padding: 0 28px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: #E5E7EB; }"
    );

    QPushButton *generateBtn = new QPushButton("生成教学案例");
    generateBtn->setFixedHeight(42);
    generateBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #DC2626, stop:1 #B91C1C);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 21px;"
        "    padding: 0 28px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #B91C1C, stop:1 #991B1B);"
        "}"
    );

    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addWidget(generateBtn);

    layout->addLayout(btnLayout);

    connect(generateBtn, &QPushButton::clicked, [this, news, dialog]() {
        dialog->accept();
        onGenerateTeachingClicked(news);
    });
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->exec();
    delete dialog;
}

bool HotspotTrackingWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QFrame *card = qobject_cast<QFrame*>(watched);
        if (card) {
            QString newsId = card->property("newsId").toString();
            for (const auto &news : m_currentNews) {
                if (news.id == newsId) {
                    if (!news.url.isEmpty()) {
                        QDesktopServices::openUrl(QUrl(news.url));
                    } else {
                        showNewsDetail(news);
                    }
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
