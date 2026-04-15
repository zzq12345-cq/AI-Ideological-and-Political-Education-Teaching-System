#ifndef CURRICULUMDATA_H
#define CURRICULUMDATA_H

#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>

struct KnowledgePoint {
    QString id;
    QString name;
    QString chapter;
    QString grade;
    QString examWeight;
    QStringList suitableTypes;
};

struct ChapterGuide {
    QString grade;
    QString chapter;
    QStringList highFrequencyPatterns;
};

namespace CurriculumData {

inline QString displayQuestionType(const QString &type)
{
    if (type == "single_choice") return "单选题";
    if (type == "multi_choice") return "多选题";
    if (type == "true_false") return "判断题";
    if (type == "short_answer") return "简答题";
    if (type == "essay") return "论述题";
    if (type == "material_analysis") return "材料分析题";
    return type;
}

inline QString baseGradeFromGradeSemester(const QString &gradeSemester)
{
    return gradeSemester.size() >= 4 ? gradeSemester.left(4) : gradeSemester;
}

inline const QList<KnowledgePoint> &allKnowledgePoints()
{
    static const QList<KnowledgePoint> points = {
        {"7s-1-1", "中学生活新起点", "第一单元 成长的节拍", "七年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"7s-1-2", "学会学习与终身成长", "第一单元 成长的节拍", "七年级上册", "高频", {"single_choice", "short_answer", "material_analysis"}},
        {"7s-1-3", "认识自己与做更好的自己", "第一单元 成长的节拍", "七年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"7s-2-1", "友谊的特质与力量", "第二单元 友谊的天空", "七年级上册", "高频", {"single_choice", "true_false", "short_answer"}},
        {"7s-2-2", "呵护友谊与交友原则", "第二单元 友谊的天空", "七年级上册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"7s-2-3", "慎重结交网友", "第二单元 友谊的天空", "七年级上册", "常考", {"single_choice", "true_false", "material_analysis"}},
        {"7s-3-1", "正确认识教师职业", "第三单元 师长情谊", "七年级上册", "常考", {"single_choice", "short_answer"}},
        {"7s-3-2", "建立教学相长的师生关系", "第三单元 师长情谊", "七年级上册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"7s-3-3", "孝亲敬长与建设和谐家庭", "第三单元 师长情谊", "七年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"7s-4-1", "生命的特点与敬畏生命", "第四单元 生命的思考", "七年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"7s-4-2", "守护生命与增强生命韧性", "第四单元 生命的思考", "七年级上册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"7s-4-3", "发现生命意义并活出精彩", "第四单元 生命的思考", "七年级上册", "常考", {"single_choice", "essay", "material_analysis"}},

        {"7x-1-1", "青春期生理与心理变化", "第一单元 青春时光", "七年级下册", "高频", {"single_choice", "true_false", "short_answer"}},
        {"7x-1-2", "独立思考与批判精神", "第一单元 青春时光", "七年级下册", "常考", {"single_choice", "material_analysis", "essay"}},
        {"7x-1-3", "青春有格与行己有耻", "第一单元 青春时光", "七年级下册", "高频", {"single_choice", "short_answer", "essay"}},
        {"7x-2-1", "情绪的作用与分类", "第二单元 做情绪情感的主人", "七年级下册", "常考", {"single_choice", "true_false"}},
        {"7x-2-2", "调节情绪的方法", "第二单元 做情绪情感的主人", "七年级下册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"7x-2-3", "体味美好情感并传递情感正能量", "第二单元 做情绪情感的主人", "七年级下册", "高频", {"single_choice", "short_answer", "essay"}},
        {"7x-3-1", "集体对个人成长的作用", "第三单元 在集体中成长", "七年级下册", "常考", {"single_choice", "short_answer"}},
        {"7x-3-2", "处理集体规则与个人意愿的关系", "第三单元 在集体中成长", "七年级下册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"7x-3-3", "共建美好集体并承担责任", "第三单元 在集体中成长", "七年级下册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"7x-4-1", "法律的特征和作用", "第四单元 走进法治天地", "七年级下册", "高频", {"single_choice", "short_answer"}},
        {"7x-4-2", "法律保护未成年人健康成长", "第四单元 走进法治天地", "七年级下册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"7x-4-3", "树立法治意识并依法办事", "第四单元 走进法治天地", "七年级下册", "高频", {"single_choice", "essay", "material_analysis"}},

        {"8s-1-1", "个人与社会的关系", "第一单元 走进社会生活", "八年级上册", "高频", {"single_choice", "short_answer"}},
        {"8s-1-2", "网络丰富日常生活", "第一单元 走进社会生活", "八年级上册", "常考", {"single_choice", "true_false"}},
        {"8s-1-3", "理性参与网络生活", "第一单元 走进社会生活", "八年级上册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"8s-2-1", "社会规则维护秩序", "第二单元 遵守社会规则", "八年级上册", "高频", {"single_choice", "short_answer"}},
        {"8s-2-2", "尊重他人、文明有礼、诚实守信", "第二单元 遵守社会规则", "八年级上册", "高频", {"single_choice", "material_analysis", "essay"}},
        {"8s-2-3", "违法与犯罪及善用法律", "第二单元 遵守社会规则", "八年级上册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"8s-3-1", "责任来自角色与关系", "第三单元 勇担社会责任", "八年级上册", "常考", {"single_choice", "short_answer"}},
        {"8s-3-2", "做负责任的人", "第三单元 勇担社会责任", "八年级上册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"8s-3-3", "关爱他人与服务社会", "第三单元 勇担社会责任", "八年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"8s-4-1", "国家利益与个人利益相统一", "第四单元 维护国家利益", "八年级上册", "高频", {"single_choice", "material_analysis", "essay"}},
        {"8s-4-2", "树立总体国家安全观", "第四单元 维护国家利益", "八年级上册", "高频", {"single_choice", "short_answer", "material_analysis"}},
        {"8s-4-3", "劳动创造未来与建设美好祖国", "第四单元 维护国家利益", "八年级上册", "常考", {"single_choice", "essay", "short_answer"}},

        {"8x-1-1", "宪法是国家的根本法", "第一单元 坚持宪法至上", "八年级下册", "高频", {"single_choice", "short_answer"}},
        {"8x-1-2", "宪法保障公民权利", "第一单元 坚持宪法至上", "八年级下册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"8x-1-3", "依宪治国与宪法监督", "第一单元 坚持宪法至上", "八年级下册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"8x-2-1", "公民基本权利", "第二单元 理解权利义务", "八年级下册", "高频", {"single_choice", "short_answer", "material_analysis"}},
        {"8x-2-2", "依法行使权利", "第二单元 理解权利义务", "八年级下册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"8x-2-3", "公民基本义务", "第二单元 理解权利义务", "八年级下册", "高频", {"single_choice", "short_answer"}},
        {"8x-2-4", "权利义务相统一", "第二单元 理解权利义务", "八年级下册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"8x-2-5", "违反义务必须担责", "第二单元 理解权利义务", "八年级下册", "常考", {"single_choice", "material_analysis", "true_false"}},
        {"8x-3-1", "我国基本经济制度", "第三单元 人民当家作主", "八年级下册", "高频", {"single_choice", "short_answer"}},
        {"8x-3-2", "根本政治制度和基本政治制度", "第三单元 人民当家作主", "八年级下册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"8x-3-3", "国家机构的性质与职权", "第三单元 人民当家作主", "八年级下册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"8x-4-1", "自由平等的真谛", "第四单元 崇尚法治精神", "八年级下册", "高频", {"single_choice", "short_answer"}},
        {"8x-4-2", "公平正义的价值", "第四单元 崇尚法治精神", "八年级下册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"8x-4-3", "守护公平正义", "第四单元 崇尚法治精神", "八年级下册", "高频", {"single_choice", "material_analysis", "essay"}},

        {"9s-1-1", "坚持改革开放", "第一单元 富强与创新", "九年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"9s-1-2", "走向共同富裕", "第一单元 富强与创新", "九年级上册", "高频", {"single_choice", "material_analysis", "short_answer"}},
        {"9s-1-3", "创新驱动发展", "第一单元 富强与创新", "九年级上册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"9s-2-1", "社会主义民主的本质与形式", "第二单元 民主与法治", "九年级上册", "高频", {"single_choice", "short_answer"}},
        {"9s-2-2", "公民参与民主生活", "第二单元 民主与法治", "九年级上册", "高频", {"single_choice", "material_analysis", "essay"}},
        {"9s-2-3", "全面依法治国", "第二单元 民主与法治", "九年级上册", "高频", {"single_choice", "short_answer", "material_analysis"}},
        {"9s-3-1", "中华文化薪火相传", "第三单元 文明与家园", "九年级上册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"9s-3-2", "社会主义核心价值观", "第三单元 文明与家园", "九年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"9s-3-3", "坚持绿色发展建设美丽中国", "第三单元 文明与家园", "九年级上册", "高频", {"single_choice", "material_analysis", "essay"}},
        {"9s-4-1", "维护民族团结", "第四单元 和谐与梦想", "九年级上册", "高频", {"single_choice", "short_answer", "essay"}},
        {"9s-4-2", "维护祖国统一与一国两制", "第四单元 和谐与梦想", "九年级上册", "高频", {"single_choice", "material_analysis", "essay"}},
        {"9s-4-3", "实现中华民族伟大复兴中国梦", "第四单元 和谐与梦想", "九年级上册", "高频", {"single_choice", "essay", "short_answer"}},

        {"9x-1-1", "经济全球化与文化多样性", "第一单元 我们共同的世界", "九年级下册", "高频", {"single_choice", "short_answer", "material_analysis"}},
        {"9x-1-2", "当今世界格局与时代主题", "第一单元 我们共同的世界", "九年级下册", "常考", {"single_choice", "short_answer"}},
        {"9x-1-3", "构建人类命运共同体", "第一单元 我们共同的世界", "九年级下册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"9x-2-1", "中国担当与中国影响", "第二单元 世界舞台上的中国", "九年级下册", "高频", {"single_choice", "material_analysis", "essay"}},
        {"9x-2-2", "中国面临的机遇与挑战", "第二单元 世界舞台上的中国", "九年级下册", "高频", {"single_choice", "short_answer", "material_analysis"}},
        {"9x-2-3", "共享发展机遇实现合作共赢", "第二单元 世界舞台上的中国", "九年级下册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"9x-3-1", "少年的担当与家国情怀", "第三单元 走向未来的少年", "九年级下册", "高频", {"single_choice", "essay", "material_analysis"}},
        {"9x-3-2", "学无止境与职业选择", "第三单元 走向未来的少年", "九年级下册", "常考", {"single_choice", "short_answer", "material_analysis"}},
        {"9x-3-3", "规划未来并走向世界", "第三单元 走向未来的少年", "九年级下册", "常考", {"single_choice", "essay", "short_answer"}}
    };
    return points;
}

inline const QList<ChapterGuide> &allChapterGuides()
{
    static const QList<ChapterGuide> guides = {
        {"七年级上册", "第一单元 成长的节拍", {"选择题常考学习方法和自我认知辨析", "简答题、论述题常考成长规划与自我完善"}},
        {"七年级上册", "第二单元 友谊的天空", {"选择题常考友谊特质和交友原则判断", "材料分析题常考网络交友与冲突处理"}},
        {"七年级上册", "第三单元 师长情谊", {"选择题常考师生交往和孝亲敬长要求", "材料分析题常考家庭矛盾或师生沟通情境"}},
        {"七年级上册", "第四单元 生命的思考", {"选择题常考生命意义和守护生命判断", "材料分析题常考挫折应对与生命教育案例"}},

        {"七年级下册", "第一单元 青春时光", {"选择题常考青春期变化和青春有格要求", "材料分析题常考独立思考、批判精神和自尊自爱"}},
        {"七年级下册", "第二单元 做情绪情感的主人", {"选择题常考情绪作用与调节方法", "材料分析题常考同学冲突、情绪管理和情感升华"}},
        {"七年级下册", "第三单元 在集体中成长", {"选择题常考集体利益与个人利益关系", "材料分析题常考班级建设、承担责任和集体荣誉"}},
        {"七年级下册", "第四单元 走进法治天地", {"选择题常考法律特征、作用和未成年人保护", "材料分析题常考校园欺凌、依法维权和遵法守法"}},

        {"八年级上册", "第一单元 走进社会生活", {"选择题常考社会化、网络利弊和理性上网", "材料分析题常考网络谣言、网络沉迷与社会参与"}},
        {"八年级上册", "第二单元 遵守社会规则", {"选择题常考规则、道德与法律关系", "材料分析题常考诚信失信、礼貌待人与违法案例"}},
        {"八年级上册", "第三单元 勇担社会责任", {"选择题常考责任来源与责任承担", "材料分析题常考志愿服务、见义勇为和岗位担当"}},
        {"八年级上册", "第四单元 维护国家利益", {"选择题常考国家利益、国家安全和劳动精神", "材料分析题常考国家安全事件与青年担当"}},

        {"八年级下册", "第一单元 坚持宪法至上", {"选择题常考宪法地位、宪法监督与宪法原则", "材料分析题常考宪法宣传、国家机关依宪履职"}},
        {"八年级下册", "第二单元 理解权利义务", {"选择题常考公民权利、义务和权利边界辨析", "材料分析题常考权利冲突、履行义务与承担责任案例"}},
        {"八年级下册", "第三单元 人民当家作主", {"选择题常考我国制度体系与国家机构职权", "材料分析题常考人大履职、政府工作和制度优势"}},
        {"八年级下册", "第四单元 崇尚法治精神", {"选择题常考自由平等、公平正义基本内涵", "材料分析题常考司法公正、校园公平与弱者保护"}},

        {"九年级上册", "第一单元 富强与创新", {"选择题常考改革开放成就、共同富裕和创新意义", "材料分析题常考科技创新、乡村振兴和高质量发展"}},
        {"九年级上册", "第二单元 民主与法治", {"选择题常考民主形式、公民参与和法治要求", "材料分析题常考听证会、网络问政和依法行政"}},
        {"九年级上册", "第三单元 文明与家园", {"选择题常考文化自信、价值观和绿色发展理念", "材料分析题常考传统文化传承、生态文明建设"}},
        {"九年级上册", "第四单元 和谐与梦想", {"选择题常考民族团结、一国两制和中国梦", "材料分析题常考民族地区发展、祖国统一与青年奋斗"}},

        {"九年级下册", "第一单元 我们共同的世界", {"选择题常考经济全球化、文化多样性和人类命运共同体", "材料分析题常考国际热点、和平发展与全球治理"}},
        {"九年级下册", "第二单元 世界舞台上的中国", {"选择题常考中国担当、机遇挑战和合作共赢", "材料分析题常考中国方案、一带一路与开放发展"}},
        {"九年级下册", "第三单元 走向未来的少年", {"选择题常考人生规划、终身学习和职业准备", "材料分析题常考毕业选择、家国担当与面向世界"}}
    };
    return guides;
}

inline QStringList gradeSemesters()
{
    QStringList grades;
    QSet<QString> seen;
    for (const auto &point : allKnowledgePoints()) {
        if (!seen.contains(point.grade)) {
            seen.insert(point.grade);
            grades.append(point.grade);
        }
    }
    return grades;
}

inline QStringList chaptersForGradeSemester(const QString &gradeSemester)
{
    QStringList chapters;
    QSet<QString> seen;
    for (const auto &point : allKnowledgePoints()) {
        if (point.grade == gradeSemester && !seen.contains(point.chapter)) {
            seen.insert(point.chapter);
            chapters.append(point.chapter);
        }
    }
    return chapters;
}

inline QList<KnowledgePoint> knowledgePointsFor(const QString &gradeSemester,
                                                const QString &chapter = QString())
{
    QList<KnowledgePoint> points;
    for (const auto &point : allKnowledgePoints()) {
        if (!gradeSemester.isEmpty() && point.grade != gradeSemester) {
            continue;
        }
        if (!chapter.isEmpty() && point.chapter != chapter) {
            continue;
        }
        points.append(point);
    }
    return points;
}

inline QStringList knowledgePointNamesFor(const QString &gradeSemester,
                                          const QString &chapter = QString())
{
    QStringList names;
    for (const auto &point : knowledgePointsFor(gradeSemester, chapter)) {
        names.append(point.name);
    }
    return names;
}

inline QStringList highFrequencyPatternsFor(const QString &gradeSemester,
                                            const QString &chapter)
{
    for (const auto &guide : allChapterGuides()) {
        if (guide.grade == gradeSemester && guide.chapter == chapter) {
            return guide.highFrequencyPatterns;
        }
    }
    return {};
}

inline QStringList suitableTypesFor(const QString &gradeSemester,
                                    const QString &chapter = QString())
{
    QStringList types;
    QSet<QString> seen;
    for (const auto &point : knowledgePointsFor(gradeSemester, chapter)) {
        for (const auto &type : point.suitableTypes) {
            if (!seen.contains(type)) {
                seen.insert(type);
                types.append(type);
            }
        }
    }
    return types;
}

inline QString defaultChapterFor(const QString &gradeSemester)
{
    const QStringList chapters = chaptersForGradeSemester(gradeSemester);
    return chapters.isEmpty() ? QString() : chapters.first();
}

inline QStringList promptKnowledgePointsFor(const QString &gradeSemester,
                                            const QString &chapter)
{
    QStringList points = knowledgePointNamesFor(gradeSemester, chapter);
    if (points.size() > 6) {
        points = points.mid(0, 6);
    }
    return points;
}

inline QStringList quickPromptsFor(const QString &gradeSemester,
                                   const QString &chapter)
{
    if (gradeSemester.isEmpty() || chapter.isEmpty()) {
        return {
            "帮我出5道选择题，主题聚焦法治与责任",
            "生成一道中等难度的材料分析题",
            "围绕青春成长出一套综合练习",
            "出3道判断说理题，强调法治意识"
        };
    }

    const QString context = QString("%1 %2").arg(gradeSemester, chapter);
    const QStringList focusPoints = promptKnowledgePointsFor(gradeSemester, chapter);
    const QString focus = focusPoints.mid(0, 2).join("、");

    return {
        QString("帮我出5道选择题，范围：%1，聚焦%2").arg(context, focus),
        QString("生成一道材料分析题，范围：%1，难度中等").arg(context),
        QString("围绕%1出综合练习（选择+判断+论述）").arg(context),
        QString("出3道简答题，重点考查%1").arg(focus.isEmpty() ? context : focus)
    };
}

} // namespace CurriculumData

#endif // CURRICULUMDATA_H
