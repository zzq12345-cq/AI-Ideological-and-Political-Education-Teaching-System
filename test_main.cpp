#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 创建一个简单的测试窗口
    QDialog testWindow;
    testWindow.setWindowTitle("AI思政智慧课堂系统 - 测试");
    testWindow.resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&testWindow);

    QLabel *titleLabel = new QLabel("AI思政智慧课堂系统");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");

    QLabel *userLabel = new QLabel("用户名:");
    QLineEdit *usernameEdit = new QLineEdit("test");

    QLabel *passLabel = new QLabel("密码:");
    QLineEdit *passwordEdit = new QLineEdit("123456");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QPushButton *loginButton = new QPushButton("登录");
    loginButton->setStyleSheet("background-color: #e74c3c; color: white; padding: 10px; border: none; border-radius: 5px;");

    layout->addWidget(titleLabel);
    layout->addWidget(userLabel);
    layout->addWidget(usernameEdit);
    layout->addWidget(passLabel);
    layout->addWidget(passwordEdit);
    layout->addWidget(loginButton);

    QObject::connect(loginButton, &QPushButton::clicked, [&testWindow]() {
        QMessageBox::information(&testWindow, "测试", "界面正常显示！");
    });

    testWindow.show();

    return app.exec();
}