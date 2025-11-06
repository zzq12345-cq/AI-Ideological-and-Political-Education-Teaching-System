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

    qDebug() << "=== 测试模式：直接显示注册窗口 ===";
    SignupWindow *signupWindow = new SignupWindow();
    signupWindow->setAttribute(Qt::WA_DeleteOnClose, false);
    signupWindow->show();
    signupWindow->raise();
    signupWindow->activateWindow();

    qDebug() << "=== 注册窗口已显示 ===";
    qDebug() << "=== 检查调试输出：SignupWindow 构造函数是否被调用 ===";

    // 运行事件循环
    int result = app.exec();

    qDebug() << "窗口关闭，返回值:" << result;

    delete signupWindow;
    signupWindow = nullptr;

    qDebug() << "=== 测试完成 ===";

    return 0;
}
