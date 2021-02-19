#include "ssdp_qt.hpp"
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    ssdp::qt::Server server("mpnet_server");
    server.setServicePort(QString::number(5757));
    if (!server.start("", QString::number(57))) {
        qDebug() << "Fail to start";
        return 0;
    }
    return a.exec();
}
