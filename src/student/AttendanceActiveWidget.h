#ifndef ATTENDANCEACTIVEWIDGET_H
#define ATTENDANCEACTIVEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include "AttendanceManager.h"

class AttendanceActiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceActiveWidget(const AttendanceManager::SessionInfo &session,
                                    int totalMembers, QWidget *parent = nullptr);

signals:
    void attendanceFinished(const QString &sessionId);

private:
    void setupUI();
    void updateSignedList(const QList<AttendanceManager::RecordInfo> &records);

    AttendanceManager::SessionInfo m_session;
    int m_totalMembers;

    QLabel *m_codeLabel;
    QLabel *m_progressLabel;
    QLabel *m_progressBar;  // 简易文字进度条
    QVBoxLayout *m_signedLayout;
    QTimer *m_pollTimer;
};

#endif
