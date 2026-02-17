#include "FailedTaskTracker.h"
#include <QDebug>
#include <QSettings>

FailedTaskTracker::FailedTaskTracker(QObject *parent)
    : QObject(parent)
{
    load();
}

void FailedTaskTracker::trackFailure(const FailedTask &task)
{
    FailedTask t = task;
    if (t.id.isEmpty()) {
        t.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    if (!t.failedAt.isValid()) {
        t.failedAt = QDateTime::currentDateTime();
    }

    m_tasks.append(t);
    save();

    qDebug() << "[FailedTaskTracker] 记录失败任务:" << t.operation << t.endpoint;
    emit taskAdded(t);
    emit pendingCountChanged(m_tasks.size());
}

void FailedTaskTracker::removeTask(const QString &id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == id) {
            m_tasks.removeAt(i);
            save();
            emit taskRemoved(id);
            emit pendingCountChanged(m_tasks.size());
            return;
        }
    }
}

QList<FailedTaskTracker::FailedTask> FailedTaskTracker::failedTasks() const
{
    return m_tasks;
}

int FailedTaskTracker::pendingCount() const
{
    return m_tasks.size();
}

void FailedTaskTracker::save()
{
    QSettings settings;
    settings.beginWriteArray("FailedTasks", m_tasks.size());
    for (int i = 0; i < m_tasks.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("id", m_tasks[i].id);
        settings.setValue("operation", m_tasks[i].operation);
        settings.setValue("endpoint", m_tasks[i].endpoint);
        settings.setValue("method", m_tasks[i].method);
        settings.setValue("data", m_tasks[i].data);
        settings.setValue("failedAt", m_tasks[i].failedAt);
        settings.setValue("errorMessage", m_tasks[i].errorMessage);
        settings.setValue("retryCount", m_tasks[i].retryCount);
    }
    settings.endArray();
}

void FailedTaskTracker::load()
{
    QSettings settings;
    int size = settings.beginReadArray("FailedTasks");
    m_tasks.clear();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        FailedTask t;
        t.id = settings.value("id").toString();
        t.operation = settings.value("operation").toString();
        t.endpoint = settings.value("endpoint").toString();
        t.method = settings.value("method").toString();
        t.data = settings.value("data").toByteArray();
        t.failedAt = settings.value("failedAt").toDateTime();
        t.errorMessage = settings.value("errorMessage").toString();
        t.retryCount = settings.value("retryCount").toInt();
        m_tasks.append(t);
    }
    settings.endArray();

    if (!m_tasks.isEmpty()) {
        qDebug() << "[FailedTaskTracker] 加载了" << m_tasks.size() << "个失败任务";
    }
}
