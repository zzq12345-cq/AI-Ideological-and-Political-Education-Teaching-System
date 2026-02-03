---
identifier: WFS-notification-system
source: "User requirements"
analysis: .workflow/WFS-notification-system/.process/ANALYSIS_RESULTS.md
created: 2026-02-02
---

# Implementation Plan: 站内通知系统

## Summary

构建完整的站内通知系统，支持作业提交通知、请假审批通知、成绩发布通知、系统公告四种类型。系统包含通知中心UI、通知数据模型、NotificationService服务层、Supabase后端集成，并支持未读数量小红点、全部已读、通知列表展示等核心功能。

## Context Analysis

### Project Overview
- **项目**: AI 思政智慧课堂系统
- **技术栈**: Qt 6 (Widgets, Network), C++17, Supabase 后端
- **现有集成点**: ModernMainWindow 中的 notificationBtn (Line 1391) 已存在但未连接功能

### Key Integration Points

| 集成点 | 文件位置 | 说明 |
|--------|----------|------|
| notificationBtn | modernmainwindow.cpp:1391 | 已存在，需连接功能 |
| SupabaseClient | src/auth/supabase/supabaseclient.h | 可复用的 Supabase 认证模式 |
| Service Patterns | DifyService, HotspotService | 参考信号槽和网络请求模式 |
| Model Patterns | Student.h, NewsItem.h | 参考数据模型和 JSON 序列化 |
| UI Patterns | ChatHistoryWidget, StyleConfig.h | 参考列表展示和样式配置 |

### Conflict Risk Assessment
- **风险等级**: LOW
- **原因**: notificationBtn 已存在但未连接任何功能，新增代码为增量添加，不影响现有功能

## Technical Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ModernMainWindow                          │
│  ┌──────────────┐  ┌───────────────┐  ┌──────────────────┐  │
│  │notificationBtn│──│NotificationBadge│  │NotificationWidget│  │
│  └──────────────┘  └───────────────┘  └──────────────────┘  │
│         │                  ▲                    ▲            │
│         │                  │                    │            │
│         ▼                  │                    │            │
│  ┌─────────────────────────┴────────────────────┘            │
│  │              NotificationService                          │
│  │  - fetchNotifications()                                   │
│  │  - fetchUnreadCount()                                     │
│  │  - markAsRead() / markAllAsRead()                         │
│  └───────────────────────┬───────────────────────────────────┘
│                          │                                    │
└──────────────────────────┼────────────────────────────────────┘
                           │ HTTP REST API
                           ▼
              ┌─────────────────────────┐
              │   Supabase Backend      │
              │  ┌───────────────────┐  │
              │  │ notifications 表  │  │
              │  │ - id (uuid)       │  │
              │  │ - type (enum)     │  │
              │  │ - title           │  │
              │  │ - content         │  │
              │  │ - sender_id       │  │
              │  │ - receiver_id     │  │
              │  │ - created_at      │  │
              │  │ - is_read         │  │
              │  └───────────────────┘  │
              └─────────────────────────┘
```

## New Files Summary

| 文件路径 | 类型 | 说明 | 预估行数 |
|----------|------|------|----------|
| src/notifications/models/Notification.h | Model | 通知数据模型头文件 | ~80 |
| src/notifications/models/Notification.cpp | Model | 通知数据模型实现 | ~60 |
| src/notifications/NotificationService.h | Service | 通知服务层头文件 | ~100 |
| src/notifications/NotificationService.cpp | Service | 通知服务层实现 | ~250 |
| src/notifications/ui/NotificationWidget.h | UI | 通知列表弹窗头文件 | ~80 |
| src/notifications/ui/NotificationWidget.cpp | UI | 通知列表弹窗实现 | ~200 |
| src/notifications/ui/NotificationBadge.h | UI | 小红点组件头文件 | ~40 |
| src/notifications/ui/NotificationBadge.cpp | UI | 小红点组件实现 | ~60 |
| docs/supabase/notifications.sql | SQL | 数据库表结构 | ~60 |

**总计**: 8 个新 C++ 文件 + 1 个 SQL 文档

## Modified Files Summary

| 文件路径 | 修改类型 | 说明 |
|----------|----------|------|
| src/dashboard/modernmainwindow.h | Add | 添加 4 个新成员变量 |
| src/dashboard/modernmainwindow.cpp | Add | 添加约 80 行集成代码 |
| CMakeLists.txt | Add | 添加 8 个新源文件 |

## Task Breakdown

### Task Overview

| ID | 标题 | 优先级 | 工作量 | 依赖 |
|----|------|--------|--------|------|
| IMPL-1 | 通知数据模型层 | high | small | - |
| IMPL-2 | 通知服务层 | high | medium | IMPL-1 |
| IMPL-3 | 通知UI组件 | high | large | IMPL-2 |
| IMPL-4 | 主窗口集成 | high | medium | IMPL-3 |
| IMPL-5 | CMakeLists 和数据库配置 | medium | small | IMPL-1,2,3 |

### Dependency Graph

```
IMPL-1 (通知数据模型层)
    │
    ▼
IMPL-2 (通知服务层)
    │
    ▼
IMPL-3 (通知UI组件)
    │
    ▼
IMPL-4 (主窗口集成)

IMPL-5 (CMakeLists 和数据库配置) ← 可并行执行，但依赖 IMPL-1,2,3 完成后编译验证
```

## Implementation Strategy

### Execution Order

1. **Phase 1: 数据层** (IMPL-1)
   - 创建 Notification 数据模型
   - 定义通知类型枚举和状态枚举
   - 实现 JSON 序列化/反序列化

2. **Phase 2: 服务层** (IMPL-2)
   - 创建 NotificationService
   - 实现 Supabase REST API 调用
   - 实现 CRUD 操作和未读统计

3. **Phase 3: UI 层** (IMPL-3)
   - 创建 NotificationBadge 小红点组件
   - 创建 NotificationWidget 通知列表弹窗
   - 实现通知项渲染和交互

4. **Phase 4: 集成** (IMPL-4)
   - 将组件集成到 ModernMainWindow
   - 连接 notificationBtn
   - 实现自动刷新逻辑

5. **Phase 5: 配置** (IMPL-5)
   - 更新 CMakeLists.txt
   - 创建数据库 DDL 文档
   - 验证编译

### Success Criteria

- [ ] 8 个新 C++ 源文件创建完成
- [ ] CMakeLists.txt 正确包含所有新文件
- [ ] 项目编译通过无错误
- [ ] notificationBtn 点击可显示通知弹窗
- [ ] 小红点正确显示未读数量
- [ ] 点击通知可标记为已读
- [ ] "全部已读" 功能正常工作
- [ ] 通知列表正确显示四种通知类型

## Reference Patterns

### Service Pattern (from HotspotService)
```cpp
class NotificationService : public QObject {
    Q_OBJECT
public:
    void fetchNotifications();
    void markAsRead(const QString &id);

signals:
    void notificationsReceived(const QList<Notification> &list);
    void unreadCountChanged(int count);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *m_networkManager;
};
```

### Model Pattern (from NewsItem)
```cpp
struct Notification {
    QString id;
    NotificationType type;
    QString title;
    QString content;
    QDateTime createdAt;
    bool isRead;

    static Notification fromJson(const QJsonObject &json);
    QJsonObject toJson() const;
    bool isValid() const;
};
```

### Style Pattern (from StyleConfig.h)
```cpp
// 使用统一的样式常量
#include "shared/StyleConfig.h"

badge->setStyleSheet(QString(
    "background-color: %1;"
    "border-radius: 9px;"
).arg(StyleConfig::PATRIOTIC_RED));
```
