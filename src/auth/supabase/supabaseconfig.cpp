#include "supabaseconfig.h"
#include <QtGlobal>

namespace {
QString fromEnvOrDefault(const char *key, const QString &defaultValue)
{
    const QString value = qEnvironmentVariable(key).trimmed();
    return value.isEmpty() ? defaultValue : value;
}
}

// Supabase 项目配置（优先读取环境变量）
const QString SupabaseConfig::SUPABASE_URL =
    fromEnvOrDefault("SUPABASE_URL", "https://your-project-id.supabase.co");
const QString SupabaseConfig::SUPABASE_ANON_KEY =
    fromEnvOrDefault("SUPABASE_ANON_KEY", "");
const QString SupabaseConfig::SUPABASE_SERVICE_KEY =
    fromEnvOrDefault("SUPABASE_SERVICE_KEY", "");

const QString SupabaseConfig::USERS_TABLE = "teachers";
const QString SupabaseConfig::AUTH_HEADER_NAME = "Authorization";
