#ifndef SIGNUPWINDOW_H
#define SIGNUPWINDOW_H

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
#include <QCoreApplication>
#include <QApplication>
#include "../supabase/supabaseclient.h"
#include "../supabase/supabaseconfig.h"

class SignUpWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SignUpWindow(QWidget *parent = nullptr);
    ~SignUpWindow();

protected:
    void setupUI();
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onSignupClicked();
    void onBackToLoginClicked();
    void onTogglePassword1Clicked();
    void onTogglePassword2Clicked();
    bool validateInput();

private:
    void openLoginWindow();
    void setupStyle();
    void buildHeroPanel();
    void buildFormPanel();
    QFrame *createInputGroup(QWidget *parent,
                             const QString &placeholderText,
                             QLineEdit **lineEdit,
                             bool isPassword = false,
                             QPushButton **toggleButton = nullptr);
    QString resolveStyleSheetPath() const;
    void showMessage(const QString &title, const QString &message, QMessageBox::Icon icon = QMessageBox::Information);

    // Supabase回调
    void onSignupSuccess(const QString &message);
    void onSignupFailed(const QString &errorMessage);

private:
    // 防止重复处理注册
    bool m_signupProcessed = false;

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
    QLabel *registerLabel;
    QLabel *descLabel;
    QLabel *emailLabel;
    QLineEdit *emailEdit;
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QLabel *passwordLabel1;
    QLineEdit *passwordEdit1;
    QPushButton *togglePassword1Btn;
    QLabel *passwordLabel2;
    QLineEdit *passwordEdit2;
    QPushButton *togglePassword2Btn;
    QPushButton *registerButton;
    QLabel *loginLabel;
    QPushButton *loginBtn;

    // Supabase客户端
    SupabaseClient *m_supabaseClient;
    
    // 图标资源
    QIcon m_eyeShowIcon;
    QIcon m_eyeHideIcon;
};

#endif // SIGNUPWINDOW_H
