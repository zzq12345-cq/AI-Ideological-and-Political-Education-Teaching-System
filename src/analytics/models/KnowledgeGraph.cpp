#include "KnowledgeGraph.h"
#include "../../questionbank/CurriculumData.h"
#include <QQueue>
#include <QMap>
#include <algorithm>

/**
 * @brief 知识点关联定义
 *
 * 思政课知识点之间的内在联系：
 * - 前置关系：学习A之前必须先掌握B
 * - 相关关系：A和B有共同主题或可互相印证
 * - 扩展关系：A是B的深入或应用
 * - 跨册关系：高年级知识用到低年级基础
 */

// 知识点关联数据：from -> (to, type)
struct RelationDef {
    QString from;
    QString to;
    RelationType type;
    double weight = 1.0;
};

// 核心关联定义（人工梳理的思政知识内在逻辑）
static const QList<RelationDef> CORE_RELATIONS = {
    // === 七年级上册 ===

    // 第一单元：成长的节拍
    {"7s-1-1", "7s-1-2", RelationType::Prerequisite},  // 认识中学 -> 学会学习
    {"7s-1-1", "7s-1-3", RelationType::Prerequisite},  // 认识中学 -> 认识自己
    {"7s-1-2", "7s-1-3", RelationType::Related},       // 学会学习 <-> 认识自己（相辅相成）

    // 第二单元：友谊的天空
    {"7s-2-1", "7s-2-2", RelationType::Prerequisite},  // 友谊特质 -> 呵护友谊
    {"7s-2-2", "7s-2-3", RelationType::Prerequisite},  // 呵护友谊 -> 网友交友（先现实后网络）

    // 第三单元：师长情谊
    {"7s-3-1", "7s-3-2", RelationType::Prerequisite},  // 认识教师 -> 师生关系
    {"7s-3-2", "7s-3-3", RelationType::Related},       // 师生关系 <-> 亲子关系（都是"情谊"主题）

    // 第四单元：生命的思考
    {"7s-4-1", "7s-4-2", RelationType::Prerequisite},  // 敬畏生命 -> 守护生命
    {"7s-4-2", "7s-4-3", RelationType::Prerequisite},  // 守护生命 -> 生命意义

    // === 七年级下册 ===

    // 第一单元：青春时光
    {"7s-1-3", "7x-1-1", RelationType::CrossGrade},    // 认识自己(7上) -> 青春变化(7下)
    {"7x-1-1", "7x-1-2", RelationType::Prerequisite},  // 青春变化 -> 独立思考
    {"7x-1-2", "7x-1-3", RelationType::Prerequisite},  // 独立思考 -> 青春有格

    // 第二单元：做情绪情感的主人
    {"7x-2-1", "7x-2-2", RelationType::Prerequisite},  // 情绪作用 -> 情绪调节
    {"7x-2-2", "7x-2-3", RelationType::Prerequisite},  // 情绪调节 -> 情感升华
    {"7s-4-2", "7x-2-2", RelationType::Related},       // 守护生命(7上) <-> 情绪调节(7下)

    // 第三单元：在集体中成长
    {"7s-2-2", "7x-3-1", RelationType::CrossGrade},    // 友谊(7上) -> 集体(7下)
    {"7x-3-1", "7x-3-2", RelationType::Prerequisite},  // 集体作用 -> 集体规则
    {"7x-3-2", "7x-3-3", RelationType::Prerequisite},  // 集体规则 -> 共建集体

    // 第四单元：走进法治天地
    {"7x-4-1", "7x-4-2", RelationType::Prerequisite},  // 法律特征 -> 未成年人保护
    {"7x-4-2", "7x-4-3", RelationType::Prerequisite},  // 未成年人保护 -> 法治意识
    {"7s-4-1", "7x-4-1", RelationType::Related},       // 敬畏生命(7上) <-> 法律基础(7下)

    // === 八年级上册 ===

    // 第一单元：走进社会生活
    {"7x-3-1", "8s-1-1", RelationType::CrossGrade},    // 集体(7下) -> 社会(8上)
    {"8s-1-1", "8s-1-2", RelationType::Prerequisite},  // 个人与社会 -> 网络
    {"8s-1-2", "8s-1-3", RelationType::Prerequisite},  // 网络丰富 -> 理性上网
    {"7s-2-3", "8s-1-3", RelationType::CrossGrade},    // 网友交友(7上) -> 理性上网(8上)

    // 第二单元：遵守社会规则
    {"7x-4-3", "8s-2-1", RelationType::CrossGrade},    // 法治意识(7下) -> 社会规则(8上)
    {"8s-2-1", "8s-2-2", RelationType::Prerequisite},  // 社会规则 -> 道德诚信
    {"8s-2-2", "8s-2-3", RelationType::Prerequisite},  // 道德诚信 -> 违法犯罪

    // 第三单元：勇担社会责任
    {"7x-3-3", "8s-3-1", RelationType::CrossGrade},    // 集体责任(7下) -> 社会责任(8上)
    {"8s-3-1", "8s-3-2", RelationType::Prerequisite},  // 责任来源 -> 负责任的人
    {"8s-3-2", "8s-3-3", RelationType::Prerequisite},  // 负责任 -> 关爱社会

    // 第四单元：维护国家利益
    {"8s-3-3", "8s-4-1", RelationType::Prerequisite},  // 服务社会 -> 国家利益
    {"8s-4-1", "8s-4-2", RelationType::Prerequisite},  // 国家利益 -> 国家安全
    {"8s-4-2", "8s-4-3", RelationType::Prerequisite},  // 国家安全 -> 劳动创造

    // === 八年级下册（法治专册） ===

    // 第一单元：坚持宪法至上
    {"7x-4-1", "8x-1-1", RelationType::CrossGrade},    // 法律基础(7下) -> 宪法(8下)
    {"8s-2-3", "8x-1-1", RelationType::CrossGrade},    // 违法犯罪(8上) -> 宪法(8下)
    {"8x-1-1", "8x-1-2", RelationType::Prerequisite},  // 宪法地位 -> 宪法保障权利
    {"8x-1-2", "8x-1-3", RelationType::Prerequisite},  // 宪法保障权利 -> 依宪治国

    // 第二单元：理解权利义务
    {"8x-1-2", "8x-2-1", RelationType::Prerequisite},  // 宪法保障权利 -> 公民基本权利
    {"8x-2-1", "8x-2-2", RelationType::Prerequisite},  // 基本权利 -> 依法行使权利
    {"8x-2-2", "8x-2-3", RelationType::Prerequisite},  // 行使权利 -> 公民基本义务
    {"8x-2-3", "8x-2-4", RelationType::Prerequisite},  // 基本义务 -> 权利义务统一
    {"8x-2-4", "8x-2-5", RelationType::Prerequisite},  // 权利义务统一 -> 违反义务担责

    // 第三单元：人民当家作主
    {"8x-1-3", "8x-3-1", RelationType::Prerequisite},  // 依宪治国 -> 基本制度
    {"8x-3-1", "8x-3-2", RelationType::Prerequisite},  // 基本经济制度 -> 政治制度
    {"8x-3-2", "8x-3-3", RelationType::Prerequisite},  // 政治制度 -> 国家机构

    // 第四单元：崇尚法治精神
    {"8x-2-4", "8x-4-1", RelationType::Prerequisite},  // 权利义务统一 -> 自由平等
    {"8x-4-1", "8x-4-2", RelationType::Prerequisite},  // 自由平等 -> 公平正义
    {"8x-4-2", "8x-4-3", RelationType::Prerequisite},  // 公平正义 -> 守护正义

    // === 九年级上册 ===

    // 第一单元：富强与创新
    {"8x-3-1", "9s-1-1", RelationType::CrossGrade},    // 基本制度(8下) -> 改革开放(9上)
    {"9s-1-1", "9s-1-2", RelationType::Prerequisite},  // 改革开放 -> 共同富裕
    {"9s-1-2", "9s-1-3", RelationType::Prerequisite},  // 共同富裕 -> 创新驱动
    {"8s-4-3", "9s-1-3", RelationType::Related},       // 劳动创造(8上) <-> 创新驱动(9上)

    // 第二单元：民主与法治
    {"8x-3-2", "9s-2-1", RelationType::CrossGrade},    // 政治制度(8下) -> 社会主义民主(9上)
    {"8x-4-3", "9s-2-3", RelationType::CrossGrade},    // 守护正义(8下) -> 依法治国(9上)
    {"9s-2-1", "9s-2-2", RelationType::Prerequisite},  // 社会主义民主 -> 公民参与
    {"9s-2-2", "9s-2-3", RelationType::Prerequisite},  // 公民参与 -> 全面依法治国

    // 第三单元：文明与家园
    {"7s-4-3", "9s-3-1", RelationType::CrossGrade},    // 生命意义(7上) -> 文化传承(9上)
    {"9s-3-1", "9s-3-2", RelationType::Prerequisite},  // 文化传承 -> 核心价值观
    {"9s-3-2", "9s-3-3", RelationType::Prerequisite},  // 核心价值观 -> 绿色发展

    // 第四单元：和谐与梦想
    {"9s-1-2", "9s-4-1", RelationType::Related},       // 共同富裕 <-> 民族团结
    {"9s-4-1", "9s-4-2", RelationType::Prerequisite},  // 民族团结 -> 祖国统一
    {"9s-4-2", "9s-4-3", RelationType::Prerequisite},  // 祖国统一 -> 中国梦

    // === 九年级下册 ===

    // 第一单元：我们共同的世界
    {"8s-1-1", "9x-1-1", RelationType::CrossGrade},    // 个人与社会(8上) -> 经济全球化(9下)
    {"9s-1-3", "9x-1-1", RelationType::CrossGrade},    // 创新驱动(9上) -> 经济全球化(9下)
    {"9x-1-1", "9x-1-2", RelationType::Prerequisite},  // 全球化 -> 世界格局
    {"9x-1-2", "9x-1-3", RelationType::Prerequisite},  // 世界格局 -> 人类命运共同体

    // 第二单元：世界舞台上的中国
    {"9s-1-1", "9x-2-1", RelationType::CrossGrade},    // 改革开放(9上) -> 中国担当(9下)
    {"9x-2-1", "9x-2-2", RelationType::Prerequisite},  // 中国担当 -> 机遇挑战
    {"9x-2-2", "9x-2-3", RelationType::Prerequisite},  // 机遇挑战 -> 合作共赢

    // 第三单元：走向未来的少年
    {"7s-1-2", "9x-3-1", RelationType::CrossGrade},    // 学会学习(7上) -> 少年担当(9下)
    {"7s-1-3", "9x-3-1", RelationType::CrossGrade},    // 认识自己(7上) -> 少年担当(9下)
    {"9x-3-1", "9x-3-2", RelationType::Prerequisite},  // 少年担当 -> 学无止境
    {"9x-3-2", "9x-3-3", RelationType::Prerequisite},  // 学无止境 -> 规划未来

    // === 跨单元强关联 ===
    {"7s-1-3", "7s-4-3", RelationType::Related},       // 认识自己 <-> 生命意义
    {"7x-2-3", "7x-3-3", RelationType::Related},       // 情感正能量 <-> 集体责任
    {"8s-2-2", "8x-4-2", RelationType::CrossGrade},    // 诚信(8上) <-> 公平正义(8下)
    {"9s-3-2", "9s-2-1", RelationType::Related},       // 核心价值观 <-> 社会主义民主
    {"9x-1-3", "9s-4-3", RelationType::CrossGrade},    // 人类命运共同体(9下) <-> 中国梦(9上)
};

const KnowledgeGraph& KnowledgeGraph::instance()
{
    static KnowledgeGraph instance;
    return instance;
}

KnowledgeGraph::KnowledgeGraph()
{
    buildGraph();
}

void KnowledgeGraph::buildGraph()
{
    // 1. 创建所有节点（基于课标数据）
    const auto& knowledgePoints = CurriculumData::allKnowledgePoints();
    m_nodes.reserve(knowledgePoints.size());

    for (int i = 0; i < knowledgePoints.size(); ++i) {
        const auto& kp = knowledgePoints[i];
        KnowledgeNode node;
        node.id = kp.id;
        node.name = kp.name;
        node.chapter = kp.chapter;
        node.grade = kp.grade;
        node.examWeight = kp.examWeight;
        node.suitableTypes = kp.suitableTypes;
        node.position = QPointF(0, 0);  // 初始位置，由布局算法计算

        m_nodes.append(node);
        m_nodeIndex[kp.id] = i;
    }

    // 2. 添加关系边
    addRelations();

    // 3. 更新节点的邻接关系
    for (const auto& edge : m_edges) {
        if (auto* fromNode = findNode(edge.from)) {
            if (auto* toNode = findNode(edge.to)) {
                switch (edge.type) {
                    case RelationType::Prerequisite:
                    case RelationType::CrossGrade:
                        fromNode->children.insert(edge.to);
                        toNode->parents.insert(edge.from);
                        break;
                    case RelationType::Related:
                    case RelationType::Extended:
                        fromNode->related.insert(edge.to);
                        toNode->related.insert(edge.from);
                        break;
                }
            }
        }
    }
}

void KnowledgeGraph::addRelations()
{
    for (const auto& rel : CORE_RELATIONS) {
        // 确保两端节点都存在
        if (m_nodeIndex.contains(rel.from) && m_nodeIndex.contains(rel.to)) {
            KnowledgeEdge edge;
            edge.from = rel.from;
            edge.to = rel.to;
            edge.type = rel.type;
            edge.weight = rel.weight;
            m_edges.append(edge);
        }
    }
}

const KnowledgeNode* KnowledgeGraph::findNode(const QString& id) const
{
    auto it = m_nodeIndex.find(id);
    return it != m_nodeIndex.end() ? &m_nodes[it.value()] : nullptr;
}

KnowledgeNode* KnowledgeGraph::findNode(const QString& id)
{
    auto it = m_nodeIndex.find(id);
    return it != m_nodeIndex.end() ? &m_nodes[it.value()] : nullptr;
}

QList<const KnowledgeNode*> KnowledgeGraph::getPrerequisites(const QString& id) const
{
    QList<const KnowledgeNode*> result;
    if (const auto* node = findNode(id)) {
        for (const QString& parentId : node->parents) {
            if (const auto* parent = findNode(parentId)) {
                result.append(parent);
            }
        }
    }
    return result;
}

QList<const KnowledgeNode*> KnowledgeGraph::getDependents(const QString& id) const
{
    QList<const KnowledgeNode*> result;
    if (const auto* node = findNode(id)) {
        for (const QString& childId : node->children) {
            if (const auto* child = findNode(childId)) {
                result.append(child);
            }
        }
    }
    return result;
}

QList<const KnowledgeNode*> KnowledgeGraph::getRelated(const QString& id) const
{
    QList<const KnowledgeNode*> result;
    if (const auto* node = findNode(id)) {
        for (const QString& relatedId : node->related) {
            if (const auto* related = findNode(relatedId)) {
                result.append(related);
            }
        }
    }
    return result;
}

RelationType KnowledgeGraph::getRelation(const QString& from, const QString& to) const
{
    for (const auto& edge : m_edges) {
        if (edge.from == from && edge.to == to) {
            return edge.type;
        }
    }
    return RelationType::Related;  // 默认返回相关
}

QList<QString> KnowledgeGraph::findPath(const QString& from, const QString& to) const
{
    QList<QString> path;
    if (!m_nodeIndex.contains(from) || !m_nodeIndex.contains(to)) {
        return path;
    }

    // BFS 最短路径
    QMap<QString, QString> parent;
    QQueue<QString> queue;
    QSet<QString> visited;

    queue.enqueue(from);
    visited.insert(from);
    parent[from] = QString();

    while (!queue.isEmpty()) {
        QString current = queue.dequeue();

        if (current == to) {
            // 重建路径
            QString node = to;
            while (!node.isEmpty()) {
                path.prepend(node);
                node = parent[node];
            }
            return path;
        }

        // 遍历邻居（前置、后续、相关都算）
        if (const auto* node = findNode(current)) {
            const auto& neighbors = node->parents + node->children + node->related;
            for (const QString& neighbor : neighbors) {
                if (!visited.contains(neighbor)) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    queue.enqueue(neighbor);
                }
            }
        }
    }

    return path;  // 未找到路径
}

QList<const KnowledgeNode*> KnowledgeGraph::getEntryNodes(const QString& grade) const
{
    QList<const KnowledgeNode*> result;
    for (auto& node : m_nodes) {
        if (!grade.isEmpty() && node.grade != grade) {
            continue;
        }
        // 入口节点：无前置节点
        if (node.parents.isEmpty()) {
            result.append(&node);
        }
    }
    return result;
}

QMap<QString, QList<const KnowledgeNode*>> KnowledgeGraph::groupByChapter(const QString& grade) const
{
    QMap<QString, QList<const KnowledgeNode*>> result;
    for (auto& node : m_nodes) {
        if (!grade.isEmpty() && node.grade != grade) {
            continue;
        }
        result[node.chapter].append(&node);
    }
    return result;
}

QList<const KnowledgeNode*> KnowledgeGraph::search(const QString& keyword) const
{
    QList<const KnowledgeNode*> result;
    if (keyword.isEmpty()) {
        return result;
    }

    const auto lowerKeyword = keyword.toLower();

    for (auto& node : m_nodes) {
        // 搜索名称、章节
        if (node.name.toLower().contains(lowerKeyword) ||
            node.chapter.toLower().contains(lowerKeyword) ||
            node.id.toLower().contains(lowerKeyword)) {
            result.append(&node);
        }
    }
    return result;
}
