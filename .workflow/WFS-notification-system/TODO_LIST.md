# Tasks: 站内通知系统

## Task Progress

### Phase 1: 数据层

- [ ] **IMPL-1**: 通知数据模型层 -> [📋](./.task/IMPL-1.json)
  - 创建 Notification.h/cpp 数据模型
  - 定义 4 种通知类型枚举
  - 实现 JSON 序列化方法

### Phase 2: 服务层

- [ ] **IMPL-2**: 通知服务层 -> [📋](./.task/IMPL-2.json)
  - 创建 NotificationService.h/cpp
  - 实现 Supabase REST API 调用
  - 实现 6 个核心业务方法

### Phase 3: UI 层

- [ ] **IMPL-3**: 通知UI组件 -> [📋](./.task/IMPL-3.json)
  - 创建 NotificationBadge 小红点组件
  - 创建 NotificationWidget 通知列表弹窗
  - 实现 4 种通知类型的渲染

### Phase 4: 集成

- [ ] **IMPL-4**: 主窗口集成 -> [📋](./.task/IMPL-4.json)
  - 修改 modernmainwindow.h 添加成员
  - 连接 notificationBtn 点击事件
  - 实现 60 秒自动刷新定时器

### Phase 5: 配置

- [ ] **IMPL-5**: CMakeLists 和数据库配置 -> [📋](./.task/IMPL-5.json)
  - 更新 CMakeLists.txt 添加 8 个新文件
  - 创建 notifications.sql 表结构文档
  - 验证项目编译通过

---

## Status Legend

- `- [ ]` = Pending task
- `- [x]` = Completed task

## Dependency Chain

```
IMPL-1 -> IMPL-2 -> IMPL-3 -> IMPL-4
              \
               -> IMPL-5 (可并行)
```

## Quick Stats

| Status | Count |
|--------|-------|
| Total | 5 |
| Pending | 5 |
| In Progress | 0 |
| Completed | 0 |

---

## File Changes Summary

### New Files (8 C++ + 1 SQL)

| Task | Files |
|------|-------|
| IMPL-1 | `src/notifications/models/Notification.{h,cpp}` |
| IMPL-2 | `src/notifications/NotificationService.{h,cpp}` |
| IMPL-3 | `src/notifications/ui/NotificationWidget.{h,cpp}` |
| IMPL-3 | `src/notifications/ui/NotificationBadge.{h,cpp}` |
| IMPL-5 | `docs/supabase/notifications.sql` |

### Modified Files (3)

| Task | File | Change |
|------|------|--------|
| IMPL-4 | `src/dashboard/modernmainwindow.h` | +4 members |
| IMPL-4 | `src/dashboard/modernmainwindow.cpp` | +80 lines |
| IMPL-5 | `CMakeLists.txt` | +8 source files |
