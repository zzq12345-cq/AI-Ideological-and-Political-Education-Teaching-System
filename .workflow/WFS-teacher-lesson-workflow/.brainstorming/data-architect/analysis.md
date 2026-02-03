# 数据架构师分析 - 教师备课上课流程设计

**角色**: 数据架构师 (Data Architect)
**分析日期**: 2026-02-02
**会话ID**: WFS-teacher-lesson-workflow
**项目**: AI思政智慧课堂系统 - 教师教学工作流模块

---

## 1. 分析摘要

### 1.1 核心设计决策回顾

基于 guidance-specification.md 中确认的决策：

| 决策ID | 类别 | 决策内容 | 设计影响 |
|--------|------|----------|----------|
| D-012 | 数据模型 | 以课程为中心的层次结构 | 核心实体设计基础 |
| D-013 | 画像维度 | 均衡多维度分析（知识+行为+成绩） | 学情数据模型设计 |
| D-014 | 存储策略 | 分层存储（Supabase元数据+本地大文件） | 存储架构设计 |
| D-018 | 文件存储 | 本地文件系统 | 课件/教案存储方案 |

### 1.2 现有系统资产分析

**已存在的数据服务**（可复用）：

| 服务 | 文件路径 | 功能 | 复用策略 |
|------|----------|------|----------|
| PaperService | src/services/PaperService.cpp | 试卷/试题CRUD | 直接复用，扩展关联 |
| SupabaseClient | src/auth/supabase/supabaseclient.cpp | 用户认证 | 直接复用 |
| SupabaseStorageService | src/services/SupabaseStorageService.cpp | 图片上传 | 扩展用于小文件 |
| AnalyticsDataService | src/analytics/AnalyticsDataService.h | 数据分析（Mock） | 重构为真实数据源 |
| DifyService | src/services/DifyService.cpp | AI对话服务 | 直接复用 |

**已存在的数据模型**：

```cpp
// 现有 Paper 结构 (PaperService.h)
struct Paper {
    QString id, title, subject, grade;
    int totalScore, duration;
    QString paperType, description, createdBy;
    QDateTime createdAt, updatedAt;
};

// 现有 PaperQuestion 结构
struct PaperQuestion {
    QString id, paperId, questionType, difficulty;
    QString stem, material, answer, explanation;
    QStringList options, subQuestions, subAnswers, tags;
    QString visibility, subject, grade, chapter;
    QStringList knowledgePoints;
    int score, orderNum;
};
```

### 1.3 需求澄清与补充（Q&A）

| 问题 | 答案 | 类别 | 数据架构影响 |
| --- | --- | --- | --- |
| P0阶段必须可用范围？ | 仅备课工作台（课件/试题/教案），课堂管理与学情分析占位 | Requirements | P0仅启用备课相关表与服务，课堂/学情表作为占位或延后上线 |
| 非实时交互刷新策略？ | 手动+定时结合（默认定时，关键操作后手动触发） | Architecture | 画像/统计采用定时批处理，关键动作后触发刷新 |
| 无学生端的数据采集方案？ | 名单导入 + 二维码签到 + 轻量H5答题 | Requirements | 需要名单表、签到记录与H5答题存储，避免依赖学生端App |
| 发布动作语义？ | 发布=备课完成，可进入授课 | Decisions | Lesson状态中“ready”对应“已发布/可授课” |
| 首批KPI埋点优先级？ | 使用行为类（进入次数、功能使用率） | Requirements | 增加使用行为事件表，优先采集访问与功能使用 |
| 文件版本策略？ | 关键节点版本（发布/导出时） | Risk | 教案/课件在发布与导出时生成版本快照 |

### 1.4 术语一致性与状态映射

| 术语 | 统一含义 | 数据字段/状态 |
| --- | --- | --- |
| 备课工作台 | P0 阶段唯一可用模块（课件/试题/教案） | courses / lessons / lesson_plans / coursewares（试题复用现有题库） |
| 课堂管理 | P1 阶段启用，P0 仅保留入口占位 | quizzes / quiz_questions / attendances / quiz_responses |
| 学情分析 | P2 阶段启用，P0 仅保留入口占位 | student_profiles / class_profiles / learning_events |
| 发布 | 备课完成，可进入授课 | lessons.status = ready |
| 非实时刷新 | 默认定时刷新 + 关键操作后手动触发 | last_calculated_at + profile_refresh_requests（手动/定时） |

### 1.5 本次增强集成（EP-001 ~ EP-006）

| 增强ID | 集成内容 | 影响范围 |
| --- | --- | --- |
| EP-001 | 分阶段表上线清单（P0备课、P1课堂、P2学情） | 表结构规划与上线节奏 |
| EP-002 | 非实时交互刷新策略（定时为主、手动为辅） | 画像计算与刷新队列 |
| EP-003 | 使用行为类KPI埋点 | usage_events 事件表与指标口径 |
| EP-004 | 无学生端采集链路（名单导入 + 二维码签到） | student_rosters / course_students / attendances |
| EP-005 | 轻量H5答题采集 | quiz_responses 与 quizzes/quiz_questions 关联 |
| EP-006 | 关键节点版本策略（发布/导出） | content_versions 与本地版本快照 |

---

## 2. 核心数据模型设计

### 2.1 实体关系概览（ER图描述）

```
                    ┌─────────────┐
                    │   Teacher   │
                    │  (用户表)    │
                    └──────┬──────┘
                           │ 1
                           │ creates
                           ▼ N
                    ┌─────────────┐
                    │   Course    │
                    │   (课程)     │
                    └──────┬──────┘
                           │ 1
                           │ contains
                           ▼ N
                    ┌─────────────┐
        ┌───────────┤   Lesson    ├───────────┐
        │           │   (课时)     │           │
        │           └──────┬──────┘           │
        │ 1                │ 1                │ 1
        ▼ N                ▼ N                ▼ N
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│  LessonPlan   │   │  Courseware   │   │     Quiz      │
│   (教案)      │   │   (课件)      │   │  (课堂测验)   │
└───────────────┘   └───────────────┘   └───────┬───────┘
                                                │ 1
                    ┌───────────────┐           │ contains
                    │  Attendance   │           ▼ N
                    │   (签到)      │   ┌───────────────┐
                    └───────────────┘   │ QuizQuestion  │
                           ▲            │ (测验题目引用) │
                           │            └───────────────┘
                           │                    │
                    ┌──────┴──────┐             │ references
                    │   Lesson    ├─────────────┤
                    └──────┬──────┘             ▼
                           │            ┌───────────────┐
                           │ 1          │   Question    │
                           ▼ N          │  (题库题目)   │
                    ┌───────────────┐   │ [现有Paper-   │
                    │   Homework    │   │  Question]    │
                    │   (作业)      │   └───────────────┘
                    └───────┬───────┘
                            │ 1
                            ▼ N
                    ┌───────────────┐
                    │  Submission   │
                    │  (作业提交)   │
                    └───────────────┘
```

**阶段说明**：以上ER为全量规划。P0 仅启用备课工作台相关数据（课程、课时、教案、课件与题库复用），课堂管理与学情分析仅保留入口占位与数据模型规划；P1 启用课堂管理数据链路；P2 启用学情分析画像计算。

**补充关系**：教师名单导入 → 学生名单与课程成员关联；二维码签到与轻量H5答题形成课堂数据；使用行为埋点用于KPI；发布/导出触发内容版本快照。

### 2.1.1 补充实体与采集链路（分阶段启用）

- **学生名单链路（P1）**：教师导入名单 → `student_rosters` → `course_students` 形成课程成员集合（不依赖学生端App）。
- **轻量H5答题链路（P1）**：二维码/链接进入 → `quiz_responses` 记录答题结果，用于课堂统计与后续学情分析。
- **行为埋点链路（P0）**：备课工作台使用行为 → `usage_events`，优先覆盖进入次数与功能使用率。
- **版本快照链路（P0）**：发布/导出触发 → `content_versions` 保存关键节点版本。
- **非实时刷新链路（P2）**：定时任务/手动触发 → `profile_refresh_requests` + `last_calculated_at` 记录刷新请求与结果时间。

### 2.2 核心实体定义

#### 2.2.1 Course（课程）

```cpp
struct Course {
    QString id;              // UUID
    QString teacherId;       // 关联教师
    QString name;            // 课程名称（如"高中政治必修一"）
    QString subject;         // 科目（思想政治）
    QString grade;           // 年级（高一、高二、高三）
    QString semester;        // 学期（2025秋、2026春）
    QString description;     // 课程描述
    QString coverImage;      // 封面图片路径
    QString status;          // 状态（draft/active/archived）
    int studentCount;        // 学生人数
    QDateTime createdAt;
    QDateTime updatedAt;
};
```

#### 2.2.2 Lesson（课时）

```cpp
struct Lesson {
    QString id;              // UUID
    QString courseId;        // 关联课程
    QString title;           // 课时标题（如"第一课 社会主义从空想到科学"）
    QString chapter;         // 所属章节
    int orderNum;            // 排序序号
    int plannedDuration;     // 计划时长（分钟）
    int actualDuration;      // 实际时长
    QString status;          // 状态（planning/ready=已发布可授课/in_progress/completed）
    QDateTime scheduledAt;   // 计划上课时间
    QDateTime startedAt;     // 实际开始时间
    QDateTime endedAt;       // 实际结束时间
    QDateTime createdAt;
    QDateTime updatedAt;
};
```

#### 2.2.3 LessonPlan（教案）

```cpp
struct LessonPlan {
    QString id;              // UUID
    QString lessonId;        // 关联课时
    QString title;           // 教案标题
    QString objectives;      // 教学目标（JSON格式存储多条）
    QString keyPoints;       // 教学重点
    QString difficulties;    // 教学难点
    QString teachingMethods; // 教学方法
    QString outline;         // 教学大纲（Markdown）
    QString activities;      // 教学活动设计（JSON数组）
    QString materials;       // 教学资源清单（JSON数组）
    QString evaluation;      // 评价方式
    QString reflection;      // 课后反思（课后填写）
    QString localFilePath;   // 本地文件路径（.docx/.md）
    QString version;         // 版本号
    QDateTime createdAt;
    QDateTime updatedAt;
};
```

#### 2.2.4 Courseware（课件）

```cpp
struct Courseware {
    QString id;              // UUID
    QString lessonId;        // 关联课时
    QString title;           // 课件标题
    QString type;            // 类型（pptx/pdf/video/image）
    QString source;          // 来源（manual/ai_generated/imported）
    QString localFilePath;   // 本地文件路径
    QString remoteUrl;       // 远程URL（如讯飞PPT生成结果）
    qint64 fileSize;         // 文件大小（字节）
    QString thumbnail;       // 缩略图路径
    int pageCount;           // 页数（PPT）
    int duration;            // 时长（视频，秒）
    QString metadata;        // 元数据（JSON，如PPT大纲）
    QDateTime createdAt;
    QDateTime updatedAt;
};
```

#### 2.2.5 Quiz（课堂测验）

```cpp
struct Quiz {
    QString id;              // UUID
    QString lessonId;        // 关联课时
    QString title;           // 测验标题
    QString type;            // 类型（warmup/practice/assessment）
    int totalScore;          // 总分
    int timeLimit;           // 时限（秒，0表示不限时）
    QString status;          // 状态（draft/active/closed）
    QDateTime startedAt;     // 开始时间
    QDateTime endedAt;       // 结束时间
    QDateTime createdAt;
    QDateTime updatedAt;
};
```

#### 2.2.6 QuizQuestion（测验题目关联）

```cpp
struct QuizQuestion {
    QString id;              // UUID
    QString quizId;          // 关联测验
    QString questionId;      // 关联题库题目（复用现有PaperQuestion）
    int orderNum;            // 排序
    int score;               // 本题分值（可覆盖原始分值）
};
```

#### 2.2.7 Attendance（签到记录）

```cpp
struct Attendance {
    QString id;              // UUID
    QString lessonId;        // 关联课时
    QString studentId;       // 学生ID
    QString studentName;     // 学生姓名
    QString status;          // 状态（present/late/absent/leave）
    QString method;          // 签到方式（qrcode/manual/auto）
    QDateTime checkinTime;   // 签到时间
    QString deviceInfo;      // 设备信息
    double latitude;         // 纬度（可选）
    double longitude;        // 经度（可选）
    QDateTime createdAt;
};
```

#### 2.2.8 Homework（作业）

```cpp
struct Homework {
    QString id;              // UUID
    QString lessonId;        // 关联课时
    QString title;           // 作业标题
    QString description;     // 作业说明
    QString type;            // 类型（paper/essay/project）
    QString paperId;         // 关联试卷（如果是做题型）
    QString content;         // 作业内容（Markdown）
    QString attachments;     // 附件列表（JSON数组）
    QDateTime deadline;      // 截止时间
    int totalScore;          // 总分
    QString status;          // 状态（draft/published/closed）
    QDateTime createdAt;
    QDateTime updatedAt;
};
```

#### 2.2.9 Submission（作业提交）

```cpp
struct Submission {
    QString id;              // UUID
    QString homeworkId;      // 关联作业
    QString studentId;       // 学生ID
    QString studentName;     // 学生姓名
    QString content;         // 提交内容（Markdown）
    QString attachments;     // 附件列表（JSON数组）
    double score;            // 得分
    QString feedback;        // 教师反馈
    QString status;          // 状态（submitted/graded/returned）
    QDateTime submittedAt;   // 提交时间
    QDateTime gradedAt;      // 批改时间
    QDateTime createdAt;
};
```

---

## 3. Supabase 表结构设计

### 3.0 分阶段表上线清单（EP-001）

| 阶段 | 业务范围 | 新建/启用表 | 说明 |
| --- | --- | --- | --- |
| P0 备课 | 备课工作台（课件/试题/教案） | courses, lessons, lesson_plans, coursewares, usage_events, content_versions | 试题复用现有 papers/questions；课堂与学情表仅做结构规划 |
| P1 课堂 | 签到 + 课堂测验 + 轻量H5答题 | student_rosters, course_students, quizzes, quiz_questions, attendances, quiz_responses | 无学生端采集链路落地 |
| P2 课后与学情分析 | 作业/画像/统计 | homeworks, submissions, learning_events, student_profiles, class_profiles, profile_refresh_requests | 非实时计算与批量刷新 |

> 说明：表结构 SQL 为全量规划，P0 仅创建/启用 P0 列表，其余按阶段上线。

### 3.1 表结构 SQL

```sql
-- =====================================================
-- 课程表
-- =====================================================
CREATE TABLE courses (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    teacher_id UUID NOT NULL REFERENCES auth.users(id),
    name VARCHAR(100) NOT NULL,
    subject VARCHAR(50) DEFAULT '思想政治',
    grade VARCHAR(20),
    semester VARCHAR(20),
    description TEXT,
    cover_image VARCHAR(500),
    status VARCHAR(20) DEFAULT 'draft' CHECK (status IN ('draft', 'active', 'archived')),
    student_count INTEGER DEFAULT 0,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_courses_teacher ON courses(teacher_id);
CREATE INDEX idx_courses_status ON courses(status);

-- =====================================================
-- 课时表
-- =====================================================
CREATE TABLE lessons (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    course_id UUID NOT NULL REFERENCES courses(id) ON DELETE CASCADE,
    title VARCHAR(200) NOT NULL,
    chapter VARCHAR(100),
    order_num INTEGER DEFAULT 0,
    planned_duration INTEGER DEFAULT 45,
    actual_duration INTEGER,
    status VARCHAR(20) DEFAULT 'planning'
        CHECK (status IN ('planning', 'ready', 'in_progress', 'completed')),
    published_at TIMESTAMPTZ, -- 备课发布（ready）时间
    scheduled_at TIMESTAMPTZ,
    started_at TIMESTAMPTZ,
    ended_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_lessons_course ON lessons(course_id);
CREATE INDEX idx_lessons_status ON lessons(status);
CREATE INDEX idx_lessons_scheduled ON lessons(scheduled_at);

-- =====================================================
-- 教案表
-- =====================================================
CREATE TABLE lesson_plans (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    lesson_id UUID NOT NULL REFERENCES lessons(id) ON DELETE CASCADE,
    title VARCHAR(200),
    objectives JSONB DEFAULT '[]'::jsonb,
    key_points TEXT,
    difficulties TEXT,
    teaching_methods TEXT,
    outline TEXT,
    activities JSONB DEFAULT '[]'::jsonb,
    materials JSONB DEFAULT '[]'::jsonb,
    evaluation TEXT,
    reflection TEXT,
    local_file_path VARCHAR(500),
    version VARCHAR(20) DEFAULT '1.0',
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_lesson_plans_lesson ON lesson_plans(lesson_id);

-- =====================================================
-- 课件表
-- =====================================================
CREATE TABLE coursewares (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    lesson_id UUID NOT NULL REFERENCES lessons(id) ON DELETE CASCADE,
    title VARCHAR(200) NOT NULL,
    type VARCHAR(20) CHECK (type IN ('pptx', 'pdf', 'video', 'image', 'other')),
    source VARCHAR(20) DEFAULT 'manual'
        CHECK (source IN ('manual', 'ai_generated', 'imported')),
    local_file_path VARCHAR(500),
    remote_url VARCHAR(1000),
    file_size BIGINT DEFAULT 0,
    thumbnail VARCHAR(500),
    page_count INTEGER,
    duration INTEGER,
    metadata JSONB DEFAULT '{}'::jsonb,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- =====================================================
-- 备课内容版本快照（EP-006）
-- =====================================================
CREATE TABLE content_versions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    lesson_id UUID NOT NULL REFERENCES lessons(id) ON DELETE CASCADE,
    content_type VARCHAR(20) NOT NULL CHECK (content_type IN ('lesson_plan', 'courseware')),
    content_id UUID NOT NULL,
    version_label VARCHAR(50) NOT NULL,               -- 如 v1.0 / v1.1
    snapshot_meta JSONB DEFAULT '{}'::jsonb,          -- 元数据快照
    local_file_path VARCHAR(500),
    created_by UUID REFERENCES auth.users(id),
    created_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(content_type, content_id, version_label)
);

CREATE INDEX idx_content_versions_lesson ON content_versions(lesson_id);
CREATE INDEX idx_content_versions_content ON content_versions(content_type, content_id);

CREATE INDEX idx_coursewares_lesson ON coursewares(lesson_id);
CREATE INDEX idx_coursewares_type ON coursewares(type);

-- =====================================================
-- 画像刷新请求（EP-002，非实时刷新）
-- =====================================================
CREATE TABLE profile_refresh_requests (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    scope VARCHAR(20) NOT NULL CHECK (scope IN ('student', 'class')),
    course_id UUID NOT NULL REFERENCES courses(id),
    student_id VARCHAR(100),
    trigger_type VARCHAR(20) NOT NULL CHECK (trigger_type IN ('manual', 'scheduled', 'post_action')),
    reason VARCHAR(100),
    status VARCHAR(20) DEFAULT 'queued' CHECK (status IN ('queued', 'processing', 'done', 'failed')),
    requested_by UUID REFERENCES auth.users(id),
    requested_at TIMESTAMPTZ DEFAULT NOW(),
    processed_at TIMESTAMPTZ
);

CREATE INDEX idx_profile_refresh_course ON profile_refresh_requests(course_id);
CREATE INDEX idx_profile_refresh_status ON profile_refresh_requests(status);
CREATE INDEX idx_profile_refresh_student ON profile_refresh_requests(student_id);

-- =====================================================
-- 使用行为事件（EP-003）
-- =====================================================
CREATE TABLE usage_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    teacher_id UUID NOT NULL REFERENCES auth.users(id),
    course_id UUID REFERENCES courses(id),
    lesson_id UUID REFERENCES lessons(id),
    event_type VARCHAR(50) NOT NULL,                 -- enter_workspace/use_feature/refresh_trigger
    event_source VARCHAR(30) DEFAULT 'desktop',       -- desktop/h5
    event_meta JSONB DEFAULT '{}'::jsonb,             -- 功能名称/入口等
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_usage_events_teacher ON usage_events(teacher_id);
CREATE INDEX idx_usage_events_course ON usage_events(course_id);
CREATE INDEX idx_usage_events_type ON usage_events(event_type);

-- =====================================================
-- 名单导入与课程成员（EP-004）
-- =====================================================
CREATE TABLE student_rosters (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    teacher_id UUID NOT NULL REFERENCES auth.users(id),
    roster_name VARCHAR(100) NOT NULL,
    source VARCHAR(20) DEFAULT 'import',             -- import/manual
    total_count INTEGER DEFAULT 0,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE course_students (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    course_id UUID NOT NULL REFERENCES courses(id) ON DELETE CASCADE,
    student_id VARCHAR(100) NOT NULL,
    student_name VARCHAR(50),
    roster_id UUID REFERENCES student_rosters(id),
    status VARCHAR(20) DEFAULT 'active' CHECK (status IN ('active', 'inactive')),
    created_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(course_id, student_id)
);

CREATE INDEX idx_course_students_course ON course_students(course_id);
CREATE INDEX idx_course_students_roster ON course_students(roster_id);

-- =====================================================
-- 轻量H5答题响应（EP-005）
-- =====================================================
CREATE TABLE quiz_responses (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    quiz_id UUID NOT NULL REFERENCES quizzes(id) ON DELETE CASCADE,
    question_id UUID NOT NULL REFERENCES questions(id) ON DELETE CASCADE,
    student_id VARCHAR(100) NOT NULL,
    student_name VARCHAR(50),
    answer JSONB DEFAULT '{}'::jsonb,
    is_correct BOOLEAN,
    score DECIMAL(5, 2),
    submitted_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_quiz_responses_quiz ON quiz_responses(quiz_id);
CREATE INDEX idx_quiz_responses_student ON quiz_responses(student_id);

-- =====================================================
-- 课堂测验表
-- =====================================================
CREATE TABLE quizzes (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    lesson_id UUID NOT NULL REFERENCES lessons(id) ON DELETE CASCADE,
    title VARCHAR(200) NOT NULL,
    type VARCHAR(20) DEFAULT 'practice'
        CHECK (type IN ('warmup', 'practice', 'assessment')),
    total_score INTEGER DEFAULT 100,
    time_limit INTEGER DEFAULT 0,
    status VARCHAR(20) DEFAULT 'draft'
        CHECK (status IN ('draft', 'active', 'closed')),
    started_at TIMESTAMPTZ,
    ended_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_quizzes_lesson ON quizzes(lesson_id);
CREATE INDEX idx_quizzes_status ON quizzes(status);

-- =====================================================
-- 测验题目关联表
-- =====================================================
CREATE TABLE quiz_questions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    quiz_id UUID NOT NULL REFERENCES quizzes(id) ON DELETE CASCADE,
    question_id UUID NOT NULL REFERENCES questions(id) ON DELETE CASCADE,
    order_num INTEGER DEFAULT 0,
    score INTEGER DEFAULT 5,
    UNIQUE(quiz_id, question_id)
);

CREATE INDEX idx_quiz_questions_quiz ON quiz_questions(quiz_id);

-- =====================================================
-- 签到记录表
-- =====================================================
CREATE TABLE attendances (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    lesson_id UUID NOT NULL REFERENCES lessons(id) ON DELETE CASCADE,
    student_id VARCHAR(100) NOT NULL,
    student_name VARCHAR(50),
    status VARCHAR(20) DEFAULT 'present'
        CHECK (status IN ('present', 'late', 'absent', 'leave')),
    method VARCHAR(20) DEFAULT 'manual'
        CHECK (method IN ('qrcode', 'manual', 'auto')),
    checkin_time TIMESTAMPTZ DEFAULT NOW(),
    device_info VARCHAR(200),
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_attendances_lesson ON attendances(lesson_id);
CREATE INDEX idx_attendances_student ON attendances(student_id);
CREATE INDEX idx_attendances_status ON attendances(status);

-- =====================================================
-- 作业表
-- =====================================================
CREATE TABLE homeworks (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    lesson_id UUID NOT NULL REFERENCES lessons(id) ON DELETE CASCADE,
    title VARCHAR(200) NOT NULL,
    description TEXT,
    type VARCHAR(20) DEFAULT 'paper'
        CHECK (type IN ('paper', 'essay', 'project')),
    paper_id UUID REFERENCES papers(id),
    content TEXT,
    attachments JSONB DEFAULT '[]'::jsonb,
    deadline TIMESTAMPTZ,
    total_score INTEGER DEFAULT 100,
    status VARCHAR(20) DEFAULT 'draft'
        CHECK (status IN ('draft', 'published', 'closed')),
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_homeworks_lesson ON homeworks(lesson_id);
CREATE INDEX idx_homeworks_status ON homeworks(status);
CREATE INDEX idx_homeworks_deadline ON homeworks(deadline);

-- =====================================================
-- 作业提交表
-- =====================================================
CREATE TABLE submissions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    homework_id UUID NOT NULL REFERENCES homeworks(id) ON DELETE CASCADE,
    student_id VARCHAR(100) NOT NULL,
    student_name VARCHAR(50),
    content TEXT,
    attachments JSONB DEFAULT '[]'::jsonb,
    score DECIMAL(5, 2),
    feedback TEXT,
    status VARCHAR(20) DEFAULT 'submitted'
        CHECK (status IN ('submitted', 'graded', 'returned')),
    submitted_at TIMESTAMPTZ DEFAULT NOW(),
    graded_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(homework_id, student_id)
);

CREATE INDEX idx_submissions_homework ON submissions(homework_id);
CREATE INDEX idx_submissions_student ON submissions(student_id);
CREATE INDEX idx_submissions_status ON submissions(status);

-- =====================================================
-- 更新时间触发器
-- =====================================================
CREATE OR REPLACE FUNCTION update_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER courses_updated_at
    BEFORE UPDATE ON courses FOR EACH ROW EXECUTE FUNCTION update_updated_at();
CREATE TRIGGER lessons_updated_at
    BEFORE UPDATE ON lessons FOR EACH ROW EXECUTE FUNCTION update_updated_at();
CREATE TRIGGER lesson_plans_updated_at
    BEFORE UPDATE ON lesson_plans FOR EACH ROW EXECUTE FUNCTION update_updated_at();
CREATE TRIGGER coursewares_updated_at
    BEFORE UPDATE ON coursewares FOR EACH ROW EXECUTE FUNCTION update_updated_at();
CREATE TRIGGER quizzes_updated_at
    BEFORE UPDATE ON quizzes FOR EACH ROW EXECUTE FUNCTION update_updated_at();
CREATE TRIGGER homeworks_updated_at
    BEFORE UPDATE ON homeworks FOR EACH ROW EXECUTE FUNCTION update_updated_at();

-- =====================================================
-- RLS 策略（基于教师ID）
-- =====================================================
ALTER TABLE courses ENABLE ROW LEVEL SECURITY;
ALTER TABLE lessons ENABLE ROW LEVEL SECURITY;
ALTER TABLE lesson_plans ENABLE ROW LEVEL SECURITY;
ALTER TABLE coursewares ENABLE ROW LEVEL SECURITY;
ALTER TABLE quizzes ENABLE ROW LEVEL SECURITY;
ALTER TABLE attendances ENABLE ROW LEVEL SECURITY;
ALTER TABLE homeworks ENABLE ROW LEVEL SECURITY;
ALTER TABLE submissions ENABLE ROW LEVEL SECURITY;

-- 课程：教师只能看自己的课程
CREATE POLICY courses_teacher_policy ON courses
    FOR ALL USING (teacher_id = auth.uid());

-- 课时：通过课程关联验证
CREATE POLICY lessons_teacher_policy ON lessons
    FOR ALL USING (
        course_id IN (SELECT id FROM courses WHERE teacher_id = auth.uid())
    );

-- 其他表类似，通过 lesson_id 关联验证...

ALTER TABLE usage_events ENABLE ROW LEVEL SECURITY;
ALTER TABLE student_rosters ENABLE ROW LEVEL SECURITY;
ALTER TABLE course_students ENABLE ROW LEVEL SECURITY;
ALTER TABLE quiz_responses ENABLE ROW LEVEL SECURITY;
ALTER TABLE content_versions ENABLE ROW LEVEL SECURITY;
ALTER TABLE profile_refresh_requests ENABLE ROW LEVEL SECURITY;

-- 使用行为：仅本人
CREATE POLICY usage_events_teacher_policy ON usage_events
    FOR ALL USING (teacher_id = auth.uid());

-- 画像刷新请求：通过课程关联验证
CREATE POLICY profile_refresh_requests_teacher_policy ON profile_refresh_requests
    FOR ALL USING (
        course_id IN (SELECT id FROM courses WHERE teacher_id = auth.uid())
    );

-- 名单：仅本人
CREATE POLICY student_rosters_teacher_policy ON student_rosters
    FOR ALL USING (teacher_id = auth.uid());

-- 课程成员：通过课程关联验证
CREATE POLICY course_students_teacher_policy ON course_students
    FOR ALL USING (
        course_id IN (SELECT id FROM courses WHERE teacher_id = auth.uid())
    );

-- H5答题：通过测验关联验证
CREATE POLICY quiz_responses_teacher_policy ON quiz_responses
    FOR ALL USING (
        quiz_id IN (SELECT id FROM quizzes WHERE lesson_id IN (
            SELECT id FROM lessons WHERE course_id IN (SELECT id FROM courses WHERE teacher_id = auth.uid())
        ))
    );

-- 版本快照：通过课时关联验证
CREATE POLICY content_versions_teacher_policy ON content_versions
    FOR ALL USING (
        lesson_id IN (SELECT id FROM lessons WHERE course_id IN (SELECT id FROM courses WHERE teacher_id = auth.uid()))
    );
```

### 3.2 与现有表的关系

现有表（保持不变）：
- `papers` - 试卷表
- `questions` - 题库题目表（现有 PaperQuestion 对应）

新增关联：
- `quizzes.quiz_questions` 通过 `question_id` 关联现有 `questions` 表
- `homeworks.paper_id` 通过 `paper_id` 关联现有 `papers` 表

---

## 4. 本地文件存储方案

### 4.1 目录结构

```
~/.ai-sizheng-classroom/
├── courses/
│   └── {course_id}/
│       ├── cover.jpg                    # 课程封面
│       └── lessons/
│           └── {lesson_id}/
│               ├── plan/
│               │   ├── lesson_plan.md   # 教案文件
│               │   └── lesson_plan.docx
│               ├── coursewares/
│               │   ├── slides.pptx      # 课件文件
│               │   ├── slides.pdf
│               │   └── thumbnails/
│               │       ├── page_01.png
│               │       └── page_02.png
│               ├── recordings/
│               │   └── class_recording.mp4
│               └── resources/
│                   ├── images/
│                   └── documents/
├── exports/
│   ├── papers/                          # 导出的试卷
│   └── reports/                         # 导出的报告
├── temp/                                # 临时文件
│   └── ai_generation/                   # AI生成中的文件
└── cache/
    └── thumbnails/                      # 缩略图缓存
```

### 4.2 文件存储服务设计

```cpp
// LocalStorageService.h
class LocalStorageService : public QObject
{
    Q_OBJECT

public:
    static LocalStorageService* instance();

    // 路径管理
    QString getBasePath() const;
    QString getCoursePath(const QString &courseId) const;
    QString getLessonPath(const QString &courseId, const QString &lessonId) const;
    QString getCoursewarePath(const QString &courseId, const QString &lessonId) const;
    QString getPlanPath(const QString &courseId, const QString &lessonId) const;

    // 文件操作
    QString saveFile(const QByteArray &data, const QString &targetPath);
    QString copyFile(const QString &sourcePath, const QString &targetDir);
    bool deleteFile(const QString &filePath);
    bool fileExists(const QString &filePath) const;
    qint64 getFileSize(const QString &filePath) const;

    // 缩略图生成
    QString generateThumbnail(const QString &sourcePath, const QString &outputDir);

    // 清理
    void cleanTempFiles();
    void cleanOrphanedFiles(const QStringList &validPaths);

signals:
    void fileOperationProgress(int current, int total);
    void fileOperationError(const QString &error);

private:
    void ensureDirectoryExists(const QString &path);
    QString m_basePath;
};
```

### 4.3 大文件处理策略

| 文件类型 | 最大大小 | 存储位置 | 元数据存储 |
|----------|----------|----------|------------|
| 课件PPT | 100MB | 本地 | Supabase coursewares |
| 教案文档 | 20MB | 本地 | Supabase lesson_plans |
| 视频录制 | 2GB | 本地 | Supabase coursewares |
| 图片资源 | 10MB | Supabase Storage | Supabase coursewares |
| 题目图片 | 5MB | Supabase Storage | 现有 question.options |

---

## 5. 学情画像数据模型

### 5.1 学情画像维度设计

基于 guidance-specification.md 确认的"均衡多维度分析"策略：

```sql
-- =====================================================
-- 学情画像表
-- =====================================================
CREATE TABLE student_profiles (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    student_id VARCHAR(100) NOT NULL,
    course_id UUID NOT NULL REFERENCES courses(id),

    -- 知识维度指标
    knowledge_mastery DECIMAL(5, 2) DEFAULT 0,        -- 知识点掌握度 (0-100)
    weak_points JSONB DEFAULT '[]'::jsonb,            -- 薄弱知识点列表
    strong_points JSONB DEFAULT '[]'::jsonb,          -- 优势知识点列表
    learning_progress DECIMAL(5, 2) DEFAULT 0,        -- 学习进度 (0-100)

    -- 行为维度指标
    attendance_rate DECIMAL(5, 2) DEFAULT 0,          -- 出勤率 (0-100)
    participation_score DECIMAL(5, 2) DEFAULT 0,      -- 课堂参与度 (0-100)
    homework_completion_rate DECIMAL(5, 2) DEFAULT 0, -- 作业完成率 (0-100)
    avg_submission_time INTERVAL,                     -- 平均提交时间（距离截止）

    -- 成绩维度指标
    quiz_avg_score DECIMAL(5, 2) DEFAULT 0,           -- 测验平均分
    homework_avg_score DECIMAL(5, 2) DEFAULT 0,       -- 作业平均分
    exam_score DECIMAL(5, 2),                         -- 考试成绩
    composite_score DECIMAL(5, 2) DEFAULT 0,          -- 综合评分

    -- 趋势数据
    score_trend JSONB DEFAULT '[]'::jsonb,            -- 成绩趋势 [{date, score}]
    participation_trend JSONB DEFAULT '[]'::jsonb,    -- 参与趋势

    -- 权重配置（教师可自定义）
    weight_config JSONB DEFAULT '{
        "knowledge": 0.4,
        "behavior": 0.3,
        "achievement": 0.3
    }'::jsonb,

    -- 更新时间
    last_calculated_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),

    UNIQUE(student_id, course_id)
);

CREATE INDEX idx_student_profiles_student ON student_profiles(student_id);
CREATE INDEX idx_student_profiles_course ON student_profiles(course_id);

-- =====================================================
-- 班级画像表
-- =====================================================
CREATE TABLE class_profiles (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    course_id UUID NOT NULL REFERENCES courses(id),

    -- 班级整体指标
    student_count INTEGER DEFAULT 0,
    avg_attendance_rate DECIMAL(5, 2) DEFAULT 0,
    avg_participation DECIMAL(5, 2) DEFAULT 0,
    avg_homework_completion DECIMAL(5, 2) DEFAULT 0,
    avg_quiz_score DECIMAL(5, 2) DEFAULT 0,
    avg_composite_score DECIMAL(5, 2) DEFAULT 0,

    -- 成绩分布
    grade_distribution JSONB DEFAULT '{
        "excellent": 0,
        "good": 0,
        "pass": 0,
        "fail": 0
    }'::jsonb,

    -- 知识点分析
    class_weak_points JSONB DEFAULT '[]'::jsonb,      -- 班级薄弱知识点
    class_strong_points JSONB DEFAULT '[]'::jsonb,    -- 班级优势知识点

    -- 趋势数据
    performance_trend JSONB DEFAULT '[]'::jsonb,

    last_calculated_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),

    UNIQUE(course_id)
);

CREATE INDEX idx_class_profiles_course ON class_profiles(course_id);

-- =====================================================
-- 学情事件记录表（用于实时计算）
-- =====================================================
CREATE TABLE learning_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    student_id VARCHAR(100) NOT NULL,
    course_id UUID NOT NULL REFERENCES courses(id),
    lesson_id UUID REFERENCES lessons(id),

    event_type VARCHAR(30) NOT NULL,                  -- attendance/quiz_answer/homework_submit/...
    event_data JSONB DEFAULT '{}'::jsonb,             -- 事件详细数据

    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_learning_events_student ON learning_events(student_id);
CREATE INDEX idx_learning_events_course ON learning_events(course_id);
CREATE INDEX idx_learning_events_type ON learning_events(event_type);
CREATE INDEX idx_learning_events_time ON learning_events(created_at);
```

### 5.2 学情指标计算逻辑

```cpp
// StudentProfileCalculator.h
class StudentProfileCalculator : public QObject
{
    Q_OBJECT

public:
    struct ProfileMetrics {
        // 知识维度
        double knowledgeMastery;
        QStringList weakPoints;
        QStringList strongPoints;
        double learningProgress;

        // 行为维度
        double attendanceRate;
        double participationScore;
        double homeworkCompletionRate;

        // 成绩维度
        double quizAvgScore;
        double homeworkAvgScore;
        double compositeScore;
    };

    struct WeightConfig {
        double knowledge = 0.4;
        double behavior = 0.3;
        double achievement = 0.3;
    };

    // 计算单个学生画像
    ProfileMetrics calculateStudentProfile(
        const QString &studentId,
        const QString &courseId,
        const WeightConfig &weights = WeightConfig()
    );

    // 计算班级画像
    ClassProfileMetrics calculateClassProfile(const QString &courseId);

private:
    // 知识维度计算
    double calculateKnowledgeMastery(
        const QList<QuizAnswer> &quizAnswers,
        const QList<Submission> &submissions
    );

    QStringList identifyWeakPoints(
        const QList<QuizAnswer> &quizAnswers,
        double threshold = 0.6
    );

    // 行为维度计算
    double calculateAttendanceRate(
        const QString &studentId,
        const QString &courseId
    );

    double calculateParticipation(
        const QString &studentId,
        const QString &courseId
    );

    // 成绩维度计算
    double calculateQuizAverage(
        const QString &studentId,
        const QString &courseId
    );

    // 综合评分计算
    double calculateCompositeScore(
        const ProfileMetrics &metrics,
        const WeightConfig &weights
    );
};
```

### 5.3 计算公式

**综合评分计算**：
```
composite_score =
    weight_knowledge * knowledge_score +
    weight_behavior * behavior_score +
    weight_achievement * achievement_score

其中：
knowledge_score = knowledge_mastery * 0.6 + learning_progress * 0.4
behavior_score = attendance_rate * 0.3 + participation * 0.4 + homework_completion * 0.3
achievement_score = quiz_avg * 0.5 + homework_avg * 0.5
```

**知识点掌握度计算**：
```
knowledge_mastery =
    SUM(正确回答的知识点权重) / SUM(涉及的知识点总权重) * 100

薄弱知识点识别：
correct_rate < 60% 的知识点标记为薄弱
```

---

## 6. 与现有系统集成

### 6.1 服务层架构

```
┌─────────────────────────────────────────────────────────┐
│                     UI Layer                             │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐│
│  │ 备课工作台  │ │ 课堂管理   │ │ 学情分析            ││
│  └──────┬──────┘ └──────┬──────┘ └──────────┬──────────┘│
└─────────┼───────────────┼───────────────────┼───────────┘
          │               │                   │
          ▼               ▼                   ▼
┌─────────────────────────────────────────────────────────┐
│                   Service Layer                          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐│
│  │CourseService│ │ClassService │ │AnalyticsService     ││
│  │ (新增)      │ │ (新增)      │ │ (重构现有)          ││
│  └──────┬──────┘ └──────┬──────┘ └──────────┬──────────┘│
│         │               │                   │            │
│  ┌──────┴───────────────┴───────────────────┴──────────┐│
│  │                 PaperService (复用)                  ││
│  └──────────────────────────────────────────────────────┘│
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                   Data Layer                             │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐│
│  │SupabaseAPI  │ │LocalStorage │ │SupabaseStorage      ││
│  │ (REST)      │ │ (文件系统)  │ │ (图片/小文件)       ││
│  └─────────────┘ └─────────────┘ └─────────────────────┘│
└─────────────────────────────────────────────────────────┘
```

### 6.2 新增服务清单

| 服务名称 | 文件 | 职责 |
|----------|------|------|
| CourseService | src/services/CourseService.h/cpp | 课程、课时、教案、课件CRUD |
| ClassroomService | src/services/ClassroomService.h/cpp | 签到、测验、实时课堂管理 |
| HomeworkService | src/services/HomeworkService.h/cpp | 作业布置、提交、批改 |
| StudentProfileService | src/services/StudentProfileService.h/cpp | 学情画像计算与查询 |
| LocalStorageService | src/services/LocalStorageService.h/cpp | 本地文件管理 |

### 6.3 现有服务修改

| 服务 | 修改内容 |
|------|----------|
| PaperService | 无需修改，通过 quiz_questions 表关联复用 |
| AnalyticsDataService | 重构为真实数据源，对接 StudentProfileService |
| SupabaseStorageService | 扩展支持更多文件类型 |

---

## 7. 数据迁移与兼容策略

### 7.1 迁移步骤

1. **Phase 1 - 表结构创建**
   - 在 Supabase 控制台执行 SQL 创建新表
   - 添加必要的索引和 RLS 策略
   - 验证表结构和关系

2. **Phase 2 - 服务层开发**
   - 实现 CourseService 等新服务
   - 保持现有 PaperService 不变
   - 添加兼容层确保旧功能正常

3. **Phase 3 - 数据初始化**
   - 为现有教师创建默认课程
   - 将现有试卷关联到课程
   - 初始化学情画像空记录

4. **Phase 4 - UI集成**
   - 逐步将新服务接入 UI
   - 保持旧入口可用（渐进式迁移）

### 7.2 兼容性保证

```cpp
// 兼容层示例
class LegacyCompatibilityService
{
public:
    // 为旧版试卷创建课程关联
    void migratePaperToCourse(const QString &paperId);

    // 从旧版分析数据迁移
    void migrateAnalyticsData();

    // 检查迁移状态
    bool isMigrationComplete() const;
};
```

---

## 8. 性能与扩展性考虑

### 8.1 查询优化

**常见查询场景及索引**：

| 场景 | 查询 | 索引 |
|------|------|------|
| 教师课程列表 | WHERE teacher_id = ? | idx_courses_teacher |
| 课时列表 | WHERE course_id = ? ORDER BY order_num | idx_lessons_course |
| 今日课程 | WHERE scheduled_at::date = CURRENT_DATE | idx_lessons_scheduled |
| 学生签到查询 | WHERE lesson_id = ? AND student_id = ? | idx_attendances_lesson, idx_attendances_student |
| 作业提交状态 | WHERE homework_id = ? AND status = ? | idx_submissions_homework, idx_submissions_status |

### 8.2 缓存策略

```cpp
// 本地缓存
class DataCache {
public:
    // 课程列表缓存（TTL: 5分钟）
    QList<Course> getCourses(const QString &teacherId);
    void invalidateCourses(const QString &teacherId);

    // 学情画像缓存（TTL: 10分钟）
    StudentProfile getStudentProfile(const QString &studentId, const QString &courseId);
    void invalidateProfile(const QString &studentId);

private:
    QCache<QString, QVariant> m_cache;
    QHash<QString, QDateTime> m_ttl;
};
```

### 8.3 后续扩展预留

**对象存储接口**（当前使用本地，预留云存储）：

```cpp
// StorageProvider 抽象接口
class IStorageProvider {
public:
    virtual QString upload(const QByteArray &data, const QString &path) = 0;
    virtual QByteArray download(const QString &path) = 0;
    virtual bool remove(const QString &path) = 0;
    virtual QString getUrl(const QString &path) = 0;
};

// 本地实现
class LocalStorageProvider : public IStorageProvider { ... };

// 未来云存储实现
class S3StorageProvider : public IStorageProvider { ... };
class AliyunOSSStorageProvider : public IStorageProvider { ... };
```

---

## 9. 建议与风险

### 9.1 设计建议

1. **数据一致性**
   - 使用数据库事务保证课程-课时-教案的原子操作
   - 本地文件与数据库记录保持同步，删除时级联处理

2. **性能优化**
   - 学情画像采用增量计算，避免全量重算
   - 通过 profile_refresh_requests 管控刷新频率，避免重复计算
   - 大数据量场景使用分页查询

3. **用户体验**
   - 文件操作提供进度反馈
   - 离线支持：本地缓存核心数据
   - 非实时刷新提供“手动刷新”入口与结果时间提示

### 9.2 风险识别

| 风险 | 级别 | 缓解措施 |
|------|------|----------|
| 本地文件丢失 | 中 | 定期备份提醒，预留云存储接口 |
| 数据库迁移复杂 | 低 | 增量迁移，保持兼容层 |
| 学情计算性能 | 低 | 异步计算，结果缓存 |

---

## 10. 总结

本数据架构设计基于以下核心原则：

1. **以课程为中心** - 清晰的层次结构（课程 > 课时 > 资源/活动）
2. **分层存储** - 元数据存 Supabase，大文件存本地
3. **复用现有资产** - 完全复用 PaperService 和题库系统
4. **渐进式迁移** - 保持现有功能，逐步引入新能力
5. **扩展性预留** - 接口抽象，支持未来云存储迁移

数据模型设计为三期 MVP 提供了完整的数据基础：
- **第一期备课**：courses, lessons, lesson_plans, coursewares, usage_events, content_versions
- **第二期课堂**：student_rosters, course_students, quizzes, quiz_questions, attendances, quiz_responses
- **第三期课后**：homeworks, submissions, student_profiles, class_profiles, learning_events, profile_refresh_requests
