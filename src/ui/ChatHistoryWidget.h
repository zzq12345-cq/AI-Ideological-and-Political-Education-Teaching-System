#ifndef CHATHISTORYWIDGET_H
#define CHATHISTORYWIDGET_H

#include <QWidget>
#include <QListWidget>

class QLabel;
class QPushButton;

class ChatHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatHistoryWidget(QWidget *parent = nullptr);

    // 添加历史记录项
    void addHistoryItem(const QString &id, const QString &title, const QString &timeStr);

    // 在指定位置插入历史记录项
    void insertHistoryItem(int index, const QString &id, const QString &title, const QString &timeStr);

    // 清空列表
    void clearHistory();

    // 清除选中状态
    void clearSelection();

    // 更新标题与操作按钮文案
    void setHeaderTitle(const QString &title);
    void setNewButtonText(const QString &text);

signals:
    // 新建对话请求
    void newChatRequested();
    
    // 选中历史记录
    void historyItemSelected(const QString &conversationId);
    
    // 返回请求
    void backRequested();

private:
    void setupUI();

    QLabel *m_titleLabel = nullptr;
    QPushButton *m_newChatBtn = nullptr;
    QListWidget *m_listWidget = nullptr;
};

#endif // CHATHISTORYWIDGET_H
