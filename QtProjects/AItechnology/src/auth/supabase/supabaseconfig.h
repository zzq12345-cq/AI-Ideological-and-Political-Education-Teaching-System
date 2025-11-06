#ifndef SUPABASECONFIG_H
#define SUPABASECONFIG_H

#include <QString>

class SupabaseConfig
{
public:
    // Supabase项目配置
    static const QString SUPABASE_URL;
    static const QString SUPABASE_ANON_KEY;
    static const QString SUPABASE_SERVICE_KEY;

    // 数据库配置
    static const QString USERS_TABLE;
    static const QString AUTH_HEADER_NAME;

    // 私有构造函数，防止实例化
private:
    SupabaseConfig() = default;
};

#endif // SUPABASECONFIG_H
