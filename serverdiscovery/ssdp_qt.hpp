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

        bool start(const QString& name, const QString& details);

        void stop()
        {
            socket_->close();
            delete socket_;
            socket_ = nullptr;
        }

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

        Client(QObject* parent = 0)
            : QObject(parent)
        {
            socket_ = new QUdpSocket(this);
        }

        QString findConnetionString(const QString& type, const QString& name, const QString& details, int timeout_ms = 500)
        {
            QString ret;
            auto list = findAllServers_(type, name, details, timeout_ms, true);
            if (!list.isEmpty()) {
                ret = list[0].socketString;
            }
            return ret;
        }

        QList<ServerInfo> findAllServers(const QString& type, const QString& name, const QString& details, int timeout_ms = 500)
        {
            return findAllServers_(type, name, details, timeout_ms, false);
        }

    private:
        QList<ServerInfo> findAllServers_(const QString& type, const QString& name, const QString& details, int timeout_ms, bool onlyOnce);

        bool sent(const QString& type, const QString& name, const QString& details)
        {
            Request req(type.toLatin1().data(), name.toLatin1().data(), details.toLatin1().data());
            auto str = req.to_string();
            return socket_->writeDatagram(str.c_str(), str.size(), QHostAddress("239.255.255.250"), 1900) != -1;
        }
        QUdpSocket* socket_ = nullptr;
    };
}
}

#endif // QTSERVER_HPP
