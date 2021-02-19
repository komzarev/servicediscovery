#include "ssdp_qt.hpp"
#include <QCoreApplication>
#include <QThread>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    ssdp::qt::Client client;
    while (true) {
        auto list = client.findAllServers("mpnet_server", "", "", 1000);
        if (list.isEmpty()) {
            qDebug() << "No servers";
        } else {
            for (auto l : list) {
                qDebug() << l.type << l.name << l.details << l.socketString;
            }

            QThread::msleep(1000);
        }
    }
    return a.exec();
}
