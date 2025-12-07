# AI 思政教学系统 - 项目结构

## 📁 核心源码目录 (`src/`)

### 🚪 入口 (`src/main/`)
| 文件 | 说明 |
|------|------|
| `main.cpp` | 程序入口，启动登录窗口 |

---

### 🔐 认证模块 (`src/auth/`)

#### `auth/login/` - 登录
| 文件 | 说明 |
|------|------|
| `simpleloginwindow.h/.cpp` | 登录窗口 UI 和逻辑 |

#### `auth/signup/` - 注册
| 文件 | 说明 |
|------|------|
| `signupwindow.h/.cpp` | 注册窗口 UI 和逻辑 |

#### `auth/supabase/` - 后端认证
| 文件 | 说明 |
|------|------|
| `supabaseclient.h/.cpp` | Supabase 认证客户端 |
| `supabaseconfig.h/.cpp` | Supabase 配置（URL、Key） |

---

### 🏠 主界面 (`src/dashboard/`)
| 文件 | 说明 |
|------|------|
| `modernmainwindow.h/.cpp` | **主窗口**，包含侧边栏、仪表板、AI 对话 |

---

### 📚 试题库 (`src/questionbank/`)
| 文件 | 说明 |
|------|------|
| `questionbankwindow.h/.cpp` | 试题库页面 UI |
| `QuestionRepository.h/.cpp` | 试题数据管理（增删改查） |

---

### ⚙️ 服务层 (`src/services/`)
| 文件 | 说明 |
|------|------|
| `DifyService.h/.cpp` | **Dify AI 对话服务**（HTTP API 调用） |
| `ExportService.h/.cpp` | 试卷导出服务（HTML/PDF） |

---

### 🎨 UI 组件 (`src/ui/`)
| 文件 | 说明 |
|------|------|
| `aipreparationwidget.h/.cpp` | AI 备课页面 |
| `moderncheckbox.h/.cpp` | 自定义复选框组件 |

---

## 📂 资源目录 (`resources/`)

| 目录 | 说明 |
|------|------|
| `styles/` | QSS 样式表 |
| `QtTheme/` | Qt 主题资源 |
| `images/` | 图片资源 |
| `qml/` | QML 组件（图表等） |

---

## 🛠️ 构建配置

| 文件 | 说明 |
|------|------|
| `CMakeLists.txt` | CMake 构建配置 |
| `resources.qrc` | Qt 资源索引 |

---

## 🔗 模块依赖关系

```
main.cpp
    └── SimpleLoginWindow (登录)
            └── SignupWindow (注册)
            └── SupabaseClient (认证)
            └── ModernMainWindow (主界面)
                    ├── DifyService (AI 对话)
                    ├── QuestionBankWindow (试题库)
                    ├── AIPreparationWidget (AI 备课)
                    └── ExportService (导出)
```
