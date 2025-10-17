# AI思政教学系统项目文档

## 📁 项目文档结构

```
AI思政教学系统项目/
├── README.md                                    # 项目说明（本文件）
├── AI_Political_Education_System_PRD_CN.md     # 产品需求文档（中文版）
├── AI_Political_Education_Database_Design_CN.md # 数据库设计文档（中文版）
├── AI_Political_Education_Business_Flows.md    # 业务流程文档（Mermaid代码）
└── AI_Political_Education_Flow_Diagrams_Visual.md # 可视化流程图指南
```

## 📋 文档说明

### 1. 产品需求文档 (PRD)
- **文件名**: `AI_Political_Education_System_PRD_CN.md`
- **内容**: 完整的产品需求规格说明
- **包含**: 用户画像、功能需求、技术架构、实施计划等

### 2. 数据库设计文档
- **文件名**: `AI_Political_Education_Database_Design_CN.md`
- **内容**: 完整的数据库架构设计
- **包含**: 8大模块50+数据表、索引策略、性能优化、安全机制

### 3. 业务流程文档
- **文件名**: `AI_Political_Education_Business_Flows.md`
- **内容**: 详细的业务流程定义
- **包含**: 10个核心业务流程的Mermaid代码

### 4. 可视化流程图指南
- **文件名**: `AI_Political_Education_Flow_Diagrams_Visual.md`
- **内容**: 如何查看可视化流程图
- **包含**: 3种查看方法和10个图形化流程图

## 🎯 项目概览

本项目是基于漳州市教育信息技术研究课题"生成式人工智能在中学思想政治教学中的全流程应用研究"而设计的AI思政教学系统。

### 核心功能模块
- 🗂️ **智能备课模块** - AI辅助教案创建和优化
- ✍️ **课堂教学模块** - 实时互动和智能分析
- 📊 **作业评价模块** - 智能评分和个性化反馈
- 🔍 **学情分析模块** - 多维度学习数据分析

### 技术架构
- **前端**: Qt 6.x 桌面应用
- **后端**: Python FastAPI
- **AI服务**: DeepSeek、OpenAI等LLM集成
- **数据库**: PostgreSQL + Redis + MongoDB

## 📖 使用建议

1. **阅读顺序**: PRD → 数据库设计 → 业务流程 → 流程图
2. **开发团队**: 重点关注数据库设计和业务流程
3. **产品设计**: 重点关注PRD和流程图
4. **项目汇报**: 可使用流程图进行可视化演示

## 🛠️ 开发环境准备

### 查看流程图
```bash
# 方法1: VSCode插件
# 1. 安装 "Mermaid Preview" 插件
# 2. 右键文档选择 "Open Preview"

# 方法2: 在线工具
# 访问 https://mermaid.live
# 复制Mermaid代码粘贴即可查看图形
```

### 数据库初始化
```sql
-- 请参考数据库设计文档中的初始化脚本
-- 包含建表语句、索引创建、基础数据插入
```

## 📞 联系信息

如需了解更多详情或有任何问题，请参考相关文档或联系项目团队。

---
*最后更新: 2025年10月17日*