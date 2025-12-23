#ifndef MOCKNEWSPROVIDER_H
#define MOCKNEWSPROVIDER_H

#include "INewsProvider.h"
#include <QTimer>

/**
 * @brief 模拟新闻提供者
 * 
 * 用于开发和测试阶段，提供模拟的新闻数据。
 * 未来对接真实新闻库后，可以替换为真实实现。
 */
class MockNewsProvider : public INewsProvider {
    Q_OBJECT
    
public:
    explicit MockNewsProvider(QObject *parent = nullptr);
    ~MockNewsProvider() override = default;
    
    void fetchHotNews(int limit = 20, const QString &category = QString()) override;
    void searchNews(const QString &keyword) override;
    void fetchNewsDetail(const QString &newsId) override;
    void refresh() override;
    
private:
    QList<NewsItem> generateMockData();
    QList<NewsItem> m_mockNews;
};

#endif // MOCKNEWSPROVIDER_H
