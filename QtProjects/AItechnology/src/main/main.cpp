#include <QApplication>
#include <QStyleFactory>
#include <QTimer>
#include "../ui/simpleloginwindow.h"

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

    // 创建登录窗口
    SimpleLoginWindow *loginWindow = new SimpleLoginWindow();

    // 设置窗口标志，防止意外关闭
    loginWindow->setAttribute(Qt::WA_DeleteOnClose, false);

    // 显示登录窗口
    loginWindow->show();

    // 确保窗口保持在顶部
    loginWindow->raise();
    loginWindow->activateWindow();

    qDebug() << "Application started successfully";

    // 运行应用程序事件循环
    int result = app.exec();

    qDebug() << "Application exiting with result:" << result;

    // 手动清理资源
    if (loginWindow) {
        delete loginWindow;
        loginWindow = nullptr;
    }

    return result;
}