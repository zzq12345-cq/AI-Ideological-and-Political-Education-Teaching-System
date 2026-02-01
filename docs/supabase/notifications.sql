-- 通知系统数据库表结构
-- 老王说：简单直接的表设计，别搞那些花里胡哨的

-- 通知表
CREATE TABLE IF NOT EXISTS notifications (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    type VARCHAR(50) NOT NULL DEFAULT 'system_announcement',
    title VARCHAR(255) NOT NULL,
    content TEXT,
    sender_id UUID REFERENCES auth.users(id),
    receiver_id UUID NOT NULL REFERENCES auth.users(id),
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 通知类型说明：
-- homework_submission: 作业提交通知
-- leave_approval: 请假审批通知
-- grade_release: 成绩发布通知
-- system_announcement: 系统公告

-- 索引优化
CREATE INDEX IF NOT EXISTS idx_notifications_receiver_id ON notifications(receiver_id);
CREATE INDEX IF NOT EXISTS idx_notifications_created_at ON notifications(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_notifications_is_read ON notifications(is_read);
CREATE INDEX IF NOT EXISTS idx_notifications_type ON notifications(type);

-- RLS（行级安全）策略
ALTER TABLE notifications ENABLE ROW LEVEL SECURITY;

-- 用户只能查看自己收到的通知
CREATE POLICY "Users can view own notifications" ON notifications
    FOR SELECT
    USING (receiver_id = auth.uid());

-- 用户可以更新自己的通知（标记已读）
CREATE POLICY "Users can update own notifications" ON notifications
    FOR UPDATE
    USING (receiver_id = auth.uid());

-- 用户可以删除自己的通知
CREATE POLICY "Users can delete own notifications" ON notifications
    FOR DELETE
    USING (receiver_id = auth.uid());

-- 系统和管理员可以创建通知
CREATE POLICY "System can create notifications" ON notifications
    FOR INSERT
    WITH CHECK (true);

-- 自动更新 updated_at 触发器
CREATE OR REPLACE FUNCTION update_notifications_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER notifications_updated_at_trigger
    BEFORE UPDATE ON notifications
    FOR EACH ROW
    EXECUTE FUNCTION update_notifications_updated_at();

-- 示例数据（可选，用于测试）
-- INSERT INTO notifications (type, title, content, receiver_id) VALUES
-- ('homework_submission', '张三提交了作业', '学生张三已提交《第一章练习》', 'your-user-id'),
-- ('leave_approval', '请假申请待审批', '学生李四申请请假3天，请及时审批', 'your-user-id'),
-- ('grade_release', '成绩已发布', '期中考试成绩已发布，请查看', 'your-user-id'),
-- ('system_announcement', '系统维护通知', '系统将于本周六凌晨2点进行维护，届时服务将暂停2小时', 'your-user-id');
