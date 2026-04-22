#ifndef ATTENDANCESIGNINDIALOG_H
#define ATTENDANCESIGNINDIALOG_H

#include <QDialog>
#include <QLineEdit>

class AttendanceSignInDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AttendanceSignInDialog(QWidget *parent = nullptr);
    QString signCode() const;

private:
    QLineEdit *m_codeEdit;
};

#endif
