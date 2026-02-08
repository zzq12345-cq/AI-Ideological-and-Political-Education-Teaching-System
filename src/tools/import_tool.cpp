/**
 * @file import_tool.cpp
 * @brief 后台试题批量导入工具
 * 
 * 用法:
 *   ./ImportTool --dir /path/to/试卷目录 --subject 道德与法治 --grade 七年级
 *   ./ImportTool --file /path/to/试卷.docx --subject 道德与法治 --grade 七年级
 * 
 * 此工具由管理员在后台运行，用于将试卷文档批量导入到公共题库。
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <QtGlobal>

#include "../services/BulkImportService.h"
#include "../services/PaperService.h"
#include "../auth/supabase/supabaseconfig.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ImportTool");
    QCoreApplication::setApplicationVersion("1.0");
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("试题批量导入工具 - 将试卷文档导入到公共题库");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 选项定义
    QCommandLineOption dirOption(
        QStringList() << "d" << "dir",
        "要导入的目录路径（导入目录下所有 DOCX 文件）",
        "directory"
    );
    parser.addOption(dirOption);
    
    QCommandLineOption fileOption(
        QStringList() << "f" << "file",
        "要导入的单个文件路径",
        "filepath"
    );
    parser.addOption(fileOption);
    
    QCommandLineOption subjectOption(
        QStringList() << "s" << "subject",
        "学科名称（如：道德与法治）",
        "subject",
        "道德与法治"  // 默认值
    );
    parser.addOption(subjectOption);
    
    QCommandLineOption gradeOption(
        QStringList() << "g" << "grade",
        "年级（如：七年级）",
        "grade",
        "七年级"  // 默认值
    );
    parser.addOption(gradeOption);
    
    QCommandLineOption tokenOption(
        QStringList() << "t" << "token",
        "Supabase 访问令牌（如未设置，将使用匿名模式）",
        "token"
    );
    parser.addOption(tokenOption);

    QCommandLineOption parserApiKeyOption(
        QStringList() << "k" << "parser-api-key",
        "Dify 解析工作流 API Key（也可通过 PARSER_API_KEY 或 DIFY_API_KEY 环境变量提供）",
        "api_key"
    );
    parser.addOption(parserApiKeyOption);
    
    parser.process(app);
    
    // 验证参数
    QString dirPath = parser.value(dirOption);
    QString filePath = parser.value(fileOption);
    QString subject = parser.value(subjectOption);
    QString grade = parser.value(gradeOption);
    QString token = parser.value(tokenOption);
    QString parserApiKey = parser.value(parserApiKeyOption).trimmed();

    if (parserApiKey.isEmpty()) {
        parserApiKey = qEnvironmentVariable("PARSER_API_KEY").trimmed();
    }
    if (parserApiKey.isEmpty()) {
        parserApiKey = qEnvironmentVariable("DIFY_API_KEY").trimmed();
    }
    
    if (dirPath.isEmpty() && filePath.isEmpty()) {
        qCritical() << "错误：必须指定 --dir 或 --file 参数";
        parser.showHelp(1);
        return 1;
    }

    if (parserApiKey.isEmpty()) {
        qCritical() << "错误：未设置解析 API Key，请使用 --parser-api-key 或环境变量 PARSER_API_KEY/DIFY_API_KEY";
        return 1;
    }
    
    qDebug() << "========================================";
    qDebug() << "试题批量导入工具";
    qDebug() << "========================================";
    qDebug() << "学科:" << subject;
    qDebug() << "年级:" << grade;
    
    if (!dirPath.isEmpty()) {
        qDebug() << "目录:" << dirPath;
    } else {
        qDebug() << "文件:" << filePath;
    }
    qDebug() << "========================================";
    
    // 创建服务
    PaperService *paperService = new PaperService(&app);
    
    // 设置访问令牌（如果提供）
    if (!token.isEmpty()) {
        paperService->setAccessToken(token);
        qDebug() << "已设置 Supabase 访问令牌";
    } else {
        qDebug() << "警告：未设置访问令牌，将使用匿名模式";
    }
    
    BulkImportService *importService = new BulkImportService(paperService, &app);
    importService->setParserApiKey(parserApiKey);
    
    // 连接信号
    QObject::connect(importService, &BulkImportService::importStarted,
        [](int total) {
            qDebug() << "\n开始导入，共" << total << "个文件...";
        });
    
    QObject::connect(importService, &BulkImportService::documentParseStarted,
        [](const QString &fileName) {
            qDebug() << "\n正在解析:" << fileName;
        });
    
    QObject::connect(importService, &BulkImportService::documentParseCompleted,
        [](const QString &fileName, int count) {
            qDebug() << "  [v]" << fileName << "- 获得" << count << "道题目";
        });
    
    QObject::connect(importService, &BulkImportService::importProgress,
        [](int current, int total) {
            int percent = (current * 100) / total;
            qDebug() << QString("进度: %1/%2 (%3%)").arg(current).arg(total).arg(percent);
        });
    
    QObject::connect(importService, &BulkImportService::importCompleted,
        [&app](int success, int failed) {
            qDebug() << "\n========================================";
            qDebug() << "导入完成！";
            qDebug() << "  成功:" << success << "道题目";
            qDebug() << "  失败:" << failed << "个文件";
            qDebug() << "========================================";
            
            // 延迟退出，确保所有网络请求完成
            QTimer::singleShot(2000, &app, &QCoreApplication::quit);
        });
    
    QObject::connect(importService, &BulkImportService::importError,
        [](const QString &error) {
            qCritical() << "错误:" << error;
        });
    
    // 开始导入
    if (!dirPath.isEmpty()) {
        importService->importFromDirectory(dirPath, subject, grade);
    } else {
        importService->importFromDocument(filePath, subject, grade);
    }
    
    return app.exec();
}
