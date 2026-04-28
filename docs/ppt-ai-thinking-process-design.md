# PPT 生成“AI 思考输出后自动消失”交互设计文档

## 1. 背景与目标

当前 PPT 生成流程已经具备真实的 AI Agent 管线：需求整理、大纲生成、布局规划、逐页 SVG 生成、预览与导出。现有界面更偏“进度日志 + 生成产物展示”，用户能看到过程，但体验上仍然像工程日志。

本设计希望将 PPT 生成过程改造成类似大模型应用的输出体验：

1. AI 先用自然语言说明它将如何生成 PPT。
2. 下方出现低权重的“思考过程”区域。
3. 思考内容随着阶段推进逐步更新。
4. 阶段完成后，思考内容短暂停留，然后折叠或淡出。
5. 最终只保留清晰的主回答、PPT 预览和导出结果。

目标不是展示真实 chain-of-thought，而是展示面向用户可理解的“过程摘要”和“执行状态”，让生成过程显得更智能、更透明，同时避免暴露过多 JSON、SVG 或模型内部推理。

## 2. 参考效果说明

参考截图中的核心体验可以拆解为以下元素：

| 元素 | 说明 |
|------|------|
| 主回答文本 | AI 用正常语气说明下一步要做什么 |
| Skill 完成胶囊 | 显示某个能力或阶段已经完成 |
| 已思考时长 | 以低权重文字展示“已思考 1.43s” |
| 可展开箭头 | 用户可以展开或折叠过程摘要 |
| 灰色过程文本 | 展示任务拆解、调用能力、阶段规划 |
| 自动消失 / 折叠 | 阶段完成后隐藏细节，避免污染最终结果 |

本项目建议复刻其中的交互逻辑，而不是完全复刻视觉样式。

## 3. 当前代码基础

当前代码中已经存在可复用的思考区域能力。

### 3.1 ChatWidget 已有接口

位置：`src/ui/ChatWidget.h`

```cpp
void updateLastAIThinking(const QString &thought);
void collapseThinking();
void expandThinking();
```

对应实现：`src/ui/ChatWidget.cpp`

- `updateLastAIThinking()`：用于更新最后一条 AI 消息的思考区域
- `collapseThinking()`：用于折叠思考区域
- `expandThinking()`：用于展开思考区域

现有实现已经包含：

- 思考区域容器
- 折叠按钮
- 思考内容 QLabel
- 默认隐藏逻辑
- 流式更新时自动展开逻辑

但当前 `updateLastAIThinking()` 是追加式更新：每次调用都会把新的 thought 拼到旧内容后面。这个行为适合流式 thinking，不完全适合 PPT 生成阶段状态。PPT 生成更适合“替换当前摘要”或“保留短历史 + 当前状态”，否则思考区会越来越长，自动折叠只是隐藏内容膨胀，并没有解决信息过载。

因此 MVP 落地时建议保留 `updateLastAIThinking()` 给流式场景使用，另行增加替换式接口：

```cpp
void setLastAIThinking(const QString &text);
```

PPT 阶段状态默认使用 `setLastAIThinking()`，只有明确需要追加短历史时才使用追加式接口。

### 3.2 PPT Agent 已有信号

位置：`src/services/ZhipuPPTAgentService.h`

```cpp
void progressUpdated(int percent, const QString &stage, const QString &detail);
void slideGenerated(int index, const QString &svgCode, const QImage &preview);
void allSlidesGenerated(const QStringList &svgCodes, const QVector<QImage> &previews);
void outlineGenerated(const QJsonObject &outline);
void artifactGenerated(const QString &title, const QString &language, const QString &content);
void errorOccurred(const QString &error);
void stateChanged(ZhipuPPTAgentService::State newState);
```

### 3.3 PPT UI 当前连接点

位置：`src/dashboard/modernmainwindow.cpp`

- `progressUpdated` 回调：用于更新当前进度消息
- `artifactGenerated` 回调：用于追加大纲、布局、SVG 等产物
- `slideGenerated` 回调：用于更新单页预览
- `allSlidesGenerated` 回调：用于完成预览并导出 PPT
- `startPPTGeneration()`：启动 PPT 生成并创建 AI 消息占位

因此，最小实现不需要新建复杂架构，只需要把 PPT 生成阶段与 `ChatWidget` 的思考区域连接起来。

## 4. 设计原则

### 4.1 思考内容展示“过程摘要”，不展示真实推理链

应展示：

- 正在分析主题
- 正在生成大纲
- 正在规划版式
- 正在生成第几页
- 已完成哪个阶段

不应展示：

- 模型真实 chain-of-thought
- 冗长 prompt
- 完整系统提示词
- 全量 JSON / SVG 代码
- 无法确认真实性的内部推理过程

### 4.2 主回答和思考过程分层

主回答承载用户真正需要看到的内容：

```text
我将帮你生成一个爱国主义主题的 PPT。首先，我会分析教学需求，并确定整体设计方向。
```

思考区承载低权重过程摘要：

```text
用户希望生成爱国主义主题 PPT。根据当前需求，我需要先确认：
1. 主题表达重点
2. 授课对象和课堂节奏
3. 页面数量和内容结构
4. 适合思政课堂的视觉风格
```

### 4.3 完成后自动折叠，最终结果保持干净

生成中可以展示更多过程，让用户感到 AI 正在工作；生成完成后，应减少过程内容存在感，让界面回到结果中心。

推荐策略：

- 单阶段完成后：延迟 800-1500ms 自动折叠
- 全部完成后：自动折叠或隐藏思考区
- 用户可手动展开查看历史过程摘要

### 4.4 与 PPT 预览并行呈现

思考区不应替代 PPT 预览。两者分工如下：

| 区域 | 作用 |
|------|------|
| 主回答 | 告诉用户当前 AI 正在做什么 |
| 思考区 | 展示可折叠的过程摘要 |
| PPT 预览区 | 展示真实生成结果 |
| 导出区 | 完成后引导保存 PPT |

### 4.5 信息分流优先于 UI 动效

本次改造的最大风险不是样式，而是信息继续混在一起。当前 `buildPPTProcessMessage()` 会把 `m_pptProcessLog` 拼进最后一条 AI 主消息，并展示“AI 制作过程”。这会让大纲 JSON、布局 JSON、SVG 代码等工程产物继续进入主回答，与“最终结果保持干净”的目标冲突。

必须先明确三类信息边界：

| 信息类型 | 展示位置 | 说明 |
|----------|----------|------|
| 用户友好阶段说明 | 主回答 | 只写当前阶段、下一步、完成结果 |
| 可理解的过程摘要 | 思考区 | 展示需求分析、大纲规划、页面生成状态 |
| 工程产物 / 调试信息 | 调试日志或技术细节入口 | JSON、SVG、prompt、原始 artifact 不默认展示 |

默认情况下，主回答不能再拼接完整 `m_pptProcessLog`。如果需要保留 `artifactGenerated` 的调试价值，应改为：

1. 写入内部日志或开发模式面板；
2. 只在“查看技术细节”入口中展开；
3. 或仅提炼为摘要后进入思考区。

### 4.6 自动折叠必须避免 race

如果每页完成后直接使用 `QTimer::singleShot(1000)` 调用 `collapseThinking()`，可能出现旧 timer 误折叠新状态的问题：例如第 3 页完成后安排折叠，但第 4 页很快开始并展开了思考区，旧 timer 到点后会把第 4 页的思考区折叠。

因此折叠调度需要带 token 或使用可停止的成员定时器。推荐接口：

```cpp
void scheduleThinkingCollapse(int delayMs, int token);
```

调用时记录当前阶段 token / 页码 token，timer 触发后先判断 token 是否仍然匹配，匹配才折叠。

## 5. 用户体验流程

### 5.1 用户发起请求

用户输入：

```text
帮我生成一个爱国主义主题的 PPT
```

AI 主回答：

```text
我将帮你生成一个爱国主义主题的 PPT。首先，我会分析教学需求，并确定整体设计方向。
```

思考区展开：

```text
已思考 0.28s  ˅

正在解析用户需求：
1. 识别主题：爱国主义
2. 判断任务类型：PPT 课件生成
3. 准备确认教学对象、页数和内容侧重点
4. 准备生成教学大纲与页面结构
```

### 5.2 需求确认阶段

主回答：

```text
为了让课件更适合课堂使用，我需要确认几个信息：授课对象、预计页数或课时长度，以及内容更偏理论讲解、案例分析还是课堂互动。
```

思考区：

```text
正在补全 PPT 生成参数：
1. 目标年级 / 授课对象
2. 课时长度 / 页面数量
3. 内容侧重点
4. 是否需要课堂互动环节
```

阶段结束后：

```text
已完成需求确认
```

随后思考区自动折叠为：

```text
已思考 2.16s  ›
```

### 5.3 大纲生成阶段

主回答：

```text
需求已经明确。现在我会生成 PPT 大纲，确定每一页的教学目标和内容顺序。
```

思考区展开：

```text
正在组织 PPT 大纲：
1. 封面：建立主题氛围
2. 导入：用现实案例引出爱国主义
3. 讲解：解释爱国主义的时代内涵
4. 案例：结合青年榜样和社会实践
5. 互动：设计课堂讨论问题
6. 总结：回到价值认同和行动倡议
```

阶段完成 chip：

```text
已完成 Skill：PPT 大纲生成
```

随后自动折叠。

### 5.4 版式规划阶段

主回答：

```text
大纲已经确定。接下来我会为每一页设计合适的版式和视觉层级。
```

思考区：

```text
正在匹配页面版式：
1. 封面页使用大标题和主题视觉
2. 概念页使用左右分栏
3. 案例页使用卡片式布局
4. 互动页突出问题和讨论引导
5. 总结页使用强调式收束
```

阶段完成 chip：

```text
已完成 Skill：页面版式规划
```

随后自动折叠。

### 5.5 逐页生成阶段

主回答：

```text
版式已经准备好，现在开始逐页生成 PPT 页面。
```

思考区：

```text
正在生成第 3 / 8 页：
页面类型：核心概念讲解
设计重点：突出“爱国主义的时代内涵”
视觉策略：标题强调 + 三点式结构 + 课堂提示卡片
```

单页完成 chip：

```text
已完成第 3 页
```

每页生成后思考区可以短暂折叠，再在下一页开始时重新展开。

### 5.6 完成阶段

主回答：

```text
PPT 已生成完成，共生成 8 页幻灯片。我正在整理最终课件并准备导出 PowerPoint 文件。
```

思考区：

```text
正在检查最终结果：
1. 页面数量是否完整
2. 缩略图是否生成成功
3. PPT 文件是否可以导出
4. 是否需要保存到历史记录
```

完成后折叠或隐藏思考区，最终保留：

```text
PPT 已生成完成
共生成 8 页幻灯片，可保存为 PowerPoint 文件。
```

## 6. 视觉设计

### 6.1 思考区样式

推荐样式接近截图：低权重、浅灰、可折叠。

| 属性 | 建议值 |
|------|--------|
| 文本颜色 | `#9CA3AF` / `#A3A3A3` |
| 标题颜色 | `#A1A1AA` |
| 背景色 | 透明或 `#F9FAFB` |
| 边框色 | `#E5E7EB` |
| 左侧竖线 | `#E5E7EB` |
| 字号 | 13px |
| 行高 | 1.5 |
| 内边距 | 8-12px |
| 折叠箭头 | `›` / `˅` |

### 6.2 Skill 完成胶囊

用于显示阶段完成：

```text
已完成 Skill：PPT 大纲生成
```

推荐样式：

| 属性 | 建议值 |
|------|--------|
| 背景 | `#FFFFFF` |
| 边框 | `#E5E7EB` |
| 圆角 | 16-20px |
| 文字 | `#A3A3A3` |
| 高度 | 28-32px |
| 图标 | 使用 SVG 或纯文本，不使用 emoji |

### 6.3 动效

| 动作 | 动效 |
|------|------|
| 思考区出现 | 150-200ms 淡入 |
| 内容更新 | 替换当前摘要，避免长文本不断追加 |
| 阶段完成 | chip 出现，停留 800-1500ms |
| 自动折叠 | 200-300ms 高度收起 + 透明度降低 |
| 全部完成 | 思考区折叠或淡出 |

需要尊重低动效偏好。如果后续引入系统级动画控制，应支持关闭动画。

## 7. 信息状态设计

### 7.1 展开状态

```text
已思考 1.43s  ˅

正在组织 PPT 大纲：
1. 封面：建立主题氛围
2. 导入：用现实案例引出主题
3. 讲解：解释时代内涵
```

### 7.2 折叠状态

```text
已思考 1.43s  ›
```

### 7.3 阶段完成状态

```text
已完成 Skill：PPT 大纲生成
已思考 2.87s  ›
```

### 7.4 错误状态

```text
生成过程遇到问题
AI 在“页面版式规划”阶段未能完成生成，请检查网络连接或稍后重试。
```

思考区可以保留最后阶段摘要：

```text
最后执行到：页面版式规划
已完成：需求分析、PPT 大纲生成
未完成：页面生成、PPT 导出
```

## 8. 与现有 PPT 阶段的映射

| 现有服务状态 | 用户可见阶段 | 主回答 | 思考区 |
|--------------|--------------|--------|--------|
| Idle | 等待开始 | 无 | 无 |
| GeneratingOutline | 生成大纲 | 正在生成 PPT 大纲 | 展示主题拆解和页面结构规划 |
| GeneratingPlan | 规划版式 | 正在设计页面结构 | 展示每页布局策略 |
| GeneratingSVG | 逐页生成 | 正在生成第 N 页 | 展示当前页类型和设计重点 |
| Finished | 完成导出 | PPT 已生成完成 | 折叠或隐藏 |
| Failed | 生成失败 | 生成过程遇到问题 | 保留失败阶段摘要 |

阶段映射应集中在 helper 函数中，避免散落在多个 signal 回调里。推荐新增：

```cpp
QString buildPPTMainMessage(const QString &stage, int percent) const;
QString buildPPTThinkingSummary(const QString &stage, const QString &detail) const;
```

职责划分：

| 函数 | 职责 | 禁止内容 |
|------|------|----------|
| `buildPPTMainMessage` | 生成主回答阶段说明 | JSON、SVG、prompt、长日志 |
| `buildPPTThinkingSummary` | 生成思考区摘要 | 原始 artifact、完整模型回复、真实推理链 |

如果后续服务层 `stage` 文案变化，只改这两个 helper，避免 UI 回调里出现多套判断。

## 9. 文案规范

### 9.1 主回答文案

主回答应该像 AI 助教在解释接下来要做什么。

示例：

```text
我将帮你生成一个爱国主义主题的 PPT。首先，我会分析教学需求，并确定整体设计方向。
```

```text
大纲已经确定。接下来我会为每一页设计合适的版式和视觉层级。
```

```text
版式已经准备好，现在开始逐页生成 PPT 页面。
```

### 9.2 思考区文案

思考区应该是过程摘要，不要写得像最终回答。

示例：

```text
正在解析用户需求：
1. 识别主题：爱国主义
2. 判断任务类型：PPT 课件生成
3. 准备生成教学大纲与页面结构
```

```text
正在生成第 4 / 8 页：
页面类型：案例分析页
设计重点：结合真实案例增强课堂代入感
视觉策略：案例卡片 + 关键结论强调
```

### 9.3 禁止文案

避免：

```text
我正在进行深度推理……
```

```text
根据我的隐藏思维链……
```

```text
以下是我的完整思考过程……
```

推荐替换为：

```text
生成过程摘要
```

```text
执行步骤
```

```text
AI 正在处理
```

## 10. 最小实现方案

### 10.1 新增 ChatWidget 替换式思考接口

位置：`src/ui/ChatWidget.h` / `src/ui/ChatWidget.cpp`

新增接口：

```cpp
void setLastAIThinking(const QString &text, int token = -1);
```

用途：替换当前思考摘要，而不是追加内容。

推荐行为：

1. text 为空时不展示思考正文，只保留或隐藏 header。
2. text 非空时显示思考区并展开。
3. 不拼接旧内容。
4. 不影响 `updateLastAIThinking()` 的原有追加语义。
5. 如果传入 token，则记录为当前思考内容 token，并停止旧折叠 timer。

伪代码：

```cpp
void ChatWidget::setLastAIThinking(const QString &text, int token)
{
    if (!m_lastAIThinkingLabel || !m_lastAIThinkingWidget) {
        return;
    }

    if (token >= 0) {
        m_thinkingContentToken = token;
    }
    if (m_thinkingCollapseTimer) {
        m_thinkingCollapseTimer->stop();
    }

    const QString cleanText = text.trimmed();
    m_lastAIThinkingWidget->setVisible(!cleanText.isEmpty());
    m_lastAIThinkingLabel->setVisible(!cleanText.isEmpty());
    m_lastAIThinkingLabel->setText(cleanText);

    if (m_lastAIThinkingToggle) {
        m_lastAIThinkingToggle->setText("v");
    }

    scrollToBottom();
}
```

### 10.2 新增带 token 的折叠调度

位置：`src/ui/ChatWidget.h` / `src/ui/ChatWidget.cpp`

新增接口：

```cpp
void scheduleThinkingCollapse(int delayMs, int token);
```

推荐新增成员：

```cpp
int m_thinkingContentToken = 0;
int m_pendingThinkingCollapseToken = 0;
QTimer *m_thinkingCollapseTimer = nullptr;
```

推荐行为：

1. 每次调度前停止旧 timer。
2. 保存本次待折叠 token。
3. timer 到点后检查待折叠 token 是否仍等于当前内容 token。
4. 只有匹配时才折叠，避免旧 timer 折叠新阶段内容。

伪代码：

```cpp
void ChatWidget::scheduleThinkingCollapse(int delayMs, int token)
{
    if (!m_thinkingCollapseTimer) {
        m_thinkingCollapseTimer = new QTimer(this);
        m_thinkingCollapseTimer->setSingleShot(true);
        connect(m_thinkingCollapseTimer, &QTimer::timeout, this, [this]() {
            if (m_pendingThinkingCollapseToken == m_thinkingContentToken) {
                collapseThinking();
            }
        });
    }

    m_pendingThinkingCollapseToken = token;
    m_thinkingCollapseTimer->stop();
    m_thinkingCollapseTimer->start(delayMs);
}
```

实现时可按实际命名调整，但必须满足“旧调度不能误折叠新内容”。

### 10.3 修改 `startPPTGeneration()`

位置：`src/dashboard/modernmainwindow.cpp`

当前逻辑会添加 AI 消息占位并开始 PPT 预览。建议主回答只展示用户友好的阶段说明，思考区展示过程摘要。

```cpp
m_bubbleChatWidget->addMessage(
    "我将帮你生成这份PPT。首先，我会分析教学需求，并确定整体设计方向。",
    false);
const int thinkingToken = ++m_pptThinkingToken;
m_bubbleChatWidget->setLastAIThinking(
    "正在解析用户需求：\n"
    "1. 识别PPT主题和教学目标\n"
    "2. 整理授课对象、课时和内容侧重点\n"
    "3. 准备生成教学大纲与页面结构",
    thinkingToken);
```

### 10.4 修改 `progressUpdated` 回调

根据 `stage` 判断当前阶段，更新主回答和思考区。

```cpp
connect(m_pptAgentService, &ZhipuPPTAgentService::progressUpdated,
        this, [this](int percent, const QString &stage, const QString &detail) {
    if (!m_bubbleChatWidget) return;

    const int thinkingToken = ++m_pptThinkingToken;
    m_bubbleChatWidget->updateLastAIMessage(buildPPTMainMessage(stage, percent));
    m_bubbleChatWidget->setLastAIThinking(
        buildPPTThinkingSummary(stage, detail), thinkingToken);
});
```

### 10.5 修改 `slideGenerated` 回调

单页生成完成后：

```cpp
const int thinkingToken = ++m_pptThinkingToken;
m_bubbleChatWidget->setLastAIThinking(
    QString("已完成第 %1 页，正在准备下一页。").arg(index + 1),
    thinkingToken);
m_bubbleChatWidget->scheduleThinkingCollapse(1000, thinkingToken);
```

下一页开始时需要再次递增 `m_pptThinkingToken` 或使用新 token 展开，确保上一页的折叠调度不会影响当前页。

### 10.6 修改 `allSlidesGenerated` 回调

全部完成后：

```cpp
const int thinkingToken = ++m_pptThinkingToken;
m_bubbleChatWidget->updateLastAIMessage(
    QString("PPT 已生成完成，共生成 %1 页幻灯片。正在准备导出 PowerPoint 文件。").arg(totalPages));
m_bubbleChatWidget->scheduleThinkingCollapse(1200, thinkingToken);
```

### 10.7 调整 `artifactGenerated` 与 `buildPPTProcessMessage`

必须避免继续把 `m_pptProcessLog` 默认拼进主回答。

建议：

1. `artifactGenerated` 仍可收集产物，但默认不进入主回答。
2. `buildPPTProcessMessage()` 改为只返回主阶段说明，不拼接完整 `m_pptProcessLog`。
3. 如果需要展示 artifact，只展示摘要或放到“查看技术细节”。

### 10.8 可选新增接口：淡出并折叠

如果希望实现截图中“思考输出后又消失”的效果，建议给 `ChatWidget` 增加：

```cpp
void fadeOutThinkingAndCollapse(int delayMs = 1000);
```

内部使用：

- `QGraphicsOpacityEffect`
- `QPropertyAnimation`
- 动画完成后隐藏内容并恢复 opacity

伪代码：

```cpp
void ChatWidget::fadeOutThinkingAndCollapse(int delayMs)
{
    if (!m_lastAIThinkingWidget || !m_lastAIThinkingLabel) {
        return;
    }

    QTimer::singleShot(delayMs, this, [this]() {
        auto *effect = new QGraphicsOpacityEffect(m_lastAIThinkingWidget);
        m_lastAIThinkingWidget->setGraphicsEffect(effect);

        auto *animation = new QPropertyAnimation(effect, "opacity", this);
        animation->setDuration(240);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);

        connect(animation, &QPropertyAnimation::finished, this, [this, effect]() {
            collapseThinking();
            m_lastAIThinkingWidget->setGraphicsEffect(nullptr);
            effect->deleteLater();
        });

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });
}
```

## 11. 推荐的分阶段落地

### 阶段一：无动画 MVP

目标：最快看到效果，同时先解决信息分流问题。

内容：

1. 新增 `setLastAIThinking()`，PPT 阶段状态使用替换式摘要。
2. 新增 `scheduleThinkingCollapse()`，延迟折叠必须带 token。
3. `progressUpdated` 时通过 helper 更新主回答和思考摘要。
4. `slideGenerated` 后调度带 token 的折叠。
5. `allSlidesGenerated` 后折叠思考区。
6. `buildPPTProcessMessage()` 不再默认拼接完整 `m_pptProcessLog`。

优点：

- 改动小
- 风险可控
- 避免思考区内容不断膨胀
- 避免主回答继续变成工程日志
- 能马上获得“思考输出后折叠”的效果

### 阶段二：增加计时与标题

目标：贴近截图中的“已思考 1.43s”。

内容：

1. 新增思考开始时间。
2. Header 显示 `已思考 X.XXs`。
3. 折叠状态只显示 header。

### 阶段三：增加淡出动画

目标：实现“输出后又消失”的高级体验。

内容：

1. 新增 `fadeOutThinkingAndCollapse()`。
2. 阶段完成后先显示完成 chip。
3. 延迟淡出，再折叠。

### 阶段四：增加 Skill 完成胶囊

目标：增强 Agent 感。

内容：

1. 增加 `addThinkingSkillChip(title)` 或复用思考文本。
2. 显示：`已完成 Skill：PPT 大纲生成`。
3. Skill chip 自动淡出或折叠进历史。

## 12. 验收标准

### 12.1 基础验收

- 用户发起 PPT 生成后，AI 主回答不再只是“正在初始化”。
- 生成过程中出现可展开的思考区域。
- 思考区域会随 PPT 阶段更新内容。
- PPT 阶段摘要使用替换式更新，不会不断追加成长文本。
- 阶段完成后思考区域会自动折叠。
- 旧折叠 timer 不会误折叠新阶段思考内容。
- PPT 预览区仍正常显示。
- PPT 导出流程不受影响。

### 12.2 体验验收

- 用户能明显感知 AI 正在“分析、规划、生成”。
- 思考区不会长期占用过多空间。
- 最终结果页面保持简洁。
- 用户可以手动展开查看过程摘要。
- 失败时能看到失败发生在哪个阶段。

### 12.3 质量验收

- 不展示真实 chain-of-thought。
- 不默认展示冗长 JSON / SVG。
- 主回答不拼接完整 `m_pptProcessLog`。
- `artifactGenerated` 的原始产物默认不进入用户主回答。
- 阶段文案集中由 `buildPPTMainMessage()` 和 `buildPPTThinkingSummary()` 管理。
- 不使用 emoji 作为 UI 图标。
- 动画时长控制在 150-300ms。
- 折叠、展开、生成中状态不会导致界面跳动严重。

## 13. 推荐结论

建议先实现“无动画 MVP”，但 MVP 不应只是把 PPT Agent 的阶段状态接入现有 `updateLastAIThinking()`。必须同时完成三件事：

1. 主回答、思考区、工程产物的信息分流；
2. `setLastAIThinking()` 替换式摘要，避免内容膨胀；
3. 带 token 的折叠调度，避免异步 race。

在这三项稳定后，再补计时、淡出动画和 Skill 完成胶囊，让体验更接近大模型 Agent 的输出过程。
