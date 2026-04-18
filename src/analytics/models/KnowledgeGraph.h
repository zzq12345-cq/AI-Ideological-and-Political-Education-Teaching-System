#ifndef KNOWLEDGEGRAPH_H
#define KNOWLEDGEGRAPH_H

#include <QString>
#include <QList>
#include <QSet>
#include <QMap>
#include <QPointF>

/**
 * @brief 知识图谱数据模型
 *
 * 定义知识点之间的关联关系，用于构建知识图谱可视化
 * 老王说：思政课的知识点是网状关联的，不是线性的
 */

/**
 * @brief 关系类型
 */
enum class RelationType {
    Prerequisite,    // 前置关系（必须先学）
    Related,         // 相关关系（知识点有联系）
    Extended,        // 扩展关系（深入拓展）
    CrossGrade       // 跨年级关联（高年级用到低年级知识）
};

/**
 * @brief 知识点节点
 */
struct KnowledgeNode {
    QString id;             // 知识点ID（如 "7s-1-1"）
    QString name;           // 知识点名称
    QString chapter;        // 所属章节
    QString grade;          // 年级学期
    QString examWeight;     // 考试权重（高频/常考）
    QStringList suitableTypes;  // 适合题型

    // 图谱相关
    mutable QPointF position;       // 可视化位置（mutable，因为布局需要修改）
    QSet<QString> parents;  // 前置知识点ID
    QSet<QString> children; // 后续知识点ID
    QSet<QString> related;  // 相关知识点ID

    // 状态
    bool mastered = false;  // 是否已掌握
    double masteryRate = 0.0;  // 掌握度 0-1
};

/**
 * @brief 知识点关系边
 */
struct KnowledgeEdge {
    QString from;           // 起点 ID
    QString to;             // 终点 ID
    RelationType type;      // 关系类型
    double weight = 1.0;    // 权重（影响布局）

    // 计算关系强度（用于可视化线条粗细）
    double strength() const {
        return weight;
    }
};

/**
 * @brief 知识图谱
 */
class KnowledgeGraph
{
public:
    // 构建完整图谱
    static const KnowledgeGraph& instance();

    // 获取所有节点
    const QList<KnowledgeNode>& nodes() const { return m_nodes; }
    QList<KnowledgeNode>& nodes() { return m_nodes; }

    // 获取所有边
    const QList<KnowledgeEdge>& edges() const { return m_edges; }

    // 查找节点
    KnowledgeNode* findNode(const QString& id);
    const KnowledgeNode* findNode(const QString& id) const;

    // 获取某个节点的前置/后续/相关节点
    QList<const KnowledgeNode*> getPrerequisites(const QString& id) const;
    QList<const KnowledgeNode*> getDependents(const QString& id) const;
    QList<const KnowledgeNode*> getRelated(const QString& id) const;

    // 获取两个节点之间的关系
    RelationType getRelation(const QString& from, const QString& to) const;

    // 计算学习路径（从起点到终点的最短路径）
    QList<QString> findPath(const QString& from, const QString& to) const;

    // 获取某个年级的起始节点（无前置的节点）
    QList<const KnowledgeNode*> getEntryNodes(const QString& grade) const;

    // 按章节分组
    QMap<QString, QList<const KnowledgeNode*>> groupByChapter(const QString& grade = QString()) const;

    // 搜索知识点（按名称或关键词）
    QList<const KnowledgeNode*> search(const QString& keyword) const;

    // 统计信息
    int nodeCount() const { return m_nodes.size(); }
    int edgeCount() const { return m_edges.size(); }

private:
    KnowledgeGraph();
    void buildGraph();
    void addRelations();  // 添加知识点之间的关联关系

    QList<KnowledgeNode> m_nodes;
    QList<KnowledgeEdge> m_edges;
    QMap<QString, int> m_nodeIndex;  // ID -> index in m_nodes
};

/**
 * @brief 关系类型转字符串（用于显示）
 */
inline QString relationTypeToString(RelationType type) {
    switch (type) {
        case RelationType::Prerequisite: return "prerequisite";
        case RelationType::Related: return "related";
        case RelationType::Extended: return "extended";
        case RelationType::CrossGrade: return "crossgrade";
    }
    return "unknown";
}

/**
 * @brief 关系类型转中文
 */
inline QString relationTypeToChinese(RelationType type) {
    switch (type) {
        case RelationType::Prerequisite: return "前置";
        case RelationType::Related: return "相关";
        case RelationType::Extended: return "拓展";
        case RelationType::CrossGrade: return "跨册";
    }
    return "未知";
}

#endif // KNOWLEDGEGRAPH_H
