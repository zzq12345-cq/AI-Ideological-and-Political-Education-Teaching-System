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
    // 图片缓存配置：最多缓存50张图片
    static constexpr int MAX_IMAGE_CACHE_SIZE = 50;
    m_imageCache.setMaxCost(MAX_IMAGE_CACHE_SIZE);

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

    // 在 label 上存储关联的 URL，用于调试和验证
    label->setProperty("imageUrl", url);

    // 缓存命中，直接使用
    if (m_imageCache.contains(url)) {
        QPixmap scaled = m_imageCache[url]->scaled(
            label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 居中裁剪
        int x = (scaled.width() - label->width()) / 2;
        int y = (scaled.height() - label->height()) / 2;
        label->setPixmap(scaled.copy(x, y, label->width(), label->height()));
        return;
    }

    // 发起网络请求
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
            } else {
                qWarning() << "[HotspotTrackingWidget] 图片加载失败:" << reply->url().toString();
            }
        } else {
            qWarning() << "[HotspotTrackingWidget] 图片下载失败:" << reply->errorString();
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
        connect(m_hotspotService, &HotspotService::teachingContentGenerated,
                this, [this]() {
                    // 当 AI 生成完成时，尝试刷新所有按钮状态（简易方案）
                    for (auto* btn : findChildren<QPushButton*>()) {
                        if (btn->text().contains("生成中")) {
                            btn->setText("生成案例");
                            btn->setEnabled(true);
                        }
                    }
                });
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
    m_mainLayout->setContentsMargins(32, 28, 32, 32);
    m_mainLayout->setSpacing(20);

    createHeader();
    createCategoryFilter();
    createNewsGrid();

    // 加载提示 - 更精致的设计
    m_loadingLabel = new QLabel();
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet("background: transparent;");

    QVBoxLayout *loadingLayout = new QVBoxLayout(m_loadingLabel);
    loadingLayout->setSpacing(16);
    loadingLayout->setAlignment(Qt::AlignCenter);

    // 加载动画容器
    QLabel *loadingIconContainer = new QLabel();
    loadingIconContainer->setFixedSize(80, 80);
    loadingIconContainer->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #FEF2F2, stop:1 #FFF7ED);"
        "border-radius: 40px;"
        "border: 2px solid #FECACA;"
    );
    loadingIconContainer->setAlignment(Qt::AlignCenter);
    QPixmap loadingPix(":/icons/resources/icons/robot.svg");
    loadingIconContainer->setPixmap(loadingPix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *loadingText = new QLabel("正在同步全球时政资讯...");
    loadingText->setStyleSheet(QString(
        "color: %1; font-size: 16px; font-weight: 600;"
    ).arg(StyleConfig::TEXT_SECONDARY));
    loadingText->setAlignment(Qt::AlignCenter);

    QLabel *loadingSubtext = new QLabel("数据来源：人民日报 · 新华社 · BBC中文网");
    loadingSubtext->setStyleSheet(QString(
        "color: %1; font-size: 13px; font-weight: 500;"
    ).arg(StyleConfig::TEXT_LIGHT));
    loadingSubtext->setAlignment(Qt::AlignCenter);

    loadingLayout->addStretch();
    loadingLayout->addWidget(loadingIconContainer, 0, Qt::AlignCenter);
    loadingLayout->addWidget(loadingText);
    loadingLayout->addWidget(loadingSubtext);
    loadingLayout->addStretch();

    m_loadingLabel->setContentsMargins(0, 80, 0, 80);
    m_loadingLabel->setVisible(false);
    m_mainLayout->addWidget(m_loadingLabel);

    // 空状态 - 更友好的设计
    m_emptyLabel = new QLabel();
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("background: transparent;");

    QVBoxLayout *emptyLayout = new QVBoxLayout(m_emptyLabel);
    emptyLayout->setSpacing(16);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QLabel *emptyIconContainer = new QLabel();
    emptyIconContainer->setFixedSize(80, 80);
    emptyIconContainer->setStyleSheet(
        "background-color: #F3F4F6;"
        "border-radius: 40px;"
        "border: 2px solid #E5E7EB;"
    );
    emptyIconContainer->setAlignment(Qt::AlignCenter);
    QPixmap emptyPix(":/icons/resources/icons/warning.svg");
    emptyIconContainer->setPixmap(emptyPix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *emptyText = new QLabel("未发现相关新闻");
    emptyText->setStyleSheet(QString(
        "color: %1; font-size: 16px; font-weight: 600;"
    ).arg(StyleConfig::TEXT_SECONDARY));
    emptyText->setAlignment(Qt::AlignCenter);

    QLabel *emptySubtext = new QLabel("请尝试更换搜索关键词或切换分类");
    emptySubtext->setStyleSheet(QString(
        "color: %1; font-size: 13px; font-weight: 500;"
    ).arg(StyleConfig::TEXT_LIGHT));
    emptySubtext->setAlignment(Qt::AlignCenter);

    emptyLayout->addStretch();
    emptyLayout->addWidget(emptyIconContainer, 0, Qt::AlignCenter);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addWidget(emptySubtext);
    emptyLayout->addStretch();

    m_emptyLabel->setContentsMargins(0, 80, 0, 80);
    m_emptyLabel->setVisible(false);
    m_mainLayout->addWidget(m_emptyLabel);
}

void HotspotTrackingWidget::createHeader()
{
    m_headerFrame = new QFrame();
    m_headerFrame->setFixedHeight(140);
    m_headerFrame->setObjectName("headerFrame");
    // 更酷炫的多段渐变 + 玻璃质感边框
    m_headerFrame->setStyleSheet(QString(
        "QFrame#headerFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #C62828, stop:0.3 %1, stop:0.7 %2, stop:1 #7F0000);"
        "    border-radius: %3px;"
        "    border: 1px solid rgba(255,255,255,0.15);"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED, StyleConfig::PATRIOTIC_RED_DARK).arg(StyleConfig::RADIUS_XL));

    // 添加高级阴影效果
    QGraphicsDropShadowEffect *headerShadow = new QGraphicsDropShadowEffect(m_headerFrame);
    headerShadow->setBlurRadius(25);
    headerShadow->setOffset(0, 8);
    headerShadow->setColor(QColor(183, 28, 28, 80));
    m_headerFrame->setGraphicsEffect(headerShadow);

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(36, 24, 36, 24);
    headerLayout->setSpacing(28);

    // 左侧标题区（增加图标装饰）
    QHBoxLayout *titleAreaLayout = new QHBoxLayout();
    titleAreaLayout->setSpacing(16);

    // 装饰图标容器
    QLabel *iconDecor = new QLabel();
    iconDecor->setFixedSize(56, 56);
    iconDecor->setStyleSheet(
        "background: rgba(255,255,255,0.15);"
        "border-radius: 16px;"
        "border: 1px solid rgba(255,255,255,0.2);"
    );
    iconDecor->setAlignment(Qt::AlignCenter);
    QPixmap newsIcon(":/icons/resources/icons/news.svg");
    iconDecor->setPixmap(newsIcon.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QVBoxLayout *titleTextArea = new QVBoxLayout();
    titleTextArea->setSpacing(6);

    m_titleLabel = new QLabel("时政热点追踪");
    m_titleLabel->setStyleSheet(
        "font-size: 30px; font-weight: 900; color: white; background: transparent;"
        "letter-spacing: 2px;"
    );

    // 副标题带装饰点
    QLabel *subtitleLabel = new QLabel("◆ 权威追踪  ◆ 深度解析  ◆ 实时资讯");
    subtitleLabel->setStyleSheet(
        "font-size: 13px; color: rgba(255,255,255,0.8); background: transparent; "
        "font-weight: 500; letter-spacing: 1px;"
    );

    titleTextArea->addWidget(m_titleLabel);
    titleTextArea->addWidget(subtitleLabel);

    titleAreaLayout->addWidget(iconDecor);
    titleAreaLayout->addLayout(titleTextArea);

    // 搜索框 - 升级玻璃拟态风格
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("搜索全球时政热点...");
    m_searchInput->setFixedSize(300, 44);

    QAction *searchAction = new QAction(QIcon(":/icons/resources/icons/search.svg"), "搜索", m_searchInput);
    m_searchInput->addAction(searchAction, QLineEdit::LeadingPosition);

    m_searchInput->setStyleSheet(
        "QLineEdit {"
        "    background-color: rgba(255,255,255,0.12);"
        "    border: 1px solid rgba(255,255,255,0.2);"
        "    border-radius: 22px;"
        "    padding: 10px 16px 10px 40px;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    color: white;"
        "    selection-background-color: rgba(255,255,255,0.3);"
        "}"
        "QLineEdit::placeholder {"
        "    color: rgba(255,255,255,0.6);"
        "}"
        "QLineEdit:focus {"
        "    background-color: rgba(255,255,255,0.2);"
        "    border: 2px solid rgba(255,255,255,0.5);"
        "}"
        "QLineEdit:hover {"
        "    background-color: rgba(255,255,255,0.18);"
        "}"
    );

    connect(m_searchInput, &QLineEdit::textChanged,
            this, &HotspotTrackingWidget::onSearchTextChanged);

    // 刷新按钮 - 更立体的设计
    m_refreshBtn = new QPushButton("刷新数据");
    m_refreshBtn->setIcon(QIcon(":/icons/resources/icons/dashboard.svg"));
    m_refreshBtn->setIconSize(QSize(18, 18));
    m_refreshBtn->setFixedHeight(44);

    m_refreshBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 rgba(255,255,255,0.25), stop:1 rgba(255,255,255,0.1));"
        "    color: white;"
        "    border: 1px solid rgba(255,255,255,0.3);"
        "    border-radius: 22px;"
        "    padding: 10px 28px;"
        "    font-size: 14px;"
        "    font-weight: 700;"
        "    letter-spacing: 1px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 rgba(255,255,255,0.35), stop:1 rgba(255,255,255,0.2));"
        "    border-color: rgba(255,255,255,0.5);"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(255,255,255,0.15);"
        "}"
    );
    m_refreshBtn->setCursor(Qt::PointingHandCursor);

    connect(m_refreshBtn, &QPushButton::clicked,
            this, &HotspotTrackingWidget::onRefreshClicked);

    headerLayout->addLayout(titleAreaLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(m_searchInput);
    headerLayout->addWidget(m_refreshBtn);

    m_mainLayout->addWidget(m_headerFrame);
}

void HotspotTrackingWidget::setupStyles()
{
    // 更高级的渐变背景
    setStyleSheet(QString(
        "HotspotTrackingWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 #FAFBFC, stop:0.5 %1, stop:1 #F0F2F5);"
        "}"
    ).arg(StyleConfig::BG_APP));
}

void HotspotTrackingWidget::createCategoryFilter()
{
    m_categoryFrame = new QFrame();
    m_categoryFrame->setObjectName("categoryFilterFrame");
    m_categoryFrame->setStyleSheet(
        "QFrame#categoryFilterFrame {"
        "    background: transparent;"
        "    padding: 8px 0;"
        "}"
    );

    QHBoxLayout *categoryLayout = new QHBoxLayout(m_categoryFrame);
    categoryLayout->setContentsMargins(4, 8, 0, 8);
    categoryLayout->setSpacing(16);

    // 分类标签装饰
    QLabel *filterIcon = new QLabel();
    filterIcon->setPixmap(QIcon(":/icons/resources/icons/menu.svg").pixmap(18, 18));
    filterIcon->setStyleSheet("background: transparent;");

    QLabel *filterLabel = new QLabel("筛选分类");
    filterLabel->setStyleSheet(QString(
        "color: %1; font-size: 14px; font-weight: 600; background: transparent;"
    ).arg(StyleConfig::TEXT_SECONDARY));

    categoryLayout->addWidget(filterIcon);
    categoryLayout->addWidget(filterLabel);
    categoryLayout->addSpacing(8);

    // 分隔线
    QFrame *separator = new QFrame();
    separator->setFixedSize(1, 24);
    separator->setStyleSheet(QString("background-color: %1;").arg(StyleConfig::BORDER_LIGHT));
    categoryLayout->addWidget(separator);
    categoryLayout->addSpacing(8);

    m_categoryGroup = new QButtonGroup(this);
    m_categoryGroup->setExclusive(true);

    // 使用图标+文字的组合
    QStringList categories = {"全部", "国内", "国际"};
    QStringList categoryIcons = {":/icons/resources/icons/dashboard.svg",
                                  ":/icons/resources/icons/pin.svg",
                                  ":/icons/resources/icons/analytics.svg"};

    for (int i = 0; i < categories.size(); ++i) {
        QPushButton *btn = new QPushButton(categories[i]);
        btn->setIcon(QIcon(categoryIcons[i]));
        btn->setIconSize(QSize(16, 16));
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(42);

        // 更精致的胶囊按钮 - 选中态有渐变背景
        btn->setStyleSheet(QString(
            "QPushButton {"
            "    background-color: %1;"
            "    color: %2;"
            "    border: 1.5px solid %3;"
            "    border-radius: 21px;"
            "    padding: 0 22px;"
            "    font-size: 14px;"
            "    font-weight: 600;"
            "}"
            "QPushButton:checked {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            "        stop:0 %4, stop:1 #C62828);"
            "    color: white;"
            "    border: none;"
            "}"
            "QPushButton:hover:!checked {"
            "    background-color: %5;"
            "    color: %4;"
            "    border-color: %6;"
            "}"
            "QPushButton:pressed {"
            "    background-color: %6;"
            "}"
        ).arg(StyleConfig::BG_CARD, StyleConfig::TEXT_SECONDARY, StyleConfig::BORDER_LIGHT,
             StyleConfig::PATRIOTIC_RED, StyleConfig::PATRIOTIC_RED_TINT, StyleConfig::PATRIOTIC_RED_LIGHT));

        // 选中态添加阴影 - 移除可能导致崩溃的阴影效果
        if (i == 0) {
            btn->setChecked(true);
        }

        m_categoryGroup->addButton(btn, i);
        m_categoryButtons.append(btn);
        categoryLayout->addWidget(btn);
    }

    categoryLayout->addStretch();

    // 右侧数据来源标识
    QLabel *sourceLabel = new QLabel("数据来源：人民日报 · 新华社 · BBC中文");
    sourceLabel->setStyleSheet(QString(
        "color: %1; font-size: 12px; background: transparent;"
    ).arg(StyleConfig::TEXT_LIGHT));
    categoryLayout->addWidget(sourceLabel);

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
        "    background: #F5F7FA;"
        "    width: 8px;"
        "    margin: 4px 2px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #D1D5DB, stop:1 #E5E7EB);"
        "    min-height: 50px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #9CA3AF, stop:1 #D1D5DB);"
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
    m_newsGridLayout->setContentsMargins(0, 0, 12, 0);
    m_newsGridLayout->setSpacing(20);
    m_newsGridLayout->setAlignment(Qt::AlignTop);

    m_scrollArea->setWidget(m_newsContainer);
    m_mainLayout->addWidget(m_scrollArea, 1);
}

// 创建头条新闻卡片 - 大气横幅样式，玻璃拟态效果
QWidget* HotspotTrackingWidget::createHeadlineCard(const NewsItem &news)
{
    QFrame *card = new QFrame();
    card->setFixedHeight(260);
    card->setCursor(Qt::PointingHandCursor);
    card->setObjectName("headlineCard");

    // 深色渐变背景 + 玻璃边框效果
    card->setStyleSheet(QString(
        "QFrame#headlineCard {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "        stop:0 #1A1A2E, stop:0.4 #16213E, stop:1 #0F0F23);"
        "    border-radius: %1px;"
        "    border: 1px solid rgba(255,255,255,0.08);"
        "}"
        "QFrame#headlineCard:hover {"
        "    border: 1.5px solid %2;"
        "}"
    ).arg(StyleConfig::RADIUS_XL).arg(StyleConfig::PATRIOTIC_RED));

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // 左侧：文字区
    QWidget *textArea = new QWidget();
    textArea->setStyleSheet("background: transparent;");
    QVBoxLayout *textLayout = new QVBoxLayout(textArea);
    textLayout->setContentsMargins(40, 36, 28, 36);
    textLayout->setSpacing(14);

    // 头条标签 - 更精致的渐变设计
    QWidget *tagContainer = new QWidget();
    QHBoxLayout *tagLayout = new QHBoxLayout(tagContainer);
    tagLayout->setContentsMargins(0, 0, 0, 0);
    tagLayout->setSpacing(10);

    QLabel *headlineTag = new QLabel("头条");
    headlineTag->setFixedSize(56, 28);
    headlineTag->setAlignment(Qt::AlignCenter);
    headlineTag->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #FFD700, stop:1 #FFA500);"
        "color: #1A1A1A; font-size: 13px; font-weight: 800; "
        "border-radius: 14px; letter-spacing: 1px;"
    );

    QLabel *liveTag = new QLabel("● LIVE");
    liveTag->setStyleSheet(
        "color: #FF6B6B; font-size: 11px; font-weight: 700; "
        "background: transparent; letter-spacing: 1px;"
    );

    tagLayout->addWidget(headlineTag);
    tagLayout->addWidget(liveTag);
    tagLayout->addStretch();

    // 标题 - 更大气的字体
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setMinimumHeight(50);
    titleLabel->setMaximumHeight(100);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleLabel->setStyleSheet(
        "font-size: 26px; font-weight: 800; color: white; "
        "line-height: 1.45; background: transparent; letter-spacing: 0.5px;"
    );

    // 摘要 - 更柔和的颜色
    QLabel *summaryLabel = new QLabel(news.summary.left(90) + (news.summary.length() > 90 ? "..." : ""));
    summaryLabel->setWordWrap(true);
    summaryLabel->setMaximumHeight(50);
    summaryLabel->setStyleSheet(
        "font-size: 15px; color: rgba(255,255,255,0.65); "
        "line-height: 1.6; background: transparent;"
    );

    // 底部信息 - 更精致的布局
    QWidget *metaWidget = new QWidget();
    metaWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *metaRow = new QHBoxLayout(metaWidget);
    metaRow->setContentsMargins(0, 0, 0, 0);
    metaRow->setSpacing(16);

    // 来源图标+文字
    QWidget *sourceWidget = new QWidget();
    QHBoxLayout *sourceLayout = new QHBoxLayout(sourceWidget);
    sourceLayout->setContentsMargins(0, 0, 0, 0);
    sourceLayout->setSpacing(6);
    QLabel *sourceIcon = new QLabel();
    sourceIcon->setPixmap(QIcon(":/icons/resources/icons/document.svg").pixmap(14, 14));
    QLabel *sourceLabel = new QLabel(news.source);
    sourceLabel->setStyleSheet("color: rgba(255,255,255,0.45); font-size: 13px; background: transparent;");
    sourceLayout->addWidget(sourceIcon);
    sourceLayout->addWidget(sourceLabel);

    // 时间图标+文字
    QWidget *timeWidget = new QWidget();
    QHBoxLayout *timeLayout = new QHBoxLayout(timeWidget);
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(6);
    QLabel *timeIcon = new QLabel();
    timeIcon->setPixmap(QIcon(":/icons/resources/icons/book.svg").pixmap(14, 14));
    QLabel *timeLabel = new QLabel(news.publishTime.isValid() ? news.publishTime.toString("MM-dd HH:mm") : "刚刚");
    timeLabel->setStyleSheet("color: rgba(255,255,255,0.45); font-size: 13px; background: transparent;");
    timeLayout->addWidget(timeIcon);
    timeLayout->addWidget(timeLabel);

    // 生成按钮 - 更酷炫的渐变
    QPushButton *generateBtn = new QPushButton("AI 生成教学案例");
    generateBtn->setIcon(QIcon(":/icons/resources/icons/sparkle.svg"));
    generateBtn->setIconSize(QSize(18, 18));
    generateBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #E53935, stop:1 #FF5722);"
        "    color: white;"
        "    border: none;"
        "    padding: 10px 24px;"
        "    font-size: 13px;"
        "    font-weight: 700;"
        "    border-radius: 17px;"
        "    letter-spacing: 0.5px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #EF5350, stop:1 #FF7043);"
        "}"
        "QPushButton:pressed {"
        "    background: #C62828;"
        "}"
    );
    generateBtn->setCursor(Qt::PointingHandCursor);

    connect(generateBtn, &QPushButton::clicked, this, [this, news]() {
        onGenerateTeachingClicked(news);
    });

    metaRow->addWidget(sourceWidget);
    metaRow->addWidget(timeWidget);
    metaRow->addStretch();
    metaRow->addWidget(generateBtn);

    textLayout->addWidget(tagContainer);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(summaryLabel);
    textLayout->addStretch();
    textLayout->addWidget(metaWidget);

    cardLayout->addWidget(textArea, 3);

    // 右侧：图片区 - 添加渐变遮罩效果
    if (!news.imageUrl.isEmpty()) {
        QLabel *imageLabel = new QLabel();
        imageLabel->setFixedWidth(420);
        imageLabel->setStyleSheet(
            "background-color: #1A1A2E;"
            "border-top-right-radius: 24px;"
            "border-bottom-right-radius: 24px;"
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

// 创建普通新闻卡片 - 精致的左右布局，悬停动效
QWidget* HotspotTrackingWidget::createNewsCard(const NewsItem &news)
{
    QFrame *card = new QFrame();
    card->setObjectName("newsCard");
    card->setMinimumHeight(160);
    card->setMaximumHeight(190);
    card->setCursor(Qt::PointingHandCursor);

    // 精致的卡片样式
    card->setStyleSheet(QString(
        "QFrame#newsCard {"
        "    background-color: %1;"
        "    border-radius: %2px;"
        "    border: 1px solid %3;"
        "}"
        "QFrame#newsCard:hover {"
        "    border-color: %4;"
        "    background-color: %5;"
        "}"
    ).arg(StyleConfig::BG_CARD).arg(StyleConfig::RADIUS_L).arg(StyleConfig::BORDER_LIGHT,
         StyleConfig::PATRIOTIC_RED_LIGHT, "#FFFBFB"));

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // 左侧：图片区域（只有有图才显示）
    if (!news.imageUrl.isEmpty()) {
        QLabel *imageLabel = new QLabel();
        imageLabel->setFixedSize(160, 160);
        imageLabel->setStyleSheet(
            "background-color: #F5F7FA;"
            "border-top-left-radius: 16px;"
            "border-bottom-left-radius: 16px;"
        );
        imageLabel->setAlignment(Qt::AlignCenter);
        loadImage(news.imageUrl, imageLabel);
        cardLayout->addWidget(imageLabel);
    }

    // 右侧：文字区域
    QWidget *textArea = new QWidget();
    textArea->setStyleSheet("background: transparent;");
    QVBoxLayout *textLayout = new QVBoxLayout(textArea);
    textLayout->setContentsMargins(20, 18, 20, 18);
    textLayout->setSpacing(10);

    // 顶部：分类 + 热度
    QWidget *topWidget = new QWidget();
    topWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *topRow = new QHBoxLayout(topWidget);
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(10);

    // 分类标签 - 更精致的样式
    QLabel *categoryLabel = new QLabel(news.category);
    bool isInternational = news.category == "国际";
    QString catColor = isInternational ? "#2563EB" : StyleConfig::PATRIOTIC_RED;
    QString catBg = isInternational ? "#EFF6FF" : "#FEF2F2";
    QString catBorder = isInternational ? "#BFDBFE" : "#FECACA";
    categoryLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 11px; "
        "padding: 3px 12px; border-radius: 11px; font-weight: 700; "
        "border: 1px solid %3;"
    ).arg(catBg, catColor, catBorder));

    // 热度指示器 - 火焰图标 + 分数
    QString hotColor, hotBg;
    if (news.hotScore >= 80) {
        hotColor = "#DC2626"; hotBg = "#FEF2F2";
    } else if (news.hotScore >= 50) {
        hotColor = "#D97706"; hotBg = "#FFFBEB";
    } else {
        hotColor = "#6B7280"; hotBg = "#F3F4F6";
    }

    QWidget *hotContainer = new QWidget();
    hotContainer->setStyleSheet("background: transparent;");
    QHBoxLayout *hotLayout = new QHBoxLayout(hotContainer);
    hotLayout->setContentsMargins(0, 0, 0, 0);
    hotLayout->setSpacing(4);

    if (news.hotScore > 0) {
        QLabel *fireIcon = new QLabel();
        QPixmap firePix(":/icons/resources/icons/fire.svg");
        fireIcon->setPixmap(firePix.scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        fireIcon->setStyleSheet("background: transparent;");

        QLabel *hotLabel = new QLabel(QString::number(news.hotScore));
        hotLabel->setStyleSheet(QString(
            "background-color: %1; color: %2; font-size: 12px; "
            "font-weight: 700; padding: 2px 10px; border-radius: 10px;"
        ).arg(hotBg, hotColor));

        hotLayout->addWidget(fireIcon);
        hotLayout->addWidget(hotLabel);
    }

    topRow->addWidget(categoryLabel);
    topRow->addStretch();
    topRow->addWidget(hotContainer);
    textLayout->addWidget(topWidget);

    // 标题 - 更好的排版
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setMinimumHeight(42);
    titleLabel->setMaximumHeight(72);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleLabel->setStyleSheet(QString(
        "font-size: 16px; font-weight: 700; color: %1; "
        "line-height: 1.55; background: transparent; border: none;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    textLayout->addWidget(titleLabel);

    textLayout->addStretch();

    // 底部：来源、时间、按钮
    QWidget *bottomWidget = new QWidget();
    bottomWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *bottomRow = new QHBoxLayout(bottomWidget);
    bottomRow->setContentsMargins(0, 0, 0, 0);
    bottomRow->setSpacing(6);

    // 来源和时间组合
    QLabel *sourceLabel = new QLabel(news.source);
    sourceLabel->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent;").arg(StyleConfig::TEXT_LIGHT));

    QLabel *dotLabel = new QLabel("·");
    dotLabel->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent;").arg(StyleConfig::TEXT_LIGHT));

    QString timeText = formatTimeAgo(news.publishTime);
    QLabel *timeLabel = new QLabel(timeText);
    timeLabel->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent;").arg(StyleConfig::TEXT_LIGHT));

    // 生成案例按钮 - 更精致的悬停效果
    QPushButton *generateBtn = new QPushButton("生成案例");
    generateBtn->setIcon(QIcon(":/icons/resources/icons/sparkle.svg"));
    generateBtn->setIconSize(QSize(12, 12));
    generateBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: %1;"
        "    border: 1.5px solid %2;"
        "    padding: 5px 14px;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    border-radius: 13px;"
        "}"
        "QPushButton:hover {"
        "    color: white;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 %3, stop:1 #FF5722);"
        "    border: none;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #C62828;"
        "}"
    ).arg(StyleConfig::TEXT_SECONDARY, StyleConfig::BORDER_LIGHT, StyleConfig::PATRIOTIC_RED));
    generateBtn->setCursor(Qt::PointingHandCursor);

    connect(generateBtn, &QPushButton::clicked, this, [this, news]() {
        onGenerateTeachingClicked(news);
    });

    bottomRow->addWidget(sourceLabel);
    bottomRow->addWidget(dotLabel);
    bottomRow->addWidget(timeLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(generateBtn);
    textLayout->addWidget(bottomWidget);

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
    // 取消所有待处理的图片请求，避免悬空指针导致图片错位
    for (auto it = m_pendingImages.begin(); it != m_pendingImages.end(); ++it) {
        QNetworkReply *reply = it.key();
        reply->abort();
        reply->deleteLater();
    }
    m_pendingImages.clear();
    
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
    if (width < 650) return 1;
    if (width < 1150) return 2;
    return 3;
}

void HotspotTrackingWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    int cols = computeGridColumns();
    if (cols != m_lastColumnCount) {
        m_lastColumnCount = cols;
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

    if (!m_difyService) {
        QMessageBox::warning(this, "提示", "AI服务未就绪，请稍后重试");
        return;
    }

    // 查找点击的按钮并显示状态
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        btn->setText(" 生成中...");
        btn->setEnabled(false);
    }

    // 发出信号，让主窗口处理页面切换和消息发送
    emit teachingContentRequested(news);
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
    dialog->setMinimumSize(780, 650);
    dialog->setStyleSheet(
        "QDialog {"
        "    background-color: white;"
        "    border-radius: 24px;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 24);
    layout->setSpacing(0);

    // 顶部图片区 - 更大气的设计
    if (!news.imageUrl.isEmpty()) {
        QLabel *bigImage = new QLabel();
        bigImage->setFixedHeight(280);
        bigImage->setStyleSheet(
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            "    stop:0 #F3F4F6, stop:1 #E5E7EB);"
            "border-top-left-radius: 24px;"
            "border-top-right-radius: 24px;"
        );
        bigImage->setAlignment(Qt::AlignCenter);
        loadImage(news.imageUrl, bigImage);
        layout->addWidget(bigImage);
    }

    // 内容区
    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(36, 28, 36, 24);
    contentLayout->setSpacing(18);

    // 分类标签 - 更精致
    QLabel *categoryLabel = new QLabel(news.category);
    bool isInternational = news.category == "国际";
    QString catColor = isInternational ? "#2563EB" : StyleConfig::PATRIOTIC_RED;
    QString catBg = isInternational ? "#EFF6FF" : "#FEF2F2";
    QString catBorder = isInternational ? "#BFDBFE" : "#FECACA";
    categoryLabel->setStyleSheet(QString(
        "background-color: %1; color: %2; font-size: 13px; "
        "padding: 5px 16px; border-radius: 14px; font-weight: 700;"
        "border: 1px solid %3;"
    ).arg(catBg, catColor, catBorder));
    categoryLabel->setFixedWidth(categoryLabel->sizeHint().width() + 32);
    contentLayout->addWidget(categoryLabel);

    // 标题 - 更大气
    QLabel *titleLabel = new QLabel(news.title);
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet(QString(
        "font-size: 28px; font-weight: 800; color: %1; line-height: 1.4; letter-spacing: 0.5px;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    contentLayout->addWidget(titleLabel);

    // 元信息 - 带图标
    QWidget *metaContainer = new QWidget();
    metaContainer->setStyleSheet("background: transparent;");
    QHBoxLayout *metaLayout = new QHBoxLayout(metaContainer);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(24);

    auto createMetaItem = [](const QString &iconPath, const QString &text) {
        QWidget *item = new QWidget();
        item->setStyleSheet("background: transparent;");
        QHBoxLayout *l = new QHBoxLayout(item);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(8);
        QLabel *ic = new QLabel();
        ic->setPixmap(QIcon(iconPath).pixmap(16, 16));
        ic->setStyleSheet("background: transparent;");
        QLabel *tx = new QLabel(text);
        tx->setStyleSheet("color: #6B7280; font-size: 14px; font-weight: 500; background: transparent;");
        l->addWidget(ic);
        l->addWidget(tx);
        return item;
    };

    metaLayout->addWidget(createMetaItem(":/icons/resources/icons/book.svg",
        news.publishTime.isValid() ? news.publishTime.toString("yyyy年MM月dd日 HH:mm") : "刚刚"));
    metaLayout->addWidget(createMetaItem(":/icons/resources/icons/document.svg", news.source));
    metaLayout->addStretch();

    contentLayout->addWidget(metaContainer);

    // 分割线 - 更柔和
    QFrame *line = new QFrame();
    line->setFixedHeight(1);
    line->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "stop:0 transparent, stop:0.2 #E5E7EB, stop:0.8 #E5E7EB, stop:1 transparent);");
    contentLayout->addWidget(line);

    // 正文 - 更好的阅读体验
    QTextEdit *contentEdit = new QTextEdit();
    QString bodyText = news.content.isEmpty() ? news.summary : news.content;
    contentEdit->setHtml(QString(
        "<div style='line-height: 2; font-size: 16px; color: #374151; "
        "font-family: -apple-system, BlinkMacSystemFont, sans-serif;'>%1</div>"
    ).arg(bodyText.replace("\n", "<br>")));
    contentEdit->setReadOnly(true);
    contentEdit->setFrameShape(QFrame::NoFrame);
    contentEdit->setStyleSheet(
        "QTextEdit { background: transparent; padding: 8px 0; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #E5E7EB; border-radius: 3px; }"
    );
    contentLayout->addWidget(contentEdit, 1);

    layout->addWidget(contentWidget, 1);

    // 底部按钮 - 更精致
    QWidget *btnContainer = new QWidget();
    btnContainer->setStyleSheet("background: white;");
    QHBoxLayout *btnLayout = new QHBoxLayout(btnContainer);
    btnLayout->setContentsMargins(36, 0, 36, 0);
    btnLayout->setSpacing(16);

    QPushButton *closeBtn = new QPushButton("关闭");
    closeBtn->setFixedSize(110, 46);
    closeBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1.5px solid %3;"
        "    border-radius: 23px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background-color: #E5E7EB;"
        "    border-color: #D1D5DB;"
        "}"
    ).arg(StyleConfig::BG_APP, StyleConfig::TEXT_SECONDARY, StyleConfig::BORDER_LIGHT));
    closeBtn->setCursor(Qt::PointingHandCursor);

    QPushButton *generateBtn = new QPushButton("AI 生成教学案例");
    generateBtn->setIcon(QIcon(":/icons/resources/icons/sparkle.svg"));
    generateBtn->setIconSize(QSize(18, 18));
    generateBtn->setFixedHeight(46);
    generateBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 %1, stop:1 #C62828);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 23px;"
        "    padding: 0 36px;"
        "    font-size: 14px;"
        "    font-weight: 700;"
        "    letter-spacing: 0.5px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #EF4444, stop:1 #D32F2F);"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED));
    generateBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addWidget(generateBtn);

    layout->addWidget(btnContainer);

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
