# AI 思政智慧课堂系统

## 项目概述

基于 Qt6/C++ 的 AI 思政教学辅助系统，集成 Dify AI 对话、Supabase 后端认证、试题库管理等功能。

## 技术栈

- **框架**: Qt 6 (Widgets, Network, Charts, QuickWidgets)
- **语言**: C++17
- **构建**: CMake 3.16+
- **后端认证**: Supabase (PostgreSQL)
- **AI 服务**: Dify API (流式响应)
- **PPT 生成**: 讯飞 PPT API

## 快速开始

### 环境变量

```bash
export DIFY_API_KEY="your-dify-api-key"
```

### 构建

```bash
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### 运行

```bash
./run_app.sh
# 或
open build/AILoginSystem.app
```

## 目录结构

```
src/
├── main/                 # 程序入口
├── auth/                 # 认证模块
│   ├── login/           # 登录窗口
│   ├── signup/          # 注册窗口
│   └── supabase/        # Supabase 客户端
├── dashboard/           # 主界面 (ModernMainWindow)
├── questionbank/        # 试题库管理
├── services/            # 服务层
│   ├── DifyService      # Dify AI 对话 (SSE 流式)
│   ├── PaperService     # 试卷服务
│   ├── QuestionParserService  # AI 试题解析
│   ├── BulkImportService     # 批量导入
│   ├── ExportService    # 导出服务
│   ├── PPTXGenerator    # PPTX 生成
│   └── XunfeiPPTService # 讯飞 PPT API
├── ui/                  # UI 组件
│   ├── AIChatDialog     # AI 对话窗口
│   ├── ChatWidget       # 聊天组件
│   └── AIPreparationWidget  # AI 备课页面
└── utils/               # 工具类
    └── MarkdownRenderer # Markdown 渲染

resources/               # 资源文件
├── styles/             # QSS 样式表
├── QtTheme/            # Qt 主题
└── qml/                # QML 组件
```

## 核心模块

### 认证流程

```
SimpleLoginWindow → SupabaseClient → ModernMainWindow
                  ↘ SignupWindow ↗
```

### AI 服务

- **DifyService**: 使用 `/chat-messages` 端点，SSE 流式响应
- **QuestionParserService**: 使用 `/workflows/run` 端点，解析文档生成试题

### 网络配置

- 所有 HTTPS 请求禁用 HTTP/2 (`Http2AllowedAttribute = false`)
- SSL 验证模式: `VerifyNone` (开发环境)
- 超时: 对话 120s，工作流 300s

## 配置文件

| 文件 | 用途 |
|------|------|
| `.env` | API Key 配置 (不提交 Git) |
| `run_app.sh` | 本地启动脚本 |
| `src/auth/supabase/supabaseconfig.cpp` | Supabase URL/Key |

## 常见问题

### 网络错误：2

`RemoteHostClosedError` - 检查代理设置，确保 `*.supabase.co` 和 `api.dify.ai` 可访问。

### API Key 未设置

确保环境变量 `DIFY_API_KEY` 已设置后再启动应用。

## 代码规范

- 类名: PascalCase (如 `DifyService`)
- 成员变量: `m_` 前缀 (如 `m_networkManager`)
- 信号/槽: camelCase (如 `messageReceived`, `onReplyFinished`)
- 中文注释和日志
