/********************************************************************************
** Form generated from reading UI file 'simpleloginwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SIMPLELOGINWINDOW_H
#define UI_SIMPLELOGINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SimpleLoginWindow
{
public:
    QVBoxLayout *mainLayout;

    void setupUi(QDialog *SimpleLoginWindow)
    {
        if (SimpleLoginWindow->objectName().isEmpty())
            SimpleLoginWindow->setObjectName("SimpleLoginWindow");
        SimpleLoginWindow->resize(1200, 700);
        SimpleLoginWindow->setMinimumSize(QSize(800, 600));
        SimpleLoginWindow->setWindowModality(Qt::NonModal);
        SimpleLoginWindow->setSizeGripEnabled(true);
        SimpleLoginWindow->setModal(false);
        mainLayout = new QVBoxLayout(SimpleLoginWindow);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);

        retranslateUi(SimpleLoginWindow);

        QMetaObject::connectSlotsByName(SimpleLoginWindow);
    } // setupUi

    void retranslateUi(QDialog *SimpleLoginWindow)
    {
        SimpleLoginWindow->setWindowTitle(QCoreApplication::translate("SimpleLoginWindow", "\346\200\235\346\203\263\346\224\277\346\262\273\346\231\272\346\205\247\350\257\276\345\240\202", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SimpleLoginWindow: public Ui_SimpleLoginWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIMPLELOGINWINDOW_H
