# 实施计划: 课程班数据分析 - 个人与全班维度

## 概述

为现有的数据分析模块添加**个人学情分析**和**班级整体分析**两个维度，支持切换查看不同维度的数据可视化。

## 目标

1. **个人学情分析**: 单个学生的成绩趋势、知识点掌握、学习轨迹
2. **班级整体分析**: 班级成绩分布、排名对比、薄弱知识点识别
3. **维度切换**: 在个人/班级视图间无缝切换
4. **数据对接**: 扩展数据服务支持真实数据查询

## 技术方案

### 架构设计

```
DataAnalyticsWidget (现有)
    ├── AnalyticsNavigationBar (新增 - 顶部切换栏)
    │   ├── [个人分析] 按钮
    │   └── [班级分析] 按钮
    │
    ├── PersonalAnalyticsPage (新增)
    │   ├── 学生选择器
    │   ├── 个人成绩趋势折线图
    │   ├── 知识点掌握雷达图
    │   ├── 学习轨迹时间线
    │   └── AI个性化建议
    │
    └── ClassAnalyticsPage (新增)
        ├── 班级选择器
        ├── 成绩分布柱状图
        ├── 学生排名表格
        ├── 薄弱知识点分析
        └── AI班级建议
```

### 数据模型

```cpp
// 学生模型
struct Student {
    int id;
    QString name;
    int classId;
    QString studentNo;
};

// 班级模型
struct CourseClass {
    int id;
    QString name;
    QString teacherId;
    int studentCount;
};

// 成绩记录
struct ScoreRecord {
    int studentId;
    QString subject;
    double score;
    QDate date;
    QString knowledgePoint;
};

// 知识点掌握度
struct KnowledgePoint {
    QString name;
    double masteryRate;  // 0-100
    int questionCount;
};
```

## 实施阶段

### 阶段1: 数据模型层 (IMPL-1)
- 创建 `src/analytics/models/` 目录
- 实现 Student、CourseClass、ScoreRecord、KnowledgePoint 模型
- 扩展 AnalyticsDataService 接口

### 阶段2: 个人分析页面 (IMPL-2)
- 创建 PersonalAnalyticsPage 组件
- 实现学生选择器（下拉框）
- 实现个人成绩趋势折线图
- 实现知识点掌握雷达图
- 集成 AI 个性化建议

### 阶段3: 班级分析页面 (IMPL-3)
- 创建 ClassAnalyticsPage 组件
- 实现班级选择器
- 实现成绩分布柱状图
- 实现学生排名表格
- 实现薄弱知识点分析
- 集成 AI 班级建议

### 阶段4: 导航与集成 (IMPL-4)
- 创建 AnalyticsNavigationBar 切换组件
- 重构 DataAnalyticsWidget 集成新页面
- 实现页面切换动画
- 更新 CMakeLists.txt

### 阶段5: 数据服务扩展 (IMPL-5)
- 扩展 AnalyticsDataService 支持个人/班级查询
- 添加模拟数据生成器
- 预留 Supabase 对接接口

## 依赖关系

```
IMPL-1 (数据模型)
    ↓
IMPL-2 (个人分析) ──→ IMPL-4 (导航集成)
    ↓                    ↓
IMPL-3 (班级分析) ──→ IMPL-5 (数据服务)
```

## 文件清单

### 新增文件
- `src/analytics/models/Student.h/.cpp`
- `src/analytics/models/CourseClass.h/.cpp`
- `src/analytics/models/ScoreRecord.h/.cpp`
- `src/analytics/models/KnowledgePoint.h/.cpp`
- `src/analytics/ui/PersonalAnalyticsPage.h/.cpp`
- `src/analytics/ui/ClassAnalyticsPage.h/.cpp`
- `src/analytics/ui/AnalyticsNavigationBar.h/.cpp`

### 修改文件
- `src/analytics/DataAnalyticsWidget.h/.cpp` - 集成新页面
- `src/analytics/AnalyticsDataService.h/.cpp` - 扩展接口
- `CMakeLists.txt` - 添加新源文件

## 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| 现有功能回归 | 高 | 保持原有代码不变，新增独立组件 |
| 数据量大性能问题 | 中 | 使用分页和懒加载 |
| 图表渲染复杂 | 中 | 复用现有Qt Charts模式 |

## 验收标准

- [ ] 个人分析页面正常显示学生成绩趋势
- [ ] 班级分析页面正常显示成绩分布
- [ ] 个人/班级切换流畅无闪烁
- [ ] AI建议功能正常工作
- [ ] 编译无错误无警告
