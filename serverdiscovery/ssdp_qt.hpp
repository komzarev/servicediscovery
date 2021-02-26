#ifndef QTSERVER_HPP
#define QTSERVER_HPP

#include <QDeadlineTimer>
#include <QElapsedTimer>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QObject>

#include "ssdp_message.hpp"
#include <QUdpSocket>
#include <QtCore/qglobal.h>

namespace ssdp
{
namespace qt
{
    class Server : public QObject
    {
        Q_OBJECT

    public:
        Server(const QString& type, QObject* parent = 0)
            : QObject(parent)
        {
            resp_.servertype = type.toLatin1().data();
        }

        ~Server()
        {
            stop();
        }

        bool start(const QString& name, const QString& details);

        void stop();

        void setServicePort(const QString& port)
        {
            port_ = port;
        }

    private slots:
        void readPendingDatagrams()
        {
            while (socket_->hasPendingDatagrams()) {
                auto datagram = socket_->receiveDatagram();
                processDatagram(datagram);
            }
        }

    private:
        void processDatagram(const QNetworkDatagram& dg);

        Response resp_;
        QUdpSocket* socket_ = nullptr;
        QString port_;
    };

    class Client : public QObject
    {
        Q_OBJECT

    public:
        struct ServerInfo
        {
            QString type;
            QString name;
            QString details;
            QString socketString;
        };

        Client(QObject* parent = 0);

        QString findConnetionString(const QString& type, const QString& name, const QString& details, int timeout_ms = 500);

        QList<ServerInfo> findAllServers(const QString& type, const QString& name, const QString& details, int timeout_ms = 500);

    private:
        QList<ServerInfo> findAllServers_(const QString& type, const QString& name, const QString& details, int timeout_ms, bool onlyOnce);

        bool sent(const QString& type, const QString& name, const QString& details);

        std::vector<std::unique_ptr<QUdpSocket>> sockets;
    };
}
}

#endif // QTSERVER_HPP
