#ifndef CREATECLASSDIALOG_H
#define CREATECLASSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class CreateClassDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateClassDialog(QWidget *parent = nullptr);

    QString className() const;

private:
    QLineEdit *m_nameEdit;
    QPushButton *m_createBtn;
    QPushButton *m_cancelBtn;
};

#endif
