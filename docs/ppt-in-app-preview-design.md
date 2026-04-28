# PPT 生成后应用内预览功能规划文档

## 1. 背景与目标

当前 PPT 生成完成后，系统会保存 PPTX 文件到用户选择的位置，并在聊天气泡内展示页面缩略图。用户如果想完整查看生成结果，通常需要回到桌面或外部 PowerPoint / WPS 中打开文件。

新的交互应调整为“先预览、再保存”：PPT 页面生成完成后，系统先进入生成完成状态，展示缩略图和操作按钮，不自动弹出保存对话框。用户可以先在应用内完整预览，确认满意后再点击“保存到桌面”导出 `.pptx` 文件。

本功能希望增加一个“页面内查看 PPT”的能力：

1. PPT 生成完成后，在生成结果区域显示“预览”和“保存到桌面”按钮。
2. 用户点击“预览”后，在应用内部打开 PPT 查看页面。
3. 用户不需要回到桌面或外部应用即可浏览幻灯片。
4. 用户在预览页确认效果后，可以直接点击“保存到桌面”。
5. 用户点击“退出预览”后，返回原来的生成完成页面。
6. 返回后，聊天记录、PPT 预览缩略图、生成完成状态、保存状态都不改变。

本次文档只做功能规划，不修改代码。

## 2. 用户需求描述

用户希望：

```text
PPT 生成完成后，我能直接在软件里点一个预览按钮查看 PPT。
不需要去桌面找文件，也不需要打开外部软件。
看完满意后，可以在预览页直接保存到桌面。
看完后点退出，就回到刚才生成完成的页面，页面状态不要变。
```

核心体验：

| 用户动作 | 系统响应 |
|----------|----------|
| PPT 生成完成 | 显示生成完成信息、缩略图、“预览PPT”和“保存到桌面”按钮 |
| 点击“预览” | 进入应用内 PPT 查看模式 |
| 在预览模式中翻页 | 查看上一页 / 下一页 / 缩略图列表 |
| 在预览模式中点击“保存到桌面” | 弹出保存对话框并导出 PPTX |
| 点击“退出预览” | 返回生成完成页面 |
| 返回后 | 原聊天页面、预览缩略图和保存信息保持不变 |

## 3. 当前代码基础

### 3.1 已有聊天内缩略图预览

`ChatWidget` 已经具备 PPT 缩略图展示能力：

- `beginPPTPreviewProgress()`
- `updatePPTPreviewProgress(int slideIndex, const QImage &preview)`
- `finishPPTPreviewProgress()`

位置：`src/ui/ChatWidget.h` / `src/ui/ChatWidget.cpp`

当前生成流程中，`ModernMainWindow` 在收到单页生成信号后会调用：

```cpp
m_bubbleChatWidget->updatePPTPreviewProgress(index, preview);
```

生成完成后会调用：

```cpp
m_bubbleChatWidget->finishPPTPreviewProgress();
```

### 3.2 已有 PPT 历史记录能力

`ModernMainWindow` 已经保存 PPT 生成记录：

- `savePPTRecord()`
- `updatePPTRecordFilePath()`
- `restorePPTRecordToChat()`
- `pptHistoryDir()`
- `pptHistoryIndexPath()`

位置：`src/dashboard/modernmainwindow.cpp`

记录中包含：

```json
{
  "id": "ppt_xxx",
  "title": "PPT 生成：8 页",
  "filePath": "...",
  "totalPages": 8,
  "createdAt": 1234567890,
  "previewPaths": [
    "slide_001.png",
    "slide_002.png"
  ]
}
```

这说明系统已经具备“用图片预览 PPT”的数据基础。

### 3.3 已有独立 PPTPreviewPage

项目中已有 `PPTPreviewPage`：

- 位置：`src/ui/pptpreviewpage.h/.cpp`
- 能设置标题与文件路径：`setPresentation(title, filePath)`
- 有返回按钮：`backRequested()`
- 有下载按钮：`downloadRequested()`
- 内部使用 `MacOSQuickLookPreview`

当前它更偏“基于已保存 PPT 文件路径的预览页”。但这次需求有一个关键点：用户点击预览时，可能还没有保存到桌面，或者用户取消保存后仍希望看生成结果。

因此不能只依赖 `filePath`，还需要支持基于 `previewPaths` 或 `QImage` 的内置幻灯片查看。

### 3.4 已有 MacOSQuickLookPreview

项目中已有 `MacOSQuickLookPreview`：

- 位置：`src/ui/macosquicklookpreview.h/.cpp`
- 当前实现会尝试从 PPTX 文件提取幻灯片图片并显示
- 如果无法提取，会提示使用外部 PowerPoint / WPS 打开

这个组件可以作为后续增强，但 MVP 更建议直接复用生成时已有的 `QImage` / `previewPaths`，避免依赖 PPTX 文件解析。

## 4. 设计原则

### 4.1 预览不改变生成结果页面状态

这是本功能最重要的规则。

用户进入预览页后，原页面应保持：

- 最后一条 AI 生成完成消息不变
- PPT 缩略图区域不变
- 保存成功 / 取消保存状态不变
- 左侧历史记录不变
- 当前聊天上下文不清空
- 不重新生成 PPT
- 不自动重新导出 PPT

预览应该是“临时查看层”，不是新的生成流程。预览页中的“保存到桌面”只是触发当前 PPT 的导出动作，不应重跑生成链路。

### 4.2 优先使用已生成的预览图片

MVP 不应依赖桌面文件，也不应依赖外部软件。

优先数据源：

1. 当前内存中的 `QVector<QImage> previews`
2. 已保存历史记录中的 `previewPaths`
3. 如果已经保存 PPTX，再补充使用 `filePath`

这样即使用户取消保存 PPTX，也可以在应用内预览已经生成的幻灯片图片。

### 4.3 预览入口必须清晰但不打扰

生成完成后按钮建议放在生成完成消息或 PPT 缩略图区域附近。

推荐按钮：

```text
预览PPT    保存到桌面
```

用户也可以先点击“预览PPT”，进入应用内查看页面后再点击右上角“保存到桌面”。聊天气泡和预览页的保存按钮应复用同一套保存逻辑。

如果 PPT 已经保存：

```text
预览PPT    打开文件位置
```

如果用户取消保存或尚未保存：

```text
预览PPT    保存到桌面
```

### 4.4 预览模式退出必须可预测

用户点击“退出预览”后，应回到打开预览前的页面。

不建议：

- 清空聊天页面
- 重新恢复历史记录
- 跳回欢迎页
- 再弹一次保存对话框
- 改变生成完成消息

推荐：

- 使用 `QStackedWidget` 切换到预览页面
- 记录打开预览前的 stack index
- 退出预览时切回原 index

## 5. 功能范围

### 5.1 MVP 范围

必须实现：

1. PPT 生成完成后显示“预览PPT”按钮。
2. 点击按钮后进入应用内预览页面。
3. 预览页面展示所有幻灯片图片。
4. 支持上一页 / 下一页。
5. 支持页码显示，例如 `3 / 8`。
6. 支持缩略图列表或侧边页码列表。
7. 预览页支持“保存到桌面”。
8. 支持“退出预览”。
9. 退出后返回原生成完成页面，状态不变。
10. 用户未保存或取消保存 PPTX 后仍能预览。
11. 历史记录恢复的 PPT 也能点击预览。
12. 聊天气泡和预览页的保存动作复用同一套导出逻辑。

### 5.2 非 MVP 范围

暂不做：

1. 真实 PPT 动画播放。
2. PowerPoint 编辑能力。
3. 修改单页后重新生成。
4. 演讲者备注。
5. 全屏放映模式。
6. 激光笔 / 批注。
7. 复杂转场动画。
8. 与外部 PowerPoint / WPS 的深度集成。

这些可以作为后续增强。

## 6. 推荐交互方案

### 6.1 生成完成后的聊天区域

生成完成后，聊天气泡建议变成：

```text
PPT 已生成完成，共生成 8 页幻灯片。

[预览PPT] [保存到桌面]
```

下方保持现有缩略图区域：

```text
PPT 页面预览
[第1页] [第2页]
[第3页] [第4页]
...
```

如果用户已经保存成功：

```text
PPT 已生成完成，共 8 页幻灯片已保存为 PowerPoint 文件：
/Users/xxx/Desktop/思政课堂PPT.pptx

[预览PPT] [打开文件位置]
```

如果用户取消保存：

```text
PPT 已生成完成，共生成 8 页幻灯片。
您已取消保存文件，可在左侧历史记录中重新查看预览。

[预览PPT] [保存到桌面]
```

生成完成后不应自动弹出系统保存对话框。保存对话框只在用户主动点击“保存到桌面”时出现。

### 6.2 应用内预览页面布局

推荐布局：

```text
┌──────────────────────────────────────────────────────────┐
│  退出预览      爱国主义主题 PPT               保存到桌面 │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────┐  ┌──────────────────────────────────┐  │
│  │ 第1页        │  │                                  │  │
│  │ 第2页        │  │          当前幻灯片大图           │  │
│  │ 第3页 选中   │  │                                  │  │
│  │ 第4页        │  │                                  │  │
│  └──────────────┘  └──────────────────────────────────┘  │
│                                                          │
│                上一页     3 / 8     下一页              │
└──────────────────────────────────────────────────────────┘
```

### 6.3 操作方式

| 操作 | 行为 |
|------|------|
| 点击“预览PPT” | 打开应用内预览页 |
| 点击“保存到桌面” | 弹出保存对话框，导出当前 PPTX |
| 点击“退出预览” | 返回生成完成页面 |
| 点击上一页 | 切换到上一张 |
| 点击下一页 | 切换到下一张 |
| 点击缩略图 | 跳转到对应页 |
| 按 Esc | 退出预览 |
| 按 ← | 上一页 |
| 按 → | 下一页 |
| 按 Home | 第一页 |
| 按 End | 最后一页 |

### 6.4 视觉风格

根据 `ui-ux-pro-max` 建议，使用专业教育类浅色风格和微交互。

推荐色彩：

| 用途 | 色值 |
|------|------|
| 背景 | `#F8FAFC` |
| 主文本 | `#1E293B` |
| 主按钮 | `#C00000` 或项目主题红 |
| 边框 | `#E5E7EB` |
| 次级文字 | `#64748B` |
| 当前页高亮 | `rgba(192, 0, 0, 0.08)` |

交互要求：

- 按钮高度不低于 40px，点击目标建议 ≥ 44px。
- 退出预览按钮必须始终可见。
- 当前页缩略图需要明显高亮。
- 禁止使用 emoji 作为 UI 图标。
- hover / focus 状态明确。
- 过渡动画控制在 150-300ms。

## 7. 数据流设计

### 7.1 生成完成时

现有流程：

```text
ZhipuPPTAgentService::allSlidesGenerated(svgCodes, previews)
  -> ModernMainWindow 保存 preview images
  -> savePPTRecord(QString(), previews, totalPages)
  -> ChatWidget 展示缩略图
  -> ChatWidget 显示“预览PPT / 保存到桌面”
```

建议新增状态：

```cpp
QString m_currentPPTRecordId;
QVector<QImage> m_currentPPTPreviews;
QString m_currentPPTFilePath;
int m_currentPPTTotalPages;
```

生成完成后：

```cpp
m_currentPPTRecordId = pptRecordId;
m_currentPPTPreviews = previews;
m_currentPPTTotalPages = totalPages;
m_currentPPTFilePath.clear();
```

用户点击“保存到桌面”后：

```cpp
saveCurrentPPTToDesktop();
```

保存成功后：

```cpp
m_currentPPTFilePath = filePath;
updatePPTRecordFilePath(m_currentPPTRecordId, filePath);
m_inAppPPTPreviewPage->setFilePath(filePath);
```

取消保存时：

```cpp
m_currentPPTFilePath.clear();
// 但 m_currentPPTPreviews 和 m_currentPPTRecordId 保留
```

无论用户从聊天气泡点击“保存到桌面”，还是从预览页点击“保存到桌面”，都应调用同一个 `saveCurrentPPTToDesktop()`，避免两套导出逻辑产生状态不一致。

### 7.2 点击预览时

优先使用内存预览：

```text
如果 m_currentPPTPreviews 非空：
  打开 Image-based PPT viewer
否则如果 m_currentPPTRecordId 非空：
  从 previewPaths 读取图片
否则如果 m_currentPPTFilePath 存在：
  尝试使用 PPTPreviewPage / MacOSQuickLookPreview
否则：
  提示暂无可预览内容
```

### 7.3 历史记录恢复时

`restorePPTRecordToChat(recordId)` 当前会：

1. 清空聊天消息
2. 恢复生成完成消息
3. 展示预览缩略图

建议同时设置：

```cpp
m_currentPPTRecordId = recordId;
m_currentPPTFilePath = record["filePath"].toString();
m_currentPPTTotalPages = record["totalPages"].toInt();
m_currentPPTPreviews = loadImages(record["previewPaths"]);
```

这样恢复历史记录后，用户也可以点击“预览PPT”。

## 8. 组件设计建议

### 8.1 新增 InAppPPTPreviewPage

建议新增组件：

```text
src/ui/InAppPPTPreviewPage.h
src/ui/InAppPPTPreviewPage.cpp
```

职责：

- 接收 `QVector<QImage>` 或图片路径列表
- 展示当前页大图
- 展示页码
- 提供上一页 / 下一页 / 缩略图跳转
- 提供退出按钮
- 提供“保存到桌面”按钮

接口建议：

```cpp
class InAppPPTPreviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit InAppPPTPreviewPage(QWidget *parent = nullptr);

    void setSlides(const QString &title, const QVector<QImage> &slides);
    void setSlidesFromPaths(const QString &title, const QStringList &paths);
    void setFilePath(const QString &filePath);
    QString filePath() const;

signals:
    void exitRequested();
    void saveRequested();

private:
    void showSlide(int index);
    void showPreviousSlide();
    void showNextSlide();
    void updateNavigationState();
};
```

### 8.2 为什么不直接只用 PPTPreviewPage

已有 `PPTPreviewPage` 依赖 `filePath`：

```cpp
void setPresentation(const QString &title, const QString &filePath);
```

它适合“已有 PPTX 文件路径”的场景，但本需求要求：

- 不回到桌面
- 即使取消保存也能在应用内查看
- 查看生成时已有的页面预览

因此 MVP 建议新增 image-based viewer，而不是只改 `PPTPreviewPage`。

后续可以整合：

- `PPTPreviewPage`：文件级预览
- `InAppPPTPreviewPage`：图片级预览
- 或把 `PPTPreviewPage` 扩展为同时支持 filePath 和 images

更推荐后者作为长期方向，但 MVP 新增独立组件风险更低。

### 8.3 ChatWidget 增加预览按钮能力

当前 `ChatWidget` 只负责展示预览缩略图，没有对外提供“预览按钮”事件。

建议新增：

```cpp
void showPPTActions(bool canPreview, bool canSave);

signals:
    void pptPreviewRequested();
    void pptSaveRequested();
```

生成完成后：

```cpp
m_bubbleChatWidget->showPPTActions(true, true);
```

点击“预览PPT”后：

```cpp
emit pptPreviewRequested();
```

点击“保存到桌面”后：

```cpp
emit pptSaveRequested();
```

由 `ModernMainWindow` 负责打开预览页面。
保存请求也由 `ModernMainWindow` 统一处理，调用和预览页相同的 `saveCurrentPPTToDesktop()`。

### 8.4 ModernMainWindow 统一保存入口

建议新增统一保存方法：

```cpp
void saveCurrentPPTToDesktop();
```

职责：

1. 检查 `m_currentPPTPreviews` 是否可用。
2. 弹出 `QFileDialog::getSaveFileName()`。
3. 调用 `m_pptxGenerator->generateFromImages(...)`。
4. 保存成功后更新 `m_currentPPTFilePath`。
5. 调用 `updatePPTRecordFilePath(m_currentPPTRecordId, filePath)`。
6. 更新聊天气泡和预览页的保存状态。
7. 用户取消保存时，只保留当前 previews，不改变预览状态。

## 9. 页面切换设计

### 9.1 推荐使用 QStackedWidget

`ModernMainWindow` 已经有主内容区域和 `QStackedWidget` 相关结构。推荐将应用内预览页作为一个 stack page。

打开预览时：

```cpp
m_beforePPTPreviewStackIndex = m_mainStack->currentIndex();
m_inAppPPTPreviewPage->setSlides(...);
m_mainStack->setCurrentWidget(m_inAppPPTPreviewPage);
```

退出预览时：

```cpp
m_mainStack->setCurrentIndex(m_beforePPTPreviewStackIndex);
```

这样可以保证原页面 Widget 没有被销毁，状态自然保持不变。

### 9.2 不建议使用方式

不建议：

1. `clearMessages()` 后重建聊天页面。
2. 用历史记录重新恢复当前 PPT。
3. 打开系统外部预览器。
4. 直接覆盖当前聊天气泡内容。
5. 退出预览后重新走生成完成逻辑。

这些都会增加状态不一致风险。

## 10. 状态设计

### 10.1 新增状态字段

建议在 `ModernMainWindow` 中新增：

```cpp
QString m_currentPPTRecordId;
QString m_currentPPTFilePath;
QVector<QImage> m_currentPPTPreviews;
int m_currentPPTTotalPages = 0;
int m_beforePPTPreviewStackIndex = -1;
InAppPPTPreviewPage *m_inAppPPTPreviewPage = nullptr;
```

### 10.2 状态更新时机

| 时机 | 更新内容 |
|------|----------|
| PPT 开始生成 | 清空当前 PPT 预览状态 |
| allSlidesGenerated | 保存 previews、totalPages、recordId |
| 点击保存到桌面 | 调用 `saveCurrentPPTToDesktop()` |
| 保存成功 | 更新 filePath，并同步聊天气泡 / 预览页状态 |
| 取消保存 | 保留 previews，filePath 可为空 |
| 历史记录恢复 | 从 record 恢复 previews、filePath、totalPages |
| 打开预览 | 设置 preview page 数据，记录原 stack index |
| 退出预览 | 只切回原 stack index，不改当前 PPT 状态 |

### 10.3 空状态处理

如果用户点击预览但没有可用图片：

```text
当前没有可预览的 PPT 页面。
请先生成 PPT，或从历史记录中选择一份已生成的 PPT。
```

如果部分图片丢失：

```text
部分幻灯片预览文件已丢失，仅展示可读取的页面。
```

如果 PPTX 文件存在但图片不存在：

```text
当前记录缺少页面预览图片，可尝试打开已保存的 PPT 文件。
```

## 11. 实现步骤建议

### 阶段一：文档和状态准备

1. 明确本设计文档。
2. 确认是否复用 `PPTPreviewPage` 或新增 `InAppPPTPreviewPage`。
3. 确认 `ModernMainWindow` 中主内容 stack 的接入点。

### 阶段二：实现图片型预览页

1. 新增 `InAppPPTPreviewPage`。
2. 支持 `setSlides(title, QVector<QImage>)`。
3. 支持 `setSlidesFromPaths(title, QStringList)`。
4. 支持上一页、下一页、缩略图点击。
5. 支持 Esc 退出。
6. 支持空状态。

### 阶段三：ChatWidget 增加操作按钮

1. 在 PPT 生成完成区域增加“预览PPT”和“保存到桌面”按钮。
2. 增加 `pptPreviewRequested()` 和 `pptSaveRequested()` signal。
3. 保证按钮不影响现有缩略图布局。
4. 历史记录恢复时也显示预览按钮。

### 阶段四：ModernMainWindow 接入预览页

1. 新增当前 PPT 状态字段。
2. `allSlidesGenerated` 保存当前 previews。
3. `savePPTRecord` 后记录当前 recordId。
4. 新增 `saveCurrentPPTToDesktop()`，统一处理聊天气泡和预览页的保存请求。
5. 保存成功后记录 filePath，并更新历史记录。
6. 取消保存不清除 previews。
7. 连接 `ChatWidget::pptPreviewRequested` 到 `openCurrentPPTPreview()`。
8. 连接 `ChatWidget::pptSaveRequested` 和 `InAppPPTPreviewPage::saveRequested` 到 `saveCurrentPPTToDesktop()`。
9. 退出预览时切回原 stack index。

### 阶段五：验证

1. 生成 PPT 后点击“预览PPT”。
2. 翻页查看所有页面。
3. 在预览页点击“保存到桌面”并成功导出 PPTX。
4. 点击退出后回到生成完成页面。
5. 取消保存后仍能预览。
6. 保存成功后仍能预览，且聊天气泡 / 历史记录显示保存路径。
7. 从历史记录恢复后仍能预览。
8. 预览过程中不触发重新生成，保存只在用户主动点击时触发。

## 12. 验收标准

### 12.1 功能验收

- PPT 生成完成后出现“预览PPT”按钮。
- PPT 生成完成后出现“保存到桌面”按钮。
- 点击按钮后在应用内打开预览页。
- 预览页能展示所有已生成幻灯片。
- 支持上一页 / 下一页。
- 支持缩略图跳转。
- 预览页支持“保存到桌面”。
- 支持退出预览。
- 退出后回到原生成完成页面。
- 原页面状态不变。
- 用户未保存或取消保存文件后仍能预览。
- 保存成功后，当前记录和历史记录中的 filePath 同步更新。
- 历史记录恢复的 PPT 也能预览。

### 12.2 体验验收

- 用户不需要回到桌面查找 PPT 文件。
- 用户可以先预览，满意后再保存到桌面。
- 退出预览后不会丢失聊天上下文。
- 预览按钮位置清晰，不遮挡生成结果。
- 预览页面有明确标题、页码、退出入口和保存入口。
- 当前页状态明显。
- 翻页操作响应及时。

### 12.3 可访问性验收

- 退出预览按钮可通过键盘聚焦。
- 保存到桌面按钮可通过键盘聚焦。
- 上一页 / 下一页按钮可通过键盘触发。
- Esc 可退出预览。
- 左右方向键可翻页。
- 当前页码文本清晰。
- 按钮文字对比度满足 4.5:1。
- 点击目标高度不低于 40px，建议 44px。

### 12.4 非回归验收

- PPT 生成流程不受影响。
- PPTX 导出流程不受影响。
- 历史记录保存不受影响。
- `ChatWidget` 原有普通聊天消息不受影响。
- Dify 普通对话不受影响。
- 应用内预览不改变 `m_pptProcessLog` 或当前生成记录。

## 13. 风险与处理

| 风险 | 说明 | 处理 |
|------|------|------|
| 只依赖 filePath 导致取消保存后无法预览 | 用户取消保存时没有 PPTX 文件 | MVP 使用 previews / previewPaths |
| 自动弹保存对话框打断预览 | 生成完成后用户还没确认效果 | 保存对话框只在用户点击“保存到桌面”时出现 |
| 聊天气泡和预览页保存逻辑分叉 | 两处入口都能保存，容易状态不一致 | 两处按钮统一调用 `saveCurrentPPTToDesktop()` |
| 退出预览后页面状态丢失 | 如果重建聊天页面会丢状态 | 使用 QStackedWidget 切换，不销毁原页面 |
| 图片太多导致内存占用 | 长 PPT 可能有很多 QImage | MVP 可接受；后续可按路径懒加载 |
| 历史图片文件丢失 | 用户清理 AppData 后无法预览 | 展示缺失提示，不崩溃 |
| ChatWidget 职责变重 | 加入按钮可能让聊天组件复杂 | 只发 signal，具体打开预览由 ModernMainWindow 负责 |
| 预览页和已有 PPTPreviewPage 重复 | 已有 filePath 预览页 | MVP 独立实现，后续再合并 |

## 14. 推荐结论

建议 MVP 使用“图片型应用内预览”方案，而不是直接依赖 PPTX 文件预览。

原因：

1. 生成流程已经有 `QVector<QImage> previews`。
2. 历史记录已经保存 `previewPaths`。
3. 用户取消保存 PPTX 后仍能查看。
4. 不依赖外部 PowerPoint / WPS。
5. 更容易保证退出后原页面状态不变。
6. 支持先预览、满意后再保存到桌面。

推荐落地顺序：

1. 新增 `InAppPPTPreviewPage`。
2. 在 `ChatWidget` 的 PPT 完成区域加“预览PPT / 保存到桌面”按钮和 signal。
3. `ModernMainWindow` 保存当前 PPT previews / recordId / filePath。
4. 新增 `saveCurrentPPTToDesktop()`，统一处理聊天气泡和预览页保存。
5. 使用 `QStackedWidget` 切换到预览页。
6. 退出时只切回原页面，不重建、不清空、不重新生成。
