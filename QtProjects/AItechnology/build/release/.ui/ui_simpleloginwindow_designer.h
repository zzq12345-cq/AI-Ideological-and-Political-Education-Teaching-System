/********************************************************************************
** Form generated from reading UI file 'simpleloginwindow_designer.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SIMPLELOGINWINDOW_DESIGNER_H
#define UI_SIMPLELOGINWINDOW_DESIGNER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SimpleLoginWindow
{
public:
    QHBoxLayout *mainLayout;
    QFrame *leftPanel;
    QLabel *label;
    QLabel *label_3;
    QLabel *label_2;
    QFrame *separator;
    QLabel *quoteLabel;
    QLabel *authorLabel;
    QLabel *translationLabel;
    QLabel *label_4;
    QLabel *mottoLabel;
    QLabel *mottoEnglish;
    QLabel *label_5;
    QFrame *rightPanel;
    QVBoxLayout *rightLayout;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QVBoxLayout *welcomeLayout;
    QLabel *welcomeLabel;
    QLabel *descLabel;
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QHBoxLayout *optionsLayout;
    QCheckBox *rememberCheck;
    QSpacerItem *horizontalSpacer;
    QPushButton *forgotPasswordBtn;
    QPushButton *loginButton;
    QHBoxLayout *signupLayout;
    QSpacerItem *horizontalSpacer_2;
    QLabel *signupLabel;
    QPushButton *signupBtn;
    QSpacerItem *horizontalSpacer_3;

    void setupUi(QDialog *SimpleLoginWindow)
    {
        if (SimpleLoginWindow->objectName().isEmpty())
            SimpleLoginWindow->setObjectName("SimpleLoginWindow");
        SimpleLoginWindow->resize(1200, 700);
        SimpleLoginWindow->setMinimumSize(QSize(800, 600));
        mainLayout = new QHBoxLayout(SimpleLoginWindow);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        leftPanel = new QFrame(SimpleLoginWindow);
        leftPanel->setObjectName("leftPanel");
        leftPanel->setMinimumSize(QSize(720, 0));
        leftPanel->setStyleSheet(QString::fromUtf8("background-color: #B71C1C;"));
        label = new QLabel(leftPanel);
        label->setObjectName("label");
        label->setGeometry(QRect(12, 12, 16, 16));
        label_3 = new QLabel(leftPanel);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(12, 456, 16, 16));
        label_2 = new QLabel(leftPanel);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(12, 492, 16, 16));
        separator = new QFrame(leftPanel);
        separator->setObjectName("separator");
        separator->setGeometry(QRect(12, 528, 696, 16));
        separator->setStyleSheet(QString::fromUtf8("background-color: #C9A64E; height: 1px; border: none;"));
        separator->setFrameShape(QFrame::Shape::HLine);
        quoteLabel = new QLabel(leftPanel);
        quoteLabel->setObjectName("quoteLabel");
        quoteLabel->setGeometry(QRect(12, 551, 283, 33));
        quoteLabel->setStyleSheet(QString::fromUtf8("color: #C9A64E; font-size: 28px; font-weight: bold; text-align: center;"));
        quoteLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        authorLabel = new QLabel(leftPanel);
        authorLabel->setObjectName("authorLabel");
        authorLabel->setGeometry(QRect(12, 604, 174, 19));
        authorLabel->setStyleSheet(QString::fromUtf8("color: #E8D5B5; font-size: 16px; font-weight: 500; font-style: italic;"));
        authorLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        translationLabel = new QLabel(leftPanel);
        translationLabel->setObjectName("translationLabel");
        translationLabel->setGeometry(QRect(12, 643, 180, 17));
        translationLabel->setStyleSheet(QString::fromUtf8("color: #D4C5A0; font-size: 14px;"));
        translationLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_4 = new QLabel(leftPanel);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(12, 48, 16, 16));
        mottoLabel = new QLabel(leftPanel);
        mottoLabel->setObjectName("mottoLabel");
        mottoLabel->setGeometry(QRect(12, 356, 325, 38));
        mottoLabel->setStyleSheet(QString::fromUtf8("color: #C9A64E; font-size: 32px; font-weight: 900; text-align: center;"));
        mottoLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        mottoEnglish = new QLabel(leftPanel);
        mottoEnglish->setObjectName("mottoEnglish");
        mottoEnglish->setGeometry(QRect(12, 414, 585, 22));
        mottoEnglish->setStyleSheet(QString::fromUtf8("color: #E8D5B5; font-size: 18px; font-weight: 500; text-align: center;"));
        mottoEnglish->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_5 = new QLabel(leftPanel);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(12, 84, 225, 225));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);
        label_5->setMinimumSize(QSize(0, 0));
        label_5->setPixmap(QPixmap(QString::fromUtf8("../../images/download.png")));

        mainLayout->addWidget(leftPanel);

        rightPanel = new QFrame(SimpleLoginWindow);
        rightPanel->setObjectName("rightPanel");
        rightPanel->setStyleSheet(QString::fromUtf8("background-color: white;"));
        rightLayout = new QVBoxLayout(rightPanel);
        rightLayout->setObjectName("rightLayout");
        rightLayout->setContentsMargins(50, 50, 50, 50);
        titleLabel = new QLabel(rightPanel);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("color: #C62828; font-size: 42px; font-weight: 900; text-align: center; margin: 10px 0;"));
        titleLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        rightLayout->addWidget(titleLabel);

        subtitleLabel = new QLabel(rightPanel);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setStyleSheet(QString::fromUtf8("color: #6B7280; font-size: 14px; text-align: center;"));
        subtitleLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        rightLayout->addWidget(subtitleLabel);

        welcomeLayout = new QVBoxLayout();
        welcomeLayout->setObjectName("welcomeLayout");
        welcomeLabel = new QLabel(rightPanel);
        welcomeLabel->setObjectName("welcomeLabel");
        welcomeLabel->setStyleSheet(QString::fromUtf8("color: #0F172A; font-size: 32px; font-weight: 900; text-align: center; margin-top: 20px;"));
        welcomeLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        welcomeLayout->addWidget(welcomeLabel);

        descLabel = new QLabel(rightPanel);
        descLabel->setObjectName("descLabel");
        descLabel->setStyleSheet(QString::fromUtf8("color: #6B7280; font-size: 14px; text-align: center;"));
        descLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        welcomeLayout->addWidget(descLabel);


        rightLayout->addLayout(welcomeLayout);

        usernameLabel = new QLabel(rightPanel);
        usernameLabel->setObjectName("usernameLabel");
        usernameLabel->setStyleSheet(QString::fromUtf8("color: #0F172A; font-size: 16px; font-weight: 500;"));

        rightLayout->addWidget(usernameLabel);

        usernameEdit = new QLineEdit(rightPanel);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 56));
        usernameEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"  border: 1px solid #CFD7E7;\n"
"  border-radius: 8px;\n"
"  padding: 16px 16px;\n"
"  font-size: 16px;\n"
"  background-color: #F6F6F8;\n"
"}\n"
"QLineEdit:focus {\n"
"  border: 2px solid #C62828;\n"
"  outline: none;\n"
"}"));

        rightLayout->addWidget(usernameEdit);

        passwordLabel = new QLabel(rightPanel);
        passwordLabel->setObjectName("passwordLabel");
        passwordLabel->setStyleSheet(QString::fromUtf8("color: #0F172A; font-size: 16px; font-weight: 500;"));

        rightLayout->addWidget(passwordLabel);

        passwordEdit = new QLineEdit(rightPanel);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 56));
        passwordEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"  border: 1px solid #CFD7E7;\n"
"  border-radius: 8px;\n"
"  padding: 16px 50px 16px 16px;\n"
"  font-size: 16px;\n"
"  background-color: #F6F6F8;\n"
"}\n"
"QLineEdit:focus {\n"
"  border: 2px solid #C62828;\n"
"  outline: none;\n"
"}"));
        passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        rightLayout->addWidget(passwordEdit);

        optionsLayout = new QHBoxLayout();
        optionsLayout->setObjectName("optionsLayout");
        rememberCheck = new QCheckBox(rightPanel);
        rememberCheck->setObjectName("rememberCheck");
        rememberCheck->setStyleSheet(QString::fromUtf8("QCheckBox {\n"
"  color: #0F172A;\n"
"  font-size: 14px;\n"
"}\n"
"QCheckBox::indicator {\n"
"  width: 18px;\n"
"  height: 18px;\n"
"  border-radius: 4px;\n"
"  border: 1px solid #CFD7E7;\n"
"  background-color: white;\n"
"}\n"
"QCheckBox::indicator:checked {\n"
"  background-color: #C62828;\n"
"  border-color: #C62828;\n"
"}"));

        optionsLayout->addWidget(rememberCheck);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        optionsLayout->addItem(horizontalSpacer);

        forgotPasswordBtn = new QPushButton(rightPanel);
        forgotPasswordBtn->setObjectName("forgotPasswordBtn");
        forgotPasswordBtn->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  color: #C62828;\n"
"  font-size: 14px;\n"
"  font-weight: 500;\n"
"  border: none;\n"
"  background: transparent;\n"
"}\n"
"QPushButton:hover {\n"
"  color: #8E0000;\n"
"  text-decoration: underline;\n"
"}"));

        optionsLayout->addWidget(forgotPasswordBtn);


        rightLayout->addLayout(optionsLayout);

        loginButton = new QPushButton(rightPanel);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 56));
        loginButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background-color: #C62828;\n"
"  color: white;\n"
"  border: 2px solid #C62828;\n"
"  border-radius: 8px;\n"
"  font-size: 16px;\n"
"  font-weight: bold;\n"
"}\n"
"QPushButton:hover {\n"
"  background-color: #D32F2F;\n"
"  border-color: #D32F2F;\n"
"  color: white;\n"
"}\n"
"QPushButton:pressed {\n"
"  background-color: #B71C1C;\n"
"  border-color: #B71C1C;\n"
"}"));

        rightLayout->addWidget(loginButton);

        signupLayout = new QHBoxLayout();
        signupLayout->setObjectName("signupLayout");
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        signupLayout->addItem(horizontalSpacer_2);

        signupLabel = new QLabel(rightPanel);
        signupLabel->setObjectName("signupLabel");
        signupLabel->setStyleSheet(QString::fromUtf8("color: #6B7280; font-size: 14px;"));

        signupLayout->addWidget(signupLabel);

        signupBtn = new QPushButton(rightPanel);
        signupBtn->setObjectName("signupBtn");
        signupBtn->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  color: #C62828;\n"
"  font-size: 14px;\n"
"  font-weight: 500;\n"
"  border: none;\n"
"  background: transparent;\n"
"}\n"
"QPushButton:hover {\n"
"  color: #8E0000;\n"
"  text-decoration: underline;\n"
"}"));

        signupLayout->addWidget(signupBtn);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        signupLayout->addItem(horizontalSpacer_3);


        rightLayout->addLayout(signupLayout);


        mainLayout->addWidget(rightPanel);


        retranslateUi(SimpleLoginWindow);

        QMetaObject::connectSlotsByName(SimpleLoginWindow);
    } // setupUi

    void retranslateUi(QDialog *SimpleLoginWindow)
    {
        SimpleLoginWindow->setWindowTitle(QCoreApplication::translate("SimpleLoginWindow", "\346\200\235\346\203\263\346\224\277\346\262\273\346\231\272\346\205\247\350\257\276\345\240\202", nullptr));
        label->setText(QString());
        label_3->setText(QString());
        label_2->setText(QString());
        quoteLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\"\344\270\272\344\270\255\345\215\216\344\271\213\345\264\233\350\265\267\350\200\214\350\257\273\344\271\246\"", nullptr));
        authorLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\342\200\224\342\200\224 \345\221\250\346\201\251\346\235\245 (Zhou Enlai)", nullptr));
        translationLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\"Study for the rise of China.\"", nullptr));
        label_4->setText(QString());
        mottoLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\"\344\270\215\345\277\230\345\210\235\345\277\203\357\274\214\347\211\242\350\256\260\344\275\277\345\221\275\"", nullptr));
        mottoEnglish->setText(QCoreApplication::translate("SimpleLoginWindow", "\"Remain true to our original aspiration and keep our mission firmly in mind.\"", nullptr));
        label_5->setText(QString());
        titleLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\346\200\235\346\203\263\346\224\277\346\262\273\346\231\272\346\205\247\350\257\276\345\240\202", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "Ideological & Political Smart Classroom", nullptr));
        welcomeLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\346\254\242\350\277\216\345\233\236\346\235\245", nullptr));
        descLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\350\257\267\347\231\273\345\275\225\346\202\250\347\232\204\350\264\246\346\210\267\344\273\245\347\273\247\347\273\255", nullptr));
        usernameLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\347\224\250\346\210\267\345\220\215\346\210\226\351\202\256\347\256\261", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("SimpleLoginWindow", "\350\257\267\350\276\223\345\205\245\346\202\250\347\232\204\347\224\250\346\210\267\345\220\215\346\210\226\351\202\256\347\256\261", nullptr));
        passwordLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\345\257\206\347\240\201", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("SimpleLoginWindow", "\350\257\267\350\276\223\345\205\245\346\202\250\347\232\204\345\257\206\347\240\201", nullptr));
        rememberCheck->setText(QCoreApplication::translate("SimpleLoginWindow", "\350\256\260\344\275\217\346\210\221", nullptr));
        forgotPasswordBtn->setText(QCoreApplication::translate("SimpleLoginWindow", "\345\277\230\350\256\260\345\257\206\347\240\201?", nullptr));
        loginButton->setText(QCoreApplication::translate("SimpleLoginWindow", "\347\231\273 \345\275\225", nullptr));
        signupLabel->setText(QCoreApplication::translate("SimpleLoginWindow", "\346\262\241\346\234\211\350\264\246\346\210\267?", nullptr));
        signupBtn->setText(QCoreApplication::translate("SimpleLoginWindow", "\347\253\213\345\215\263\346\263\250\345\206\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SimpleLoginWindow: public Ui_SimpleLoginWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIMPLELOGINWINDOW_DESIGNER_H
