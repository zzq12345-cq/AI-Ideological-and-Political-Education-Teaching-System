# 安全配置指南

## 🔐 API 密钥管理

### 重要说明
本项目已移除硬编码的 API 密钥，改为使用环境变量配置，以提高安全性。

### 配置方法

#### 方法1：环境变量 (推荐)
```bash
# 在终端中设置环境变量
export DIFY_API_KEY=app-your-api-key-here

# 然后运行应用
./build/release/AIPoliticsClassroom.app/Contents/MacOS/AIPoliticsClassroom
```

#### 方法2：.env 文件
1. 复制示例配置文件：
```bash
cp .env.example .env
```

2. 编辑 `.env` 文件，填入真实的 API Key：
```
DIFY_API_KEY=app-your-api-key-here
```

3. 运行应用（会自动读取 .env 文件）

### ⚠️ 安全注意事项

1. **永远不要将真实的 API Key 提交到版本控制系统**
   - `.env` 文件已添加到 `.gitignore` 中
   - 只提交 `.env.example` 作为配置示例

2. **定期轮换 API Key**
   - 建议每 3-6 个月更换一次 API Key
   - 如果怀疑密钥泄露，立即更换

3. **使用最小权限原则**
   - 为不同环境使用不同的 API Key
   - 限制 API Key 的权限范围

### 🔑 如果密钥已泄露

如果你之前提交过真实的 API Key 到 GitHub：

1. **立即撤销泄露的密钥**
   - 登录 Dify 控制台
   - 删除或重新生成 API Key

2. **检查使用记录**
   - 查看 Dify 控制台的 API 调用记录
   - 确认是否有异常使用

3. **生成新的 API Key**
   - 使用新的密钥更新配置
   - 不要重复使用已泄露的密钥

### 📋 验证配置

运行应用时，检查控制台输出：
- ✅ 正常：会显示 `[DifyService] Using model: "glm-4.6"`
- ⚠️ 警告：如果未设置环境变量，会显示 `[Security Warning] DIFY_API_KEY environment variable not set`

### 🛡️ 其他安全建议

1. **使用 .env 文件时**：确保 `.env` 文件权限设置正确
```bash
chmod 600 .env  # 仅所有者可读写
```

2. **生产环境**：使用专门的环境变量管理方案
3. **团队协作**：使用安全的密钥共享工具，不要通过聊天工具发送密钥

### 📞 如有疑问

如果对安全配置有疑问，请参考：
- Qt 环境变量文档
- Dify API 安全指南
- OWASP API Security Top 10