#ifndef SUPABASECONFIG_H
#define SUPABASECONFIG_H

#include <QString>

class SupabaseConfig
{
public:
    // Supabase项目配置
    static QString supabaseUrl();
    static QString supabaseAnonKey();
    static QString supabaseServiceKey();

    // 当前登录用户的 access token
    static void setAccessToken(const QString &token);
    static QString accessToken();

    // 数据库配置
    static const QString USERS_TABLE;
    static const QString AUTH_HEADER_NAME;

private:
    SupabaseConfig() = default;
    static QString s_accessToken;
};

#endif // SUPABASECONFIG_H
