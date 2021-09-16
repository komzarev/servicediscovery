#include "ssdp_qt.hpp"
#include <QCoreApplication>
#include <QThread>

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    QCoreApplication a(argc, argv);

    ssdp::qt::Client client;
    client.setDebugMode(true);
    while (true) {
        auto list = client.resolveAll({ "mpnet_server", "", "", "" }, 5s);
        if (list.isEmpty()) {
            qDebug() << "No servers";
        } else {
            for (const auto& l : list) {
                qDebug() << l.type << l.name << l.details << l.socketString;
            }
            qDebug() << "======";
        }
        QThread::msleep(1000);
    }
    return a.exec();
}
