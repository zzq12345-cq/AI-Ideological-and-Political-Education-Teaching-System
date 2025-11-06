#ifndef SIMPLELOGINWINDOW_H
#define SIMPLELOGINWINDOW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QFrame>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QApplication>
#include "../supabase/supabaseclient.h"
#include "../supabase/supabaseconfig.h"
#include "../signup/signupwindow.h"

class SimpleLoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleLoginWindow(QWidget *parent = nullptr);
    ~SimpleLoginWindow();

protected:
    void setupUI();

private slots:
    void onLoginClicked();
    void onSignupClicked();
    void onRememberMeToggled(bool checked);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void openMainWindow(const QString &username, const QString &role);
    void setupStyle();
    void loadRememberedCredentials();
    void saveRememberedCredentials();
    void clearRememberedCredentials();
    bool hasRememberedCredentials();

    // Supabase回调
    void onLoginSuccess(const QString &userId, const QString &email);
    void onLoginFailed(const QString &errorMessage);

private:
    // 不再需要UI文件指针

    bool m_loginProcessed = false;  // 防止重复处理登录

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
    QCheckBox *rememberMeCheck;
    QPushButton *forgotPasswordBtn;
    QPushButton *loginButton;

    // 记住我功能
    QSettings *m_settings;
    QLabel *signupLabel;
    QPushButton *signupBtn;

    // Supabase客户端
    SupabaseClient *m_supabaseClient;
};

#endif // SIMPLELOGINWINDOW_H