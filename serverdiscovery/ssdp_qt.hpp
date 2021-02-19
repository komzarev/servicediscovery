#ifndef QTSERVER_HPP
#define QTSERVER_HPP

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

        bool start(const QString& name, const QString& details)
        {
            if (port_.isEmpty()) {
                return false;
            }
            socket_ = new QUdpSocket(this);
            socket_->bind(QHostAddress::AnyIPv4, 1900, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

            auto list = QNetworkInterface::allInterfaces();
            QHostAddress groupAddress("239.255.255.250");
            bool result = true;
            for (auto iface : list) {
                auto flgs = iface.flags();
                if (flgs.testFlag(QNetworkInterface::CanMulticast) && flgs.testFlag(QNetworkInterface::IsRunning)) {
                    result &= socket_->joinMulticastGroup(groupAddress, iface);
                }
            }

            connect(socket_, &QUdpSocket::readyRead, this, &Server::readPendingDatagrams);

            resp_.servername = name.toLatin1().data();
            resp_.serverdetails = details.toLatin1().data();
            return result;
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
        void processDatagram(const QNetworkDatagram& dg)
        {
            auto data = dg.data();
            if (auto req = Request::from_string(data.data())) {
                if (resp_.matchRequest(req.value())) {
                    auto iface = QNetworkInterface::interfaceFromIndex(dg.interfaceIndex());
                    auto ips = iface.addressEntries();
                    if (ips.empty()) {
                        //should never happen
                        return;
                    }

                    auto str = ips[0].ip().toString() + ":" + port_;
                    resp_.location = str.toLatin1().data();
                    socket_->writeDatagram(resp_.to_string().c_str(), dg.senderAddress(), dg.senderPort());
                }
            }
        }

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
            QString socket;
        };

        Client(QObject* parent = 0)
            : QObject(parent)
        {
            socket_ = new QUdpSocket(this);
        }

        QString findConnetionString(const QString& type, const QString& name, const QString& details, int timeout_ms = 500)
        {
            QString ret;
            auto list = findAllServers(type, name, details, timeout_ms);
            if (!list.isEmpty()) {
                ret = list[0].socket;
            }
            return ret;
        }

        QList<ServerInfo> findAllServers(const QString& type, const QString& name, const QString& details, int timeout_ms = 500)
        {
            QList<ServerInfo> ret;
            if (!sent(type, name, details)) {
                return ret;
            }

            if (!socket_->waitForReadyRead(timeout_ms)) {
                return ret;
            }

            while (socket_->hasPendingDatagrams()) {
                auto dg = socket_->receiveDatagram();
                auto res = Response::from_string(dg.data().data());
                if (res.has_value()) {
                    ServerInfo si;
                    si.socket = QString::fromStdString(res->location);
                    si.name = QString::fromStdString(res->servername);
                    si.type = QString::fromStdString(res->servertype);
                    si.details = QString::fromStdString(res->serverdetails);
                    ret.push_back(si);
                }
            }

            return ret;
        }

    private:
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
