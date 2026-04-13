#include "NewsCategoryUtils.h"

namespace {

QStringList keywordsForCategory(const QString &category)
{
    if (category == "党建") {
        return {
            "党委", "党组", "党建", "党员", "党代会", "党中央",
            "从严治党", "纪检", "巡视", "反腐", "廉政", "作风建设",
            "纪律检查", "主题教育", "基层党组织", "党员干部",
            "民主集中制", "走群众路线", "组织建设",
            "中纪委", "中组部", "中宣部", "中央统战部", "统战部",
            "总书记", "政治局", "中央全会", "常委会",
            "初心使命", "红色基因", "理论学习",
            "马克思主义", "社会主义", "中国特色", "新时代"
        };
    }

    if (category == "经济") {
        return {
            "经济", "财经", "金融", "产业", "消费", "投资",
            "外贸", "货币", "税收", "就业", "GDP", "高质量发展",
            "经济工作", "资本市场", "财政", "央行", "银行",
            "股市", "债券", "通胀", "CPI", "PPI", "企业",
            "市场", "制造业", "供应链", "商务"
        };
    }

    if (category == "外交") {
        return {
            "外交", "外长", "外事", "使馆", "大使", "领事",
            "峰会", "会晤", "双边", "多边", "联合国", "国际关系",
            "命运共同体", "一带一路", "出访", "涉外",
            "金砖", "上合组织", "东盟", "欧盟", "APEC",
            "G20", "中美", "中俄", "中欧", "中非"
        };
    }

    if (category == "教育") {
        return {
            "教育部", "高考", "职业教育", "学校", "大学", "高校",
            "思政课", "科教兴国", "义务教育", "学生", "教师",
            "人才培养", "素质教育", "教学", "课程", "研究生",
            "学科", "招生", "学位", "考试", "教育改革",
            "双减", "中小学", "幼儿园", "托育",
            "产教融合", "学术", "科研", "实验室"
        };
    }

    if (category == "科技") {
        return {
            "科技", "创新", "技术", "人工智能", "AI", "芯片",
            "半导体", "航天", "卫星", "量子", "算力", "数字经济",
            "互联网", "5G", "6G", "机器人", "研发", "专利",
            "实验室", "探月", "深海", "大模型", "智能制造"
        };
    }

    if (category == "军事") {
        return {
            "军事", "国防", "军队", "解放军", "武警", "演习",
            "战备", "海军", "空军", "陆军", "导弹", "航母",
            "军工", "国防部", "战机", "舰艇", "边防", "联演",
            "实战", "战略支援", "无人机", "军演"
        };
    }

    return {};
}

bool matchesKeywordCategory(const NewsItem &item, const QString &category)
{
    const QStringList keywords = keywordsForCategory(category);
    if (keywords.isEmpty()) {
        return true;
    }

    const QString text = item.title + " " + item.summary + " " + item.content + " "
        + item.keywords.join(" ");

    for (const QString &keyword : keywords) {
        if (text.contains(keyword, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

} // namespace

namespace NewsCategoryUtils {

QString normalizeCategory(const QString &category)
{
    if (category == "全部") {
        return {};
    }
    if (category == "国外") {
        return "国际";
    }
    return category.trimmed();
}

QStringList allCategories()
{
    return {"全部", "国内", "国际", "党建", "经济", "外交", "教育", "科技", "军事"};
}

bool isRemoteCategory(const QString &category)
{
    const QString normalizedCategory = normalizeCategory(category);
    return normalizedCategory.isEmpty() ||
           normalizedCategory == "国内" ||
           normalizedCategory == "国际";
}

QList<NewsItem> filterNewsByCategory(const QList<NewsItem> &items, const QString &category)
{
    const QString normalizedCategory = normalizeCategory(category);
    if (normalizedCategory.isEmpty()) {
        return items;
    }

    QList<NewsItem> filtered;
    filtered.reserve(items.size());

    for (const NewsItem &item : items) {
        if (normalizedCategory == "国内") {
            if (item.category == "国内") {
                filtered.append(item);
            }
            continue;
        }

        if (normalizedCategory == "国际") {
            if (item.category == "国际" || item.category == "国外") {
                filtered.append(item);
            }
            continue;
        }

        if (matchesKeywordCategory(item, normalizedCategory)) {
            filtered.append(item);
        }
    }

    return filtered;
}

QList<NewsItem> searchNews(const QList<NewsItem> &items, const QString &keyword)
{
    const QString trimmedKeyword = keyword.trimmed();
    if (trimmedKeyword.isEmpty()) {
        return items;
    }

    QList<NewsItem> results;
    results.reserve(items.size());

    for (const NewsItem &item : items) {
        const QString haystack = item.title + " " + item.summary + " " + item.content + " "
            + item.keywords.join(" ");
        if (haystack.contains(trimmedKeyword, Qt::CaseInsensitive)) {
            results.append(item);
        }
    }

    return results;
}

} // namespace NewsCategoryUtils
