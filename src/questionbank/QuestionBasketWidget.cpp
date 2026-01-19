#include "QuestionBasketWidget.h"
#include "QuestionBasket.h"

#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QScrollArea>

namespace {
constexpr int kCollapsedWidth = 100;
constexpr int kCollapsedHeight = 48;
constexpr int kExpandedWidth = 360;
constexpr int kExpandedHeight = 480;
constexpr int kItemHeight = 60;

void applyShadow(QWidget *target, qreal blurRadius, const QPointF &offset, const QColor &color)
{
    if (!target) return;
    auto *shadow = new QGraphicsDropShadowEffect(target);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(offset);
    shadow->setColor(color);
    target->setGraphicsEffect(shadow);
}
}

QuestionBasketWidget::QuestionBasketWidget(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("questionBasketWidget");
    setupUI();
    setupConnections();
    refresh();
}

void QuestionBasketWidget::setupUI()
{
    // 主布局使用堆叠效果：收起视图和展开视图
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 收起状态视图 ==========
    m_collapsedView = new QFrame(this);
    m_collapsedView->setObjectName("basketCollapsed");
    m_collapsedView->setFixedSize(kCollapsedWidth, kCollapsedHeight);
    m_collapsedView->setCursor(Qt::PointingHandCursor);
    m_collapsedView->setStyleSheet(
        "QFrame#basketCollapsed {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #D9001B, stop:1 #B80018);"
        "  border-radius: 24px;"
        "}"
        "QFrame#basketCollapsed:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #E52B3C, stop:1 #C9001F);"
        "}"
    );
    applyShadow(m_collapsedView, 16, QPointF(0, 4), QColor(217, 0, 27, 100));

    auto *collapsedLayout = new QHBoxLayout(m_collapsedView);
    collapsedLayout->setContentsMargins(12, 8, 12, 8);
    collapsedLayout->setSpacing(6);

    auto *iconLabel = new QLabel(m_collapsedView);
    iconLabel->setText("试题篮");
    iconLabel->setStyleSheet("QLabel { font-size: 13px; font-weight: bold; color: #fff; background: transparent; }");

    m_countBadge = new QLabel(m_collapsedView);
    m_countBadge->setObjectName("countBadge");
    m_countBadge->setMinimumSize(22, 22);
    m_countBadge->setAlignment(Qt::AlignCenter);
    m_countBadge->setStyleSheet(
        "QLabel#countBadge {"
        "  background: #fff;"
        "  color: #D9001B;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  border-radius: 11px;"
        "  padding: 2px 6px;"
        "}"
    );
    m_countBadge->setText("0");

    collapsedLayout->addWidget(iconLabel);
    collapsedLayout->addWidget(m_countBadge);

    // ========== 展开状态视图 ==========
    m_expandedView = new QFrame(this);
    m_expandedView->setObjectName("basketExpanded");
    m_expandedView->setFixedSize(kExpandedWidth, kExpandedHeight);
    m_expandedView->setStyleSheet(
        "QFrame#basketExpanded {"
        "  background: #fff;"
        "  border-radius: 16px;"
        "  border: 1px solid #e0e0e0;"
        "}"
    );
    m_expandedView->hide();
    applyShadow(m_expandedView, 30, QPointF(0, 10), QColor(0, 0, 0, 40));

    auto *expandedLayout = new QVBoxLayout(m_expandedView);
    expandedLayout->setContentsMargins(16, 16, 16, 16);
    expandedLayout->setSpacing(12);

    // 头部：标题 + 关闭按钮
    auto *headerRow = new QFrame(m_expandedView);
    auto *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *titleLabel = new QLabel("试题篮", headerRow);
    titleLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #333; }");

    m_countLabel = new QLabel("0 道题", headerRow);
    m_countLabel->setStyleSheet("QLabel { font-size: 14px; color: #666; }");

    m_toggleButton = new QPushButton("收起", headerRow);
    m_toggleButton->setObjectName("closeBasketButton");
    m_toggleButton->setCursor(Qt::PointingHandCursor);
    m_toggleButton->setStyleSheet(
        "QPushButton#closeBasketButton {"
        "  background: transparent;"
        "  color: #666;"
        "  border: none;"
        "  font-size: 14px;"
        "}"
        "QPushButton#closeBasketButton:hover {"
        "  color: #D9001B;"
        "}"
    );

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(m_countLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_toggleButton);

    expandedLayout->addWidget(headerRow);

    // 试题列表滚动区域
    m_scrollArea = new QScrollArea(m_expandedView);
    m_scrollArea->setObjectName("basketScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea#basketScrollArea { background: #fff; border: none; }"
        "QScrollArea#basketScrollArea > QWidget > QWidget { background: #fff; }"
    );

    auto *scrollContent = new QWidget(m_scrollArea);
    scrollContent->setStyleSheet("background: #fff;");
    m_questionListLayout = new QVBoxLayout(scrollContent);
    m_questionListLayout->setContentsMargins(0, 0, 0, 0);
    m_questionListLayout->setSpacing(8);
    m_questionListLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    expandedLayout->addWidget(m_scrollArea, 1);

    // 底部按钮区
    auto *buttonRow = new QFrame(m_expandedView);
    auto *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    m_clearButton = new QPushButton("清空", buttonRow);
    m_clearButton->setObjectName("clearBasketButton");
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setFixedHeight(40);
    m_clearButton->setStyleSheet(
        "QPushButton#clearBasketButton {"
        "  background: #f5f5f5;"
        "  color: #666;"
        "  border: 1px solid #ddd;"
        "  border-radius: 8px;"
        "  padding: 0 20px;"
        "  font-size: 14px;"
        "}"
        "QPushButton#clearBasketButton:hover {"
        "  background: #eee;"
        "  border-color: #ccc;"
        "}"
    );

    m_composeButton = new QPushButton("生成试卷", buttonRow);
    m_composeButton->setObjectName("composeButton");
    m_composeButton->setCursor(Qt::PointingHandCursor);
    m_composeButton->setFixedHeight(40);
    m_composeButton->setStyleSheet(
        "QPushButton#composeButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #D9001B, stop:1 #B80018);"
        "  color: #fff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 0 24px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#composeButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #E52B3C, stop:1 #C9001F);"
        "}"
        "QPushButton#composeButton:disabled {"
        "  background: #ccc;"
        "}"
    );

    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_composeButton, 1);

    expandedLayout->addWidget(buttonRow);

    // 添加到主布局
    mainLayout->addWidget(m_collapsedView);
    mainLayout->addWidget(m_expandedView);

    // 初始大小
    setFixedSize(kCollapsedWidth, kCollapsedHeight);
}

void QuestionBasketWidget::setupConnections()
{
    // 点击收起视图展开
    connect(m_collapsedView, &QFrame::destroyed, this, []{}); // placeholder
    m_collapsedView->installEventFilter(this);

    // 收起按钮
    connect(m_toggleButton, &QPushButton::clicked, this, &QuestionBasketWidget::onToggleExpand);

    // 生成试卷
    connect(m_composeButton, &QPushButton::clicked, this, &QuestionBasketWidget::onComposePaper);

    // 清空
    connect(m_clearButton, &QPushButton::clicked, this, &QuestionBasketWidget::onClearBasket);

    // 监听试题篮变化
    connect(QuestionBasket::instance(), &QuestionBasket::countChanged,
            this, &QuestionBasketWidget::onBasketCountChanged);
}

void QuestionBasketWidget::refresh()
{
    onBasketCountChanged(QuestionBasket::instance()->count());
    updateQuestionList();
}

void QuestionBasketWidget::onBasketCountChanged(int count)
{
    m_countBadge->setText(QString::number(count));
    m_countLabel->setText(QString("%1 道题").arg(count));
    m_composeButton->setEnabled(count > 0);
    m_clearButton->setEnabled(count > 0);

    // 更新列表
    updateQuestionList();
}

void QuestionBasketWidget::onToggleExpand()
{
    m_expanded = !m_expanded;

    if (m_expanded) {
        m_collapsedView->hide();
        m_expandedView->show();
        setFixedSize(kExpandedWidth, kExpandedHeight);
        updateQuestionList();
    } else {
        m_expandedView->hide();
        m_collapsedView->show();
        setFixedSize(kCollapsedWidth, kCollapsedHeight);
    }

    // 通知父窗口重新定位
    emit sizeChanged();
}

void QuestionBasketWidget::onComposePaper()
{
    emit composePaperRequested();
}

void QuestionBasketWidget::onClearBasket()
{
    QuestionBasket::instance()->clear();
}

void QuestionBasketWidget::onRemoveQuestion(const QString &questionId)
{
    QuestionBasket::instance()->removeQuestion(questionId);
    emit questionRemoved(questionId);
}

QWidget *QuestionBasketWidget::createQuestionItem(const PaperQuestion &question, int index)
{
    auto *item = new QFrame(this);
    item->setObjectName("basketQuestionItem");
    item->setFixedHeight(kItemHeight);
    item->setStyleSheet(
        "QFrame#basketQuestionItem {"
        "  background: #f8f9fa;"
        "  border-radius: 8px;"
        "  border: 1px solid #eee;"
        "}"
        "QFrame#basketQuestionItem:hover {"
        "  background: #f0f0f0;"
        "  border-color: #ddd;"
        "}"
    );

    auto *layout = new QHBoxLayout(item);
    layout->setContentsMargins(12, 8, 8, 8);
    layout->setSpacing(8);

    // 序号
    auto *indexLabel = new QLabel(QString::number(index), item);
    indexLabel->setFixedSize(24, 24);
    indexLabel->setAlignment(Qt::AlignCenter);
    indexLabel->setStyleSheet(
        "QLabel {"
        "  background: #D9001B;"
        "  color: #fff;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  border-radius: 12px;"
        "}"
    );

    // 题目预览
    QString preview = question.stem.left(30);
    if (question.stem.length() > 30) {
        preview += "...";
    }
    // 去除HTML标签
    preview.remove(QRegularExpression("<[^>]*>"));

    auto *contentLabel = new QLabel(preview, item);
    contentLabel->setStyleSheet("QLabel { color: #333; font-size: 13px; }");
    contentLabel->setWordWrap(false);

    // 题型标签
    static const QMap<QString, QString> typeNames = {
        {"single_choice", "选择"},
        {"fill_blank", "填空"},
        {"true_false", "判断"},
        {"material_essay", "材料"},
        {"short_answer", "简答"}
    };
    QString typeName = typeNames.value(question.questionType, question.questionType);
    auto *typeLabel = new QLabel(typeName, item);
    typeLabel->setStyleSheet(
        "QLabel {"
        "  background: #e3f2fd;"
        "  color: #1976d2;"
        "  font-size: 11px;"
        "  padding: 2px 6px;"
        "  border-radius: 4px;"
        "}"
    );

    // 删除按钮
    auto *removeButton = new QPushButton("×", item);
    removeButton->setFixedSize(24, 24);
    removeButton->setCursor(Qt::PointingHandCursor);
    removeButton->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #999;"
        "  border: none;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  color: #D9001B;"
        "}"
    );

    QString questionId = question.id;
    connect(removeButton, &QPushButton::clicked, this, [this, questionId]() {
        onRemoveQuestion(questionId);
    });

    layout->addWidget(indexLabel);
    layout->addWidget(contentLabel, 1);
    layout->addWidget(typeLabel);
    layout->addWidget(removeButton);

    return item;
}

void QuestionBasketWidget::updateQuestionList()
{
    // 清空现有列表
    while (m_questionListLayout->count() > 1) {  // 保留 stretch
        QLayoutItem *item = m_questionListLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // 添加试题项
    const QList<PaperQuestion> &questions = QuestionBasket::instance()->questions();
    for (int i = 0; i < questions.size(); ++i) {
        QWidget *item = createQuestionItem(questions[i], i + 1);
        m_questionListLayout->insertWidget(m_questionListLayout->count() - 1, item);
    }
}

// 重写事件过滤器以处理收起视图的点击
bool QuestionBasketWidget::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        if (!m_expanded && m_collapsedView->underMouse()) {
            onToggleExpand();
            return true;
        }
    }
    return QFrame::event(event);
}
