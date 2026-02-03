# TODO List - 课程班数据分析

## Session: WFS-class-data-analytics
## Created: 2026-02-01
## Updated: 2026-02-01

---

## 任务依赖关系

```
IMPL-1 (数据模型) ─────┬──→ IMPL-2 (个人分析)
                      │
                      ├──→ IMPL-3 (班级分析)
                      │
                      └──→ IMPL-5 (数据服务)
                                │
IMPL-2 + IMPL-3 ────────────────┴──→ IMPL-4 (导航集成)
```

---

## 已完成任务

### IMPL-1: 数据模型层 ✅ Completed
- **优先级**: 🔴 High
- **预估**: Medium
- **依赖**: 无

**已创建文件**:
- [x] `src/analytics/models/Student.h/.cpp`
- [x] `src/analytics/models/CourseClass.h/.cpp`
- [x] `src/analytics/models/ScoreRecord.h/.cpp`
- [x] `src/analytics/models/KnowledgePoint.h/.cpp`
- [x] `src/analytics/models/ClassStatistics.h/.cpp` (额外)
- [x] `src/analytics/models/AnalysisResult.h/.cpp` (额外)

**已修改文件**:
- [x] `CMakeLists.txt` - 添加模型源文件

---

### IMPL-2: 个人学情分析页面 ✅ Completed
- **优先级**: 🔴 High
- **预估**: High
- **依赖**: IMPL-1

**已创建文件**:
- [x] `src/analytics/ui/PersonalAnalyticsPage.h/.cpp`
- [x] `src/analytics/ui/RadarChartWidget.h/.cpp` (额外-雷达图组件)

**核心功能**:
- [x] 学生选择器 (QComboBox)
- [x] 成绩趋势折线图
- [x] 知识点掌握雷达图
- [x] AI个性化建议
- [x] ScrollArea布局 (避免重叠)

---

### IMPL-3: 班级整体分析页面 ✅ Completed
- **优先级**: 🔴 High
- **预估**: High
- **依赖**: IMPL-1

**已创建文件**:
- [x] `src/analytics/ui/ClassAnalyticsPage.h/.cpp`

**核心功能**:
- [x] 班级选择器 (QComboBox)
- [x] 成绩分布柱状图
- [x] 学生排名表格 (优秀生光荣榜)
- [x] 薄弱知识点分析 (水平条形图)
- [x] AI班级建议

---

### IMPL-4: 导航栏与页面集成 ✅ Completed
- **优先级**: 🔴 High
- **预估**: Medium
- **依赖**: IMPL-2, IMPL-3

**已创建文件**:
- [x] `src/analytics/ui/AnalyticsNavigationBar.h/.cpp`

**已修改文件**:
- [x] `src/analytics/DataAnalyticsWidget.h/.cpp`
- [x] `CMakeLists.txt`

**核心功能**:
- [x] 导航栏组件 (概览/个人/班级)
- [x] QStackedWidget页面切换
- [x] 切换动画效果 (淡入淡出)

---

### IMPL-5: 数据服务扩展 ✅ Completed
- **优先级**: 🟡 Medium
- **预估**: Medium
- **依赖**: IMPL-1

**已创建文件**:
- [x] `src/analytics/interfaces/IAnalyticsDataSource.h`
- [x] `src/analytics/datasources/MockDataSource.h/.cpp`

**已修改文件**:
- [x] `src/analytics/AnalyticsDataService.h/.cpp`
- [x] `CMakeLists.txt`

**已实现接口**:
- [x] getStudentList(classId)
- [x] getClassList()
- [x] getStudentScores(studentId, dateRange)
- [x] getClassScores(classId, dateRange)
- [x] getStudentKnowledgePoints(studentId)
- [x] getClassKnowledgePoints(classId)
- [x] getClassRanking(classId)

---

## 执行顺序

1. **第一轮** (可并行): ✅ 完成
   - IMPL-1 数据模型层

2. **第二轮** (可并行): ✅ 完成
   - IMPL-2 个人分析页面
   - IMPL-3 班级分析页面
   - IMPL-5 数据服务扩展

3. **第三轮**: ✅ 完成
   - IMPL-4 导航与集成

---

## 🎉 项目状态: 全部完成！

所有IMPL任务已完成，数据分析模块功能完整：
- 数据概览页面
- 个人学情分析页面
- 班级整体分析页面
- 导航切换动画
- Mock数据服务

后续可选优化：
- [ ] 接入真实数据库数据源
- [ ] 导出PDF报告功能完善
- [ ] 更多图表类型支持
