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

    // === 简单测试模式 ===
    qDebug() << "\n=== 启动应用：手动测试注册功能 ===\n";
    qDebug() << "请手动点击\"立即注册\"按钮测试窗口切换\n";

    SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
    loginWindow->setAttribute(Qt::WA_DeleteOnClose, false);
    loginWindow->show();
    loginWindow->raise();
    loginWindow->activateWindow();

    qDebug() << "\n登录窗口已显示\n";

    // 运行事件循环
    int result = app.exec();

    qDebug() << "\n登录窗口关闭，返回值:" << result;
    qDebug() << "返回值解释：";
    qDebug() << "  QDialog::Accepted (1) = 点击了注册按钮或正常完成";
    qDebug() << "  QDialog::Rejected (0) = 点击了X关闭按钮";
    qDebug() << "\n";

    // 检查当前窗口类型并处理结果
    SimpleLoginWindow *currentLoginWindow = qobject_cast<SimpleLoginWindow*>(loginWindow);
    SignupWindow *currentSignupWindow = qobject_cast<SignupWindow*>(loginWindow);

    if (currentLoginWindow) {
        qDebug() << "检测到当前窗口是登录窗口";
        if (result == QDialog::Accepted) {
            qDebug() << "用户点击了注册按钮，准备创建注册窗口...\n";

            delete loginWindow;
            loginWindow = nullptr;

            qDebug() << "创建注册窗口...";
            SignupWindow *signupWindow = new SignupWindow();
            signupWindow->setAttribute(Qt::WA_DeleteOnClose, false);
            signupWindow->show();
            signupWindow->raise();
            signupWindow->activateWindow();

            qDebug() << "注册窗口已显示\n";

            int result2 = app.exec();

            qDebug() << "\n注册窗口关闭，返回值:" << result2;
            qDebug() << "返回值解释：";
            qDebug() << "  QDialog::Accepted (1) = 用户点击返回或注册成功";
            qDebug() << "  QDialog::Rejected (0) = 用户点击了X关闭按钮";

            delete signupWindow;
            signupWindow = nullptr;

        } else {
            qDebug() << "用户关闭了登录窗口，程序退出";
            delete loginWindow;
            return 0;
        }
    } else if (currentSignupWindow) {
        qDebug() << "检测到当前窗口是注册窗口";
        qDebug() << "用户从注册窗口返回或关闭";

        delete loginWindow;
        loginWindow = nullptr;
    }

    qDebug() << "\n=== 程序结束 ===\n";

    return 0;
}
