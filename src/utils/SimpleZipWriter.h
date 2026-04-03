#ifndef SIMPLEZIPWRITER_H
#define SIMPLEZIPWRITER_H

#include <QString>
#include <QStringList>

/**
 * @brief 纯 Qt 实现的轻量 ZIP 打包器
 *
 * 使用 DEFLATE (通过 qCompress / zlib) 或 STORE 方式将一个目录打包为 ZIP 文件。
 * 适用于生成 DOCX/PPTX 等 Office Open XML 格式。
 *
 * 不依赖任何外部进程（PowerShell / zip 命令），跨平台无歧义。
 */
class SimpleZipWriter
{
public:
    /**
     * @brief 将 sourceDir 下的所有文件递归打包为 outputZipPath
     * @param sourceDir   待打包的目录根路径
     * @param outputZipPath  输出的 ZIP 文件路径（可以是 .zip / .docx / .pptx 等）
     * @return true 成功, false 失败（可通过 lastError() 获取原因）
     */
    static bool packDirectory(const QString &sourceDir, const QString &outputZipPath);

    /// 最近一次错误描述
    static QString lastError();

private:
    static QString s_lastError;

    /// 递归收集 sourceDir 下所有相对路径
    static QStringList collectFiles(const QString &baseDir, const QString &subDir = QString());
};

#endif // SIMPLEZIPWRITER_H
