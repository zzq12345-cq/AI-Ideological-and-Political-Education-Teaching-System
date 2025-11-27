#include "ExportService.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>

ExportService::ExportService(QObject *parent)
    : QObject(parent)
{
}

// 导出为HTML格式
bool ExportService::exportToHtml(const QString &filePath, const QString &paperTitle, const QList<Question> &questions)
{
    if (questions.isEmpty()) {
        qWarning() << "没有题目可以导出";
        emit exportFailed("没有题目可以导出");
        return false;
    }

    // 生成HTML内容
    QString htmlContent = generateHtmlContent(paperTitle, questions);

    // 确保目录存在
    QDir dir(QFileInfo(filePath).absolutePath());
    if (!dir.exists()) {
        if (!dir.mkpath(dir.absolutePath())) {
            qWarning() << "无法创建目录:" << dir.absolutePath();
            emit exportFailed("无法创建目录");
            return false;
        }
    }

    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        emit exportFailed("无法打开文件进行写入");
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << htmlContent;
    file.close();

    qInfo() << "成功导出试卷到:" << filePath;
    emit exportSuccess(filePath);
    return true;
}

// 导出为PDF格式（TODO：待实现）
bool ExportService::exportToPdf(const QString &filePath, const QString &paperTitle, const QList<Question> &questions)
{
    // TODO: 实现PDF导出
    // 可以使用QPrinter、第三方库如QPrinter、poppler或其他PDF生成库

    qWarning() << "PDF导出功能尚未实现";
    emit exportFailed("PDF导出功能尚未实现");
    return false;
}

// 生成HTML内容
QString ExportService::generateHtmlContent(const QString &paperTitle, const QList<Question> &questions)
{
    QString html = R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" + paperTitle + R"(</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: "PingFang SC", -apple-system, sans-serif;
            line-height: 1.6;
            color: #333;
            background-color: #fff;
            padding: 40px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        .header {
            text-align: center;
            margin-bottom: 40px;
            padding-bottom: 20px;
            border-bottom: 3px solid #D9001B;
        }
        .header h1 {
            color: #D9001B;
            font-size: 32px;
            margin-bottom: 10px;
        }
        .header p {
            color: #666;
            font-size: 16px;
        }
        .question {
            margin-bottom: 40px;
            padding: 24px;
            background-color: #f9f9f9;
            border-radius: 8px;
            border-left: 4px solid #D9001B;
        }
        .question-header {
            margin-bottom: 16px;
        }
        .question-meta {
            display: flex;
            gap: 12px;
            margin-bottom: 16px;
        }
        .badge {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 14px;
            font-weight: 500;
        }
        .badge-type {
            background-color: #E8F4FD;
            color: #4A90E2;
        }
        .badge-difficulty-easy {
            background-color: #D1FAE5;
            color: #10B981;
        }
        .badge-difficulty-medium {
            background-color: #FEF3C7;
            color: #F59E0B;
        }
        .badge-difficulty-hard {
            background-color: #FEE2E2;
            color: #EF4444;
        }
        .question-stem {
            font-size: 18px;
            margin-bottom: 16px;
            line-height: 1.8;
        }
        .options {
            margin-bottom: 20px;
        }
        .option {
            padding: 12px 16px;
            margin-bottom: 8px;
            background-color: white;
            border-radius: 6px;
            border: 1px solid #e5e7eb;
        }
        .option-label {
            font-weight: bold;
            margin-right: 8px;
        }
        .answer-section {
            margin-top: 20px;
            padding-top: 20px;
            border-top: 1px solid #e5e7eb;
        }
        .answer-title {
            font-size: 16px;
            font-weight: bold;
            margin-bottom: 8px;
            color: #D9001B;
        }
        .answer-content {
            font-size: 18px;
            font-weight: bold;
            color: #10B981;
            margin-bottom: 16px;
        }
        .explain {
            font-size: 14px;
            color: #666;
            line-height: 1.8;
        }
        .footer {
            margin-top: 40px;
            text-align: center;
            color: #999;
            font-size: 14px;
            padding-top: 20px;
            border-top: 1px solid #e5e7eb;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>)" + paperTitle + R"(</h1>
            <p>共 )" + QString::number(questions.size()) + R"( 题</p>
        </div>
)";

    // 生成每个题目
    for (int i = 0; i < questions.size(); i++) {
        html += generateQuestionHtml(questions[i], i + 1);
    }

    html += R"(
        <div class="footer">
            <p>AI 智能试题库 | 生成时间: )" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + R"(</p>
        </div>
    </div>
</body>
</html>)";

    return html;
}

// 生成单个题目的HTML
QString ExportService::generateQuestionHtml(const Question &question, int index)
{
    QString typeText;
    switch(question.type) {
        case QuestionType::SingleChoice: typeText = "单选题"; break;
        case QuestionType::MultiChoice: typeText = "多选题"; break;
        case QuestionType::TrueFalse: typeText = "判断题"; break;
        case QuestionType::ShortAnswer: typeText = "简答题"; break;
        default: typeText = "未知题型"; break;
    }

    QString difficultyText;
    switch(question.difficulty) {
        case Difficulty::Easy: difficultyText = "简单"; break;
        case Difficulty::Medium: difficultyText = "中等"; break;
        case Difficulty::Hard: difficultyText = "困难"; break;
        default: difficultyText = "未知难度"; break;
    }

    QString difficultyClass;
    switch(question.difficulty) {
        case Difficulty::Easy: difficultyClass = "badge-difficulty-easy"; break;
        case Difficulty::Medium: difficultyClass = "badge-difficulty-medium"; break;
        case Difficulty::Hard: difficultyClass = "badge-difficulty-hard"; break;
        default: difficultyClass = ""; break;
    }

    QString html = R"(
        <div class="question">
            <div class="question-header">
                <h3 style="margin-bottom: 12px; font-size: 20px; color: #D9001B;">第 )" + QString::number(index) + R"( 题</h3>
                <div class="question-meta">
                    <span class="badge badge-type">)" + typeText + R"(</span>
                    <span class="badge )" + difficultyClass + R"(>)" + difficultyText + R"(</span>
                </div>
            </div>
            <div class="question-stem">
                )" + question.stem + R"(
            </div>)";

    // 添加选项（仅当有选项时）
    if (!question.options.isEmpty()) {
        html += R"(
            <div class="options">)";

        for (int i = 0; i < question.options.size(); i++) {
            QString label = QString(QChar('A' + i));
            html += R"(
                <div class="option">
                    <span class="option-label">)" + label + R"(.</span>
                    <span class="option-text">)" + question.options[i] + R"(</span>
                </div>)";
        }

        html += R"(
            </div>)";
    }

    // 添加答案和解析
    if (!question.answer.isEmpty()) {
        html += R"(
            <div class="answer-section">
                <div class="answer-title">正确答案</div>
                <div class="answer-content">)" + question.answer + R"(</div>)";

        if (!question.explain.isEmpty()) {
            html += R"(
                <div class="answer-title">解析</div>
                <div class="explain">)" + question.explain + R"(</div>)";
        }

        html += R"(
            </div>)";
    }

    html += R"(
        </div>
)";

    return html;
}
