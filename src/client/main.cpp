#include <QApplication>
#include "app/ClientApp.h"

/**
 * @brief 客户端入口
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Radar Client");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Radar");

    // 创建并启动客户端
    ClientApp clientApp;
    if (!clientApp.initialize()) {
        return -1;
    }

    clientApp.showMainWindow();

    return app.exec();
}
