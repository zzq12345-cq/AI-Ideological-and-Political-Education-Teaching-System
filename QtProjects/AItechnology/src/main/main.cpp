#include <QApplication>
#include <QStyleFactory>
#include "../login/simpleloginwindow.h"

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

    // 使用系统默认字体，避免字体问题

    // 设置应用程序图标 - 使用系统默认图标避免资源依赖
    // app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    // 显示简化的登录窗口
    SimpleLoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}