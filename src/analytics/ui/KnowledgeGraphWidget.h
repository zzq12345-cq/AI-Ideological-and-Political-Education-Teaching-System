#ifndef KNOWLEDGEGRAPHWIDGET_H
#define KNOWLEDGEGRAPHWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QMap>
#include <QTimer>
#include <QPropertyAnimation>
#include <QComboBox>
#include <QVBoxLayout>

#include "../models/KnowledgeGraph.h"

/**
 * @brief 知识点节点图形项
 */
class GraphNodeItem : public QGraphicsEllipseItem
{
public:
    explicit GraphNodeItem(const KnowledgeNode* node, QGraphicsItem* parent = nullptr);

    const KnowledgeNode* node() const { return m_node; }
    void updatePosition(const QPointF& pos);
    void highlight(bool highlight);
    void setMastered(bool mastered);

    // 节点半径
    constexpr static double BASE_RADIUS = 25.0;
    constexpr static double HIGH_FREQ_RADIUS = 35.0;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    const KnowledgeNode* m_node;
    QGraphicsTextItem* m_label;
    bool m_isHighlighted;
    QColor m_baseColor;
};

/**
 * @brief 知识点关系边图形项
 */
class GraphEdgeItem : public QGraphicsLineItem
{
public:
    explicit GraphEdgeItem(const KnowledgeNode* from, const KnowledgeNode* to,
                          RelationType type, QGraphicsItem* parent = nullptr);

    void updatePosition(const QPointF& from, const QPointF& to);
    RelationType relationType() const { return m_type; }
    void highlight(bool highlight);

    const KnowledgeNode* fromNode() const { return m_from; }
    const KnowledgeNode* toNode() const { return m_to; }

    // 线条宽度
    constexpr static double PREREQUISITE_WIDTH = 2.5;
    constexpr static double RELATED_WIDTH = 1.5;
    constexpr static double CROSSGRADE_WIDTH = 2.0;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    const KnowledgeNode* m_from;
    const KnowledgeNode* m_to;
    RelationType m_type;
    bool m_isHighlighted;
};

/**
 * @brief 知识图谱视图
 *
 * 老王说：用力导向布局让节点自动排布，看起来像神经元网络
 */
class KnowledgeGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KnowledgeGraphWidget(QWidget* parent = nullptr);
    ~KnowledgeGraphWidget();

    // 加载数据
    void loadGraph(const KnowledgeGraph& graph);
    void filterByGrade(const QString& grade);
    void filterByChapter(const QString& grade, const QString& chapter);

    // 视图控制
    void zoomIn();
    void zoomOut();
    void resetView();
    void fitToScreen();

    // 选中操作
    void selectNode(const QString& nodeId);
    void clearSelection();
    void highlightPath(const QString& from, const QString& to);

    // 搜索
    void searchAndHighlight(const QString& keyword);

    // 获取当前选中节点
    const KnowledgeNode* selectedNode() const { return m_selectedNode; }

signals:
    void nodeSelected(const KnowledgeNode* node);
    void nodeClicked(const KnowledgeNode* node);      // 点击节点
    void edgeHovered(const KnowledgeNode* from, const KnowledgeNode* to, RelationType type);

public slots:
    void startLayoutAnimation();
    void stopLayoutAnimation();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void setupGraph();
    void createSceneItems();
    void runForceDirectedLayout();
    void applyForces();
    void centerView();
    void updateNodePositions();

    // 颜色配置
    QColor nodeColor(const KnowledgeNode* node) const;
    QColor edgeColor(RelationType type) const;
    double nodeRadius(const KnowledgeNode* node) const;
    double edgeWidth(RelationType type) const;

    // UI 组件
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;

    // 图数据
    const KnowledgeGraph* m_graph;
    QList<const KnowledgeNode*> m_visibleNodes;

    // 图形项
    QMap<QString, GraphNodeItem*> m_nodeItems;
    QList<GraphEdgeItem*> m_edgeItems;

    // 选中状态
    const KnowledgeNode* m_selectedNode;
    QString m_filterGrade;
    QString m_filterChapter;

    // 布局动画
    QTimer* m_layoutTimer;
    int m_layoutIteration;
    constexpr static int MAX_LAYOUT_ITERATIONS = 300;
    constexpr static int LAYOUT_INTERVAL_MS = 16;  // ~60fps

    // 缩放
    double m_scaleFactor;

    // 力导向参数
    constexpr static double REPULSION = 5000.0;    // 斥力系数
    constexpr static double ATTRACTION = 0.05;     // 引力系数（弹簧）
    constexpr static double DAMPING = 0.85;        // 阻尼系数
    constexpr static double IDEAL_LENGTH = 120.0;  // 理想边长

    // 速度缓存（用于平滑动画）
    QMap<QString, QPointF> m_velocities;
};

/**
 * @brief 图谱图例面板
 */
class GraphLegendWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphLegendWidget(QWidget* parent = nullptr);

private:
    void setupUI();
    void setupStyles();
    void addLegendItem(QVBoxLayout* layout, const QString& text, const QColor& color);
    void addEdgeLegendItem(QVBoxLayout* layout, const QString& text,
                          Qt::PenStyle style, const QColor& color);
};

#endif // KNOWLEDGEGRAPHWIDGET_H
