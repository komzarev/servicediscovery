#include "ssdp_qt.hpp"
#include <QCoreApplication>
#include <QThread>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    ssdp::qt::Client client;
    client.setDebugMode(true);
    while (true) {
        auto list = client.resolve("mpnet_server", "", "", 5000);
        if (list.isEmpty()) {
            qDebug() << "No servers";
        } else {
            for (auto l : list) {
                qDebug() << l.type << l.name << l.details << l.socketString;
            }
            qDebug() << "======";
        }
        QThread::msleep(1000);
    }
    return a.exec();
}
