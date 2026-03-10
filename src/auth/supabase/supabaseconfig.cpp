#include "supabaseconfig.h"
#include "../../config/embedded_keys.h"
#include <QtGlobal>
#include <cstring>

namespace {
QString fromEnvEmbeddedOrDefault(const char *envKey, const char *embeddedValue, const QString &defaultValue)
{
    const QString envValue = qEnvironmentVariable(envKey).trimmed();
    if (!envValue.isEmpty()) {
        return envValue;
    }

    if (embeddedValue != nullptr && std::strlen(embeddedValue) > 0) {
        return QString::fromUtf8(embeddedValue).trimmed();
    }

    return defaultValue;
}
}

// Supabase 项目配置（优先读取环境变量，其次内嵌发布配置）
const QString SupabaseConfig::SUPABASE_URL =
    fromEnvEmbeddedOrDefault("SUPABASE_URL", EmbeddedKeys::SUPABASE_URL, "https://your-project-id.supabase.co");
const QString SupabaseConfig::SUPABASE_ANON_KEY =
    fromEnvEmbeddedOrDefault("SUPABASE_ANON_KEY", EmbeddedKeys::SUPABASE_ANON_KEY, "");
const QString SupabaseConfig::SUPABASE_SERVICE_KEY =
    fromEnvEmbeddedOrDefault("SUPABASE_SERVICE_KEY", nullptr, "");

const QString SupabaseConfig::USERS_TABLE = "teachers";
const QString SupabaseConfig::AUTH_HEADER_NAME = "Authorization";
