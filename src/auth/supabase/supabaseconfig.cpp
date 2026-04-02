#include "supabaseconfig.h"

#include "../../config/AppConfig.h"

#include <QString>

QString SupabaseConfig::supabaseUrl()
{
    static const QString value = AppConfig::get("SUPABASE_URL", "https://your-project-id.supabase.co");
    return value;
}

QString SupabaseConfig::supabaseAnonKey()
{
    static const QString value = AppConfig::get("SUPABASE_ANON_KEY");
    return value;
}

QString SupabaseConfig::supabaseServiceKey()
{
    static const QString value = AppConfig::get("SUPABASE_SERVICE_KEY");
    return value;
}

const QString SupabaseConfig::USERS_TABLE = "teachers";
const QString SupabaseConfig::AUTH_HEADER_NAME = "Authorization";
