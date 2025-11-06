/********************************************************************************
** Form generated from reading UI file 'signupwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SIGNUPWINDOW_H
#define UI_SIGNUPWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SignupWindow
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *titleLabel;
    QFormLayout *formLayout;
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QLabel *emailLabel;
    QLineEdit *emailEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QLabel *confirmLabel;
    QLineEdit *confirmEdit;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *registerBtn;
    QPushButton *backBtn;

    void setupUi(QWidget *SignupWindow)
    {
        if (SignupWindow->objectName().isEmpty())
            SignupWindow->setObjectName("SignupWindow");
        SignupWindow->setWindowModality(Qt::ApplicationModal);
        SignupWindow->resize(500, 600);
        verticalLayout = new QVBoxLayout(SignupWindow);
        verticalLayout->setObjectName("verticalLayout");
        titleLabel = new QLabel(SignupWindow);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(titleLabel);

        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        formLayout->setHorizontalSpacing(15);
        formLayout->setVerticalSpacing(20);
        usernameLabel = new QLabel(SignupWindow);
        usernameLabel->setObjectName("usernameLabel");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, usernameLabel);

        usernameEdit = new QLineEdit(SignupWindow);
        usernameEdit->setObjectName("usernameEdit");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, usernameEdit);

        emailLabel = new QLabel(SignupWindow);
        emailLabel->setObjectName("emailLabel");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, emailLabel);

        emailEdit = new QLineEdit(SignupWindow);
        emailEdit->setObjectName("emailEdit");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, emailEdit);

        passwordLabel = new QLabel(SignupWindow);
        passwordLabel->setObjectName("passwordLabel");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, passwordLabel);

        passwordEdit = new QLineEdit(SignupWindow);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, passwordEdit);

        confirmLabel = new QLabel(SignupWindow);
        confirmLabel->setObjectName("confirmLabel");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, confirmLabel);

        confirmEdit = new QLineEdit(SignupWindow);
        confirmEdit->setObjectName("confirmEdit");
        confirmEdit->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, confirmEdit);


        verticalLayout->addLayout(formLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(20);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        registerBtn = new QPushButton(SignupWindow);
        registerBtn->setObjectName("registerBtn");

        horizontalLayout->addWidget(registerBtn);

        backBtn = new QPushButton(SignupWindow);
        backBtn->setObjectName("backBtn");

        horizontalLayout->addWidget(backBtn);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(SignupWindow);

        QMetaObject::connectSlotsByName(SignupWindow);
    } // setupUi

    void retranslateUi(QWidget *SignupWindow)
    {
        SignupWindow->setWindowTitle(QCoreApplication::translate("SignupWindow", "\347\224\250\346\210\267\346\263\250\345\206\214 - AI\346\224\277\346\262\273\350\257\276\345\240\202", nullptr));
        titleLabel->setText(QCoreApplication::translate("SignupWindow", "\347\224\250\346\210\267\346\263\250\345\206\214", nullptr));
        usernameLabel->setText(QCoreApplication::translate("SignupWindow", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("SignupWindow", "\350\257\267\350\276\223\345\205\245\347\224\250\346\210\267\345\220\215", nullptr));
        emailLabel->setText(QCoreApplication::translate("SignupWindow", "\351\202\256\347\256\261\357\274\232", nullptr));
        emailEdit->setPlaceholderText(QCoreApplication::translate("SignupWindow", "\350\257\267\350\276\223\345\205\245\351\202\256\347\256\261\345\234\260\345\235\200", nullptr));
        passwordLabel->setText(QCoreApplication::translate("SignupWindow", "\345\257\206\347\240\201\357\274\232", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("SignupWindow", "\350\257\267\350\276\223\345\205\245\345\257\206\347\240\201\357\274\210\350\207\263\345\260\2216\344\275\215\357\274\211", nullptr));
        confirmLabel->setText(QCoreApplication::translate("SignupWindow", "\347\241\256\350\256\244\345\257\206\347\240\201\357\274\232", nullptr));
        confirmEdit->setPlaceholderText(QCoreApplication::translate("SignupWindow", "\350\257\267\345\206\215\346\254\241\350\276\223\345\205\245\345\257\206\347\240\201", nullptr));
        registerBtn->setText(QCoreApplication::translate("SignupWindow", "\346\263\250\345\206\214", nullptr));
        backBtn->setText(QCoreApplication::translate("SignupWindow", "\350\277\224\345\233\236\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SignupWindow: public Ui_SignupWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIGNUPWINDOW_H
