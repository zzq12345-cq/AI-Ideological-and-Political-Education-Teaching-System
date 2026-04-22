#ifndef CLASSSETTINGSWIDGET_H
#define CLASSSETTINGSWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include "ClassInfo.h"

class QPushButton;

class ClassSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClassSettingsWidget(const ClassInfo &info, QWidget *parent = nullptr);

signals:
    void settingsSaved(const ClassInfo &info);
    void deleteRequested(const QString &classId);

private slots:
    void onSaveClicked();
    void onDeleteClicked();

private:
    void setupUI();
    QWidget* createToggle(bool isPublic);

    ClassInfo m_classInfo;
    QLineEdit *m_nameEdit = nullptr;
    QTextEdit *m_descEdit = nullptr;
    QPushButton *m_publicToggle = nullptr;
    bool m_isPublic = false;
};

#endif
