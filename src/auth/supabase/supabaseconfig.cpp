#include "supabaseconfig.h"

#include "../../config/embedded_keys.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QtGlobal>
#include <cstring>

namespace {
QString readValueFromEnvFile(const QString &envKey)
{
    static const QHash<QString, QString> cachedValues = []() {
        QHash<QString, QString> values;
        QStringList candidatePaths;

        candidatePaths << QDir::current().absoluteFilePath(QStringLiteral(".env.local"));

        const QString appDir = QCoreApplication::applicationDirPath();
        if (!appDir.isEmpty()) {
            candidatePaths << QDir::cleanPath(appDir + QStringLiteral("/../../../../.env.local"));
            candidatePaths << QDir::cleanPath(appDir + QStringLiteral("/../../../.env.local"));
            candidatePaths << QDir::cleanPath(appDir + QStringLiteral("/../../.env.local"));
            candidatePaths << QDir::cleanPath(appDir + QStringLiteral("/../.env.local"));
        }

        const QString sourceRootEnv = QDir(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath())
            .absoluteFilePath(QStringLiteral("../../../.env.local"));
        candidatePaths << QDir::cleanPath(sourceRootEnv);

        for (const QString &path : std::as_const(candidatePaths)) {
            QFile file(path);
            if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }

            while (!file.atEnd()) {
                const QString line = QString::fromUtf8(file.readLine()).trimmed();
                if (line.isEmpty() || line.startsWith('#')) {
                    continue;
                }

                const int separatorIndex = line.indexOf('=');
                if (separatorIndex <= 0) {
                    continue;
                }

                const QString key = line.left(separatorIndex).trimmed();
                QString value = line.mid(separatorIndex + 1).trimmed();
                if ((value.startsWith('"') && value.endsWith('"'))
                    || (value.startsWith('\'') && value.endsWith('\''))) {
                    value = value.mid(1, value.size() - 2);
                }
                if (!key.isEmpty() && !value.isEmpty() && !values.contains(key)) {
                    values.insert(key, value);
                }
            }

            if (!values.isEmpty()) {
                break;
            }
        }

        return values;
    }();

    return cachedValues.value(envKey).trimmed();
}

QString fromEnvFileEmbeddedOrDefault(const char *envKey,
                                     const char *embeddedValue,
                                     const QString &defaultValue)
{
    const QString envValue = qEnvironmentVariable(envKey).trimmed();
    if (!envValue.isEmpty()) {
        return envValue;
    }

    const QString envFileValue = readValueFromEnvFile(QString::fromUtf8(envKey));
    if (!envFileValue.isEmpty()) {
        return envFileValue;
    }

    if (embeddedValue != nullptr && std::strlen(embeddedValue) > 0) {
        return QString::fromUtf8(embeddedValue).trimmed();
    }

    return defaultValue;
}
}

QString SupabaseConfig::supabaseUrl()
{
    static const QString value = fromEnvFileEmbeddedOrDefault(
        "SUPABASE_URL", EmbeddedKeys::SUPABASE_URL, "https://your-project-id.supabase.co");
    return value;
}

QString SupabaseConfig::supabaseAnonKey()
{
    static const QString value = fromEnvFileEmbeddedOrDefault(
        "SUPABASE_ANON_KEY", EmbeddedKeys::SUPABASE_ANON_KEY, "");
    return value;
}

QString SupabaseConfig::supabaseServiceKey()
{
    static const QString value = fromEnvFileEmbeddedOrDefault(
        "SUPABASE_SERVICE_KEY", nullptr, "");
    return value;
}

const QString SupabaseConfig::USERS_TABLE = "teachers";
const QString SupabaseConfig::AUTH_HEADER_NAME = "Authorization";
