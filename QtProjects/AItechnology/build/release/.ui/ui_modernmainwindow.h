/********************************************************************************
** Form generated from reading UI file 'modernmainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODERNMAINWINDOW_H
#define UI_MODERNMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ModernMainWindow
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *ModernMainWindow)
    {
        if (ModernMainWindow->objectName().isEmpty())
            ModernMainWindow->setObjectName("ModernMainWindow");
        ModernMainWindow->resize(1600, 1000);
        ModernMainWindow->setMinimumSize(QSize(1400, 900));
        centralwidget = new QWidget(ModernMainWindow);
        centralwidget->setObjectName("centralwidget");
        ModernMainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(ModernMainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1600, 22));
        ModernMainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(ModernMainWindow);
        statusbar->setObjectName("statusbar");
        ModernMainWindow->setStatusBar(statusbar);

        retranslateUi(ModernMainWindow);

        QMetaObject::connectSlotsByName(ModernMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ModernMainWindow)
    {
        ModernMainWindow->setWindowTitle(QCoreApplication::translate("ModernMainWindow", "AI\346\200\235\346\224\277\346\231\272\346\205\247\350\257\276\345\240\202\347\263\273\347\273\237", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ModernMainWindow: public Ui_ModernMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODERNMAINWINDOW_H
