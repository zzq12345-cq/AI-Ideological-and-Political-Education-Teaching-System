#ifndef ATTENDANCERESULTWIDGET_H
#define ATTENDANCERESULTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "AttendanceManager.h"

class AttendanceResultWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceResultWidget(const QString &sessionId, QWidget *parent = nullptr);

signals:
    void backToDetail();

private:
    void setupUI();
    void updateResultList(const QList<AttendanceManager::RecordInfo> &records);

    QString m_sessionId;
    QVBoxLayout *m_listLayout;
    QLabel *m_summaryLabel;
    QList<AttendanceManager::RecordInfo> m_records;
};

#endif
