#ifndef JOINCLASSDIALOG_H
#define JOINCLASSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class JoinClassDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinClassDialog(QWidget *parent = nullptr);

    QString classCode() const;

private:
    QLineEdit *m_codeEdit;
    QPushButton *m_joinBtn;
    QPushButton *m_cancelBtn;
};

#endif
