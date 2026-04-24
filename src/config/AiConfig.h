#ifndef AI_CONFIG_H
#define AI_CONFIG_H

#include "AppConfig.h"
#include "embedded_keys.h"

#include <initializer_list>
#include <QString>

namespace AiConfig {

inline constexpr const char* DEFAULT_BASE_URL = "https://api.zzqloveca.online/v1";
inline constexpr const char* DEFAULT_MODEL = "MiniMax-M2.7";

inline QString firstConfiguredValue(std::initializer_list<QString> keys,
                                    const QString &defaultValue = QString())
{
    for (const QString &key : keys) {
        const QString value = AppConfig::get(key);
        if (!value.trimmed().isEmpty()) {
            return value.trimmed();
        }
    }
    return defaultValue;
}

inline QString apiKey()
{
    const QString configured = firstConfiguredValue({
        QStringLiteral("MINIMAX_API_KEY"),
        QStringLiteral("OPENAI_API_KEY"),
        QStringLiteral("DIFY_API_KEY"),
        QStringLiteral("ZHIPU_API_KEY")
    });
    if (!configured.isEmpty()) {
        return configured;
    }
    return QString::fromUtf8(EmbeddedKeys::MINIMAX_API_KEY);
}

inline QString baseUrl()
{
    return firstConfiguredValue({
        QStringLiteral("MINIMAX_API_BASE_URL"),
        QStringLiteral("OPENAI_API_BASE_URL"),
        QStringLiteral("DIFY_API_BASE_URL"),
        QStringLiteral("ZHIPU_BASE_URL")
    }, QString::fromUtf8(EmbeddedKeys::MINIMAX_API_BASE_URL));
}

inline QString model()
{
    return firstConfiguredValue({
        QStringLiteral("MINIMAX_MODEL"),
        QStringLiteral("OPENAI_MODEL"),
        QStringLiteral("AI_MODEL")
    }, QString::fromUtf8(EmbeddedKeys::MINIMAX_MODEL));
}

inline QString chatCompletionsUrl()
{
    QString url = baseUrl();
    while (url.endsWith('/')) {
        url.chop(1);
    }
    return url + QStringLiteral("/chat/completions");
}

} // namespace AiConfig

#endif // AI_CONFIG_H
