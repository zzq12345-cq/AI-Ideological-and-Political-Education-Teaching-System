# 教师备课上课流程 - 实施计划

**会话ID**: WFS-teacher-lesson-workflow
**生成时间**: 2026-02-02
**目标**: P0 备课工作台可用，课堂管理/学情分析占位

---

## 1. 项目概述

### 1.1 目标范围

| 阶段 | 范围 | 交付状态 |
|------|------|----------|
| **P0 (本期)** | 备课工作台（课件/试题/教案）、数据模型、迁移引导 | ✅ 可用 |
| P1 | 课堂管理（签到/答题/H5采集） | 占位页 |
| P2 | 学情分析（画像/报告） | 占位页 |

### 1.2 技术栈

- **前端**: Qt6 Widgets + QML, C++17
- **后端**: Supabase (PostgreSQL)
- **AI服务**: Dify API (SSE), 讯飞 PPT API
- **构建**: CMake 3.16+

### 1.3 核心决策

| 决策 | 内容 |
|------|------|
| MVP范围 | 仅备课工作台，课堂/学情占位 |
| 刷新策略 | 定时刷新 + 手动触发，显示"上次更新" |
| 发布语义 | 发布=备课完成，可进入授课 |
| 版本策略 | 仅发布/导出时生成版本快照 |
| 数据采集 | 名单导入 + 二维码签到 + 轻量H5答题 |
| 迁移引导 | 首次弹窗 + 教学向导 |

---

## 2. 任务分解

### 2.1 任务总览

| ID | 任务 | 优先级 | 依赖 | 预估 |
|----|------|--------|------|------|
| IMPL-001 | 数据模型与Supabase表结构 | P0 | - | 2天 |
| IMPL-002 | CourseService 服务层 | P0 | IMPL-001 | 2天 |
| IMPL-003 | LocalStorageService 本地存储 | P0 | - | 1天 |
| IMPL-004 | 备课工作台主容器 | P0 | IMPL-002 | 2天 |
| IMPL-005 | 课程导航树组件 | P0 | IMPL-004 | 1.5天 |
| IMPL-006 | 课件编辑器集成 | P0 | IMPL-005 | 2天 |
| IMPL-007 | 试题编辑器集成 | P0 | IMPL-005 | 1.5天 |
| IMPL-008 | 教案编辑器 | P0 | IMPL-005 | 1.5天 |
| IMPL-009 | 主窗口导航集成与占位页 | P0 | IMPL-004 | 1天 |
| IMPL-010 | 迁移引导与埋点 | P0 | IMPL-009 | 1天 |

**总计**: 10个任务，约15.5人天

### 2.2 依赖关系图

```
IMPL-001 (数据模型) ──┬──> IMPL-002 (CourseService)
                      │
IMPL-003 (本地存储) ──┴──> IMPL-004 (备课工作台主容器)
                              │
                              ├──> IMPL-005 (导航树)
                              │         │
                              │         ├──> IMPL-006 (课件编辑器)
                              │         ├──> IMPL-007 (试题编辑器)
                              │         └──> IMPL-008 (教案编辑器)
                              │
                              └──> IMPL-009 (主窗口集成)
                                        │
                                        └──> IMPL-010 (迁移引导)
```

---

## 3. 详细任务说明

### IMPL-001: 数据模型与Supabase表结构

**目标**: 创建P0阶段所需的Supabase表结构

**交付物**:
- [ ] `courses` 表 + RLS策略
- [ ] `lessons` 表 + 状态字段(planning/ready/in_progress/completed)
- [ ] `lesson_plans` 表
- [ ] `coursewares` 表
- [ ] `usage_events` 表 (KPI埋点)
- [ ] `content_versions` 表 (版本快照)
- [ ] SQL迁移脚本 `migrations/001_lesson_workflow.sql`

**验收标准**:
- 6张表创建成功，RLS策略生效
- 教师只能访问自己的课程数据

**参考**: `data-architect/analysis.md` § 3.0-3.1

---

### IMPL-002: CourseService 服务层

**目标**: 实现课程、课时、教案、课件的CRUD服务

**交付物**:
- [ ] `src/services/CourseService.h/cpp`
  - Course CRUD (create/read/update/delete)
  - Lesson CRUD + 状态流转
  - LessonPlan CRUD
  - Courseware CRUD
  - 发布功能 (status → ready)
- [ ] 与SupabaseClient集成
- [ ] 单元测试 (≥5个核心用例)

**验收标准**:
- 课程创建/编辑/删除正常
- 课时发布后status变为"ready"
- API调用错误有明确提示

**参考**: `data-architect/analysis.md` § 6.2

---

### IMPL-003: LocalStorageService 本地存储

**目标**: 管理课件/教案的本地文件存储

**交付物**:
- [ ] `src/services/LocalStorageService.h/cpp`
  - 目录结构: `~/.ai-sizheng-classroom/courses/{id}/lessons/{id}/`
  - saveFile / copyFile / deleteFile
  - generateThumbnail (课件缩略图)
  - 版本快照保存 (发布/导出时)
- [ ] 文件操作进度信号

**验收标准**:
- 文件保存到正确目录
- 版本快照在发布时自动生成
- 文件删除时级联清理

**参考**: `data-architect/analysis.md` § 4

---

### IMPL-004: 备课工作台主容器

**目标**: 创建备课工作台的主Widget容器

**交付物**:
- [ ] `src/workbench/LessonWorkbenchWidget.h/cpp`
  - 左右分栏布局 (QSplitter)
  - 左侧导航树区域 (240px)
  - 右侧编辑器区域 (QStackedWidget)
  - 顶部工具栏 (保存/发布)
  - 底部状态栏 (上次保存/同步状态)
- [ ] 发布按钮二次确认对话框
- [ ] "上次更新"提示组件

**验收标准**:
- 左右分栏可拖拽调整
- 发布按钮点击后弹出确认
- 状态栏显示保存时间

**参考**: `ui-designer/analysis.md` § 4.1

---

### IMPL-005: 课程导航树组件

**目标**: 实现课程-课时-资源的树形导航

**交付物**:
- [ ] `src/workbench/TreeNavigator.h/cpp`
  - 三级树结构: 课程 → 课时 → 资源(课件/试题/教案)
  - 拖拽排序
  - 右键菜单 (新建/复制/删除)
  - 资源类型图标
- [ ] 与CourseService集成

**验收标准**:
- 树形结构正确展示
- 拖拽可调整课时顺序
- 点击资源切换右侧编辑器

**参考**: `ui-designer/analysis.md` § 4.2

---

### IMPL-006: 课件编辑器集成

**目标**: 集成现有AI课件生成功能到备课工作台

**交付物**:
- [ ] `src/workbench/editors/CoursewareEditor.h/cpp`
  - 幻灯片缩略图列表
  - 预览区域
  - AI生成按钮 (复用XunfeiPPTService)
  - 导出功能 (复用PPTXGenerator)
- [ ] 复用SlidePreviewDialog
- [ ] 版本快照触发 (导出时)

**验收标准**:
- AI生成PPT流程正常
- 缩略图正确显示
- 导出时生成版本快照

**参考**: `ui-designer/analysis.md` § 4.3.1

---

### IMPL-007: 试题编辑器集成

**目标**: 集成现有试题库选题功能到备课工作台

**交付物**:
- [ ] `src/workbench/editors/QuestionEditor.h/cpp`
  - 试题列表展示
  - "从题库添加"按钮 (嵌入QuestionBankWindow筛选)
  - "AI出题"按钮 (复用DifyService)
  - 试题统计信息
- [ ] 复用QuestionBasketWidget

**验收标准**:
- 从题库添加试题正常
- AI出题流程正常
- 试题统计准确

**参考**: `ui-designer/analysis.md` § 4.3.2

---

### IMPL-008: 教案编辑器

**目标**: 实现教案的富文本编辑功能

**交付物**:
- [ ] `src/workbench/editors/LessonPlanEditor.h/cpp`
  - 富文本编辑器 (QTextEdit + Markdown支持)
  - 教案模板填充
  - AI生成大纲 (复用DifyService)
  - 版本快照触发 (发布时)
- [ ] 复用MarkdownRenderer

**验收标准**:
- Markdown编辑和预览正常
- AI生成大纲流程正常
- 发布时生成版本快照

**参考**: `ui-designer/analysis.md` § 4.3.3

---

### IMPL-009: 主窗口导航集成与占位页

**目标**: 将备课工作台集成到ModernMainWindow

**交付物**:
- [ ] 修改`src/dashboard/modernmainwindow.cpp`
  - 侧边栏添加"备课工作台"导航项
  - 侧边栏添加"课堂管理"导航项 (占位)
  - PageIndex枚举扩展
- [ ] `src/workbench/PlaceholderPage.h/cpp`
  - 课堂管理占位页 ("即将开放" + 流程说明)
  - 学情分析占位页 ("授课完成后可查看")
  - 禁用态按钮 + 演示入口
- [ ] 更新SidebarManager

**验收标准**:
- 侧边栏显示新导航项
- 占位页正确显示提示信息
- 页面切换正常

**参考**: `ui-designer/analysis.md` § 3.2, § 5.1

---

### IMPL-010: 迁移引导与埋点

**目标**: 实现用户迁移引导和使用行为埋点

**交付物**:
- [ ] `src/workbench/MigrationGuideDialog.h/cpp`
  - 首次进入弹窗引导
  - "本次不再提示"选项
  - 教学向导分步任务
- [ ] `src/services/UsageEventService.h/cpp`
  - 进入工作台埋点
  - 功能使用埋点
  - 发送到usage_events表
- [ ] 旧入口"即将迁移"提示

**验收标准**:
- 首次进入显示引导弹窗
- 关闭后下次不再显示
- 埋点数据正确写入

**参考**: `ui-designer/analysis.md` § 3.2 (EP-004), `data-architect/analysis.md` § 3.1 (usage_events)

---

## 4. 文件结构

```
src/
├── workbench/                          # 新增备课工作台模块
│   ├── LessonWorkbenchWidget.h/cpp     # 主容器 (IMPL-004)
│   ├── TreeNavigator.h/cpp             # 导航树 (IMPL-005)
│   ├── PlaceholderPage.h/cpp           # 占位页 (IMPL-009)
│   ├── MigrationGuideDialog.h/cpp      # 迁移引导 (IMPL-010)
│   └── editors/
│       ├── CoursewareEditor.h/cpp      # 课件编辑器 (IMPL-006)
│       ├── QuestionEditor.h/cpp        # 试题编辑器 (IMPL-007)
│       └── LessonPlanEditor.h/cpp      # 教案编辑器 (IMPL-008)
│
├── services/
│   ├── CourseService.h/cpp             # 课程服务 (IMPL-002)
│   ├── LocalStorageService.h/cpp       # 本地存储 (IMPL-003)
│   └── UsageEventService.h/cpp         # 埋点服务 (IMPL-010)
│
└── dashboard/
    └── modernmainwindow.cpp            # 修改：导航集成 (IMPL-009)

migrations/
└── 001_lesson_workflow.sql             # 数据库迁移 (IMPL-001)
```

---

## 5. 质量门

### 5.1 代码规范

- [ ] 类名PascalCase，成员变量m_前缀
- [ ] 中文注释和日志
- [ ] 信号槽异步错误回调

### 5.2 测试覆盖

- [ ] CourseService 单元测试 ≥5用例
- [ ] LocalStorageService 单元测试 ≥3用例
- [ ] 集成测试：完整备课流程

### 5.3 验收清单

- [ ] 备课工作台可正常打开
- [ ] 课程/课时CRUD正常
- [ ] 课件AI生成正常
- [ ] 试题从题库添加正常
- [ ] 教案编辑保存正常
- [ ] 发布后状态变为"可授课"
- [ ] 占位页正确显示
- [ ] 迁移引导正常弹出

---

## 6. 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| 现有组件复用困难 | 开发延期 | 优先验证集成点，必要时重构 |
| Supabase表结构变更 | 数据迁移复杂 | 使用迁移脚本，保留兼容层 |
| 本地文件丢失 | 数据丢失 | 定期备份提醒，预留云存储接口 |

---

## 7. 下一步

1. **执行开发**: `/workflow:execute --session WFS-teacher-lesson-workflow`
2. **查看状态**: `/workflow:status --session WFS-teacher-lesson-workflow`
3. **验证计划**: `/workflow:action-plan-verify --session WFS-teacher-lesson-workflow`
