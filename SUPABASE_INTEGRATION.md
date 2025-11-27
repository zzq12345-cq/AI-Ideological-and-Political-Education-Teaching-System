# Qt + Supabase 认证系统集成

## 项目概述

本项目成功集成了Supabase云数据库与Qt登录/注册系统，实现了完整的用户认证功能。

## 集成组件

### 1. Supabase配置 (`src/auth/supabase/supabaseconfig.h/cpp`)
- 存储Supabase项目URL和API密钥
- 配置数据库表名和认证头

### 2. Supabase客户端 (`src/auth/supabase/supabaseclient.h/cpp`)
- 处理所有与Supabase的HTTP通信
- 支持登录、注册和用户检查
- 异步操作，基于Qt信号槽机制

### 3. 登录窗口 (`src/auth/login/simpleloginwindow.h/cpp`)
- 保持原有UI设计
- 集成Supabase认证
- 兼容测试账号（teacher01, student01, admin01）
- 邮箱格式输入自动使用Supabase登录

### 4. 注册窗口 (`src/auth/login/signupwindow.h/cpp`)
- 全新设计的教师注册界面
- 表单验证（邮箱格式、密码确认等）
- Supabase用户创建

### 5. 窗口管理 (`src/main/main.cpp`)
- 智能窗口切换逻辑
- 登录/注册窗口无缝衔接

## Supabase配置

项目已配置你的Supabase信息：
- **URL**: `https://sbp-7wa5y7wjcyw0cpry.supabase.opentrust.net`
- **数据库表**: `teachers`
- **认证方式**: Supabase Auth

## 认证流程

### 登录流程
1. 用户输入用户名/邮箱和密码
2. 系统检查是否为测试账号（teacher01等）
3. 如果是邮箱格式，调用Supabase Auth API
4. 登录成功进入主界面

### 注册流程
1. 点击"立即注册"按钮
2. 填写用户名、邮箱、密码、确认密码
3. 表单验证通过后调用Supabase Auth API
4. 注册成功提示用户验证邮箱

## 构建和运行

### 构建项目
```bash
cd /Users/zhouzhiqi/QtProjects/AItechnology/build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
```

### 运行应用
```bash
./AILoginSystem.app/Contents/MacOS/AILoginSystem
```

## 测试账号

为了方便测试，系统保留以下硬编码账号：
- `teacher01` / `Teacher@2024` - 教师端
- `student01` / `Student@2024` - 学生端
- `admin01` / `Admin@2024` - 管理员端

## 数据库配置建议

在Supabase控制台中：

### 1. 启用Authentication
- 进入Authentication > Settings
- 确认启用Email认证

### 2. 配置RLS策略（建议）
```sql
-- 允许注册用户查看自己的数据
CREATE POLICY "Users can view own data" ON teachers
  FOR SELECT USING (auth.uid()::text = id);
```

### 3. 用户表扩展（可选）
如果需要存储额外用户信息，可创建扩展表：
```sql
CREATE TABLE teacher_profiles (
  id UUID REFERENCES auth.users(id) PRIMARY KEY,
  username TEXT UNIQUE NOT NULL,
  full_name TEXT,
  created_at TIMESTAMP DEFAULT NOW()
);
```

## 扩展功能建议

1. **密码重置**: 集成Supabase密码重置功能
2. **社交登录**: 添加Google、GitHub等OAuth提供商
3. **邮箱验证**: 实现邮箱验证流程
4. **会话管理**: 保存用户登录状态
5. **用户资料**: 创建用户资料管理页面

## 技术栈

- **Qt 6.9.3** - UI框架
- **Qt Network** - HTTP通信
- **Qt Charts** - 数据可视化
- **Supabase** - 云数据库和认证
- **CMake** - 构建系统

## 文件结构

```
src/
├── main/
│   └── main.cpp              # 主入口，窗口管理
├── auth/
│   ├── supabase/
│   │   ├── supabaseconfig.h  # Supabase配置
│   │   ├── supabaseconfig.cpp
│   │   ├── supabaseclient.h  # Supabase客户端
│   │   └── supabaseclient.cpp
│   └── login/
│       ├── simpleloginwindow.h     # 登录窗口
│       ├── simpleloginwindow.cpp
│       ├── signupwindow.h          # 注册窗口
│       └── signupwindow.cpp
└── dashboard/
    └── modernmainwindow.h    # 主界面
```

## 注意事项

1. **API密钥安全**: 当前使用匿名密钥，适合测试。生产环境请使用行级安全(RLS)
2. **HTTPS**: 确保所有API调用使用HTTPS
3. **错误处理**: 网络错误和API错误都有相应处理
4. **性能**: HTTP请求都是异步的，不会阻塞UI

## 下一步

1. 在Supabase中创建`teachers`表（如需要）
2. 配置RLS策略保护数据
3. 测试邮箱验证流程
4. 添加更多用户角色支持

---
**集成完成时间**: 2025-11-02
**版本**: v1.0.0
