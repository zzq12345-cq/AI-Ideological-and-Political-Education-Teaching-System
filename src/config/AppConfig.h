#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <QString>
#include <QStringList>

/**
 * @brief 统一的应用配置读取器
 *
 * 按优先级加载配置：
 *   1. 环境变量（最高）
 *   2. 随包配置文件 config.env（exe/app 同级目录）
 *   3. .env.local（开发环境项目根目录）
 *   4. 编译时默认值（仅无敏感信息的项）
 *
 * 发布版本通过打包脚本生成 config.env 随包分发，
 * 用户无需手动配置任何 API Key。
 */
class AppConfig
{
public:
    /// 获取配置值，按优先级查找
    static QString get(const QString &key, const QString &defaultValue = QString());

    /// 配置文件文件名
    static const QString CONFIG_FILENAME;

private:
    AppConfig() = default;

    /// 从 key=value 格式文件中读取指定 key
    static QString readFromEnvFile(const QString &filePath, const QString &key);

    /// 获取随包 config.env 的候选路径列表
    static QStringList bundledConfigPaths();

    /// 获取开发环境 .env.local 的候选路径列表
    static QStringList devEnvPaths();
};

#endif // APP_CONFIG_H
