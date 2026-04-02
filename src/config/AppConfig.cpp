#include "AppConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QtGlobal>
#include <cstring>

namespace {

/// 从 key=value 文件中解析所有键值对（首次调用时缓存）
QHash<QString, QString> parseEnvFile(const QString &filePath)
{
    QHash<QString, QString> values;
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return values;
    }

    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const int sep = line.indexOf('=');
        if (sep <= 0) {
            continue;
        }

        const QString key = line.left(sep).trimmed();
        QString value = line.mid(sep + 1).trimmed();
        // 去掉引号包裹
        if ((value.startsWith('"') && value.endsWith('"'))
            || (value.startsWith('\'') && value.endsWith('\''))) {
            value = value.mid(1, value.size() - 2);
        }
        if (!key.isEmpty() && !values.contains(key)) {
            values.insert(key, value);
        }
    }
    return values;
}

/// 单文件缓存：路径 -> 键值对
QHash<QString, QHash<QString, QString>> &fileCache()
{
    static QHash<QString, QHash<QString, QString>> cache;
    return cache;
}

} // namespace

const QString AppConfig::CONFIG_FILENAME = QStringLiteral("config.env");

QString AppConfig::get(const QString &key, const QString &defaultValue)
{
    // 优先级 1: 环境变量
    const QString envValue = qEnvironmentVariable(key.toUtf8().constData());
    if (!envValue.trimmed().isEmpty()) {
        return envValue.trimmed();
    }

    // 优先级 2: 随包 config.env
    const auto bundledPaths = bundledConfigPaths();
    for (const QString &path : bundledPaths) {
        auto &cache = fileCache()[path];
        if (cache.isEmpty() && QFile::exists(path)) {
            cache = parseEnvFile(path);
        }
        if (cache.contains(key)) {
            return cache.value(key);
        }
    }

    // 优先级 3: 开发环境 .env.local
    const auto devPaths = devEnvPaths();
    for (const QString &path : devPaths) {
        auto &cache = fileCache()[path];
        if (cache.isEmpty() && QFile::exists(path)) {
            cache = parseEnvFile(path);
        }
        if (cache.contains(key)) {
            return cache.value(key);
        }
    }

    return defaultValue;
}

QString AppConfig::readFromEnvFile(const QString &filePath, const QString &key)
{
    auto &cache = fileCache()[filePath];
    if (cache.isEmpty() && QFile::exists(filePath)) {
        cache = parseEnvFile(filePath);
    }
    return cache.value(key);
}

QStringList AppConfig::bundledConfigPaths()
{
    QStringList paths;
    const QString appDir = QCoreApplication::applicationDirPath();

    // macOS: AILoginSystem.app/Contents/MacOS/  → config.env 在 MacOS/ 同级或 Resources/
    paths << QDir::cleanPath(appDir + QStringLiteral("/config.env"));
    paths << QDir::cleanPath(appDir + QStringLiteral("/../Resources/config.env"));

    // Windows: exe 同级目录
    // (appDir 本身就是 exe 所在目录，已覆盖)

    // Linux/AppImage: 同级目录
    paths << QDir::cleanPath(appDir + QStringLiteral("/../config.env"));

    return paths;
}

QStringList AppConfig::devEnvPaths()
{
    QStringList paths;
    const QString appDir = QCoreApplication::applicationDirPath();

    // 从构建输出目录反推项目根目录
    paths << QDir::current().absoluteFilePath(QStringLiteral(".env.local"));

    if (!appDir.isEmpty()) {
        paths << QDir::cleanPath(appDir + QStringLiteral("/../../../../.env.local"));
        paths << QDir::cleanPath(appDir + QStringLiteral("/../../../.env.local"));
        paths << QDir::cleanPath(appDir + QStringLiteral("/../../.env.local"));
        paths << QDir::cleanPath(appDir + QStringLiteral("/../.env.local"));
    }

    // 源码相对路径
    const QString sourceRoot = QDir(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath())
        .absoluteFilePath(QStringLiteral("../../../.env.local"));
    paths << QDir::cleanPath(sourceRoot);

    return paths;
}
