#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <QNetworkProxy>
#include <QTcpSocket>
#include <QUrl>
#include <QPalette>
#include <iostream>
#include "../auth/login/simpleloginwindow.h"

namespace {
QNetworkProxy::ProxyType proxyTypeFromScheme(const QString &scheme)
{
    const QString normalizedScheme = scheme.trimmed().toLower();
    if (normalizedScheme == "socks5") {
        return QNetworkProxy::Socks5Proxy;
    }
    return QNetworkProxy::HttpProxy;
}

bool isLocalProxyHost(const QString &host)
{
    const QString normalizedHost = host.trimmed().toLower();
    return normalizedHost == "127.0.0.1"
        || normalizedHost == "localhost"
        || normalizedHost == "::1";
}

bool canReachProxy(const QString &host, quint16 port, int timeoutMs = 500)
{
    QTcpSocket socket;
    socket.connectToHost(host, port);
    const bool connected = socket.waitForConnected(timeoutMs);
    if (connected) {
        socket.disconnectFromHost();
    }
    return connected;
}

void configureApplicationProxy()
{
    QString proxyUrl = qEnvironmentVariable("https_proxy").trimmed();
    if (proxyUrl.isEmpty()) proxyUrl = qEnvironmentVariable("HTTPS_PROXY").trimmed();
    if (proxyUrl.isEmpty()) proxyUrl = qEnvironmentVariable("http_proxy").trimmed();
    if (proxyUrl.isEmpty()) proxyUrl = qEnvironmentVariable("HTTP_PROXY").trimmed();
    if (proxyUrl.isEmpty()) {
        return;
    }

    const QUrl url(proxyUrl);
    if (!url.isValid() || url.host().isEmpty()) {
        qWarning() << "[Proxy] 代理地址无效，已忽略:" << proxyUrl;
        return;
    }

    const quint16 port = static_cast<quint16>(url.port(7897));
    if (isLocalProxyHost(url.host()) && !canReachProxy(url.host(), port)) {
        qWarning() << "[Proxy] 本地代理不可用，已跳过代理配置:" << url.host() << ":" << port;
        return;
    }

    QNetworkProxy proxy(proxyTypeFromScheme(url.scheme()), url.host(), port, url.userName(), url.password());
    QNetworkProxy::setApplicationProxy(proxy);
    qDebug() << "[Proxy] 已设置全局代理:" << url.scheme() << url.host() << ":" << port;
}
}

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

    // 设置全局调色板，确保 Windows 上文字颜色正确（不依赖系统主题）
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#FAFAFA"));
    palette.setColor(QPalette::WindowText, QColor("#212121"));
    palette.setColor(QPalette::Base, QColor("#FFFFFF"));
    palette.setColor(QPalette::AlternateBase, QColor("#F5F5F5"));
    palette.setColor(QPalette::Text, QColor("#212121"));
    palette.setColor(QPalette::Button, QColor("#F5F5F5"));
    palette.setColor(QPalette::ButtonText, QColor("#212121"));
    palette.setColor(QPalette::BrightText, QColor("#FFFFFF"));
    palette.setColor(QPalette::Highlight, QColor("#E53935"));
    palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    palette.setColor(QPalette::ToolTipBase, QColor("#FFFDE7"));
    palette.setColor(QPalette::ToolTipText, QColor("#212121"));
    palette.setColor(QPalette::PlaceholderText, QColor("#9E9E9E"));
    app.setPalette(palette);

    // 全局修正 QMessageBox / QDialog 在 Windows 暗色模式下的样式
    app.setStyleSheet(app.styleSheet() + QStringLiteral(
        "QMessageBox { background-color: #FFFFFF; }"
        "QMessageBox QLabel { color: #212121; background: transparent; }"
        "QMessageBox QPushButton {"
        "    background-color: #F5F5F5;"
        "    color: #212121;"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    padding: 6px 20px;"
        "    min-width: 70px;"
        "}"
        "QMessageBox QPushButton:hover { background-color: #EEEEEE; }"
        "QMessageBox QPushButton:pressed { background-color: #E0E0E0; }"
    ));

    // 配置网络代理（从环境变量读取）
    configureApplicationProxy();

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
