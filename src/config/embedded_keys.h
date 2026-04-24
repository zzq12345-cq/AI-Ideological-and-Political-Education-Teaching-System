#ifndef EMBEDDED_KEYS_H
#define EMBEDDED_KEYS_H

// 内嵌 API Keys — 保留兼容，但不再存放真实密钥。
// 发布版本由打包脚本生成 config.env 随包分发。
// 开发环境使用 .env.local。
// 运行时优先使用 AppConfig::get() 读取。

namespace EmbeddedKeys {

inline const char* DIFY_API_KEY = "";
inline const char* MINIMAX_API_KEY = "";
inline const char* MINIMAX_API_BASE_URL = "https://api.zzqloveca.online/v1";
inline const char* MINIMAX_MODEL = "MiniMax-M2.7";
inline const char* TIANXING_API_KEY = "";
inline const char* SUPABASE_URL = "";
inline const char* SUPABASE_ANON_KEY = "";
inline const char* ZHIPU_API_KEY = "";
inline const char* ZHIPU_BASE_URL = "https://api.zzqloveca.online/v1";

}  // namespace EmbeddedKeys

#endif  // EMBEDDED_KEYS_H
