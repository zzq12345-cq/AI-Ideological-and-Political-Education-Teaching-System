#include "QuestionRepository.h"

#include <QFile>
#include <QDebug>

QuestionRepository::QuestionRepository(QObject *parent)
    : QObject(parent)
{
}

void QuestionRepository::loadQuestions(const QString &path)
{
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "[QuestionRepository] 题库文件不存在，跳过加载:" << path;
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[QuestionRepository] 题库文件打开失败:" << path
                   << file.errorString();
        return;
    }

    qInfo() << "[QuestionRepository] 已读取题库文件:" << path
            << "大小:" << file.size();
    file.close();
}
