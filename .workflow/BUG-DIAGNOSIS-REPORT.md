# 项目整体Bug诊断报告

**Date**: 2026-02-02
**Reviewer**: 老王
**Project**: AI 思政智慧课堂系统
**代码行数**: ~80,000 行

---

## 📊 诊断概览

| 类别 | 数量 |
|------|------|
| 严重问题 (Critical) | 1 |
| 中等问题 (Medium) | 5 |
| 轻微问题 (Low) | 8 |
| TODO/未实现功能 | 13 |
| 整体评级 | ⭐⭐⭐⭐ (良好) |

---

## 🔴 严重问题 (Critical)

### 1. SupabaseClient SSL错误处理不当
- **文件**: `src/auth/supabase/supabaseclient.cpp:253-262`
- **问题**: SSL错误时直接发送`loginFailed`信号，但未忽略SSL错误继续请求
- **代码**:
```cpp
void SupabaseClient::onSslErrors(const QList<QSslError> &errors)
{
    // ...
    emit loginFailed("SSL连接错误: " + ...);  // 直接失败
}
```
- **影响**: 在某些网络环境下可能导致无法登录
- **建议**: 参考DifyService的处理方式，在开发环境下忽略SSL错误
```cpp
if (reply) {
    reply->ignoreSslErrors(errors);  // 添加这一行
}
```

---

## 🟠 中等问题 (Medium)

### 1. NotificationWidget通知项点击事件未实现
- **文件**: `src/notifications/ui/NotificationWidget.cpp:490-492`
- **问题**: 安装了事件过滤器但未在`eventFilter`中处理通知项点击
- **影响**: 点击通知项不会触发`onNotificationItemClicked`
- **建议**: 在eventFilter中添加通知项点击处理逻辑

### 2. NotificationService.h存在未实现的声明
- **文件**: `src/notifications/NotificationService.h:52`
- **问题**: 声明了`handleNetworkReply`方法但cpp中未实现
- **影响**: 死代码，增加维护成本
- **建议**: 删除未使用的声明

### 3. NotificationWidget.h存在未使用的成员变量
- **文件**: `src/notifications/ui/NotificationWidget.h:62`
- **问题**: `m_loadingOverlay`声明但从未初始化和使用
- **影响**: 浪费内存（虽然是nullptr）
- **建议**: 实现加载状态UI或删除该变量

### 4. eventFilter中重复调用removeEventFilter
- **文件**: `src/notifications/ui/NotificationWidget.cpp:262-264`
- **问题**: `hidePopup()`已调用`removeEventFilter`，这里又调用一次
- **影响**: 冗余代码
- **建议**: 删除重复调用

### 5. SupabaseClient用户检查逻辑有问题
- **文件**: `src/auth/supabase/supabaseclient.cpp:180`
- **问题**: `json.contains("")` 检查空字符串键，逻辑不合理
- **代码**:
```cpp
if (json.contains("")) {  // 空字符串作为键？
    QJsonValue usersValue = json.value("");
```
- **影响**: 用户存在检查可能永远返回false
- **建议**: 修正逻辑，Supabase REST API返回数组，应直接检查doc.isArray()

---

## 🟡 轻微问题 (Low)

### 1. PaperService请求未设置HTTP/2禁用
- **文件**: `src/services/PaperService.cpp:337`
- **问题**: 未设置`Http2AllowedAttribute = false`，与项目其他服务不一致
- **影响**: 可能在macOS上出现网络问题
- **建议**: 添加`request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);`

### 2. SupabaseClient请求未设置HTTP/2禁用
- **文件**: `src/auth/supabase/supabaseclient.cpp:60-66`
- **问题**: 同上，未禁用HTTP/2
- **建议**: 统一网络请求配置

### 3. ExportService PDF导出未实现
- **文件**: `src/services/ExportService.cpp:70`
- **代码**: `// TODO: 实现PDF导出`
- **影响**: PDF导出功能不可用
- **建议**: 实现或从UI中移除该功能入口

### 4. SidebarManager图标加载TODO
- **文件**: `src/dashboard/SidebarManager.cpp:204`
- **代码**: `// TODO: 从资源文件加载图标`
- **影响**: 侧边栏图标可能显示不正确

### 5. ChatManager PPT生成TODO
- **文件**: `src/dashboard/ChatManager.cpp:124`
- **代码**: `// TODO: 处理 PPT 生成流程`
- **影响**: PPT生成流程未完成

### 6. modernmainwindow用户ID硬编码
- **文件**: `src/dashboard/modernmainwindow.cpp:857`
- **代码**: `// TODO: 从登录状态获取用户ID，暂时用用户名模拟`
- **影响**: 通知系统使用用户名而非真实用户ID

### 7. QML中的未实现功能
- **文件**: `src/ui/qml/questionbank/QuestionBankPage.qml:64,219`
- **问题**: 返回主界面和导出功能的TODO
- **影响**: 相关功能不可用

### 8. connection_snippet中多个TODO
- **文件**: `src/dashboard/modernmainwindow_connection_snippet.cpp`
- **问题**: 7个未实现的TODO（AI生成、预览、下载、保存、编辑、重新生成、幻灯片顺序）
- **影响**: PPT相关功能不完整

---

## ✅ 代码质量优点

### 1. 网络请求处理规范
- 大部分服务使用`deleteLater()`正确释放reply
- DifyService和QuestionParserService有完善的错误处理
- 流式响应处理正确（SSE解析）

### 2. Qt内存管理正确
- 使用父子关系自动管理内存
- 使用智能指针（如ChatWidget的m_markdownRenderer）
- 正确使用deleteLater()而非直接delete

### 3. 代码架构清晰
- MVC模式：Model → Service → View
- 信号槽解耦
- 服务层与UI层分离

### 4. 错误处理完善
- 网络错误处理有详细日志
- 空值检查充分
- JSON解析错误处理

### 5. 命名规范统一
- 成员变量使用`m_`前缀
- 方法使用camelCase
- 信号使用过去时（如`loginFailed`）

---

## 📋 修复优先级

### 立即修复 (P0)
1. SupabaseClient SSL错误处理
2. SupabaseClient用户检查逻辑

### 高优先级 (P1)
3. NotificationWidget通知项点击事件
4. 删除NotificationService死代码

### 中优先级 (P2)
5. 统一HTTP/2禁用配置
6. 删除NotificationWidget未使用变量
7. 修复eventFilter重复调用

### 低优先级 (P3)
8. 处理各处TODO（按功能重要性）

---

## 🔧 Action Items

### 立即修复
- [ ] 修复SupabaseClient SSL错误处理
- [ ] 修复SupabaseClient用户检查逻辑（handleUserCheckResponse）

### 高优先级
- [ ] 实现NotificationWidget通知项点击事件
- [ ] 删除NotificationService.h中未使用的handleNetworkReply声明

### 中优先级
- [ ] 为PaperService和SupabaseClient添加HTTP/2禁用
- [ ] 删除NotificationWidget.h中未使用的m_loadingOverlay
- [ ] 修复eventFilter中重复removeEventFilter调用

### 文档
- [ ] 整理并评估13个TODO的实现优先级

---

## 📁 审核文件统计

| 模块 | 文件数 | 主要问题 |
|------|--------|----------|
| auth/supabase | 2 | SSL处理、用户检查逻辑 |
| notifications | 4 | 点击事件、死代码 |
| services | 8 | HTTP/2配置、TODO |
| dashboard | 4 | TODO |
| ui | 6 | 正常 |
| questionbank | 5 | 正常 |

---

**诊断结论**: 项目整体代码质量良好，架构清晰，遵循Qt最佳实践。主要问题集中在：
1. SSL/网络配置不统一
2. 通知系统有少量未完成功能
3. 存在较多TODO待实现

建议优先处理P0和P1级别问题，其余可在后续迭代中逐步完善。
