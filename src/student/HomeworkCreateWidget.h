#ifndef HOMEWORKCREATEWIDGET_H
#define HOMEWORKCREATEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>

class HomeworkCreateWidget : public QWidget
{
    Q_OBJECT

public:
    HomeworkCreateWidget(const QString &classId, const QString &teacherEmail, QWidget *parent = nullptr);

signals:
    void created();
    void backRequested();

private:
    void setupUI();
    void onPublishClicked();

    QString m_classId;
    QString m_teacherEmail;
    QLineEdit *m_titleEdit = nullptr;
    QTextEdit *m_descEdit = nullptr;
    QSpinBox *m_scoreSpin = nullptr;
    QDateTimeEdit *m_endTimeEdit = nullptr;
    QPushButton *m_publishBtn = nullptr;
};

#endif
