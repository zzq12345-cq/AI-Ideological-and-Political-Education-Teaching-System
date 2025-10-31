#ifndef IDENTICALLOGINWINDOW_H
#define IDENTICALLOGINWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QProgressBar>
#include <QPaintEvent>
#include <QPixmap>
#include <QEvent>
#include <QSettings>
#include <QTimer>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
namespace Ui { class IdenticalLoginWindow; }
QT_END_NAMESPACE

class IdenticalLoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit IdenticalLoginWindow(QWidget *parent = nullptr);
    ~IdenticalLoginWindow();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onForgotPasswordClicked();
    void onTogglePasswordVisibility();
    void onPasswordChanged(const QString &password);
    void onLoginAnimationFinished();
    void onRoleSelectionChanged();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void initUI();
    void setupStyles();
    void setupAnimations();
    void connectSignals();
    void performLogin(const QString &username, const QString &password);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void loadSettings();
    void saveSettings();
    void validateInput();
    void updatePasswordStrength(const QString &password);
    void startLoginAnimation();

    // UI组件 - 左侧面板
    QFrame *leftPanel;
    QLabel *leftPanelTitle;
    QLabel *leftPanelSubtitle;
    QLabel *quoteLabel;
    QLabel *authorLabel;

    // UI组件 - 右侧面板
    QFrame *rightPanel;
    QLabel *titleLabel;
    QLabel *englishTitle;
    QLabel *welcomeLabel;
    QLabel *descriptionLabel;

    // 登录表单
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QPushButton *togglePasswordButton;
    QCheckBox *rememberMeCheck;
    QCheckBox *autoLoginCheck;
    QPushButton *forgotPasswordButton;
    QLabel *roleSelectionLabel;
    QButtonGroup *roleGroup;
    QRadioButton *teacherRadio;
    QRadioButton *studentRadio;
    QRadioButton *adminRadio;
    QFrame *roleSelectionFrame;
    QLabel *passwordStrengthLabel;
    QProgressBar *passwordStrengthBar;

    // 操作按钮
    QPushButton *loginButton;
    QPushButton *registerButton;

    // 加载动画
    QProgressBar *loginProgress;
    QLabel *statusLabel;

    // 动画效果
    QTimer *loginTimer;
    QSettings *settings;

    // 布局
    QVBoxLayout *mainLayout;
    QFrame *contentFrame;
    QHBoxLayout *contentLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    QFrame *usernameFieldFrame;
    QFrame *passwordFieldFrame;

    // 背景资源
    QPixmap backgroundPixmap;
};

#endif // IDENTICALLOGINWINDOW_H
