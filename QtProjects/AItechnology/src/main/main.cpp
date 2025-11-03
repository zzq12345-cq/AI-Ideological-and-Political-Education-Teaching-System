#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <iostream>
#include "../auth/login/simpleloginwindow.h"

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

    qDebug() << "\n=== 应用启动 ===\n";
    std::cout << "应用启动" << std::endl;

    // 创建并显示登录窗口
    SimpleLoginWindow *loginWindow = new SimpleLoginWindow();
    loginWindow->show();
    loginWindow->raise();
    loginWindow->activateWindow();

    qDebug() << "登录窗口已显示\n";
    std::cout << "登录窗口已显示" << std::endl;

    // 运行事件循环
    return app.exec();
}
