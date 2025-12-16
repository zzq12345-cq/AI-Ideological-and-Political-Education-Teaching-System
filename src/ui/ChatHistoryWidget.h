#ifndef CHATHISTORYWIDGET_H
#define CHATHISTORYWIDGET_H

#include <QWidget>
#include <QListWidget>

class ChatHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatHistoryWidget(QWidget *parent = nullptr);

    // 添加历史记录项
    void addHistoryItem(const QString &id, const QString &title, const QString &timeStr);
    
    // 清空列表
    void clearHistory();

signals:
    // 新建对话请求
    void newChatRequested();
    
    // 选中历史记录
    void historyItemSelected(const QString &conversationId);
    
    // 返回请求
    void backRequested();

private:
    void setupUI();

    QListWidget *m_listWidget;
};

#endif // CHATHISTORYWIDGET_H
