时政热点追踪模块是 AI 思政智慧课堂系统中负责"新闻数据获取 → 分类筛选 → 教学案例生成"这一核心链路的功能单元。它采用**策略模式（Strategy Pattern）**将新闻数据源抽象为 `INewsProvider` 接口，通过依赖注入在 `HotspotService` 业务层实现 Mock 数据与真实网络数据的无缝切换。本文将深入剖析接口契约、两类 Provider 实现的设计取舍、多源聚合与降级策略，以及从 UI 到数据源的完整调用链。

Sources: [INewsProvider.h](src/hotspot/INewsProvider.h#L1-L82), [HotspotService.h](src/services/HotspotService.h#L1-L126), [HotspotTrackingWidget.h](src/ui/HotspotTrackingWidget.h#L1-L135)

## 架构总览：三层分离与策略模式

该模块遵循系统整体的分层架构思想，将职责清晰地划分为三个层次。**UI 层**（`HotspotTrackingWidget`）仅负责展示与用户交互；**服务层**（`HotspotService`）管理缓存、分类切换、AI 教学内容生成等业务逻辑；**数据源层**（`INewsProvider` 及其实现）封装一切网络请求和解析细节。三者之间的依赖关系通过 Qt 信号/槽机制和依赖注入来解耦。

```
┌──────────────────────────────────────────────────────────────────┐
│                        UI 层                                     │
│  HotspotTrackingWidget                                           │
│  ┌──────────┐  ┌──────────────┐  ┌───────────────────────────┐  │
│  │ Header   │  │ CategoryBar  │  │ NewsCard Grid / Detail    │  │
│  └──────────┘  └──────────────┘  └───────────────────────────┘  │
└───────────────────────┬──────────────────────────────────────────┘
                        │ setHotspotService() + signal/slot
┌───────────────────────▼──────────────────────────────────────────┐
│                      服务层                                      │
│  HotspotService                                                  │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────────────┐ │
│  │ Cache Mgmt  │  │ Category     │  │ AI Teaching Content     │ │
│  │ (fullFeed + │  │ Switching    │  │ Generation (DifyService)│ │
│  │  display)   │  │ Logic        │  │                         │ │
│  └─────────────┘  └──────────────┘  └─────────────────────────┘ │
└───────────────────────┬──────────────────────────────────────────┘
                        │ setNewsProvider() (依赖注入)
┌───────────────────────▼──────────────────────────────────────────┐
│                     数据源层                                     │
│  INewsProvider (抽象接口)                                        │
│  ┌──────────────────┐   ┌──────────────────────────────────────┐ │
│  │ MockNewsProvider │   │ RealNewsProvider                     │ │
│  │ 本地硬编码数据    │   │ 多源聚合 (天行/网易/BBC RSS/人民网)   │ │
│  └──────────────────┘   └──────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
```

Sources: [INewsProvider.h](src/hotspot/INewsProvider.h#L20-L79), [HotspotService.cpp](src/services/HotspotService.cpp#L25-L74), [HotspotTrackingWidget.cpp](src/ui/HotspotTrackingWidget.cpp#L289-L330)

## INewsProvider 接口契约

`INewsProvider` 是整个模块的核心抽象。它继承自 `QObject` 以支持 Qt 元对象系统（信号/槽、`Q_OBJECT` 宏），定义了四个纯虚方法和四个信号，构成了一套完整的"请求-响应"异步契约。

**接口方法**承担四种操作语义：

| 方法 | 语义 | 参数说明 |
|------|------|----------|
| `fetchHotNews(limit, category)` | 获取热点新闻列表 | `limit` 控制返回数量上限；`category` 为空字符串表示全部分类 |
| `searchNews(keyword)` | 关键词搜索 | 在标题、摘要、关键词字段中模糊匹配 |
| `fetchNewsDetail(newsId)` | 获取单条新闻详情 | 通过 `id` 精确查找 |
| `refresh()` | 刷新数据 | 语义上等价于重新调用 `fetchHotNews` |

**信号侧**定义了四个标准事件，所有 Provider 实现必须在适当的时机发射这些信号：`newsListReceived` 和 `newsDetailReceived` 承载正常数据返回；`errorOccurred` 传递错误信息；`loadingStarted` / `loadingFinished` 这一对信号用于 UI 层控制加载动画。这一设计确保了上层 `HotspotService` 对所有数据源拥有统一的监听方式，无需关心底层是本地 Mock 还是多路网络聚合。

Sources: [INewsProvider.h](src/hotspot/INewsProvider.h#L20-L79)

## NewsItem：贯穿三层的统一数据结构

`NewsItem` 是一个纯值类型结构体（非 QObject），承载一条新闻的所有业务字段。它在数据源层被构造，经过服务层的缓存与过滤，最终传递到 UI 层渲染为卡片。

```cpp
struct NewsItem {
    QString id;              // 唯一标识（UUID）
    QString title;           // 标题
    QString summary;         // 摘要
    QString content;         // 完整内容
    QString source;          // 来源（如"新华社"、"BBC中文-国际"）
    QString category;        // 分类（国内、国际、党建等）
    QString imageUrl;        // 封面图片 URL
    QString url;             // 新闻原文链接
    QDateTime publishTime;   // 发布时间
    int hotScore;            // 热度评分 (0-100)
    QStringList keywords;    // 关键词标签
    
    bool isValid() const;    // 快速校验：id 和 title 非空
};
```

值得注意的设计细节：`hotScore` 在 Mock 提供者中被赋予硬编码值（85-98），而在真实数据源中默认设为 `0`。这反映了工程决策——真实 API 的热度数据不可靠或不可获取，因此 UI 层不再依赖热度排序，转而按发布时间排列。

Sources: [NewsItem.h](src/hotspot/NewsItem.h#L1-L33)

## MockNewsProvider：开发阶段的本地沙箱

`MockNewsProvider` 是 `INewsProvider` 的最简实现，其核心设计目标是**在无网络环境下提供可预测的、即时的测试数据**，让 UI 开发和业务逻辑调试不依赖任何外部服务。

**数据生成策略**：构造函数中一次性调用 `generateMockData()` 生成 6 条预置新闻（3 条国内、3 条国外），涵盖政治局会议、立法工作、科技创新、外交动态、一带一路、金砖国家等典型思政教学题材。每条新闻的 `id` 通过 `QUuid::createUuid()` 生成，确保唯一性。

**延迟模拟**：这是 Mock 提供者最精巧的设计。每个方法都使用 `QTimer::singleShot` 引入人工延迟——`fetchHotNews` 延迟 300ms、`searchNews` 延迟 200ms、`fetchNewsDetail` 延迟 100ms。这个设计使得 Mock 的行为更接近真实网络请求，让开发者在测试阶段就能观察到加载动画和状态切换的完整流程。

**筛选与搜索逻辑**：`fetchHotNews` 通过简单的 `category` 字段精确匹配实现分类筛选；`searchNews` 在 `title`、`summary`、`keywords` 三个字段中执行大小写不敏感的包含匹配。两者均无分页机制，直接在内存中全量遍历。

Sources: [MockNewsProvider.h](src/hotspot/MockNewsProvider.h#L1-L30), [MockNewsProvider.cpp](src/hotspot/MockNewsProvider.cpp#L1-L181)

## RealNewsProvider：多源聚合、智能路由与降级机制

`RealNewsProvider` 是面向生产环境的实现，其复杂度远超 Mock 版本。它同时对接**四种异构数据源**，根据用户选择的分类智能路由到最优数据源组合，并内置了完整的降级链条。

### 四种数据源及其特性

| 数据源 | 类型 | 是否需要 API Key | 图片支持 | 稳定性 | 定位 |
|--------|------|-----------------|---------|--------|------|
| 天行数据 API | REST JSON | 是（`TIANXING_API_KEY`） | 部分匹配 | 中等 | 主力数据源（国内各领域） |
| 网易新闻频道 | JSONP（GBK 编码） | 否 | 有 | 中等 | 补充数据源（带缩略图） |
| BBC 中文 RSS | XML RSS 2.0 | 否 | 有（`media:content`） | 高 | 国际新闻主力 |
| 人民网分频道 RSS | XML RSS | 否 | 部分（`enclosure`） | 高 | 权威兜底保障 |

Sources: [RealNewsProvider.h](src/hotspot/RealNewsProvider.h#L18-L99), [RealNewsProvider.cpp](src/hotspot/RealNewsProvider.cpp#L130-L189)

### 分类到数据源的路由策略

`fetchHotNews` 方法中实现了基于分类的智能路由。每种分类对应一组 `FetchPlan`（数据获取计划），支持并行请求多个源然后聚合结果。以下表格展示了各分类的源组合策略：

| 分类 | 数据源组合 | 策略说明 |
|------|-----------|---------|
| 全部 | 天行 `guonei` + BBC RSS + 人民网时政 RSS | 三源聚合，覆盖国内+国际 |
| 国内 | 天行 `guonei` + 人民网时政 RSS | 双源互补 |
| 国际 | BBC RSS（优先） | 专用源 |
| 党建 | 天行 `guonei` + 网易国内 + 人民网时政 | 三源聚合 + 关键词过滤 |
| 经济 | 天行 `caijing` + 网易财经 + 人民网财经 | 三源聚合 |
| 外交 | 天行 `world` + 网易国际 + BBC RSS + 人民网国际 | 四源聚合 |
| 教育 | 人民网教育 RSS + 天行 `guonei` + 网易国内 | 教育专用 RSS 为主力 |
| 科技 | 天行 `keji` + 网易科技 | 双源互补 |
| 军事 | 人民网军事 RSS + 网易军事 + 天行 `junshi` | 军事专用 RSS 为主力 |

当 `TIANXING_API_KEY` 为空时，系统会自动移除所有天行 API 请求计划；如果所有计划都被移除，则降级到纯 RSS 模式。

Sources: [RealNewsProvider.cpp](src/hotspot/RealNewsProvider.cpp#L661-L763)

### 聚合、去重与排序

`finalizeNewsAggregation()` 是多源请求完成后的统一处理入口。它执行三个关键步骤：

1. **按发布时间降序排序**：确保最新新闻排在最前
2. **三级去重**：基于完整 URL → 完整标题 → 标题前 30 字符前缀，逐级检测重复。前缀匹配（15 字符最低门槛）能有效捕获同一新闻在不同来源中的微小标题差异
3. **分类再过滤**：调用 `NewsCategoryUtils::filterNewsByCategory` 对聚合结果进行最终筛选

**请求版本控制**：`m_activeRequestId` 是一个递增的 `quint64` 计数器。每次新的 `fetchHotNews` 调用都会递增此 ID，所有异步回调首先检查 `requestId != m_activeRequestId`，如果当前请求已过期则直接 `return`。这优雅地解决了"快速切换分类时旧请求的响应覆盖新请求结果"的经典竞态问题。

Sources: [RealNewsProvider.cpp](src/hotspot/RealNewsProvider.cpp#L1337-L1419)

### 思政新闻筛选：双重过滤引擎

`RealNewsProvider` 内嵌了一套**时政相关性过滤引擎**，其目的是从通用新闻流中筛选出适合思政课堂的教学素材。过滤分为三步：

1. **排除阶段**：遍历约 80 个排除关键词（覆盖事故灾难、犯罪、娱乐八卦、博彩赌博、生活娱乐、社会琐事、奇闻异事、医疗健康、消费维权、家庭矛盾、情感故事、标题党等类别），命中任一关键词即排除
2. **时政匹配**：检查是否包含约 70 个时政关键词（领导人姓名、政府机构、会议活动、政策理论、外交国防等）
3. **来源加分**：即使未命中时政关键词，如果来源属于约 26 家官方权威媒体（人民日报、新华社、央视等），同样保留

最终保留条件为 `(时政关键词匹配 ∥ 官方媒体来源) ∧ ¬排除关键词命中`。

Sources: [RealNewsProvider.cpp](src/hotspot/RealNewsProvider.cpp#L236-L371)

## HotspotService：业务编排与依赖注入枢纽

`HotspotService` 是连接 UI 层和数据源层的业务中间件。它的核心设计模式是**通过 `setNewsProvider()` 实现策略注入**，在运行时动态切换新闻数据来源。

### 依赖注入与生命周期管理

构造函数默认创建 `MockNewsProvider` 作为初始提供者（`m_ownsProvider = true`），确保系统在无配置状态下也能正常展示。当 `setNewsProvider()` 被调用时，执行以下清理序列：

1. 断开旧 Provider 的所有信号连接
2. 如果拥有旧 Provider 的所有权且其 parent 为自身，则 `delete` 旧 Provider
3. 连接新 Provider 的四个标准信号到对应的私有槽
4. 清空所有缓存和状态标记

在 `ModernMainWindow` 的初始化代码中，实际注入的是 `RealNewsProvider`，并从环境变量或内嵌密钥中配置天行 API Key：

```cpp
m_hotspotService = new HotspotService(this);
RealNewsProvider *newsProvider = new RealNewsProvider(this);
// API Key 优先级：环境变量 > 内嵌Key
m_hotspotService->setNewsProvider(newsProvider);
```

Sources: [HotspotService.cpp](src/services/HotspotService.cpp#L25-L74), [modernmainwindow.cpp](src/dashboard/modernmainwindow.cpp#L1087-L1100)

### 分类切换的智能缓存策略

`setActiveCategory()` 方法实现了一套分层缓存策略，避免每次分类切换都触发网络请求：

- **远程分类**（国内、国际、全部）：直接请求网络，因为这些分类的数据无法从其他分类的缓存中推导
- **主题分类**（党建、经济、教育、科技、军事、外交）：首先尝试从 `m_fullNewsCache`（全量缓存）中进行本地关键词过滤，仅当本地结果不足 8 条时才发起专用数据源请求

`m_fullNewsCache` 只在请求"全部"分类时填充，作为后续主题分类本地筛选的数据基础。

Sources: [HotspotService.cpp](src/services/HotspotService.cpp#L98-L137), [HotspotService.cpp](src/services/HotspotService.cpp#L207-L237)

### AI 教学内容生成

`generateTeachingContent()` 是该模块与 AI 能力的集成点。它接收一条 `NewsItem` 和 `DifyService` 实例，构造一段结构化 prompt（包含案例背景、思政价值、讨论话题、延伸思考四个维度），通过 `DifyService::sendMessage()` 发送到 Dify AI 平台，并通过一次性信号连接将 AI 响应转发为 `teachingContentGenerated` 信号。

值得注意的是连接管理使用了 `std::shared_ptr<QMetaObject::Connection>` 的惯用写法——在首次收到响应后自动断开连接，避免后续无关的 `messageReceived` 信号触发重复处理。

Sources: [HotspotService.cpp](src/services/HotspotService.cpp#L161-L200)

## NewsCategoryUtils：分类规范化与关键词过滤工具

`NewsCategoryUtils` 命名空间提供了三个层次的工具函数，服务于服务层和数据源层：

**分类规范化**：`normalizeCategory()` 将"全部"映射为空字符串、"国外"映射为"国际"，确保内部逻辑使用统一的分类标识。

**全量分类定义**：`allCategories()` 返回 9 个分类标签 `{"全部", "国内", "国际", "党建", "经济", "外交", "教育", "科技", "军事"}`，UI 层的分类按钮顺序与此严格对应。

**分类过滤引擎**：`filterNewsByCategory()` 实现了双层过滤逻辑。对于"国内"和"国际"，直接按 `category` 字段精确匹配；对于其他主题分类，采用"专用来源直接放行 + 关键词匹配"的双重策略。其中**专用来源映射表**记录了各分类对应的 RSS 源名称（如"人民网-军事"、"中国教育报"），来自这些源的新闻无需关键词校验即可通过。关键词匹配函数 `matchesKeywordCategory()` 在新闻标题、摘要、内容和关键词的拼接文本中搜索对应分类的关键词列表。

Sources: [NewsCategoryUtils.h](src/hotspot/NewsCategoryUtils.h#L1-L20), [NewsCategoryUtils.cpp](src/hotspot/NewsCategoryUtils.cpp#L1-L195)

## UI 层：HotspotTrackingWidget 的数据驱动渲染

`HotspotTrackingWidget` 是一个纯展示型组件，它不持有任何数据源实例，而是通过 `setHotspotService()` 获取业务层的引用，所有数据请求通过 `HotspotService` 间接发起。

### 界面结构

界面由四个区域纵向排列：**Header**（标题、搜索框、刷新按钮）→ **CategoryFilter**（9 个胶囊形分类按钮 + 数据来源标识）→ **NewsGrid**（单列列表布局的新闻卡片）→ **状态提示**（加载动画 / 空状态提示）。

### 图片加载与缓存

`HotspotTrackingWidget` 内建了一个独立的图片加载子系统：`QNetworkAccessManager` 负责异步下载，`QCache<QString, QPixmap>`（最大 50 张）提供内存缓存。`loadImage()` 方法在发起请求前先查询缓存；`onImageDownloaded()` 在下载成功后将 `QPixmap` 存入缓存（同时以原始 URL 和最终 URL 作为 key，处理重定向导致的 URL 变化）。`clearNewsGrid()` 在刷新列表时先 `abort()` 所有待处理的图片请求，再清空 `m_pendingImages` 映射，有效防止了图片错位和悬空指针崩溃。

### 新闻详情与 AI 案例生成

点击新闻卡片触发 `showNewsDetail()`，弹出一个模态对话框展示完整新闻内容（含大图、分类标签、标题、元信息、正文）。每张卡片和头条卡片上的"生成案例"按钮触发 `onGenerateTeachingClicked()`，该方法将按钮置为禁用状态并发出 `teachingContentRequested` 信号。`ModernMainWindow` 监听此信号后执行：切换到 AI 对话页面 → 更新侧边栏高亮 → 构建教学案例 prompt → 在聊天界面添加用户消息 → 调用 `DifyService` 发送请求。

Sources: [HotspotTrackingWidget.cpp](src/ui/HotspotTrackingWidget.cpp#L1-L200), [HotspotTrackingWidget.cpp](src/ui/HotspotTrackingWidget.cpp#L1053-L1173), [modernmainwindow.cpp](src/dashboard/modernmainwindow.cpp#L1120-L1160)

## Provider 切换实战：从 Mock 到 Real

理解了各层职责后，实际的 Provider 切换操作非常简洁。以下展示两种典型场景：

**场景一：生产环境使用真实数据源**（即当前代码的实际初始化逻辑）：

```cpp
// 在 ModernMainWindow 初始化中
m_hotspotService = new HotspotService(this);               // 默认注入 MockNewsProvider
RealNewsProvider *newsProvider = new RealNewsProvider(this);
// 配置 API Key（环境变量优先，内嵌 Key 备用）
newsProvider->setTianXingApiKey(tianxingKey);
m_hotspotService->setNewsProvider(newsProvider);            // 运行时替换为 RealNewsProvider
```

**场景二：开发调试回退 Mock**：

```cpp
// 只需注释掉 RealNewsProvider 的创建和注入
// HotspotService 构造函数中的 MockNewsProvider 就会保持生效
HotspotService *service = new HotspotService(this);
// 不调用 setNewsProvider()，MockNewsProvider 持续工作
```

这一设计使得开发者可以在完全离线的环境中进行 UI 调试和业务逻辑验证，而生产部署时只需一行 `setNewsProvider()` 调用即可切换到真实数据。

Sources: [HotspotService.cpp](src/services/HotspotService.cpp#L25-L34), [modernmainwindow.cpp](src/dashboard/modernmainwindow.cpp#L1087-L1104)

## 关键设计决策总结

| 设计决策 | 选择 | 理由 |
|---------|------|------|
| 数据源抽象方式 | 策略模式（纯虚接口 + 依赖注入） | 新增数据源只需实现接口，无需修改服务层和 UI 层 |
| Mock 延迟模拟 | `QTimer::singleShot` 300ms | 使加载状态在开发阶段可见，提前发现 UI 竞态 |
| 请求版本控制 | 递增 `requestId` + 回调校验 | 防止快速切换分类时旧响应覆盖新数据 |
| 多源聚合方式 | 并行请求 + 计数器归零触发 | 最大化数据丰富度，同时保持响应速度 |
| 去重策略 | URL → 全标题 → 标题前缀三级 | 平衡去重精度和性能 |
| 分类缓存 | 全量缓存 + 本地关键词过滤 | 减少主题分类切换时的网络请求 |
| 思政筛选 | 排除词表 + 时政词表 + 官方来源白名单 | 三重保障确保内容适合课堂教学 |
| 图片管理 | 独立 NAM + QCache + 请求中断 | 避免图片错位和内存泄漏 |

Sources: [INewsProvider.h](src/hotspot/INewsProvider.h#L1-L82), [RealNewsProvider.h](src/hotspot/RealNewsProvider.h#L1-L99), [HotspotService.h](src/services/HotspotService.h#L1-L126)

## 延伸阅读

- 理解该模块在整体分层中的位置：[分层架构总览：UI 层 → 服务层 → 网络与工具层](5-fen-ceng-jia-gou-zong-lan-ui-ceng-fu-wu-ceng-wang-luo-yu-gong-ju-ceng)
- AI 教学内容生成的底层通信机制：[DifyService：SSE 流式对话、多事件类型处理与会话管理](10-difyservice-sse-liu-shi-dui-hua-duo-shi-jian-lei-xing-chu-li-yu-hui-hua-guan-li)
- 统一网络请求创建的底层约定：[NetworkRequestFactory：统一请求创建、SSL 策略与 HTTP/2 禁用约定](23-networkrequestfactory-tong-qing-qiu-chuang-jian-ssl-ce-lue-yu-http-2-jin-yong-yue-ding)
- API Key 的配置加载链路：[统一配置加载机制 AppConfig：环境变量 → 随包配置 → 开发配置](7-tong-pei-zhi-jia-zai-ji-zhi-appconfig-huan-jing-bian-liang-sui-bao-pei-zhi-kai-fa-pei-zhi)