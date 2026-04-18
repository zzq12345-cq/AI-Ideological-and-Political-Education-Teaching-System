#include "KnowledgeGraphWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QRandomGenerator>
#include <QWheelEvent>
#include <cmath>

// ============================================================================
// GraphNodeItem
// ============================================================================

// 辅助函数：获取节点半径
static inline double nodeRadius(const KnowledgeNode* node) {
    return (node->examWeight == "高频") ? GraphNodeItem::HIGH_FREQ_RADIUS : GraphNodeItem::BASE_RADIUS;
}

// 辅助函数：获取节点颜色
static inline QColor nodeColor(const KnowledgeNode* node) {
    if (node->examWeight == "高频") {
        return QColor(255, 120, 120);
    } else if (node->examWeight == "常考") {
        return QColor(255, 180, 100);
    }
    return QColor(100, 150, 255);
}

GraphNodeItem::GraphNodeItem(const KnowledgeNode* node, QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent)
    , m_node(node)
    , m_label(nullptr)
    , m_isHighlighted(false)
{
    double radius = ::nodeRadius(node);
    setRect(-radius, -radius, radius * 2, radius * 2);

    // 设置颜色
    m_baseColor = ::nodeColor(node);

    setBrush(QBrush(m_baseColor));
    setPen(QPen(QColor(60, 60, 60), 2));

    // 可交互
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);

    // 标签
    m_label = new QGraphicsTextItem(node->name, this);
    QFont font = m_label->font();
    font.setPointSize(9);
    m_label->setFont(font);
    m_label->setDefaultTextColor(QColor(40, 40, 40));

    // 居中标签
    QRectF labelRect = m_label->boundingRect();
    m_label->setPos(-labelRect.width() / 2, radius + 5);

    // 设置提示
    QString tooltip = QString("<b>%1</b><br>")
                      .arg(node->name) +
                      QString("章节：%1<br>").arg(node->chapter) +
                      QString("年级：%1<br>").arg(node->grade) +
                      QString("权重：%1<br>").arg(node->examWeight) +
                      QString("题型：%1").arg(node->suitableTypes.join("、"));
    setToolTip(tooltip);

    // 动画效果
    setOpacity(0);
}

void GraphNodeItem::updatePosition(const QPointF& pos)
{
    setPos(pos);

    // 淡入效果
    if (opacity() < 1.0) {
        setOpacity(opacity() + 0.1);
    }
}

void GraphNodeItem::highlight(bool highlight)
{
    m_isHighlighted = highlight;

    if (highlight) {
        setBrush(QBrush(QColor(100, 255, 150)));  // 高亮绿色
        setPen(QPen(QColor(0, 150, 50), 3));
        setZValue(100);
    } else {
        setBrush(QBrush(m_baseColor));
        setPen(QPen(QColor(60, 60, 60), 2));
        setZValue(10);
    }
}

void GraphNodeItem::setMastered(bool mastered)
{
    if (mastered) {
        setBrush(QBrush(QColor(100, 220, 150)));  // 掌握绿色
    } else {
        setBrush(QBrush(m_baseColor));
    }
}

void GraphNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsEllipseItem::hoverEnterEvent(event);

    // 悬停放大
    double radius = ::nodeRadius(m_node) * 1.2;
    setRect(-radius, -radius, radius * 2, radius * 2);
    setZValue(50);
}

void GraphNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsEllipseItem::hoverLeaveEvent(event);

    // 恢复大小
    double radius = ::nodeRadius(m_node);
    setRect(-radius, -radius, radius * 2, radius * 2);
    if (!m_isHighlighted) {
        setZValue(10);
    }
}

void GraphNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsEllipseItem::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        // 触发选中信号（通过父视图）
        if (scene() && scene()->views().size() > 0) {
            if (auto* view = qobject_cast<KnowledgeGraphWidget*>(scene()->views().first()->parentWidget())) {
                view->selectNode(m_node->id);
            }
        }
    }
}

// ============================================================================
// GraphEdgeItem
// ============================================================================

// 辅助函数：获取边颜色
static inline QColor edgeColor(RelationType type) {
    switch (type) {
        case RelationType::Prerequisite:
            return QColor(80, 80, 80);
        case RelationType::Related:
            return QColor(150, 150, 180);
        case RelationType::Extended:
            return QColor(100, 180, 150);
        case RelationType::CrossGrade:
            return QColor(200, 120, 180);
    }
    return QColor(150, 150, 150);
}

// 辅助函数：获取边宽度
static inline double edgeWidth(RelationType type) {
    switch (type) {
        case RelationType::Prerequisite:
            return GraphEdgeItem::PREREQUISITE_WIDTH;
        case RelationType::Related:
            return GraphEdgeItem::RELATED_WIDTH;
        case RelationType::Extended:
        case RelationType::CrossGrade:
            return GraphEdgeItem::CROSSGRADE_WIDTH;
    }
    return 1.5;
}

// 辅助函数：获取边线样式
static inline Qt::PenStyle edgePenStyle(RelationType type) {
    return (type == RelationType::Prerequisite || type == RelationType::CrossGrade)
        ? Qt::SolidLine
        : Qt::DashLine;
}

// 辅助函数：创建边画笔
static inline QPen edgePen(RelationType type, const QColor& color, double widthScale = 1.0) {
    return QPen(color, ::edgeWidth(type) * widthScale, edgePenStyle(type), Qt::RoundCap, Qt::RoundJoin);
}

GraphEdgeItem::GraphEdgeItem(const KnowledgeNode* from, const KnowledgeNode* to,
                             RelationType type, QGraphicsItem* parent)
    : QGraphicsLineItem(parent)
    , m_from(from)
    , m_to(to)
    , m_type(type)
    , m_isHighlighted(false)
{
    setPen(::edgePen(type, ::edgeColor(type)));

    setAcceptHoverEvents(true);
    setZValue(5);

    QString tooltip;
    switch (type) {
        case RelationType::Prerequisite:
            tooltip = QString("<b>前置关系</b><br>%1 → %2").arg(from->name, to->name);
            break;
        case RelationType::Related:
            tooltip = QString("<b>相关关系</b><br>%1 ↔ %2").arg(from->name, to->name);
            break;
        case RelationType::Extended:
            tooltip = QString("<b>拓展关系</b><br>%1 → %2").arg(from->name, to->name);
            break;
        case RelationType::CrossGrade:
            tooltip = QString("<b>跨册关联</b><br>%1 → %2").arg(from->name, to->name);
            break;
    }
    setToolTip(tooltip);
}

void GraphEdgeItem::updatePosition(const QPointF& from, const QPointF& to)
{
    setLine(from.x(), from.y(), to.x(), to.y());
}

void GraphEdgeItem::highlight(bool highlight)
{
    m_isHighlighted = highlight;

    if (highlight) {
        setPen(::edgePen(m_type, QColor(255, 200, 50), 1.5));
        setZValue(50);
    } else {
        setPen(::edgePen(m_type, ::edgeColor(m_type)));
        setZValue(5);
    }
}

void GraphEdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsLineItem::hoverEnterEvent(event);

    if (!m_isHighlighted) {
        setPen(::edgePen(m_type, ::edgeColor(m_type), 1.5));
    }
}

void GraphEdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsLineItem::hoverLeaveEvent(event);

    if (!m_isHighlighted) {
        setPen(::edgePen(m_type, ::edgeColor(m_type)));
    }
}

// ============================================================================
// KnowledgeGraphWidget
// ============================================================================

KnowledgeGraphWidget::KnowledgeGraphWidget(QWidget* parent)
    : QWidget(parent)
    , m_view(nullptr)
    , m_scene(nullptr)
    , m_graph(nullptr)
    , m_selectedNode(nullptr)
    , m_layoutTimer(nullptr)
    , m_layoutIteration(0)
    , m_scaleFactor(1.0)
{
    setupUI();
    setupGraph();
}

KnowledgeGraphWidget::~KnowledgeGraphWidget()
{
    stopLayoutAnimation();
}

void KnowledgeGraphWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 工具栏
    auto* toolbar = new QWidget;
    toolbar->setStyleSheet("background: #f5f5f5; border-bottom: 1px solid #ddd;");
    auto* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(10, 8, 10, 8);

    // 搜索框
    auto* searchLabel = new QLabel("搜索：");
    auto* searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("输入知识点名称...");
    searchEdit->setMaximumWidth(200);
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (!text.isEmpty()) {
            searchAndHighlight(text);
        } else {
            clearSelection();
        }
    });

    // 控制按钮
    auto* zoomInBtn = new QPushButton("放大");
    auto* zoomOutBtn = new QPushButton("缩小");
    auto* fitBtn = new QPushButton("适应");
    auto* refreshBtn = new QPushButton("重新布局");

    connect(zoomInBtn, &QPushButton::clicked, this, &KnowledgeGraphWidget::zoomIn);
    connect(zoomOutBtn, &QPushButton::clicked, this, &KnowledgeGraphWidget::zoomOut);
    connect(fitBtn, &QPushButton::clicked, this, &KnowledgeGraphWidget::fitToScreen);
    connect(refreshBtn, &QPushButton::clicked, this, [this]() {
        // 重置节点位置
        for (auto* node : m_visibleNodes) {
            node->position = QPointF(0, 0);
        }
        m_velocities.clear();
        startLayoutAnimation();
    });

    // 年级筛选
    auto* gradeLabel = new QLabel("年级：");
    auto* gradeCombo = new QComboBox;
    gradeCombo->addItem("全部", "");
    gradeCombo->addItem("七年级上册", "七年级上册");
    gradeCombo->addItem("七年级下册", "七年级下册");
    gradeCombo->addItem("八年级上册", "八年级上册");
    gradeCombo->addItem("八年级下册", "八年级下册");
    gradeCombo->addItem("九年级上册", "九年级上册");
    gradeCombo->addItem("九年级下册", "九年级下册");
    connect(gradeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, gradeCombo]() {
        filterByGrade(gradeCombo->currentData().toString());
    });

    toolbarLayout->addWidget(searchLabel);
    toolbarLayout->addWidget(searchEdit);
    toolbarLayout->addSpacing(20);
    toolbarLayout->addWidget(gradeLabel);
    toolbarLayout->addWidget(gradeCombo);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(zoomInBtn);
    toolbarLayout->addWidget(zoomOutBtn);
    toolbarLayout->addWidget(fitBtn);
    toolbarLayout->addWidget(refreshBtn);

    // 图形视图
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QBrush(QColor(250, 250, 252)));

    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    layout->addWidget(toolbar);
    layout->addWidget(m_view);
}

void KnowledgeGraphWidget::setupGraph()
{
    // 加载完整图谱
    loadGraph(KnowledgeGraph::instance());
}

void KnowledgeGraphWidget::loadGraph(const KnowledgeGraph& graph)
{
    m_graph = &graph;

    // 显示所有节点
    m_visibleNodes.clear();
    for (auto& node : m_graph->nodes()) {
        m_visibleNodes.append(&node);
    }

    createSceneItems();
    startLayoutAnimation();
}

void KnowledgeGraphWidget::filterByGrade(const QString& grade)
{
    if (!m_graph) return;

    m_filterGrade = grade;
    m_filterChapter.clear();
    m_visibleNodes.clear();

    for (auto& node : m_graph->nodes()) {
        if (grade.isEmpty() || node.grade == grade) {
            m_visibleNodes.append(const_cast<KnowledgeNode*>(&node));
        }
    }

    createSceneItems();
    startLayoutAnimation();
}

void KnowledgeGraphWidget::filterByChapter(const QString& grade, const QString& chapter)
{
    if (!m_graph) return;

    m_filterGrade = grade;
    m_filterChapter = chapter;
    m_visibleNodes.clear();

    for (auto& node : m_graph->nodes()) {
        if (node.grade == grade && node.chapter == chapter) {
            m_visibleNodes.append(const_cast<KnowledgeNode*>(&node));
        }
    }

    createSceneItems();
    startLayoutAnimation();
}

void KnowledgeGraphWidget::createSceneItems()
{
    // 清空场景
    m_scene->clear();
    m_nodeItems.clear();
    m_edgeItems.clear();
    m_velocities.clear();

    // 创建节点项
    for (auto* node : m_visibleNodes) {
        auto* item = new GraphNodeItem(node);
        m_scene->addItem(item);
        m_nodeItems[node->id] = item;

        // 初始随机位置
        double x = (QRandomGenerator::global()->bounded(200.0)) - 100;
        double y = (QRandomGenerator::global()->bounded(200.0)) - 100;
        item->setPos(x, y);
        node->position = QPointF(x, y);
        m_velocities[node->id] = QPointF(0, 0);
    }

    // 创建边项（只连接可见节点）
    QSet<QString> visibleIds;
    for (auto* node : m_visibleNodes) {
        visibleIds.insert(node->id);
    }

    for (const auto& edge : m_graph->edges()) {
        if (!visibleIds.contains(edge.from) || !visibleIds.contains(edge.to)) {
            continue;
        }

        auto* fromNode = m_graph->findNode(edge.from);
        auto* toNode = m_graph->findNode(edge.to);

        if (fromNode && toNode) {
            auto* edgeItem = new GraphEdgeItem(fromNode, toNode, edge.type);
            m_scene->addItem(edgeItem);
            m_edgeItems.append(edgeItem);
        }
    }

    // 设置场景大小
    m_scene->setSceneRect(-2000, -2000, 4000, 4000);
}

void KnowledgeGraphWidget::startLayoutAnimation()
{
    if (!m_layoutTimer) {
        m_layoutTimer = new QTimer(this);
        connect(m_layoutTimer, &QTimer::timeout, this, &KnowledgeGraphWidget::applyForces);
    }

    m_layoutIteration = 0;
    m_layoutTimer->start(LAYOUT_INTERVAL_MS);
}

void KnowledgeGraphWidget::stopLayoutAnimation()
{
    if (m_layoutTimer) {
        m_layoutTimer->stop();
    }
}

void KnowledgeGraphWidget::applyForces()
{
    if (m_layoutIteration >= MAX_LAYOUT_ITERATIONS) {
        stopLayoutAnimation();
        centerView();
        return;
    }

    // 力导向布局算法
    for (auto* node : m_visibleNodes) {
        QPointF force(0, 0);
        QPointF& pos = node->position;
        QPointF& vel = m_velocities[node->id];

        // 1. 斥力（所有节点之间）
        for (auto* other : m_visibleNodes) {
            if (node == other) continue;

            QPointF delta = pos - other->position;
            double dist = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
            if (dist < 1.0) dist = 1.0;  // 避免除零

            double f = REPULSION / (dist * dist);
            force += delta / dist * f;
        }

        // 2. 引力（有连接的节点之间）
        const auto& graphNode = m_graph->findNode(node->id);
        if (graphNode) {
            // 处理前置/后续关系
            const auto& connected = graphNode->parents + graphNode->children;
            for (const QString& connectedId : connected) {
                auto* connectedNode = m_graph->findNode(connectedId);
                if (!connectedNode || !m_visibleNodes.contains(connectedNode)) {
                    continue;
                }

                QPointF delta = connectedNode->position - pos;
                double dist = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
                if (dist < 1.0) dist = 1.0;

                double f = ATTRACTION * (dist - IDEAL_LENGTH);
                force += delta / dist * f;
            }

            // 处理相关关系（弱引力）
            for (const QString& relatedId : graphNode->related) {
                auto* relatedNode = m_graph->findNode(relatedId);
                if (!relatedNode || !m_visibleNodes.contains(const_cast<KnowledgeNode*>(relatedNode))) {
                    continue;
                }

                QPointF delta = relatedNode->position - pos;
                double dist = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
                if (dist < 1.0) dist = 1.0;

                double f = ATTRACTION * 0.3 * (dist - IDEAL_LENGTH * 1.5);
                force += delta / dist * f;
            }
        }

        // 3. 中心引力（让图谱居中）
        double distToCenter = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y());
        if (distToCenter > 0) {
            force += -pos / distToCenter * distToCenter * 0.01;
        }

        // 更新速度（带阻尼）
        vel = (vel + force * 0.05) * DAMPING;

        // 限制最大速度
        double velMag = std::sqrt(vel.x() * vel.x() + vel.y() * vel.y());
        if (velMag > 50) {
            vel = vel / velMag * 50;
        }

        // 更新位置
        pos += vel;
    }

    // 更新图形项位置
    updateNodePositions();

    m_layoutIteration++;
}

void KnowledgeGraphWidget::updateNodePositions()
{
    // 先更新节点
    for (auto* node : m_visibleNodes) {
        auto* item = m_nodeItems.value(node->id);
        if (item) {
            item->updatePosition(node->position);
        }
    }

    // 再更新边
    for (auto* edge : m_edgeItems) {
        auto* fromItem = m_nodeItems.value(edge->fromNode()->id);
        auto* toItem = m_nodeItems.value(edge->toNode()->id);
        if (fromItem && toItem) {
            edge->updatePosition(fromItem->pos(), toItem->pos());
        }
    }
}

void KnowledgeGraphWidget::centerView()
{
    m_view->centerOn(0, 0);
}

void KnowledgeGraphWidget::zoomIn()
{
    m_view->scale(1.2, 1.2);
    m_scaleFactor *= 1.2;
}

void KnowledgeGraphWidget::zoomOut()
{
    m_view->scale(1.0 / 1.2, 1.0 / 1.2);
    m_scaleFactor /= 1.2;
}

void KnowledgeGraphWidget::resetView()
{
    m_view->resetTransform();
    m_scaleFactor = 1.0;
    centerView();
}

void KnowledgeGraphWidget::fitToScreen()
{
    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    m_scaleFactor = 1.0;
}

void KnowledgeGraphWidget::selectNode(const QString& nodeId)
{
    // 清除之前的高亮
    for (auto* item : m_nodeItems) {
        item->highlight(false);
    }
    for (auto* edge : m_edgeItems) {
        edge->highlight(false);
    }

    // 查找并高亮选中节点
    m_selectedNode = m_graph->findNode(nodeId);
    if (m_selectedNode) {
        auto* item = m_nodeItems.value(nodeId);
        if (item) {
            item->highlight(true);
        }

        // 高亮相关的边和节点
        for (auto* edge : m_edgeItems) {
            if (edge->fromNode()->id == nodeId || edge->toNode()->id == nodeId) {
                edge->highlight(true);
                // 高亮连接的节点
                QString otherId = (edge->fromNode()->id == nodeId) ? edge->toNode()->id : edge->fromNode()->id;
                if (auto* otherItem = m_nodeItems.value(otherId)) {
                    otherItem->highlight(true);
                }
            }
        }

        emit nodeSelected(m_selectedNode);
        emit nodeClicked(m_selectedNode);
    }
}

void KnowledgeGraphWidget::clearSelection()
{
    for (auto* item : m_nodeItems) {
        item->highlight(false);
    }
    for (auto* edge : m_edgeItems) {
        edge->highlight(false);
    }
    m_selectedNode = nullptr;
}

void KnowledgeGraphWidget::highlightPath(const QString& from, const QString& to)
{
    auto path = m_graph->findPath(from, to);
    if (path.isEmpty()) {
        return;
    }

    // 高亮路径上的节点和边
    clearSelection();

    for (int i = 0; i < path.size(); ++i) {
        QString nodeId = path[i];
        if (auto* item = m_nodeItems.value(nodeId)) {
            item->highlight(true);
        }

        // 高亮路径上的边
        if (i > 0) {
            QString prevId = path[i - 1];
            for (auto* edge : m_edgeItems) {
                if ((edge->fromNode()->id == prevId && edge->toNode()->id == nodeId) ||
                    (edge->fromNode()->id == nodeId && edge->toNode()->id == prevId)) {
                    edge->highlight(true);
                }
            }
        }
    }
}

void KnowledgeGraphWidget::searchAndHighlight(const QString& keyword)
{
    if (!m_graph || keyword.isEmpty()) {
        clearSelection();
        return;
    }

    const auto allResults = m_graph->search(keyword);
    QList<const KnowledgeNode*> visibleResults;
    visibleResults.reserve(allResults.size());

    for (const auto* node : allResults) {
        if (m_nodeItems.contains(node->id)) {
            visibleResults.append(node);
        }
    }

    if (visibleResults.isEmpty()) {
        clearSelection();
        return;
    }

    // 高亮搜索结果
    clearSelection();
    for (const auto* node : visibleResults) {
        if (auto* item = m_nodeItems.value(node->id)) {
            item->highlight(true);
        }
    }

    // 如果只有一个结果，选中它
    if (visibleResults.size() == 1) {
        selectNode(visibleResults.first()->id);
    }
}

void KnowledgeGraphWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_scene && !m_scene->items().isEmpty() &&
        (!m_layoutTimer || !m_layoutTimer->isActive()) &&
        qFuzzyCompare(m_scaleFactor, 1.0)) {
        fitToScreen();
    }
}

void KnowledgeGraphWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (!m_layoutTimer || !m_layoutTimer->isActive()) {
        // 确保视图居中
        centerView();
    }
}

// ============================================================================
// 颜色配置
// ============================================================================

QColor KnowledgeGraphWidget::nodeColor(const KnowledgeNode* node) const
{
    if (node->examWeight == "高频") {
        return QColor(255, 120, 120);
    } else if (node->examWeight == "常考") {
        return QColor(255, 180, 100);
    }
    return QColor(100, 150, 255);
}

QColor KnowledgeGraphWidget::edgeColor(RelationType type) const
{
    switch (type) {
        case RelationType::Prerequisite:
            return QColor(80, 80, 80);     // 深灰色
        case RelationType::Related:
            return QColor(150, 150, 180);  // 浅蓝灰色
        case RelationType::Extended:
            return QColor(100, 180, 150);  // 浅绿色
        case RelationType::CrossGrade:
            return QColor(200, 120, 180);  // 紫红色
    }
    return QColor(150, 150, 150);
}

double KnowledgeGraphWidget::nodeRadius(const KnowledgeNode* node) const
{
    if (node->examWeight == "高频") {
        return GraphNodeItem::HIGH_FREQ_RADIUS;
    }
    return GraphNodeItem::BASE_RADIUS;
}

double KnowledgeGraphWidget::edgeWidth(RelationType type) const
{
    switch (type) {
        case RelationType::Prerequisite:
            return GraphEdgeItem::PREREQUISITE_WIDTH;
        case RelationType::Related:
            return GraphEdgeItem::RELATED_WIDTH;
        case RelationType::Extended:
        case RelationType::CrossGrade:
            return GraphEdgeItem::CROSSGRADE_WIDTH;
    }
    return 1.5;
}

// ============================================================================
// GraphLegendWidget
// ============================================================================

GraphLegendWidget::GraphLegendWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
}

void GraphLegendWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 10, 15, 10);

    auto* title = new QLabel("图例");
    title->setStyleSheet("font-weight: bold; font-size: 12px;");
    layout->addWidget(title);

    // 节点颜色
    auto* nodeSection = new QWidget;
    auto* nodeLayout = new QVBoxLayout(nodeSection);
    nodeLayout->setContentsMargins(0, 0, 0, 0);
    nodeLayout->setSpacing(5);

    addLegendItem(nodeLayout, "高频考点", QColor(255, 120, 120));
    addLegendItem(nodeLayout, "常考考点", QColor(255, 180, 100));
    addLegendItem(nodeLayout, "普通考点", QColor(100, 150, 255));
    addLegendItem(nodeLayout, "已掌握", QColor(100, 220, 150));

    layout->addWidget(nodeSection);

    // 边类型
    auto* edgeSection = new QWidget;
    auto* edgeLayout = new QVBoxLayout(edgeSection);
    edgeLayout->setContentsMargins(0, 0, 0, 0);
    edgeLayout->setSpacing(5);

    addEdgeLegendItem(edgeLayout, "前置关系", Qt::SolidLine, QColor(80, 80, 80));
    addEdgeLegendItem(edgeLayout, "相关关系", Qt::DashLine, QColor(150, 150, 180));
    addEdgeLegendItem(edgeLayout, "跨册关联", Qt::SolidLine, QColor(200, 120, 180));

    layout->addWidget(edgeSection);
    layout->addStretch();
}

void GraphLegendWidget::setupStyles()
{
    setStyleSheet(R"(
        GraphLegendWidget {
            background: #f9f9f9;
            border-radius: 8px;
        }
        QLabel {
            color: #333;
        }
    )");
}

void GraphLegendWidget::addLegendItem(QVBoxLayout* layout, const QString& text, const QColor& color)
{
    auto* item = new QWidget;
    auto* itemLayout = new QHBoxLayout(item);
    itemLayout->setContentsMargins(0, 0, 0, 0);

    auto* colorBox = new QLabel;
    colorBox->setFixedSize(16, 16);
    colorBox->setStyleSheet(QString("background: %1; border-radius: 3px;")
                           .arg(color.name()));

    auto* label = new QLabel(text);
    label->setStyleSheet("font-size: 11px;");

    itemLayout->addWidget(colorBox);
    itemLayout->addWidget(label, 1);
    itemLayout->addStretch();

    layout->addWidget(item);
}

void GraphLegendWidget::addEdgeLegendItem(QVBoxLayout* layout, const QString& text,
                                          Qt::PenStyle style, const QColor& color)
{
    auto* item = new QWidget;
    auto* itemLayout = new QHBoxLayout(item);
    itemLayout->setContentsMargins(0, 0, 0, 0);

    auto* lineLabel = new QLabel;
    lineLabel->setFixedSize(30, 16);
    lineLabel->setStyleSheet(QString(
        "border-bottom: 2px %1 %2;")
        .arg(style == Qt::DashLine ? "dashed" : "solid")
        .arg(color.name()));

    auto* label = new QLabel(text);
    label->setStyleSheet("font-size: 11px;");

    itemLayout->addWidget(lineLabel);
    itemLayout->addWidget(label, 1);
    itemLayout->addStretch();

    layout->addWidget(item);
}
