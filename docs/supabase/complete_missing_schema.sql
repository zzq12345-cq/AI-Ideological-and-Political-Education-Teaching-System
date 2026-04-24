-- 补齐协作者 database.sql 中当前 Supabase 缺失的基础表
--
-- 使用方式：
-- 1. 打开 Supabase Dashboard -> SQL Editor
-- 2. 粘贴并执行本文件
-- 3. 执行后等待数秒，或保留最后的 NOTIFY 语句刷新 PostgREST schema cache
--
-- 说明：
-- - 本脚本不会重建已有 teachers / papers / questions / assignments / submissions 表。
-- - 当前桌面端代码多处仍使用 anon key 访问 REST，所以这里按现有开发模式授予
--   anon/authenticated 基础 CRUD 权限。正式上线前应改成用户 access token + RLS 策略。

create extension if not exists "pgcrypto";
create extension if not exists "uuid-ossp";

create table if not exists public.schools (
  id uuid primary key default gen_random_uuid(),
  name text not null,
  class_quota integer default 5,
  created_at timestamp with time zone default now()
);

alter table public.teachers
  add column if not exists school_id uuid references public.schools(id);

create table if not exists public.classes (
  id uuid primary key default gen_random_uuid(),
  name text not null,
  teacher_email text not null,
  teacher_name text,
  code character varying not null unique,
  status text default 'active',
  color_index integer default 0,
  student_count integer default 0,
  created_at timestamp with time zone default now(),
  description text default '',
  is_public boolean default false,
  school_id uuid references public.schools(id)
);

create table if not exists public.class_members (
  id uuid primary key default gen_random_uuid(),
  class_id uuid references public.classes(id) on delete cascade,
  student_email text not null,
  joined_at timestamp with time zone default now(),
  student_name text default '',
  student_number text default '',
  unique (class_id, student_email)
);

create table if not exists public.attendance_sessions (
  id uuid primary key default gen_random_uuid(),
  class_id uuid references public.classes(id) on delete cascade,
  code character varying not null,
  status text default 'active',
  created_at timestamp with time zone default now(),
  ended_at timestamp with time zone,
  name text default ''
);

create table if not exists public.attendance_records (
  id uuid primary key default gen_random_uuid(),
  session_id uuid references public.attendance_sessions(id) on delete cascade,
  student_email text not null,
  student_name text default '',
  status text default 'absent',
  signed_at timestamp with time zone
);

create table if not exists public.invitation_codes (
  id uuid primary key default gen_random_uuid(),
  school_id uuid references public.schools(id) on delete cascade,
  code text not null unique,
  used boolean default false,
  used_by_email text,
  created_at timestamp with time zone default now()
);

create table if not exists public.materials (
  id uuid primary key default gen_random_uuid(),
  class_id uuid not null references public.classes(id) on delete cascade,
  folder_id uuid,
  name text not null,
  type text not null default 'file',
  file_url text,
  file_size bigint default 0,
  mime_type text,
  uploader_email text not null,
  created_at timestamp with time zone default now()
);

do $$
begin
  if not exists (
    select 1
    from pg_constraint
    where conname = 'assignments_class_id_fkey'
      and conrelid = 'public.assignments'::regclass
  ) then
    alter table public.assignments
      add constraint assignments_class_id_fkey
      foreign key (class_id) references public.classes(id) on delete cascade
      not valid;
  end if;
end $$;

do $$
begin
  if not exists (
    select 1
    from pg_constraint
    where conname = 'submissions_assignment_id_fkey'
      and conrelid = 'public.submissions'::regclass
  ) then
    alter table public.submissions
      add constraint submissions_assignment_id_fkey
      foreign key (assignment_id) references public.assignments(id) on delete cascade
      not valid;
  end if;
end $$;

create index if not exists idx_teachers_school_id
  on public.teachers(school_id);

create index if not exists idx_classes_teacher_email
  on public.classes(teacher_email);

create index if not exists idx_classes_school_id
  on public.classes(school_id);

create index if not exists idx_class_members_class_id
  on public.class_members(class_id);

create index if not exists idx_class_members_student_email
  on public.class_members(student_email);

create index if not exists idx_attendance_sessions_class_id
  on public.attendance_sessions(class_id);

create index if not exists idx_attendance_sessions_code
  on public.attendance_sessions(code);

create index if not exists idx_attendance_records_session_id
  on public.attendance_records(session_id);

create index if not exists idx_attendance_records_student_email
  on public.attendance_records(student_email);

create index if not exists idx_invitation_codes_school_id
  on public.invitation_codes(school_id);

create index if not exists idx_materials_class_id
  on public.materials(class_id);

grant usage on schema public to anon, authenticated;

grant select, insert, update, delete on public.schools to anon, authenticated;
grant select, insert, update, delete on public.classes to anon, authenticated;
grant select, insert, update, delete on public.class_members to anon, authenticated;
grant select, insert, update, delete on public.attendance_sessions to anon, authenticated;
grant select, insert, update, delete on public.attendance_records to anon, authenticated;
grant select, insert, update, delete on public.invitation_codes to anon, authenticated;
grant select, insert, update, delete on public.materials to anon, authenticated;

insert into public.schools (name)
select '默认学校'
where not exists (
  select 1 from public.schools where name = '默认学校'
);

insert into public.invitation_codes (school_id, code)
select s.id, 'AIEDU2024'
from public.schools s
where s.name = '默认学校'
  and not exists (
    select 1 from public.invitation_codes where code = 'AIEDU2024'
  );

update public.teachers
set school_id = (select id from public.schools where name = '默认学校' limit 1)
where school_id is null;

notify pgrst, 'reload schema';
