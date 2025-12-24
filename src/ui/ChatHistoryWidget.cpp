#include "ChatHistoryWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QDateTime>
#include <QListWidgetItem>
#include <QFontMetrics>

// 自定义列表项组件
class HistoryItemWidget : public QWidget {
public:
    HistoryItemWidget(const QString &title, const QString &timeStr, QWidget *parent = nullptr) 
        : QWidget(parent)
    {
        // 设置鼠标穿透，让 QListWidget 处理悬停和点击选中效果
        setAttribute(Qt::WA_TransparentForMouseEvents);

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(12, 10, 12, 10); // 稍微增加内边距
        layout->setSpacing(12);

        // 1. 图标区域 - 圆形箭头图标
        QLabel *iconLabel = new QLabel(this);
        iconLabel->setFixedSize(20, 20);
        // 使用类似圆形箭头的 Unicode 字符，或者用 emoji
        iconLabel->setText("↻"); // 圆形箭头符号
        iconLabel->setStyleSheet("color: #8E8E93; border: none; font-size: 16px; background: transparent;");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        // 2. 标题和时间区域
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(3);
        textLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *titleLabel = new QLabel();
        titleLabel->setStyleSheet("font-weight: 500; font-size: 14px; color: #1C1C1E; border: none; background: transparent;");
        titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        titleLabel->setMaximumWidth(220);  // 限制最大宽度
        // 使用省略号显示过长的文字
        QFontMetrics fm(titleLabel->font());
        QString elidedTitle = fm.elidedText(title, Qt::ElideRight, 220);
        titleLabel->setText(elidedTitle);
        
        QLabel *timeLabel = new QLabel(timeStr);
        timeLabel->setStyleSheet("color: #8E8E93; font-size: 12px; border: none; background: transparent;"); 
        timeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(timeLabel);
        
        layout->addWidget(iconLabel);
        layout->addLayout(textLayout);
        layout->addStretch();
    }
};

ChatHistoryWidget::ChatHistoryWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ChatHistoryWidget::setupUI()
{
    setFixedWidth(300); //加宽侧边栏适应内容
    // 整体背景色 #F7F7F8，右侧边框
    setStyleSheet("ChatHistoryWidget { background-color: #F7F7F8; border-right: 1px solid #E5E5E5; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // 0. 顶部标题栏（带返回按钮）- 保持逻辑，优化样式
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(4);
    
    QPushButton *backBtn = new QPushButton("←"); // 建议换成 SVG 图标，这里暂时保持字符
    backBtn->setFixedSize(28, 28);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   color: #555555;"
        "   font-size: 16px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #EBEBEB;"
        "   color: #333333;"
        "}"
    );
    connect(backBtn, &QPushButton::clicked, this, &ChatHistoryWidget::backRequested);
    
    QLabel *titleLabel = new QLabel("对话历史");
    titleLabel->setStyleSheet("color: #666666; font-size: 13px; font-weight: 600; border: none;");
    
    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // 1. 新建对话按钮 - 极简风格
    QPushButton *newChatBtn = new QPushButton("    新建对话"); 
    // 图标处理：这里为了方便演示，还是用 Emoji 但建议用 QIcon
    // 更好的做法是 qproperty-icon: url(:/icons/plus.svg);
    // 暂时用文字排版模拟 "➕ 新建对话"
    newChatBtn->setText("➕  新建对话");
    newChatBtn->setFixedHeight(36); // 更矮一点，精致
    newChatBtn->setCursor(Qt::PointingHandCursor);
    newChatBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;" // 默认透明或极浅
        "   border: 1px solid #E0E0E0;" // 极细边框
        "   border-radius: 6px;" // 小圆角
        "   color: #444444;"
        "   font-size: 13px;"
        "   text-align: left;"
        "   padding-left: 10px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #EBEBEB;" // 悬停浅灰
        "   border-color: #D6D6D6;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #E0E0E0;"
        "}"
    );
    connect(newChatBtn, &QPushButton::clicked, this, &ChatHistoryWidget::newChatRequested);
    mainLayout->addWidget(newChatBtn);

    // 2. 历史记录列表 - 卡片样式
    m_listWidget = new QListWidget();
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSpacing(6); // 列表项之间的间距
    
    // 卡片样式：浅灰背景，蓝色选中边框
    m_listWidget->setStyleSheet(
        "QListWidget {"
        "   background: transparent;"
        "   outline: none;"
        "   border: none;"
        "}"
        "QListWidget::item {"
        "   background-color: #F2F2F7;" // 默认浅灰背景
        "   border: 2px solid transparent;"
        "   border-radius: 10px;"
        "   padding: 0px;" 
        "   min-height: 60px;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #E5E5EA;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #E8F0FE;"
        "   border: 2px solid #007AFF;" // 蓝色边框
        "}"
    );
    
    // 取消 Focus 虚线框
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    
    connect(m_listWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        QString id = item->data(Qt::UserRole).toString();
        emit historyItemSelected(id);
    });

    mainLayout->addWidget(m_listWidget);
}

void ChatHistoryWidget::addHistoryItem(const QString &id, const QString &title, const QString &timeStr)
{
    QListWidgetItem *item = new QListWidgetItem(m_listWidget);
    item->setData(Qt::UserRole, id);
    
    HistoryItemWidget *widget = new HistoryItemWidget(title, timeStr);
    
    // 设置item大小提示
    item->setSizeHint(widget->sizeHint());
    
    m_listWidget->addItem(item);
    m_listWidget->setItemWidget(item, widget);
}

void ChatHistoryWidget::insertHistoryItem(int index, const QString &id, const QString &title, const QString &timeStr)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setData(Qt::UserRole, id);
    
    HistoryItemWidget *widget = new HistoryItemWidget(title, timeStr);
    
    // 设置item大小提示
    item->setSizeHint(widget->sizeHint());
    
    // 在指定位置插入
    m_listWidget->insertItem(index, item);
    m_listWidget->setItemWidget(item, widget);
}

void ChatHistoryWidget::clearHistory()
{
    m_listWidget->clear();
}

void ChatHistoryWidget::clearSelection()
{
    m_listWidget->clearSelection();
}
