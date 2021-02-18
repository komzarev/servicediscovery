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
        socket_ = new QUdpSocket();
        socket_->bind(QHostAddress::AnyIPv4, 1900, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

        auto list = QNetworkInterface::allInterfaces();
        QHostAddress groupAddress("239.255.255.250");
        bool result = true;
        for (auto iface : list) {
            result &= socket_->joinMulticastGroup(groupAddress, iface);
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
}

#endif // QTSERVER_HPP
