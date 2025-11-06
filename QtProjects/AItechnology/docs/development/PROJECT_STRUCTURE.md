# AI智慧课堂 - 项目结构

## 📂 当前项目结构（需要整理）

```
/Users/zhouzhiqi/QtProjects/AItechnology/
├── src/                                    # 源代码
├── Pages/                                  # 散落在根目录
├── build/                                  # 构建输出
├── AIPoliticsClassroom.pro                 # 项目文件
├── qml.qrc
├── resources.qrc
├── REMEMBER_ME_FEATURE.md                  # 文档散落
└── SIGNUP_WINDOW.md
```

## 🎯 建议的新项目结构

```
/Users/zhouzhiqi/QtProjects/AItechnology/
├── src/                                    # 源代码
│   ├── main/                               # 程序入口
│   │   └── main.cpp
│   ├── auth/                               # 身份认证模块
│   │   ├── login/
│   │   │   ├── loginwindow.h
│   │   │   └── loginwindow.cpp
│   │   ├── signup/
│   │   │   ├── signupwindow.h
│   │   │   └── signupwindow.cpp
│   │   └── supabase/
│   │       ├── supabaseclient.h
│   │       ├── supabaseclient.cpp
│   │       ├── supabaseconfig.h
│   │       └── supabaseconfig.cpp
│   ├── dashboard/                          # 主界面模块
│   │   ├── modernmainwindow.h
│   │   └── modernmainwindow.cpp
│   ├── questionbank/                       # 试题库模块
│   │   ├── QuestionRepository.h
│   │   └── QuestionRepository.cpp
│   ├── ai/                                 # AI功能模块
│   │   ├── aipreparation/
│   │   │   ├── aipreparationwidget.h
│   │   │   └── aipreparationwidget.cpp
│   │   └── engine/                         # AI引擎
│   │       ├── aiengine.h
│   │       └── aiengine.cpp
│   ├── services/                           # 服务层
│   │   ├── ExportService.h
│   │   └── ExportService.cpp
│   └── common/                             # 公共组件
│       ├── utils.h
│       ├── constants.h
│       └── enums.h
├── resources/                              # 资源文件
│   ├── images/
│   │   ├── icons/
│   │   └── backgrounds/
│   ├── styles/                             # QSS样式
│   ├── data/                               # 数据文件
│   │   └── questions.json
│   └── qml/                                # QML文件
│       ├── questionbank/
│       │   ├── QuestionBankPage.qml
│       │   ├── FilterPanel.qml
│       │   ├── Badge.qml
│       │   ├── OptionItem.qml
│       │   ├── QuestionView.qml
│       │   └── Theme.qml
│       └── components/                     # 通用组件
├── docs/                                   # 文档目录
│   ├── features/                           # 功能文档
│   │   ├── REMEMBER_ME_FEATURE.md
│   │   ├── SIGNUP_WINDOW.md
│   │   └── AUTHENTICATION.md
│   ├── api/                                # API文档
│   └── development/                        # 开发文档
│       ├── README.md
│       └── CHANGELOG.md
├── tests/                                  # 测试文件
│   ├── unit/                               # 单元测试
│   └── integration/                        # 集成测试
├── scripts/                                # 脚本文件
│   ├── build.sh
│   ├── test.sh
│   └── deploy.sh
├── build/                                  # 构建输出
│   ├── debug/
│   └── release/
├── qml.qrc                                 # QML资源
├── resources.qrc                           # 通用资源
├── AIPoliticsClassroom.pro                 # 项目文件
├── README.md                               # 项目说明
└── LICENSE                                 # 许可证
```

## 🔄 重构计划

### 第一阶段：移动文件和目录
1. 将 `Pages/` 目录移动到 `resources/pages/`
2. 将文档文件移动到 `docs/features/`
3. 将QML文件整理到 `resources/qml/`
4. 重新组织源代码结构

### 第二阶段：更新项目文件
1. 更新 `.pro` 文件中的路径
2. 更新 `#include` 路径
3. 更新资源文件引用

### 第三阶段：清理和优化
1. 删除空目录
2. 统一命名规范
3. 更新文档

