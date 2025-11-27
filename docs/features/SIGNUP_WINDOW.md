# 注册窗口功能文档

## 概述
根据登录窗口的模版代码，使用Codex AI助手生成了完整的用户注册窗口（SignUpWindow），实现了用户注册功能。

## 功能特性

### 1. 界面设计
- **双栏布局**：与登录窗口保持一致的设计风格
- **左侧品牌区**：显示品牌信息和励志名言
- **右侧表单区**：注册表单和操作按钮
- **红色主题**：与整体UI保持一致

### 2. 输入字段
- **邮箱地址**（必填）
  - 验证邮箱格式
  - Supabase注册必需
  
- **用户名**（可选）
  - 用于个性化显示
  - 可以留空

- **设置密码**（必填）
  - 至少8位字符
  - 密码可见性切换功能
  - 实时验证

- **确认密码**（必填）
  - 与设置密码必须一致
  - 防止输入错误

### 3. 交互功能
- **注册按钮**：提交注册信息
- **返回登录**：切换到登录窗口
- **密码可见性切换**：👁 / 👁‍🗨 图标
- **表单验证**：实时检查输入有效性
- **成功提示**：注册成功后显示确认信息

### 4. 表单验证
```cpp
bool SignUpWindow::validateInput()
{
    // 验证邮箱格式
    // 验证密码长度（至少8位）
    // 验证两次密码一致
    // 错误提示和焦点设置
}
```

### 5. Supabase集成
```cpp
// 注册信号连接
connect(m_supabaseClient, &SupabaseClient::signupSuccess,
        this, &SignUpWindow::onSignupSuccess);
connect(m_supabaseClient, &SupabaseClient::signupFailed,
        this, &SignUpWindow::onSignupFailed);

// 调用注册API
m_supabaseClient->signup(email, password, username);
```

## 文件结构

```
src/auth/signup/
├── signupwindow.h      // 注册窗口头文件
└── signupwindow.cpp    // 注册窗口实现文件
```

### 头文件 (signupwindow.h)
- 包含所有必要的Qt头文件
- SupabaseClient依赖
- 窗体控件声明
- 槽函数和回调函数声明

### 源文件 (signupwindow.cpp)
- 完整的UI布局实现
- 样式设置
- 表单验证逻辑
- Supabase信号处理
- 窗口切换功能

## 集成到项目

### 1. 项目配置 (AIPoliticsClassroom.pro)
```pro
# 添加源文件
SOURCES += \
    src/auth/signup/signupwindow.cpp

# 添加头文件
HEADERS += \
    src/auth/signup/signupwindow.h
```

### 2. 登录窗口集成
```cpp
// simpleloginwindow.h
#include "../signup/signupwindow.h"

// simpleloginwindow.cpp
void SimpleLoginWindow::onSignupClicked()
{
    this->close();
    SignUpWindow *signupWindow = new SignUpWindow();
    signupWindow->show();
}
```

## 使用流程

1. **用户操作**
   - 用户在登录窗口点击"立即注册"按钮
   - 打开注册窗口

2. **输入信息**
   - 填写邮箱（必填）
   - 填写用户名（可选）
   - 设置密码（至少8位）
   - 确认密码（必须一致）

3. **提交注册**
   - 点击"注册账户"按钮
   - 验证表单输入
   - 调用Supabase API

4. **注册结果**
   - 成功：显示成功消息，2秒后跳转登录
   - 失败：显示错误信息，恢复按钮状态

## 样式设计

### 输入框样式
```css
QLineEdit {
  border: 2px solid #E2E8F0;
  border-radius: 8px;
  background-color: white;
}
QLineEdit:focus {
  border-color: #C62828;
  background-color: #FEF2F2;
}
```

### 按钮样式
```css
QPushButton {
  background-color: #C62828;
  color: white;
  border-radius: 8px;
}
QPushButton:hover {
  background-color: #B71C1C;
}
```

### 面板样式
```css
QFrame#leftPanel {
  background-color: #B71C1C;
}
QFrame#rightPanel {
  background-color: white;
}
```

## 技术细节

### 信号槽连接
- 注册按钮点击 → onSignupClicked()
- 返回登录按钮 → onBackToLoginClicked()
- 密码可见性切换 → onTogglePassword1/2Clicked()
- Supabase注册成功 → onSignupSuccess()
- Supabase注册失败 → onSignupFailed()

### 防重复处理
- `m_signupProcessed` 标志位
- 防止在注册过程中重复提交
- 失败时重置状态

### 错误处理
- 表单验证错误：立即提示并设置焦点
- 网络错误：显示Supabase返回的错误信息
- 重置按钮状态，允许用户重新尝试

## 已知问题
- 暂无

## 后续优化
- [ ] 邮箱验证
- [ ] 密码强度指示器
- [ ] 验证码功能
- [ ] 注册成功后的自动登录
- [ ] 用户协议和隐私政策

## 测试用例

### 正常流程
1. 输入有效邮箱和密码
2. 点击注册
3. 查看成功消息
4. 跳转到登录页面

### 验证错误
1. 不输入邮箱 → 显示"请输入邮箱地址"
2. 输入无效邮箱 → 显示"请输入有效的邮箱地址"
3. 密码少于8位 → 显示"密码至少需要8位字符"
4. 两次密码不一致 → 显示"两次输入的密码不一致"

### 网络错误
1. Supabase返回错误
2. 显示错误消息
3. 恢复注册按钮状态

## 总结
注册窗口已完全实现并成功集成到项目中。功能完整，UI美观，与登录窗口保持一致的设计风格。通过Supabase实现用户注册功能，提供完善的错误处理和用户反馈。
