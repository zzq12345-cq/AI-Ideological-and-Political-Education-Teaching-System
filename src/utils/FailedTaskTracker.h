#ifndef FAILEDTASKTRACKER_H
#define FAILEDTASKTRACKER_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QUuid>

/**
 * @brief 失败任务跟踪器
 *
 * 记录重试耗尽后的写操作失败（POST/PATCH/DELETE），
 * 持久化到 QSettings，支持后续手动重试。
 */
class FailedTaskTracker : public QObject
{
    Q_OBJECT

public:
    struct FailedTask {
        QString id;            // UUID
        QString operation;     // "addQuestion", "updateQuestion" 等
        QString endpoint;
        QString method;
        QByteArray data;
        QDateTime failedAt;
        QString errorMessage;
        int retryCount = 0;
    };

    explicit FailedTaskTracker(QObject *parent = nullptr);

    void trackFailure(const FailedTask &task);
    void removeTask(const QString &id);
    QList<FailedTask> failedTasks() const;
    int pendingCount() const;

    // 持久化到 QSettings
    void save();
    void load();

signals:
    void taskAdded(const FailedTask &task);
    void taskRemoved(const QString &id);
    void pendingCountChanged(int count);

private:
    QList<FailedTask> m_tasks;
};

#endif // FAILEDTASKTRACKER_H
