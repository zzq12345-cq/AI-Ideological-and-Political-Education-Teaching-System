试卷导出是智能组卷流水线的**最终交付环节**。当用户通过 AI 出题或手动组卷完成题目编排后，系统需要将这些结构化的试题数据转化为可分发、可打印的文档。本文深入剖析 `ExportService` 及其协作组件 `DocxGenerator`、`SimpleZipWriter` 的设计架构，揭示三格式导出的实现原理与数据流向。

## 导出管道总体架构

系统的试卷导出并非单一类的职责，而是由三个层次明确的组件协作完成。`ExportService` 作为**对外门面**，统一封装 HTML、DOCX、PDF 三种格式的导出入口；`DocxGenerator` 是 DOCX 格式的**专业生成器**，独立负责 Office Open XML 的底层构建；`SimpleZipWriter` 则是 DOCX/PPTX 共用的**底层打包引擎**，将 XML 文件集合压缩为标准 ZIP 容器。

```
┌─────────────────────────────────────────────────────────┐
│                     UI 调用层                             │
│  PaperComposerDialog   QuestionBankWindow                │
│  (试题篮导出)           (AI出题 Markdown 导出)             │
└───────┬──────────────────────┬───────────────────────────┘
        │                      │
        │ PaperQuestion 列表    │ Markdown 原文
        ▼                      ▼
┌──────────────────┐   ┌──────────────────┐
│  ExportService   │   │  DocxGenerator   │  ← 直接调用
│  (门面/协调器)    │   │  generatePaper() │
│                  │   │  generateFrom    │
│  exportToHtml()  │   │  Markdown()      │
│  exportToDocx()──┼──►│                  │
│  exportToPdf()   │   └────────┬─────────┘
└──────────────────┘            │
                                ▼
                       ┌──────────────────┐
                       │ SimpleZipWriter  │
                       │ (ZIP/DEFLATE)    │
                       └──────────────────┘
```

上图展示了双入口设计：`ExportService` 接收结构化 `PaperQuestion` 列表，适用于试题篮导出等场景；`DocxGenerator` 可被 UI 层直接调用，接收 AI 输出的 Markdown 原文直接转换为 DOCX，跳过结构化解析步骤。

Sources: [ExportService.h](src/services/ExportService.h#L14-L40), [DocxGenerator.h](src/services/DocxGenerator.h#L15-L72)

## ExportService：三格式统一门面

`ExportService` 继承自 `QObject`，通过三个 `Q_INVOKABLE` 方法提供格式无关的导出接口。所有方法共享相同的参数签名：`(filePath, paperTitle, questions)`，调用方无需关心底层格式差异。

### 接口总览

| 方法 | 输出格式 | 核心依赖 | 实现策略 |
|------|---------|---------|---------|
| `exportToHtml()` | `.html` | Qt 文件 I/O | 内联 CSS + 模板拼接 |
| `exportToDocx()` | `.docx` | `DocxGenerator` | 委托给 `m_docxGenerator->generatePaper()` |
| `exportToPdf()` | `.pdf` | `QPrinter` + `QTextDocument` | 复用 HTML 内容 + Qt 打印引擎 |

三个方法均遵循统一的防御性编程模式：首先检查 `questions` 是否为空，其次确保目标目录存在，最后执行核心逻辑。成功时发射 `exportSuccess(filePath)` 信号，失败则发射 `exportFailed(error)` 并返回 `false`。

Sources: [ExportService.h](src/services/ExportService.h#L18-L39), [ExportService.cpp](src/services/ExportService.cpp#L12-L29)

## HTML 导出：模板驱动的自包含文档

HTML 导出是整个管道中最**自主完整**的格式——生成的 HTML 文件内嵌全部 CSS 样式，无需任何外部依赖即可在浏览器中呈现排版精美的试卷。

### HTML 文档结构

`generateHtmlContent()` 方法通过 C++ 原始字符串字面量（`R"(...)"`）拼接完整的 HTML 文档。文档分为三个层次：

1. **头部区域**（`.header`）：红色主标题（`#D9001B`，32px）+ 题目总数，底部 3px 红色分隔线
2. **题目循环**（`.question`）：每道题一个卡片，含题号、题型徽章、难度徽章、题干、选项、答案与解析
3. **页脚**（`.footer`）：系统标识 + 生成时间戳

`generateQuestionHtml()` 负责单题渲染，其中题型和难度通过硬编码映射表转换为中文标签和对应颜色：

```
题型映射:  single_choice → 单选题(蓝)  multi_choice → 多选题(蓝)  ...
难度映射:  easy → 简单(绿)  medium → 中等(黄)  hard → 困难(红)
```

选项标签按 `A.`、`B.`、`C.`... 自动编号，答案和解析仅在对应字段非空时渲染，确保简答题等无选项题型不会出现空区域。

Sources: [ExportService.cpp](src/services/ExportService.cpp#L135-L372)

## PDF 导出：HTML 复用 + Qt 打印引擎

PDF 导出的关键设计决策是**不重新实现排版逻辑**，而是复用 HTML 生成管道的输出。`exportToPdf()` 调用相同的 `generateHtmlContent()`，但在写入文件前注入一段打印优化 CSS：

```css
body { padding: 0; }          /* 取消屏幕浏览的内边距 */
.question { break-inside: avoid; }  /* 避免题目跨页断裂 */
```

这段 CSS 通过字符串替换 `htmlContent.replace("</head>", printCss + "</head>")` 注入到 `<head>` 标签闭合前，实现对原始 HTML 的**无损增强**。

随后通过 Qt 的 `QTextDocument` 加载 HTML，配合 `QPrinter` 以高分辨率输出为 A4 尺寸 PDF：

- 页面尺寸：A4，四边各 15mm 边距
- 文档宽度匹配打印区域像素宽度，避免内容溢出
- `QPrinter::HighResolution` 模式确保文字清晰度

这种 **HTML → QTextDocument → QPrinter** 的链路意味着 PDF 的排版质量受限于 Qt 的 HTML 渲染引擎，对复杂 CSS（如 flex 布局）的支持有限，但对试卷场景的表格、文字排版足够使用。

Sources: [ExportService.cpp](src/services/ExportService.cpp#L72-L118)

## DOCX 导出：OOXML 原生构建管道

DOCX 是导出管道中最复杂的格式，因为它需要生成符合 **Office Open XML (OOXML)** 标准的真实 Word 文档，而非简单的 HTML 转换。`DocxGenerator` 是独立于 `ExportService` 的完整组件，提供两条生成路径。

### 双路径生成架构

| 路径 | 方法 | 输入数据 | 使用场景 |
|------|------|---------|---------|
| 结构化路径 | `generatePaper()` | `QList<PaperQuestion>` | 试题篮导出、PaperComposerDialog |
| Markdown 路径 | `generateFromMarkdown()` | `QString` Markdown 原文 | AI 出题直接导出 |

### DOCX = ZIP(XML) 的文件结构

DOCX 本质上是一个 ZIP 压缩包，内部包含多个 XML 文件。`DocxGenerator` 在临时目录中构建完整的 OOXML 目录树，再通过 `SimpleZipWriter` 打包：

```
output.docx (ZIP)
├── [Content_Types].xml      ← 内容类型声明
├── _rels/
│   └── .rels                ← 包级关系（指向 word/document.xml）
└── word/
    ├── document.xml         ← 主文档内容（题目、选项、答案）
    ├── styles.xml           ← 样式定义（Title/Heading1/Question/Option）
    ├── settings.xml         ← 文档设置（缩放/制表位/字符间距）
    └── _rels/
        └── document.xml.rels ← 文档级关系（引用样式和设置）
```

每个 XML 文件的生成都由一个独立的私有方法负责（`createContentTypes()`、`createRels()`、`createDocument()` 等），保证了**单一职责**和**可独立测试**。

Sources: [DocxGenerator.cpp](src/services/DocxGenerator.cpp#L165-L213), [DocxGenerator.cpp](src/services/DocxGenerator.cpp#L215-L378)

### 结构化路径：`generatePaper()`

接收 `QList<PaperQuestion>` 后，该方法按**题型分组**并按固定顺序输出题目。题型顺序为：选择题 → 多选题 → 判断题 → 填空题 → 简答题 → 论述题 → 材料分析题。

`createDocument()` 构建的 `document.xml` 包含完整的 A4 页面定义（`w:pgSz` 11906×16838 twips，即 210×297mm），`styles.xml` 定义了四种段落样式：

| 样式 ID | 用途 | 字号 | 特殊属性 |
|---------|------|------|---------|
| `Title` | 试卷标题 | 22pt (44 half-points) | 居中、段后 400 |
| `Heading1` | 题型标题 | 16pt | 段前 400、加粗 |
| `Question` | 题目正文 | 12pt | 段前 200 |
| `Option` | 选项 | 12pt | 左缩进 480 twips |

选项标签（A-H）通过 `generateOptionsXml()` 生成，且内置智能检测：如果选项文本已包含 `A.`、`A、`等前缀，不会重复添加。所有文本通过 `escapeXml()` 进行 XML 实体转义（`&`、`<`、`>`、`"`、`'`）。

Sources: [DocxGenerator.cpp](src/services/DocxGenerator.cpp#L380-L515)

### Markdown 路径：`generateFromMarkdown()`

这条路径是系统的**隐藏核心**——AI 出题功能产生的原始 Markdown 文本不经结构化解析，直接转换为 DOCX。`createDocumentFromMarkdown()` 实现了一个精巧的两阶段 Markdown 解析器：

**第一阶段：逐行扫描与块收集**

解析器按行遍历 Markdown 文本，维护一个状态机来识别题目块和答案块：

```
┌────────────────────────────────────────────────┐
│ 行类型识别                                       │
├────────────────────────────────────────────────┤
│ 跳过: <think/> 块、空行、---分割线、斜体注释      │
│ 检测: 全局答案区标题(参考答案与解析)               │
│ 识别: 题型标题(# / 一、/ 选择题)                  │
│ 识别: 题目起始行(1. / （1）)                      │
│ 识别: 内联答案(【答案】/ 【解析】)                 │
│ 识别: 全局答案区中的编号答案(1.【答案】)           │
└────────────────────────────────────────────────┘
```

核心数据结构是 `MarkdownQuestionBlock`，每个块记录 section 标题、题号、答案行列表和解析行列表。`answerState` 枚举（`None`/`Answer`/`Analysis`）追踪当前正在收集的内容类型，支持多行连续答案的 `appendContinuation()` 机制。

**第二阶段：OOXML 段落生成**

`markdownLineToXml()` 方法是行级转换器，按优先级匹配不同行类型：

| 行类型 | 正则匹配 | OOXML 输出 |
|-------|---------|-----------|
| Markdown 标题 `# ...` | `^(#{1,4})\s+(.+)` | `Heading1` 样式段落 |
| 中文题型标题 `一、选择题` | 中文序号/题型名正则 | `Heading1` 样式段落 |
| 答案行 `【答案】` | `^[\*]*【答案】` | 绿色加粗段落 (`#2E7D32`) |
| 解析行 `【解析】` | `^[\*]*【解[析释]】` | 灰色段落 (`#666666`) |
| 选项行 `A. ...` | `^([A-Ha-h])[.、)）]` | `Option` 样式缩进段落 |
| 编号题目 `1. ...` | `^(\d+)[.、)）]` | `Question` 样式加粗编号 |
| 加粗行 `**text**` | `^\*{2}(.+)\*{2}$` | 加粗段落 |
| 普通文本 | 兜底 | 默认段落（去除 `*` 标记） |

当存在任何答案/解析内容时，文档会在题目正文与答案区之间插入**分页符**（`<w:br w:type="page"/>`），答案区按题号逐一渲染。

Sources: [DocxGenerator.cpp](src/services/DocxGenerator.cpp#L517-L847), [DocxGenerator.cpp](src/services/DocxGenerator.cpp#L849-L972)

## SimpleZipWriter：跨平台 ZIP 打包引擎

`DocxGenerator` 生成的 XML 文件集合最终需要打包为标准 ZIP 格式。`SimpleZipWriter` 是一个**纯 Qt + zlib 实现**的轻量 ZIP 写入器，不依赖任何外部进程（如 `zip` 命令或 PowerShell），确保在 macOS 和 Windows 上的行为一致。

### ZIP 写入流程

```
遍历临时目录文件
    ↓
逐文件：读取原始数据 → CRC32 校验 → DEFLATE 压缩
    ↓  (压缩失败/膨胀时回退为 STORE)
写入 Local File Header + 压缩数据
    ↓
收集 Central Directory Entry
    ↓
写入 Central Directory + End of Central Directory Record
```

关键实现细节：

- **Raw DEFLATE**：使用 `deflateInit2()` 配合 `-MAX_WBITS` 参数，生成无 zlib/gzip 头的原始 DEFLATE 流，符合 ZIP 规范
- **智能压缩策略**：当 DEFLATE 后数据反而更大（如已压缩的小文件）时，自动回退为 `METHOD_STORE`
- **字节序安全**：通过 `qToLittleEndian()` 确保在大小端平台上的 ZIP 格式兼容性
- **DOS 时间戳**：将 `QDateTime` 转换为 ZIP 格式要求的 DOS 日期时间编码

Sources: [SimpleZipWriter.h](src/utils/SimpleZipWriter.h#L15-L34), [SimpleZipWriter.cpp](src/utils/SimpleZipWriter.cpp#L1-L237)

## UI 层调用模式

导出功能在 UI 层有两个主要调用入口，分别对应不同的使用场景和数据流：

### 场景一：PaperComposerDialog（试题篮导出）

`PaperComposerDialog` 的"导出试卷"按钮直接实例化一个局部 `DocxGenerator`，从全局 `QuestionBasket` 单例获取 `QList<PaperQuestion>`，调用 `generatePaper()` 生成 DOCX。这是**结构化数据 → DOCX** 的标准路径。

### 场景二：QuestionBankWindow（AI 出题导出）

`QuestionBankWindow` 持有成员变量 `m_docxGenerator`（在构造时初始化），当用户在 `AIQuestionGenWidget` 中点击"导出试卷"时，将 AI 生成的**原始 Markdown 文本**传递给 `onExportToDocx()`，该方法调用 `m_docxGenerator->generateFromMarkdown()` 完成转换。这条路径**跳过了 PaperQuestion 结构化解析**，直接从 Markdown 生成 DOCX，效率更高。

两个入口导出成功后均会弹出文件对话框询问是否立即打开，通过 `QDesktopServices::openUrl()` 调用系统默认程序。

Sources: [PaperComposerDialog.cpp](src/questionbank/PaperComposerDialog.cpp#L630-L680), [questionbankwindow.cpp](src/questionbank/questionbankwindow.cpp#L520-L585)

## 信号传播与错误处理

`ExportService` 在构造时连接了 `DocxGenerator` 的两个信号，建立了级联错误传播机制：

```
DocxGenerator::generationFinished(true, path)  → ExportService::exportSuccess(path)
DocxGenerator::errorOccurred(error)             → ExportService::exportFailed(error)
```

但需要注意：`ExportService::exportToDocx()` 返回的是 `m_docxGenerator->generatePaper()` 的布尔值，而 `DocxGenerator` 内部已经在失败时通过 `emit errorOccurred()` 和 `emit generationFinished(false, "")` 双重通知。这意味着在通过 `ExportService` 调用时，**信号和返回值两种机制并行传递结果**，UI 层只需监听信号即可，无需额外检查返回值。

Sources: [ExportService.cpp](src/services/ExportService.cpp#L12-L25), [ExportService.cpp](src/services/ExportService.cpp#L120-L132)

## 相关阅读

- [智能组卷引擎 SmartPaperService：贪心选题算法与换题机制](14-zhi-neng-zu-juan-yin-qing-smartpaperservice-tan-xin-xuan-ti-suan-fa-yu-huan-ti-ji-zhi) — 了解导出上游的组卷数据来源
- [PPTXGenerator：基于 XML + ZIP 的原生 PPTX 文件构建](17-pptxgenerator-ji-yu-xml-zip-de-yuan-sheng-pptx-wen-jian-gou-jian) — 同样基于 `SimpleZipWriter` 的 Office Open XML 生成器
- [试题库管理：题目录入、篮子、质量检查与批量导入](13-shi-ti-ku-guan-li-ti-mu-lu-ru-lan-zi-zhi-liang-jian-cha-yu-pi-liang-dao-ru) — 试题篮 `QuestionBasket` 的完整工作流