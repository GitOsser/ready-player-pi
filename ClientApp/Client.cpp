#include <QApplication>
#include <QMetaType>
#include "ClientController.h"
#include "gui/menus/GyroPiCommunicatorWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    qRegisterMetaType<GyroCourseInfo>("GyroCourseInfo");

    const char *serverIp = "127.0.0.1";
    if (argc > 1) serverIp = argv[1];

    ClientController controller(serverIp, 9000);
    return controller.run();
}

