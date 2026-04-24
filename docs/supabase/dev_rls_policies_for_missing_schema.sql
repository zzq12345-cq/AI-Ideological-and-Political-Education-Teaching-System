-- 开发期 RLS 策略：允许当前桌面端使用 anon/authenticated 访问新补齐的表
--
-- 背景：
-- Supabase SQL Editor 如果选择 "Run and enable RLS"，新表会启用 RLS。
-- 启用 RLS 但没有 policy 时，REST API 使用 anon key 会查询不到数据。
--
-- 注意：
-- - 这是开发期策略，方便当前 Qt 桌面端继续用 anon key 调试。
-- - 正式上线前应改成按 auth.uid()、teacher_email、student_email 限制访问范围。

alter table public.schools enable row level security;
alter table public.classes enable row level security;
alter table public.class_members enable row level security;
alter table public.attendance_sessions enable row level security;
alter table public.attendance_records enable row level security;
alter table public.invitation_codes enable row level security;
alter table public.materials enable row level security;

drop policy if exists "dev_all_schools" on public.schools;
create policy "dev_all_schools"
  on public.schools
  for all
  to anon, authenticated
  using (true)
  with check (true);

drop policy if exists "dev_all_classes" on public.classes;
create policy "dev_all_classes"
  on public.classes
  for all
  to anon, authenticated
  using (true)
  with check (true);

drop policy if exists "dev_all_class_members" on public.class_members;
create policy "dev_all_class_members"
  on public.class_members
  for all
  to anon, authenticated
  using (true)
  with check (true);

drop policy if exists "dev_all_attendance_sessions" on public.attendance_sessions;
create policy "dev_all_attendance_sessions"
  on public.attendance_sessions
  for all
  to anon, authenticated
  using (true)
  with check (true);

drop policy if exists "dev_all_attendance_records" on public.attendance_records;
create policy "dev_all_attendance_records"
  on public.attendance_records
  for all
  to anon, authenticated
  using (true)
  with check (true);

drop policy if exists "dev_all_invitation_codes" on public.invitation_codes;
create policy "dev_all_invitation_codes"
  on public.invitation_codes
  for all
  to anon, authenticated
  using (true)
  with check (true);

drop policy if exists "dev_all_materials" on public.materials;
create policy "dev_all_materials"
  on public.materials
  for all
  to anon, authenticated
  using (true)
  with check (true);

notify pgrst, 'reload schema';
