# AI 思政智慧课堂系统

基于 Qt 6 / C++17 的桌面端教学辅助系统，面向思政课堂场景，集成 AI 对话、AI 备课、试题库管理、通知、考勤、数据分析与发布打包能力。

## 项目亮点

- 桌面端 Qt Widgets 应用，适合校内教学环境部署
- 集成 Dify AI 对话与工作流能力，支持流式响应
- 使用 Supabase 提供认证、数据存储与部分业务后端能力
- 内置试题库、智能组卷、AI 解析、教案编辑、热点追踪等教学功能
- 支持 macOS / Windows 双平台打包发布
- 已内建发布流程，可通过 Git tag 自动触发 GitHub Actions 生成安装包

## 功能模块

### 1. 用户认证

- 邮箱登录 / 注册 / 密码重置
- Supabase 认证接入
- 记住我
- 登录后进入主工作台

认证链路：

```text
SimpleLoginWindow → SupabaseClient → ModernMainWindow
                  ↘ SignupWindow ↗
```

### 2. AI 教学能力

- AI 对话助手
- AI 备课页面
- 文档解析生成试题
- PPT 生成与模板资源支持
- Markdown 渲染展示

### 3. 教学业务能力

- 试题库管理
- 智能组卷
- 通知中心
- 考勤管理
- 热点追踪
- 学情与数据分析

## 技术栈

- **框架**: Qt 6（Widgets、Network、Charts、QuickWidgets、Svg）
- **语言**: C++17
- **构建系统**: CMake 3.16+
- **AI 服务**: Dify API
- **认证 / 数据**: Supabase
- **图表**: Qt Charts
- **发布**: GitHub Actions + macOS DMG / Windows ZIP + EXE

## 运行环境

### 本地开发依赖

- Qt 6.6+
- CMake 3.16+
- C++17 编译器
- macOS 或 Windows

### 必要环境变量

可参考 `.env.example`：

```bash
DIFY_API_KEY=app-your-api-key-here
PARSER_API_KEY=app-your-parser-key-here
SUPABASE_URL=https://your-project-id.supabase.co
SUPABASE_ANON_KEY=your-supabase-anon-key
```

可选变量：

```bash
SUPABASE_SERVICE_KEY=your-service-role-key
ALLOW_INSECURE_SSL=1
http_proxy=http://127.0.0.1:7897
https_proxy=http://127.0.0.1:7897
```

说明：

- `ALLOW_INSECURE_SSL=1` 仅建议开发调试使用
- 如果配置本地代理，应用启动时会读取代理环境变量
- 当前版本会在检测到本地代理不可用时自动跳过代理配置，避免登录请求直接失败

## 快速开始

### macOS / Linux 风格命令

```bash
mkdir -p build
cmake -S . -B build
cmake --build build -j$(sysctl -n hw.ncpu)
```

### 运行应用

如果你有本地启动脚本：

```bash
./run_app.sh
```

或直接启动构建产物：

```bash
open build/AILoginSystem.app
```

## 项目结构

```text
src/
├── main/                        # 程序入口
├── auth/                        # 登录 / 注册 / Supabase 认证
│   ├── login/
│   ├── signup/
│   └── supabase/
├── dashboard/                   # 主界面与工作台
├── questionbank/                # 试题库与组卷相关界面
├── services/                    # 业务服务层
├── ui/                          # 通用 UI 组件
├── utils/                       # 工具类（网络请求工厂、重试、Markdown 等）
├── notifications/               # 通知模块
├── attendance/                  # 考勤模块
├── analytics/                   # 数据分析模块
├── smartpaper/                  # 智能组卷模块
├── hotspot/                     # 热点数据提供者
└── config/                      # 内嵌配置与密钥头文件

resources/
├── styles/                      # 样式资源
├── templates/                   # 模板资源
├── ppt/                         # PPT 示例资源
└── QtTheme/                     # Qt 主题资源

scripts/
├── package_app.sh               # macOS 打包脚本
├── package_windows.ps1          # Windows 打包脚本
└── windows-installer.iss        # Inno Setup 安装包脚本
```

## 网络与请求约定

项目中的网络请求主要通过 `src/utils/NetworkRequestFactory.*` 统一创建。

当前约定：

- 默认禁用 HTTP/2，避免特定平台协议兼容问题
- 支持统一超时配置
- 支持开发环境下通过 `ALLOW_INSECURE_SSL` 放宽 SSL 校验
- Supabase 与 Dify 请求分别走统一工厂方法
- 登录链路对代理、SSL、DNS、超时等错误进行了更明确的映射

## 构建与发布

### 本地打包

#### macOS

```bash
./scripts/package_app.sh --version 2.1.7 --build-dir build_release --output-dir dist --arch-label arm64
```

如果需要将发布密钥写入 `src/config/embedded_keys.h`：

```bash
./scripts/package_app.sh --version 2.1.7 --build-dir build_release --output-dir dist --arch-label arm64 --embed-release-keys
```

#### Windows

```powershell
./scripts/package_windows.ps1 -Version 2.1.7 -BuildDir build -OutputDir build
```

如果需要内嵌发布密钥：

```powershell
./scripts/package_windows.ps1 -Version 2.1.7 -BuildDir build -OutputDir build -EmbedReleaseKeys
```

### GitHub Actions 自动发布

项目已配置两个发布工作流：

- `.github/workflows/build-macos.yml`
- `.github/workflows/build-windows.yml`

当推送 tag（如 `v2.1.7`）时，会自动：

1. 解析版本号
2. 构建对应平台产物
3. 上传 artifact
4. 创建 GitHub Release 并附带安装包

推荐发布流程：

```bash
git commit -m "chore: release v2.1.7"
git tag -a v2.1.7 -m "Release v2.1.7"
git push origin main
git push origin v2.1.7
```

## 常见问题

### 1. 登录显示“网络错误”

优先检查：

- `SUPABASE_URL` / `SUPABASE_ANON_KEY` 是否正确
- 本地代理是否可用
- `*.supabase.co` 是否可访问
- 系统时间是否正确
- 是否存在 SSL 证书拦截

### 2. Dify 无法响应

检查：

- `DIFY_API_KEY` 是否配置
- Dify 服务端点是否可访问
- 当前网络是否允许流式连接

### 3. macOS 打包失败

检查：

- `macdeployqt` 是否可用
- `QT_PATH` / `QT_ROOT_DIR` 是否正确
- Qt 版本是否与项目依赖匹配

### 4. Windows 打包失败

检查：

- `windeployqt` 是否在 PATH 中
- Inno Setup (`iscc`) 是否已安装
- Qt MSVC 套件是否完整

## 代码规范

- 类名使用 PascalCase
- 成员变量使用 `m_` 前缀
- 信号 / 槽使用 camelCase
- 注释与日志以中文为主
- 尽量复用现有服务模式与网络请求工厂

## 版本信息

当前版本：`2.1.7`

## License

如需开源发布，建议在此补充正式许可证信息。
