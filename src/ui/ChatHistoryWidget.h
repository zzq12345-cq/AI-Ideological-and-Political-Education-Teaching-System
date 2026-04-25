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

    // 按 id 删除单个条目
    void removeHistoryItem(const QString &id);

    // 按 id 选中条目
    void selectItem(const QString &id);

    // 清除选中状态
    void clearSelection();

    // 返回条目数量
    int itemCount() const;

    // 更新标题与操作按钮文案
    void setHeaderTitle(const QString &title);
    void setNewButtonText(const QString &text);

    // 设置主题色（以适配不同模块的主题）
    void setThemeColors(const QString &accentColor, const QString &accentDarkColor,
                        const QString &selectedBgColor, const QString &selectedBorderColor);

signals:
    // 新建对话请求
    void newChatRequested();
    
    // 选中历史记录
    void historyItemSelected(const QString &conversationId);

    // 删除本地历史记录
    void historyDeleteRequested(const QString &conversationId);
    
    // 返回请求
    void backRequested();

private:
    void setupUI();
    void applySelectionState();

    QLabel *m_titleLabel = nullptr;
    QPushButton *m_newChatBtn = nullptr;
    QListWidget *m_listWidget = nullptr;
    QString m_selectedConversationId;
    QString m_accentColor = "#C62828";
    QString m_accentDarkColor = "#8E0000";
    QString m_selectedBgColor = "#FFF4F4";
    QString m_selectedBorderColor = "#F3C4C4";
};

#endif // CHATHISTORYWIDGET_H
