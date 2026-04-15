#include "ChatHistoryWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QDateTime>
#include <QListWidgetItem>
#include <QFontMetrics>
#include <QMenu>

namespace {
const QString kHistoryAccentColor = QStringLiteral("#C62828");
const QString kHistoryAccentDarkColor = QStringLiteral("#8E0000");
const QString kHistoryTitleColor = QStringLiteral("#1C1C1E");
const QString kHistorySecondaryColor = QStringLiteral("#8E8E93");
} // namespace

// 自定义列表项组件
class HistoryItemWidget : public QWidget {
public:
    HistoryItemWidget(const QString &title, const QString &timeStr, QWidget *parent = nullptr) 
        : QWidget(parent)
    {
        // 设置鼠标穿透，让 QListWidget 处理悬停和点击选中效果
        setAttribute(Qt::WA_TransparentForMouseEvents);

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 10, 12, 10);
        layout->setSpacing(12);

        m_accentBar = new QFrame(this);
        m_accentBar->setFixedWidth(4);
        m_accentBar->setStyleSheet(QStringLiteral("background: transparent; border-radius: 2px;"));
        m_accentBar->setAttribute(Qt::WA_TransparentForMouseEvents);
        layout->addWidget(m_accentBar);

        // 1. 图标区域 - 圆形箭头图标
        m_iconLabel = new QLabel(this);
        m_iconLabel->setFixedSize(20, 20);
        // 使用类似圆形箭头的 Unicode 字符，或者用 emoji
        m_iconLabel->setText(QStringLiteral("↻"));
        m_iconLabel->setAlignment(Qt::AlignCenter);
        m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        // 2. 标题和时间区域
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(3);
        textLayout->setContentsMargins(0, 0, 0, 0);
        
        m_titleLabel = new QLabel();
        m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_titleLabel->setMaximumWidth(212);
        // 使用省略号显示过长的文字
        QFontMetrics fm(m_titleLabel->font());
        QString elidedTitle = fm.elidedText(title, Qt::ElideRight, 212);
        m_titleLabel->setText(elidedTitle);
        
        m_timeLabel = new QLabel(timeStr);
        m_timeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        
        textLayout->addWidget(m_titleLabel);
        textLayout->addWidget(m_timeLabel);
        
        layout->addWidget(m_iconLabel);
        layout->addLayout(textLayout);
        layout->addStretch();

        updateAppearance();
    }

    void setSelected(bool selected)
    {
        if (m_selected == selected) {
            return;
        }

        m_selected = selected;
        updateAppearance();
    }

private:
    void updateAppearance()
    {
        const bool selected = m_selected;

        const QString accentColor = selected ? kHistoryAccentColor : QStringLiteral("transparent");
        const QString iconColor = selected ? kHistoryAccentColor : kHistorySecondaryColor;
        const QString titleColor = selected ? kHistoryAccentDarkColor : kHistoryTitleColor;
        const QString timeColor = selected ? kHistoryAccentColor : kHistorySecondaryColor;
        const QString titleWeight = selected ? QStringLiteral("600") : QStringLiteral("500");

        m_accentBar->setStyleSheet(QStringLiteral(
            "background: %1; border-radius: 2px;"
        ).arg(accentColor));
        m_iconLabel->setStyleSheet(QStringLiteral(
            "color: %1; border: none; font-size: 16px; background: transparent;"
        ).arg(iconColor));
        m_titleLabel->setStyleSheet(QStringLiteral(
            "font-weight: %1; font-size: 14px; color: %2; border: none; background: transparent;"
        ).arg(titleWeight, titleColor));
        m_timeLabel->setStyleSheet(QStringLiteral(
            "color: %1; font-size: 12px; border: none; background: transparent;"
        ).arg(timeColor));
    }
    QFrame *m_accentBar = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    bool m_selected = false;
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
    
    m_titleLabel = new QLabel("对话历史");
    m_titleLabel->setStyleSheet("color: #666666; font-size: 13px; font-weight: 600; border: none;");

    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // 1. 新建对话按钮 - 极简风格
    m_newChatBtn = new QPushButton("➕  新建对话");
    // 图标处理：这里为了方便演示，还是用 Emoji 但建议用 QIcon
    // 更好的做法是 qproperty-icon: url(:/icons/plus.svg);
    m_newChatBtn->setFixedHeight(36); // 更矮一点，精致
    m_newChatBtn->setCursor(Qt::PointingHandCursor);
    m_newChatBtn->setStyleSheet(
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
    connect(m_newChatBtn, &QPushButton::clicked, this, &ChatHistoryWidget::newChatRequested);
    mainLayout->addWidget(m_newChatBtn);

    // 2. 历史记录列表 - 卡片样式
    m_listWidget = new QListWidget();
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSpacing(8);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    
    m_listWidget->setStyleSheet(
        "QListWidget {"
        "   background: transparent;"
        "   outline: none;"
        "   border: none;"
        "}"
        "QListWidget::item {"
        "   background-color: #F2F2F7;"
        "   border: 1px solid transparent;"
        "   border-radius: 12px;"
        "   padding: 0px;"
        "   min-height: 64px;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #ECECF1;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #FFF4F4;"
        "   border: 1px solid #F3C4C4;"
        "}"
    );
    
    // 取消 Focus 虚线框
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    
    connect(m_listWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }

        m_selectedConversationId = item->data(Qt::UserRole).toString();
        applySelectionState();
        emit historyItemSelected(m_selectedConversationId);
    });

    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_listWidget, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *item = m_listWidget->itemAt(pos);
        if (!item) {
            return;
        }

        const QString id = item->data(Qt::UserRole).toString();
        if (id.isEmpty()) {
            return;
        }

        QMenu menu(this);
        QAction *deleteAction = menu.addAction(QStringLiteral("删除本地记录"));
        QAction *selectedAction = menu.exec(m_listWidget->viewport()->mapToGlobal(pos));
        if (selectedAction == deleteAction) {
            emit historyDeleteRequested(id);
        }
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
    if (id == m_selectedConversationId) {
        m_listWidget->setCurrentItem(item);
    }
    applySelectionState();
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
    if (id == m_selectedConversationId) {
        m_listWidget->setCurrentItem(item);
    }
    applySelectionState();
}

void ChatHistoryWidget::clearHistory()
{
    m_listWidget->clear();
}

void ChatHistoryWidget::clearSelection()
{
    m_selectedConversationId.clear();
    m_listWidget->clearSelection();
    m_listWidget->setCurrentItem(nullptr);
    applySelectionState();
}

void ChatHistoryWidget::setHeaderTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void ChatHistoryWidget::setNewButtonText(const QString &text)
{
    if (m_newChatBtn) {
        m_newChatBtn->setText(text);
    }
}

void ChatHistoryWidget::removeHistoryItem(const QString &id)
{
    if (!m_listWidget) return;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item && item->data(Qt::UserRole).toString() == id) {
            if (m_selectedConversationId == id) {
                m_selectedConversationId.clear();
            }
            delete m_listWidget->takeItem(i);
            applySelectionState();
            return;
        }
    }
}

void ChatHistoryWidget::selectItem(const QString &id)
{
    if (!m_listWidget) return;
    m_selectedConversationId = id;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item && item->data(Qt::UserRole).toString() == id) {
            m_listWidget->setCurrentItem(item);
            applySelectionState();
            return;
        }
    }

    applySelectionState();
}

int ChatHistoryWidget::itemCount() const
{
    return m_listWidget ? m_listWidget->count() : 0;
}

void ChatHistoryWidget::applySelectionState()
{
    if (!m_listWidget) {
        return;
    }

    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        QWidget *widget = m_listWidget->itemWidget(item);
        auto *historyWidget = static_cast<HistoryItemWidget *>(widget);
        if (!item || !historyWidget) {
            continue;
        }

        const bool isSelected = item->data(Qt::UserRole).toString() == m_selectedConversationId;
        item->setSelected(isSelected);
        historyWidget->setSelected(isSelected);
    }
}
