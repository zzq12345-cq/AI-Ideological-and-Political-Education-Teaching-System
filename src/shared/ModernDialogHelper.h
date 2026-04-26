#ifndef MODERNDIALOGHELPER_H
#define MODERNDIALOGHELPER_H

#include <QWidget>
#include <QString>

class ModernDialogHelper
{
public:
    static bool confirm(QWidget *parent, const QString &title, const QString &message);
    static void info(QWidget *parent, const QString &title, const QString &message);
    static void warning(QWidget *parent, const QString &title, const QString &message);
    static QString input(QWidget *parent, const QString &title, const QString &hint,
                         const QString &defaultText = QString());
    static int getInt(QWidget *parent, const QString &title, const QString &label,
                      int value = 0, int min = -2147483647, int max = 2147483647,
                      int step = 1, bool *ok = nullptr);
};

#endif // MODERNDIALOGHELPER_H
