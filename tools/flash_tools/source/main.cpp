#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

#include "src/esptoolrunner.h"
#include "src/serialmonitor.h"
#include "src/portlister.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("ESP32 Flasher");
    app.setOrganizationName("louhui-hello");

    qmlRegisterType<EsptoolRunner>("EspFlasher", 1, 0, "EsptoolRunner");
    qmlRegisterType<SerialMonitor>("EspFlasher", 1, 0, "SerialMonitor");
    qmlRegisterType<PortLister>("EspFlasher", 1, 0, "PortLister");

    QQmlApplicationEngine engine;
    engine.loadFromModule("EspFlasher", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
