# AI思政教学系统数据库设计

## 数据库架构概述

### 数据库选择
- **主数据库（结构化数据）：** PostgreSQL 14+
- **缓存数据库：** Redis 6+
- **文档存储：** MongoDB 5.0+
- **搜索引擎：** Elasticsearch 7+

### 设计原则
- 遵循第三范式（3NF）
- 支持水平扩展
- 数据安全性和隐私保护
- 高性能读写分离
- 支持事务一致性

---

## 1. 用户管理模块

### 1.1 用户基础信息表 (users)

```sql
CREATE TABLE users (
    user_id BIGSERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,           -- 用户名
    password_hash VARCHAR(255) NOT NULL,            -- 密码哈希
    salt VARCHAR(32) NOT NULL,                      -- 密码盐值
    email VARCHAR(100) UNIQUE,                      -- 邮箱
    phone VARCHAR(20),                              -- 手机号
    real_name VARCHAR(100),                         -- 真实姓名
    user_type SMALLINT NOT NULL,                    -- 用户类型：1:教师 2:学生 3:管理员 4:研究者
    status SMALLINT DEFAULT 1,                      -- 状态：1:正常 2:禁用 3:删除
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- 创建时间
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- 更新时间
    last_login_at TIMESTAMP,                        -- 最后登录时间
    login_count INTEGER DEFAULT 0                   -- 登录次数
);

-- 索引
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_type_status ON users(user_type, status);
```

### 1.2 教师详细信息表 (teachers)

```sql
CREATE TABLE teachers (
    teacher_id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(user_id),
    employee_id VARCHAR(20) UNIQUE,                -- 工号
    department VARCHAR(100),                        -- 所属部门
    title VARCHAR(50),                              -- 职称：高级教师、一级教师等
    subject VARCHAR(50),                            -- 主教学科
    teaching_years INTEGER,                         -- 教学年限
    education_background VARCHAR(100),              -- 学历背景
    professional_title VARCHAR(100),                -- 专业职务
    research_focus TEXT,                            -- 研究专长
    school_id BIGINT,                               -- 所属学校ID
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_teachers_user_id ON teachers(user_id);
CREATE INDEX idx_teachers_school_id ON teachers(school_id);
CREATE INDEX idx_teachers_subject ON teachers(subject);
```

### 1.3 学生详细信息表 (students)

```sql
CREATE TABLE students (
    student_id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(user_id),
    student_number VARCHAR(20) UNIQUE,              -- 学号
    grade_level SMALLINT,                           -- 年级：7-12年级
    class_name VARCHAR(50),                         -- 班级名称
    class_id BIGINT,                                -- 班级ID
    school_id BIGINT,                               -- 学校ID
    enrollment_date DATE,                           -- 入学日期
    parent_phone VARCHAR(20),                       -- 家长电话
    parent_email VARCHAR(100),                      -- 家长邮箱
    learning_style VARCHAR(50),                     -- 学习风格
    interests TEXT,                                 -- 兴趣爱好
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_students_user_id ON students(user_id);
CREATE INDEX idx_students_school_id ON students(school_id);
CREATE INDEX idx_students_class_id ON students(class_id);
CREATE INDEX idx_students_grade ON students(grade_level);
```

### 1.4 学校信息表 (schools)

```sql
CREATE TABLE schools (
    school_id BIGSERIAL PRIMARY KEY,
    school_name VARCHAR(200) NOT NULL,              -- 学校名称
    school_code VARCHAR(20) UNIQUE,                 -- 学校代码
    school_type SMALLINT,                           -- 学校类型：1:小学 2:初中 3:高中 4:完全中学
    district VARCHAR(100),                          -- 所属区县
    address TEXT,                                   -- 学校地址
    contact_phone VARCHAR(20),                      -- 联系电话
    principal_name VARCHAR(100),                    -- 校长姓名
    description TEXT,                               -- 学校简介
    status SMALLINT DEFAULT 1,                      -- 状态
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_schools_code ON schools(school_code);
CREATE INDEX idx_schools_district ON schools(district);
```

### 1.5 班级信息表 (classes)

```sql
CREATE TABLE classes (
    class_id BIGSERIAL PRIMARY KEY,
    class_name VARCHAR(50) NOT NULL,                -- 班级名称
    grade_level SMALLINT,                           -- 年级
    school_id BIGINT REFERENCES schools(school_id), -- 所属学校
    head_teacher_id BIGINT REFERENCES teachers(teacher_id), -- 班主任
    student_count INTEGER DEFAULT 0,                -- 学生人数
    academic_year VARCHAR(10),                      -- 学年：2025-2026
    semester SMALLINT,                              -- 学期：1:上学期 2:下学期
    status SMALLINT DEFAULT 1,                      -- 状态
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_classes_school_id ON classes(school_id);
CREATE INDEX idx_classes_teacher_id ON classes(head_teacher_id);
CREATE INDEX idx_classes_grade_year ON classes(grade_level, academic_year);
```

### 1.6 用户角色权限表 (user_roles)

```sql
CREATE TABLE user_roles (
    role_id BIGSERIAL PRIMARY KEY,
    role_name VARCHAR(50) NOT NULL,                 -- 角色名称
    role_code VARCHAR(20) UNIQUE,                   -- 角色代码
    description TEXT,                               -- 角色描述
    permissions JSONB,                              -- 权限列表
    status SMALLINT DEFAULT 1,                      -- 状态
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 用户角色关联表
CREATE TABLE user_role_relations (
    relation_id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(user_id),
    role_id BIGINT NOT NULL REFERENCES user_roles(role_id),
    assigned_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- 分配时间
    assigned_by BIGINT REFERENCES users(user_id),    -- 分配人
    expires_at TIMESTAMP,                           -- 过期时间
    UNIQUE(user_id, role_id)
);
```

---

## 2. 教学资源管理模块

### 2.1 教学资源表 (teaching_resources)

```sql
CREATE TABLE teaching_resources (
    resource_id BIGSERIAL PRIMARY KEY,
    title VARCHAR(200) NOT NULL,                    -- 资源标题
    description TEXT,                               -- 资源描述
    resource_type SMALLINT NOT NULL,                -- 资源类型：1:文档 2:图片 3:视频 4:音频 5:PPT 6:其他
    subject VARCHAR(50),                            -- 学科
    grade_level SMALLINT,                           -- 适用年级
    topic_tags TEXT[],                              -- 主题标签
    keywords TEXT[],                                -- 关键词
    difficulty_level SMALLINT,                      -- 难度等级：1:简单 2:中等 3:困难
    file_path VARCHAR(500),                         -- 文件路径
    file_size BIGINT,                               -- 文件大小
    file_format VARCHAR(20),                        -- 文件格式
    thumbnail_path VARCHAR(500),                    -- 缩略图路径
    download_count INTEGER DEFAULT 0,               -- 下载次数
    view_count INTEGER DEFAULT 0,                   -- 查看次数
    rating DECIMAL(3,2) DEFAULT 0.00,              -- 评分
    rating_count INTEGER DEFAULT 0,                 -- 评分人数
    creator_id BIGINT REFERENCES users(user_id),    -- 创建者
    reviewer_id BIGINT REFERENCES users(user_id),   -- 审核者
    review_status SMALLINT DEFAULT 0,               -- 审核状态：0:待审核 1:已通过 2:已拒绝
    is_public BOOLEAN DEFAULT TRUE,                 -- 是否公开
    ai_generated BOOLEAN DEFAULT FALSE,             -- 是否AI生成
    ai_prompt TEXT,                                 -- AI生成时的提示词
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_resources_type ON teaching_resources(resource_type);
CREATE INDEX idx_resources_subject ON teaching_resources(subject);
CREATE INDEX idx_resources_grade ON teaching_resources(grade_level);
CREATE INDEX idx_resources_creator ON teaching_resources(creator_id);
CREATE INDEX idx_resources_status ON teaching_resources(review_status);
CREATE INDEX idx_resources_tags ON teaching_resources USING GIN(topic_tags);
CREATE INDEX idx_resources_keywords ON teaching_resources USING GIN(keywords);
```

### 2.2 资源分类表 (resource_categories)

```sql
CREATE TABLE resource_categories (
    category_id BIGSERIAL PRIMARY KEY,
    category_name VARCHAR(100) NOT NULL,            -- 分类名称
    parent_id BIGINT REFERENCES resource_categories(category_id), -- 父分类ID
    category_code VARCHAR(20) UNIQUE,               -- 分类代码
    description TEXT,                               -- 分类描述
    sort_order INTEGER DEFAULT 0,                   -- 排序
    status SMALLINT DEFAULT 1,                      -- 状态
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 资源分类关联表
CREATE TABLE resource_category_relations (
    relation_id BIGSERIAL PRIMARY KEY,
    resource_id BIGINT NOT NULL REFERENCES teaching_resources(resource_id),
    category_id BIGINT NOT NULL REFERENCES resource_categories(category_id),
    UNIQUE(resource_id, category_id)
);
```

### 2.3 教材信息表 (textbooks)

```sql
CREATE TABLE textbooks (
    textbook_id BIGSERIAL PRIMARY KEY,
    textbook_name VARCHAR(200) NOT NULL,            -- 教材名称
    publisher VARCHAR(100),                         -- 出版社
    isbn VARCHAR(20) UNIQUE,                        -- ISBN号
    subject VARCHAR(50),                            -- 学科
    grade_level SMALLINT,                           -- 年级
    semester SMALLINT,                              -- 学期
    edition_year INTEGER,                           -- 版本年份
    author VARCHAR(100),                            -- 作者
    cover_image VARCHAR(500),                       -- 封面图片
    description TEXT,                               -- 教材描述
    status SMALLINT DEFAULT 1,                      -- 状态
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 教材章节表
CREATE TABLE textbook_chapters (
    chapter_id BIGSERIAL PRIMARY KEY,
    textbook_id BIGINT NOT NULL REFERENCES textbooks(textbook_id),
    chapter_number INTEGER NOT NULL,                -- 章节序号
    chapter_title VARCHAR(200) NOT NULL,            -- 章节标题
    chapter_content TEXT,                           -- 章节内容
    key_concepts TEXT[],                            -- 核心概念
    learning_objectives TEXT[],                     -- 学习目标
    difficulty_level SMALLINT,                      -- 难度等级
    estimated_hours INTEGER,                        -- 预计课时
    sort_order INTEGER,                             -- 排序
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 3. 备课模块

### 3.1 教案表 (lesson_plans)

```sql
CREATE TABLE lesson_plans (
    plan_id BIGSERIAL PRIMARY KEY,
    plan_title VARCHAR(200) NOT NULL,               -- 教案标题
    subject VARCHAR(50),                            -- 学科
    grade_level SMALLINT,                           -- 年级
    class_id BIGINT REFERENCES classes(class_id),    -- 班级
    teacher_id BIGINT NOT NULL REFERENCES teachers(teacher_id), -- 教师
    textbook_id BIGINT REFERENCES textbooks(textbook_id), -- 教材
    chapter_id BIGINT REFERENCES textbook_chapters(chapter_id), -- 章节
    teaching_date DATE,                             -- 授课日期
    duration_minutes INTEGER DEFAULT 45,            -- 课时长度

    -- 教学目标（JSON格式存储三维目标）
    teaching_objectives JSONB,                      -- 教学目标

    -- 教学重难点
    key_points TEXT[],                              -- 教学重点
    difficult_points TEXT[],                        -- 教学难点

    -- 教学过程
    teaching_process JSONB,                         -- 详细教学过程

    -- 教学方法
    teaching_methods TEXT[],                        -- 教学方法

    -- 教学资源
    resource_ids BIGINT[],                          -- 使用的资源ID数组

    -- AI生成相关
    ai_generated BOOLEAN DEFAULT FALSE,             -- 是否AI生成
    ai_suggestions JSONB,                           -- AI建议
    ai_optimization_score DECIMAL(5,2),            -- AI优化评分

    -- 状态和版本
    version INTEGER DEFAULT 1,                      -- 版本号
    status SMALLINT DEFAULT 1,                      -- 状态：1:草稿 2:待审核 3:已发布 4:已归档

    -- 统计信息
    view_count INTEGER DEFAULT 0,                   -- 查看次数
    like_count INTEGER DEFAULT 0,                   -- 点赞次数
    copy_count INTEGER DEFAULT 0,                   -- 复制次数

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_lesson_plans_teacher ON lesson_plans(teacher_id);
CREATE INDEX idx_lesson_plans_class ON lesson_plans(class_id);
CREATE INDEX idx_lesson_plans_subject ON lesson_plans(subject);
CREATE INDEX idx_lesson_plans_date ON lesson_plans(teaching_date);
CREATE INDEX idx_lesson_plans_status ON lesson_plans(status);
```

### 3.2 学情分析表 (student_analysis)

```sql
CREATE TABLE student_analysis (
    analysis_id BIGSERIAL PRIMARY KEY,
    class_id BIGINT REFERENCES classes(class_id),    -- 班级
    teacher_id BIGINT REFERENCES teachers(teacher_id), -- 教师
    subject VARCHAR(50),                            -- 学科
    analysis_type SMALLINT,                         -- 分析类型：1:预习分析 2:作业分析 3:考试分析 4:综合分析
    analysis_date DATE,                             -- 分析日期

    -- 分析数据
    total_students INTEGER,                         -- 总学生数
    excellent_count INTEGER,                        -- 优秀学生数
    good_count INTEGER,                             -- 良好学生数
    average_count INTEGER,                          -- 中等学生数
    poor_count INTEGER,                             -- 待提高学生数

    -- 知识点掌握情况
    knowledge_points JSONB,                         -- 各知识点掌握率

    -- AI分析结果
    ai_analysis_result JSONB,                       -- AI详细分析结果
    ai_recommendations JSONB,                       -- AI教学建议

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 3.3 备课模板表 (lesson_templates)

```sql
CREATE TABLE lesson_templates (
    template_id BIGSERIAL PRIMARY KEY,
    template_name VARCHAR(100) NOT NULL,            -- 模板名称
    template_type SMALLINT,                         -- 模板类型：1:通用模板 2:学科模板 3:专题模板
    subject VARCHAR(50),                            -- 学科
    grade_level SMALLINT,                           -- 年级
    template_structure JSONB,                       -- 模板结构
    example_content JSONB,                          -- 示例内容
    usage_count INTEGER DEFAULT 0,                  -- 使用次数
    creator_id BIGINT REFERENCES users(user_id),    -- 创建者
    is_public BOOLEAN DEFAULT FALSE,                -- 是否公开
    status SMALLINT DEFAULT 1,                      -- 状态
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 4. 授课互动模块

### 4.1 课堂教学记录表 (class_sessions)

```sql
CREATE TABLE class_sessions (
    session_id BIGSERIAL PRIMARY KEY,
    lesson_plan_id BIGINT REFERENCES lesson_plans(plan_id), -- 教案ID
    teacher_id BIGINT REFERENCES teachers(teacher_id),     -- 教师ID
    class_id BIGINT REFERENCES classes(class_id),           -- 班级ID
    session_title VARCHAR(200),                              -- 课堂标题
    start_time TIMESTAMP NOT NULL,                           -- 开始时间
    end_time TIMESTAMP,                                      -- 结束时间
    actual_duration INTEGER,                                 -- 实际时长（分钟）

    -- 课堂状态
    session_status SMALLINT DEFAULT 1,                       -- 状态：1:进行中 2:已结束 3:已取消

    -- 参与统计
    total_students INTEGER,                                  -- 应到学生数
    present_students INTEGER,                                 -- 实到学生数
    participation_rate DECIMAL(5,2),                         -- 参与率

    -- 互动统计
    question_count INTEGER DEFAULT 0,                        -- 提问次数
    answer_count INTEGER DEFAULT 0,                          -- 回答次数
    poll_count INTEGER DEFAULT 0,                            -- 投票次数

    -- AI辅助
    ai_assistant_enabled BOOLEAN DEFAULT TRUE,               -- AI助手是否启用
    ai_suggestions_count INTEGER DEFAULT 0,                  -- AI建议次数

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_sessions_teacher ON class_sessions(teacher_id);
CREATE INDEX idx_sessions_class ON class_sessions(class_id);
CREATE INDEX idx_sessions_date ON class_sessions(start_time);
CREATE INDEX idx_sessions_plan ON class_sessions(lesson_plan_id);
```

### 4.2 课堂互动记录表 (class_interactions)

```sql
CREATE TABLE class_interactions (
    interaction_id BIGSERIAL PRIMARY KEY,
    session_id BIGINT NOT NULL REFERENCES class_sessions(session_id), -- 课堂ID
    student_id BIGINT REFERENCES students(student_id),               -- 学生ID
    teacher_id BIGINT REFERENCES teachers(teacher_id),               -- 教师ID

    interaction_type SMALLINT NOT NULL,                              -- 互动类型：1:提问 2:回答 3:投票 4:讨论 5:其他
    interaction_content TEXT,                                        -- 互动内容

    -- 时间信息
    interaction_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,           -- 互动时间
    response_time INTEGER,                                          -- 响应时间（秒）

    -- AI相关
    ai_response TEXT,                                               -- AI生成的回复
    ai_confidence DECIMAL(5,2),                                     -- AI回复置信度

    -- 评价
    quality_score SMALLINT,                                         -- 质量评分 1-5分
    teacher_feedback TEXT,                                          -- 教师反馈

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_interactions_session ON class_interactions(session_id);
CREATE INDEX idx_interactions_student ON class_interactions(student_id);
CREATE INDEX idx_interactions_type ON class_interactions(interaction_type);
CREATE INDEX idx_interactions_time ON class_interactions(interaction_time);
```

### 4.3 实时投票表 (live_polls)

```sql
CREATE TABLE live_polls (
    poll_id BIGSERIAL PRIMARY KEY,
    session_id BIGINT NOT NULL REFERENCES class_sessions(session_id), -- 课堂ID
    teacher_id BIGINT REFERENCES teachers(teacher_id),               -- 教师ID

    poll_question TEXT NOT NULL,                                     -- 投票题目
    poll_type SMALLINT DEFAULT 1,                                    -- 投票类型：1:单选 2:多选 3:文本 4:评分
    poll_options JSONB,                                              -- 选项内容
    correct_answer TEXT,                                             -- 正确答案

    -- 时间控制
    start_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                 -- 开始时间
    end_time TIMESTAMP,                                              -- 结束时间
    duration_seconds INTEGER,                                        -- 持续时间（秒）

    -- 参与统计
    total_participants INTEGER DEFAULT 0,                            -- 总参与人数
    correct_rate DECIMAL(5,2),                                       -- 正确率

    -- AI分析
    ai_analysis JSONB,                                               -- AI对结果的分析
    difficulty_level SMALLINT,                                       -- 难度等级：1:简单 2:中等 3:困难

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 投票答案记录表
CREATE TABLE poll_responses (
    response_id BIGSERIAL PRIMARY KEY,
    poll_id BIGINT NOT NULL REFERENCES live_polls(poll_id),         -- 投票ID
    student_id BIGINT REFERENCES students(student_id),              -- 学生ID
    selected_option TEXT,                                            -- 选择的选项
    response_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,              -- 回答时间
    time_spent INTEGER,                                              -- 答题用时（秒）
    is_correct BOOLEAN                                               -- 是否正确
);
```

### 4.4 情境创设记录表 (scenario_sessions)

```sql
CREATE TABLE scenario_sessions (
    scenario_id BIGSERIAL PRIMARY KEY,
    session_id BIGINT REFERENCES class_sessions(session_id),        -- 课堂ID
    scenario_title VARCHAR(200) NOT NULL,                            -- 情境标题
    scenario_type SMALLINT,                                          -- 情境类型：1:案例分析 2:角色扮演 3:模拟演练 4:虚拟体验

    -- 情境内容
    scenario_content JSONB,                                          -- 情境详细内容
    background_material TEXT,                                         -- 背景材料

    -- AI生成内容
    ai_generated BOOLEAN DEFAULT FALSE,                              -- 是否AI生成
    ai_prompt TEXT,                                                  -- AI生成提示词

    -- 实施情况
    start_time TIMESTAMP,                                            -- 开始时间
    end_time TIMESTAMP,                                              -- 结束时间
    participant_count INTEGER,                                       -- 参与人数

    -- 效果评估
    engagement_score DECIMAL(5,2),                                   -- 参与度评分
    learning_outcome JSONB,                                          -- 学习效果

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 5. 评价分析模块

### 5.1 作业任务表 (assignments)

```sql
CREATE TABLE assignments (
    assignment_id BIGSERIAL PRIMARY KEY,
    assignment_title VARCHAR(200) NOT NULL,                          -- 作业标题
    description TEXT,                                                 -- 作业描述
    assignment_type SMALLINT,                                         -- 作业类型：1:课后作业 2:课堂练习 3:项目作业 4:考试

    -- 关联信息
    teacher_id BIGINT REFERENCES teachers(teacher_id),               -- 教师ID
    class_id BIGINT REFERENCES classes(class_id),                    -- 班级ID
    lesson_plan_id BIGINT REFERENCES lesson_plans(plan_id),          -- 教案ID
    subject VARCHAR(50),                                             -- 学科

    -- 时间安排
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                 -- 创建时间
    publish_time TIMESTAMP,                                          -- 发布时间
    deadline TIMESTAMP,                                              -- 截止时间

    -- 作业内容
    content JSONB,                                                   -- 作业详细内容
    attachments TEXT[],                                              -- 附件列表
    total_score DECIMAL(5,2) DEFAULT 100.00,                        -- 总分

    -- AI相关
    ai_generated BOOLEAN DEFAULT FALSE,                              -- 是否AI生成
    ai_difficulty_level SMALLINT,                                    -- AI评估难度
    ai_estimated_time INTEGER,                                       -- AI预估完成时间

    -- 统计
    total_submissions INTEGER DEFAULT 0,                             -- 总提交数
    completion_rate DECIMAL(5,2),                                    -- 完成率
    average_score DECIMAL(5,2),                                      -- 平均分

    status SMALLINT DEFAULT 1,                                       -- 状态：1:草稿 2:已发布 3:已截止 4:已归档

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_assignments_teacher ON assignments(teacher_id);
CREATE INDEX idx_assignments_class ON assignments(class_id);
CREATE INDEX idx_assignments_deadline ON assignments(deadline);
CREATE INDEX idx_assignments_status ON assignments(status);
```

### 5.2 作业提交表 (assignment_submissions)

```sql
CREATE TABLE assignment_submissions (
    submission_id BIGSERIAL PRIMARY KEY,
    assignment_id BIGINT NOT NULL REFERENCES assignments(assignment_id), -- 作业ID
    student_id BIGINT NOT NULL REFERENCES students(student_id),              -- 学生ID

    -- 提交内容
    submission_content JSONB,                                                  -- 提交内容
    attachments TEXT[],                                                         -- 附件列表
    text_content TEXT,                                                          -- 文本内容（用于AI分析）

    -- 时间记录
    submit_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                            -- 提交时间
    time_spent INTEGER,                                                         -- 完成用时（分钟）

    -- 评分信息
    score DECIMAL(5,2),                                                         -- 得分
    max_score DECIMAL(5,2),                                                     -- 满分

    -- AI评价
    ai_score DECIMAL(5,2),                                                      -- AI评分
    ai_feedback JSONB,                                                          -- AI详细反馈
    ai_plagiarism_check JSONB,                                                  -- AI查重结果

    -- 教师评价
    teacher_feedback TEXT,                                                      -- 教师反馈
    teacher_rating SMALLINT,                                                    -- 教师评分 1-5星

    -- 状态
    submission_status SMALLINT DEFAULT 1,                                       -- 状态：1:已提交 2:已评分 3:已退回 4:逾期

    graded_by BIGINT REFERENCES teachers(teacher_id),                           -- 评分教师
    graded_time TIMESTAMP,                                                      -- 评分时间

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_submissions_assignment ON assignment_submissions(assignment_id);
CREATE INDEX idx_submissions_student ON assignment_submissions(student_id);
CREATE INDEX idx_submissions_time ON assignment_submissions(submit_time);
CREATE INDEX idx_submissions_status ON assignment_submissions(submission_status);
```

### 5.3 学生成绩表 (student_grades)

```sql
CREATE TABLE student_grades (
    grade_id BIGSERIAL PRIMARY KEY,
    student_id BIGINT NOT NULL REFERENCES students(student_id),       -- 学生ID
    assignment_id BIGINT REFERENCES assignments(assignment_id),        -- 作业ID

    -- 成绩信息
    score DECIMAL(5,2) NOT NULL,                                      -- 得分
    max_score DECIMAL(5,2) NOT NULL,                                  -- 满分
    percentage DECIMAL(5,2),                                          -- 百分比
    grade_level SMALLINT,                                              -- 等级：1:A 2:B 3:C 4:D 5:F

    -- 知识点掌握情况
    topic_scores JSONB,                                                -- 各知识点得分

    -- 能力维度评价
    knowledge_score DECIMAL(5,2),                                     -- 知识掌握
    application_score DECIMAL(5,2),                                   -- 应用能力
    analysis_score DECIMAL(5,2),                                      -- 分析能力
    innovation_score DECIMAL(5,2),                                    -- 创新能力

    -- 进步情况
    previous_score DECIMAL(5,2),                                      -- 之前成绩
    improvement_rate DECIMAL(5,2),                                    -- 进步率

    graded_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                 -- 评分时间
    graded_by BIGINT REFERENCES teachers(teacher_id),                 -- 评分教师

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_grades_student ON student_grades(student_id);
CREATE INDEX idx_grades_assignment ON student_grades(assignment_id);
CREATE INDEX idx_grades_score ON student_grades(score);
CREATE INDEX idx_grades_time ON student_grades(graded_time);
```

### 5.4 学习分析报告表 (learning_analytics)

```sql
CREATE TABLE learning_analytics (
    analytics_id BIGSERIAL PRIMARY KEY,
    student_id BIGINT REFERENCES students(student_id),               -- 学生ID
    class_id BIGINT REFERENCES classes(class_id),                    -- 班级ID
    subject VARCHAR(50),                                             -- 学科

    -- 分析周期
    start_date DATE,                                                  -- 开始日期
    end_date DATE,                                                    -- 结束日期
    analytics_type SMALLINT,                                          -- 分析类型：1:周报 2:月报 3:学期报 4:年报

    -- 学习表现
    attendance_rate DECIMAL(5,2),                                     -- 出勤率
    assignment_completion_rate DECIMAL(5,2),                          -- 作业完成率
    average_score DECIMAL(5,2),                                      -- 平均分
    participation_score DECIMAL(5,2),                                 -- 参与度得分

    -- 能力雷达图数据
    ability_radar JSONB,                                              -- 六维能力雷达图

    -- 学习轨迹
    learning_trajectory JSONB,                                         -- 学习轨迹数据

    -- AI分析结果
    ai_analysis JSONB,                                                -- AI深度分析
    ai_recommendations JSONB,                                         -- AI学习建议
    ai_prediction JSONB,                                              -- AI预测分析

    -- 趋势分析
    score_trend JSONB,                                                -- 成绩趋势
    behavior_trend JSONB,                                             -- 行为趋势

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 5.5 教学效果评估表 (teaching_effectiveness)

```sql
CREATE TABLE teaching_effectiveness (
    evaluation_id BIGSERIAL PRIMARY KEY,
    teacher_id BIGINT REFERENCES teachers(teacher_id),               -- 教师ID
    class_id BIGINT REFERENCES classes(class_id),                    -- 班级ID
    subject VARCHAR(50),                                             -- 学科

    -- 评估周期
    evaluation_period VARCHAR(20),                                    -- 评估周期：如"2025春季学期"
    start_date DATE,                                                  -- 开始日期
    end_date DATE,                                                    -- 结束日期

    -- 学生评价维度
    student_satisfaction DECIMAL(5,2),                                -- 学生满意度
    knowledge_gain DECIMAL(5,2),                                     -- 知识收获
    skill_improvement DECIMAL(5,2),                                  -- 能力提升
    engagement_level DECIMAL(5,2),                                   -- 参与度

    -- 教学质量指标
    teaching_clarity DECIMAL(5,2),                                    -- 教学清晰度
    content_richness DECIMAL(5,2),                                   -- 内容丰富度
    interaction_quality DECIMAL(5,2),                                -- 互动质量
    innovation_level DECIMAL(5,2),                                   -- 创新程度

    -- AI评估结果
    ai_evaluation_score DECIMAL(5,2),                                -- AI综合评分
    ai_strengths TEXT[],                                             -- AI识别的优势
    ai_improvements TEXT[],                                           -- AI改进建议

    -- 同行评价
    peer_evaluation JSONB,                                            -- 同行评价

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 6. AI服务相关表

### 6.1 AI服务调用记录表 (ai_service_logs)

```sql
CREATE TABLE ai_service_logs (
    log_id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(user_id),                        -- 用户ID
    service_type VARCHAR(50),                                         -- 服务类型：content_gen, analysis, recommendation等
    model_name VARCHAR(100),                                          -- 使用的AI模型
    request_content JSONB,                                            -- 请求内容
    response_content JSONB,                                           -- 响应内容

    -- 性能指标
    request_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                 -- 请求时间
    response_time TIMESTAMP,                                          -- 响应时间
    duration_ms INTEGER,                                              -- 耗时（毫秒）
    token_count INTEGER,                                              -- Token消耗量

    -- 质量评估
    success BOOLEAN DEFAULT TRUE,                                     -- 是否成功
    error_message TEXT,                                               -- 错误信息
    user_rating SMALLINT,                                             -- 用户评分 1-5
    user_feedback TEXT,                                               -- 用户反馈

    -- 成本统计
    cost_amount DECIMAL(10,4),                                        -- 费用

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 索引
CREATE INDEX idx_ai_logs_user ON ai_service_logs(user_id);
CREATE INDEX idx_ai_logs_type ON ai_service_logs(service_type);
CREATE INDEX idx_ai_logs_time ON ai_service_logs(request_time);
CREATE INDEX idx_ai_logs_model ON ai_service_logs(model_name);
```

### 6.2 AI模型配置表 (ai_model_configs)

```sql
CREATE TABLE ai_model_configs (
    config_id BIGSERIAL PRIMARY KEY,
    model_name VARCHAR(100) NOT NULL,                                 -- 模型名称
    model_type VARCHAR(50),                                           -- 模型类型：llm, nlp, ml等
    provider VARCHAR(50),                                             -- 提供商：openai, deepseek, baidu等

    -- 配置参数
    api_endpoint VARCHAR(500),                                        -- API端点
    api_key_encrypted TEXT,                                           -- 加密存储的API密钥
    model_parameters JSONB,                                           -- 模型参数
    rate_limit INTEGER,                                               -- 调用频率限制

    -- 使用统计
    total_calls INTEGER DEFAULT 0,                                    -- 总调用次数
    success_rate DECIMAL(5,2),                                        -- 成功率
    average_response_time INTEGER,                                     -- 平均响应时间

    -- 状态管理
    is_active BOOLEAN DEFAULT TRUE,                                   -- 是否启用
    priority INTEGER DEFAULT 1,                                       -- 优先级

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 6.3 内容生成模板表 (content_generation_templates)

```sql
CREATE TABLE content_generation_templates (
    template_id BIGSERIAL PRIMARY KEY,
    template_name VARCHAR(100) NOT NULL,                              -- 模板名称
    template_type VARCHAR(50),                                        -- 模板类型：lesson_plan, question, scenario等
    subject VARCHAR(50),                                              -- 学科
    grade_level SMALLINT,                                             -- 年级

    -- 模板内容
    prompt_template TEXT NOT NULL,                                    -- 提示词模板
    system_prompt TEXT,                                               -- 系统提示词
    parameters JSONB,                                                 -- 模板参数

    -- 输出格式
    output_format JSONB,                                              -- 输出格式规范
    example_output JSONB,                                             -- 示例输出

    -- 质量控制
    quality_criteria JSONB,                                           -- 质量标准
    validation_rules JSONB,                                           -- 验证规则

    -- 使用统计
    usage_count INTEGER DEFAULT 0,                                    -- 使用次数
    success_rate DECIMAL(5,2),                                        -- 成功率
    user_rating DECIMAL(3,2),                                         -- 用户评分

    creator_id BIGINT REFERENCES users(user_id),                      -- 创建者
    is_public BOOLEAN DEFAULT FALSE,                                  -- 是否公开
    status SMALLINT DEFAULT 1,                                        -- 状态

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 7. 系统配置和日志

### 7.1 系统配置表 (system_configs)

```sql
CREATE TABLE system_configs (
    config_id BIGSERIAL PRIMARY KEY,
    config_key VARCHAR(100) UNIQUE NOT NULL,                          -- 配置键
    config_value JSONB,                                                -- 配置值
    config_type VARCHAR(50),                                           -- 配置类型：string, number, boolean, json等
    description TEXT,                                                  -- 配置描述
    is_public BOOLEAN DEFAULT FALSE,                                  -- 是否公开
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 7.2 操作日志表 (operation_logs)

```sql
CREATE TABLE operation_logs (
    log_id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(user_id),                          -- 用户ID
    operation_type VARCHAR(50),                                        -- 操作类型
    operation_module VARCHAR(50),                                      -- 操作模块
    operation_detail JSONB,                                            -- 操作详情

    -- 请求信息
    ip_address INET,                                                  -- IP地址
    user_agent TEXT,                                                   -- 用户代理
    request_url VARCHAR(500),                                          -- 请求URL

    -- 结果信息
    success BOOLEAN DEFAULT TRUE,                                      -- 是否成功
    error_code VARCHAR(50),                                            -- 错误代码
    error_message TEXT,                                                -- 错误信息

    operation_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                -- 操作时间

    -- 关联信息
    related_object_type VARCHAR(50),                                   -- 关联对象类型
    related_object_id BIGINT                                           -- 关联对象ID
);

-- 索引
CREATE INDEX idx_operation_logs_user ON operation_logs(user_id);
CREATE INDEX idx_operation_logs_time ON operation_logs(operation_time);
CREATE INDEX idx_operation_logs_type ON operation_logs(operation_type);
```

### 7.3 消息通知表 (notifications)

```sql
CREATE TABLE notifications (
    notification_id BIGSERIAL PRIMARY KEY,
    recipient_id BIGINT NOT NULL REFERENCES users(user_id),           -- 接收者ID
    sender_id BIGINT REFERENCES users(user_id),                       -- 发送者ID

    -- 消息内容
    title VARCHAR(200) NOT NULL,                                       -- 标题
    content TEXT,                                                      -- 内容
    notification_type SMALLINT,                                        -- 通知类型：1:系统通知 2:作业通知 3:评价通知 4:互动通知

    -- 关联信息
    related_object_type VARCHAR(50),                                   -- 关联对象类型
    related_object_id BIGINT,                                          -- 关联对象ID

    -- 状态
    is_read BOOLEAN DEFAULT FALSE,                                     -- 是否已读
    read_time TIMESTAMP,                                               -- 阅读时间

    -- 发送时间
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,                    -- 创建时间
    expires_at TIMESTAMP,                                              -- 过期时间

    -- 发送渠道
    channels SMALLINT[],                                               -- 发送渠道：1:站内信 2:邮件 3:短信 4:微信
    sent_status JSONB                                                  -- 各渠道发送状态
);

-- 索引
CREATE INDEX idx_notifications_recipient ON notifications(recipient_id);
CREATE INDEX idx_notifications_read ON notifications(is_read);
CREATE INDEX idx_notifications_time ON notifications(created_at);
```

---

## 8. 数据统计和报表

### 8.1 数据统计缓存表 (statistics_cache)

```sql
CREATE TABLE statistics_cache (
    cache_id BIGSERIAL PRIMARY KEY,
    cache_key VARCHAR(200) UNIQUE NOT NULL,                           -- 缓存键
    cache_data JSONB,                                                  -- 缓存数据
    data_type VARCHAR(50),                                             -- 数据类型

    -- 时间信息
    cache_date DATE,                                                   -- 缓存日期
    start_time TIMESTAMP,                                              -- 开始时间
    end_time TIMESTAMP,                                                -- 结束时间

    -- 缓存控制
    expire_time TIMESTAMP,                                             -- 过期时间
    refresh_interval INTEGER DEFAULT 3600,                             -- 刷新间隔（秒）

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 8.2 报表模板表 (report_templates)

```sql
CREATE TABLE report_templates (
    template_id BIGSERIAL PRIMARY KEY,
    template_name VARCHAR(100) NOT NULL,                              -- 模板名称
    template_type VARCHAR(50),                                        -- 模板类型：daily, weekly, monthly, custom

    -- 报表配置
    data_sources JSONB,                                                -- 数据源配置
    metrics_config JSONB,                                              -- 指标配置
    chart_config JSONB,                                                -- 图表配置

    -- 格式设置
    output_format VARCHAR(20),                                         -- 输出格式：pdf, excel, html等
    template_style JSONB,                                              -- 样式配置

    -- 权限控制
    allowed_roles TEXT[],                                              -- 允许的角色
    creator_id BIGINT REFERENCES users(user_id),                      -- 创建者

    is_active BOOLEAN DEFAULT TRUE,                                   -- 是否启用
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 数据库初始化脚本

### 创建数据库和用户

```sql
-- 创建数据库
CREATE DATABASE ai_political_education_db;
CREATE DATABASE ai_political_education_cache;
CREATE DATABASE ai_political_education_logs;

-- 创建用户
CREATE USER ai_edu_user WITH PASSWORD 'your_secure_password';
CREATE USER ai_edu_readonly WITH PASSWORD 'readonly_password';

-- 授权
GRANT CONNECT ON DATABASE ai_political_education_db TO ai_edu_user;
GRANT USAGE ON SCHEMA public TO ai_edu_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ai_edu_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO ai_edu_user;

-- 只读用户权限
GRANT CONNECT ON DATABASE ai_political_education_db TO ai_edu_readonly;
GRANT USAGE ON SCHEMA public TO ai_edu_readonly;
GRANT SELECT ON ALL TABLES IN SCHEMA public TO ai_edu_readonly;
```

### 基础数据初始化

```sql
-- 插入默认配置
INSERT INTO system_configs (config_key, config_value, config_type, description) VALUES
('system_name', '"AI思政教学系统"', 'string', '系统名称'),
('version', '"1.0.0"', 'string', '系统版本'),
('max_file_size', '100', 'number', '最大文件上传大小(MB)'),
('supported_file_types', '["pdf","doc","docx","ppt","pptx","jpg","png","mp4","mp3"]', 'json', '支持的文件类型');

-- 插入默认角色
INSERT INTO user_roles (role_name, role_code, description, permissions) VALUES
('系统管理员', 'admin', '系统超级管理员', '{"all": true}'),
('教师', 'teacher', '授课教师', '{"teaching": true, "resource": true, "student": true}'),
('学生', 'student', '在校学生', '{"learning": true, "assignment": true}'),
('研究者', 'researcher', '教育研究者', '{"research": true, "analytics": true}');

-- 插入资源分类
INSERT INTO resource_categories (category_name, category_code, description) VALUES
('思政教材', 'ideology_textbook', '思想政治课教材资源'),
('教学案例', 'teaching_case', '教学案例分析'),
('时事政治', 'current_affairs', '时事政治材料'),
('历史资料', 'historical_materials', '历史文献资料'),
('多媒体资源', 'multimedia', '图片、音视频资源');
```

---

## 性能优化建议

### 1. 索引优化
- 为经常查询的字段创建索引
- 使用复合索引优化多字段查询
- 定期分析索引使用情况

### 2. 分区策略
```sql
-- 按时间分区的表
CREATE TABLE operation_logs_y2025m01 PARTITION OF operation_logs
FOR VALUES FROM ('2025-01-01') TO ('2025-02-01');

CREATE TABLE assignment_submissions_y2025m01 PARTITION OF assignment_submissions
FOR VALUES FROM ('2025-01-01') TO ('2025-02-01');
```

### 3. 缓存策略
- 热点数据使用Redis缓存
- 统计数据定期更新缓存
- 用户会话信息缓存

### 4. 读写分离
- 主库负责写操作
- 从库负责读操作
- 使用连接池管理数据库连接

---

## 数据备份和恢复

### 1. 备份策略
```bash
# 每日全量备份
pg_dump -h localhost -U ai_edu_user -d ai_political_education_db > backup_$(date +%Y%m%d).sql

# 增量备份（WAL归档）
archive_command = 'cp %p /backup/wal/%f'
```

### 2. 恢复策略
```bash
# 从备份恢复
psql -h localhost -U ai_edu_user -d ai_political_education_db < backup_20251017.sql

# 时间点恢复
pg_basebackup -h localhost -D /backup/base -U ai_edu_user -v -P
```

---

## 数据安全

### 1. 敏感数据加密
- 密码使用bcrypt加密
- API密钥使用AES加密存储
- 个人信息字段加密

### 2. 访问控制
- 基于角色的权限控制（RBAC）
- 数据行级安全策略
- API访问频率限制

### 3. 审计日志
- 记录所有数据修改操作
- 定期审计异常访问
- 数据访问日志分析

---

## 监控和维护

### 1. 性能监控
```sql
-- 慢查询监控
SELECT query, mean_time, calls, total_time
FROM pg_stat_statements
ORDER BY mean_time DESC
LIMIT 10;

-- 连接数监控
SELECT count(*) as active_connections
FROM pg_stat_activity
WHERE state = 'active';
```

### 2. 定期维护
```sql
-- 更新表统计信息
ANALYZE;

-- 重建索引
REINDEX DATABASE ai_political_education_db;

-- 清理无用数据
VACUUM FULL;
```

---

## 总结

这个数据库设计专门针对AI思政教学系统量身定制，具有以下特点：

1. **完整性**：涵盖用户管理、资源管理、备课、授课、评价、AI服务等所有核心功能
2. **扩展性**：支持水平扩展，可适应大规模使用场景
3. **安全性**：多层次的数据安全保护机制
4. **性能**：优化的索引策略和分区设计
5. **智能化**：深度集成AI功能，支持智能教学
6. **实用性**：基于实际教学场景设计，操作便捷

该数据库架构能够有效支撑AI思政教学系统的稳定运行，为师生提供高质量的智能化教学服务。