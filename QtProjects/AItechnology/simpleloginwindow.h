#ifndef SIMPLELOGINWINDOW_H
#define SIMPLELOGINWINDOW_H

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QFrame>

class SimpleLoginWindow : public QDialog
{
    Q_OBJECT

public:
    SimpleLoginWindow(QWidget *parent = nullptr);
    ~SimpleLoginWindow();

private slots:
    void onLoginClicked();
    void onCloseClicked();

private:
    void setupUI();
    void setupStyle();

    QHBoxLayout *mainLayout;
    QFrame *leftPanel;
    QFrame *rightPanel;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;

    // 左侧面板组件
    QLabel *mottoLabel;
    QLabel *quoteLabel;
    QLabel *authorLabel;
    QLabel *translationLabel;

    // 右侧面板组件
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLabel *welcomeLabel;
    QLabel *descLabel;
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QPushButton *togglePasswordBtn;
    QCheckBox *rememberCheck;
    QPushButton *forgotPasswordBtn;
    QPushButton *loginButton;
    QLabel *signupLabel;
    QPushButton *signupBtn;
    QPushButton *closeButton;
};

#endif // SIMPLELOGINWINDOW_H