#include <QApplication>
#include <QStyleFactory>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include "../auth/login/simpleloginwindow.h"
#include "../auth/login/signupwindow.h"
#include "../dashboard/modernmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序基本信息
    app.setApplicationName("AI思政智慧课堂系统");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("智慧教育科技有限公司");
    app.setOrganizationDomain("aiedu.com");

    // 设置应用程序样式
    app.setStyle(QStyleFactory::create("Fusion"));

    // 关键修复：确保应用程序不会自动退出
    app.setQuitOnLastWindowClosed(false);

    QWidget *currentWindow = nullptr;
    QWidget *mainWindow = nullptr;
    bool shouldExit = false;

    while (!shouldExit) {
        if (!currentWindow) {
            qDebug() << "Creating new window...";
            // 创建登录窗口
            currentWindow = new SimpleLoginWindow();
            currentWindow->setAttribute(Qt::WA_DeleteOnClose, false);
            qDebug() << "Window created successfully";
        }

        // 显示当前窗口
        qDebug() << "Showing window...";
        currentWindow->show();
        currentWindow->raise();
        currentWindow->activateWindow();

        qDebug() << "Window opened";

        // 运行事件循环，返回值表示窗口如何关闭
        int result = app.exec();

        qDebug() << "Window closed with result:" << result;

        // 检查当前窗口类型并处理结果
        SimpleLoginWindow *loginWindow = qobject_cast<SimpleLoginWindow*>(currentWindow);
        SignupWindow *signupWindow = qobject_cast<SignupWindow*>(currentWindow);

        if (loginWindow) {
            qDebug() << "Current window is LoginWindow";
            // 登录窗口关闭，检查用户是否点击了注册按钮
            if (result == QDialog::Accepted) {
                qDebug() << "LoginWindow accepted, creating SignupWindow...";
                // 用户点击了注册按钮，打开注册窗口
                delete currentWindow;
                currentWindow = new SignupWindow();
                currentWindow->setAttribute(Qt::WA_DeleteOnClose, false);
                qDebug() << "SignupWindow created, continuing loop...";
                continue;
            } else {
                qDebug() << "LoginWindow rejected or closed, exiting...";
                // 正常退出
                shouldExit = true;
            }
        } else if (signupWindow) {
            qDebug() << "Current window is SignupWindow";
            // 注册窗口关闭
            if (result == QDialog::Accepted) {
                qDebug() << "SignupWindow accepted, creating LoginWindow...";
                // 用户完成注册或返回登录，重新打开登录窗口
                delete currentWindow;
                currentWindow = new SimpleLoginWindow();
                currentWindow->setAttribute(Qt::WA_DeleteOnClose, false);
                qDebug() << "LoginWindow recreated, continuing loop...";
                continue;
            } else {
                qDebug() << "SignupWindow rejected or closed, exiting...";
                // 正常退出
                shouldExit = true;
            }
        } else {
            qDebug() << "Unknown window type, exiting...";
            // 其他窗口，直接退出
            shouldExit = true;
        }

        delete currentWindow;
        currentWindow = nullptr;
    }

    qDebug() << "Application exiting";

    // 手动清理主窗口资源
    if (mainWindow) {
        delete mainWindow;
        mainWindow = nullptr;
    }

    return 0;
}
