#ifndef HOTSPOTTRACKINGWIDGET_H
#define HOTSPOTTRACKINGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QFrame>
#include "../hotspot/NewsItem.h"

// 前向声明
class HotspotService;
class DifyService;

/**
 * @brief 政治热点追踪界面
 * 
 * 主要功能：
 * - 展示热点新闻列表（卡片式布局）
 * - 分类筛选（国内/国外）
 * - 搜索功能
 * - 查看新闻详情
 * - 生成教学案例（AI 辅助）
 */
class HotspotTrackingWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit HotspotTrackingWidget(QWidget *parent = nullptr);
    ~HotspotTrackingWidget();
    
    /**
     * @brief 设置服务
     */
    void setHotspotService(HotspotService *service);
    void setDifyService(DifyService *service);
    
    /**
     * @brief 刷新数据
     */
    void refresh();
    
signals:
    /**
     * @brief 请求生成教学内容
     */
    void teachingContentRequested(const NewsItem &news);
    
private slots:
    void onRefreshClicked();
    void onSearchTextChanged(const QString &text);
    void onCategoryChanged(int categoryIndex);
    void onNewsListUpdated(const QList<NewsItem> &newsList);
    void onNewsCardClicked(const NewsItem &news);
    void onGenerateTeachingClicked(const NewsItem &news);
    void onLoadingStateChanged(bool isLoading);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void setupStyles();
    void createHeader();
    void createCategoryFilter();
    void createNewsGrid();
    QWidget* createNewsCard(const NewsItem &news);
    void clearNewsGrid();
    void showNewsDetail(const NewsItem &news);
    
    // UI 组件
    QVBoxLayout *m_mainLayout;
    
    // 头部区域
    QFrame *m_headerFrame;
    QLabel *m_titleLabel;
    QPushButton *m_refreshBtn;
    QLineEdit *m_searchInput;
    
    // 分类筛选
    QFrame *m_categoryFrame;
    QButtonGroup *m_categoryGroup;
    QList<QPushButton*> m_categoryButtons;
    
    // 新闻列表区域
    QScrollArea *m_scrollArea;
    QWidget *m_newsContainer;
    QGridLayout *m_newsGridLayout;
    
    // 加载状态
    QLabel *m_loadingLabel;
    QLabel *m_emptyLabel;
    
    // 服务
    HotspotService *m_hotspotService;
    DifyService *m_difyService;
    
    // 状态
    QString m_currentCategory;
    QList<NewsItem> m_currentNews;
};

#endif // HOTSPOTTRACKINGWIDGET_H
