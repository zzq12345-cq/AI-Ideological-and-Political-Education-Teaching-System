#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>
#include "../questionbank/QuestionRepository.h"

// 导出服务类
class ExportService : public QObject
{
    Q_OBJECT

public:
    explicit ExportService(QObject *parent = nullptr);

    // 导出为HTML格式
    Q_INVOKABLE bool exportToHtml(const QString &filePath, const QString &paperTitle, const QList<Question> &questions);

    // 导出为PDF格式（TODO：待实现）
    Q_INVOKABLE bool exportToPdf(const QString &filePath, const QString &paperTitle, const QList<Question> &questions);

signals:
    void exportSuccess(const QString &filePath);
    void exportFailed(const QString &error);

private:
    QString generateHtmlContent(const QString &paperTitle, const QList<Question> &questions);
    QString generateQuestionHtml(const Question &question, int index);
};
