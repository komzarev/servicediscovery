#include "ssdp_qt.hpp"

QList<ssdp::qt::Client::ServerInfo> ssdp::qt::Client::findAllServers_(const QString& type, const QString& name, const QString& details, int timeout_ms, bool onlyOnce)
{
    QList<ServerInfo> ret;
    if (!sent(type, name, details)) {
        return ret;
    }

    QDeadlineTimer deadline(timeout_ms);
    if (!socket_->waitForReadyRead(timeout_ms)) {
        return ret;
    }

    while (socket_->hasPendingDatagrams() && !deadline.hasExpired()) {
        auto dg = socket_->receiveDatagram();
        auto res = Response::from_string(dg.data().data());
        if (res.has_value()) {
            ServerInfo si;
            si.socketString = QString::fromStdString(res->location);
            si.name = QString::fromStdString(res->servername);
            si.type = QString::fromStdString(res->servertype);
            si.details = QString::fromStdString(res->serverdetails);
            ret.push_back(si);
            if (onlyOnce) {
                break;
            }
        }
    }

    return ret;
}

bool ssdp::qt::Server::start(const QString& name, const QString& details)
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

void ssdp::qt::Server::processDatagram(const QNetworkDatagram& dg)
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

            for (auto ip : ips) {
                if (ip.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    auto str = ip.ip().toString() + ":" + port_;
                    resp_.location = str.toLatin1().data();
                    socket_->writeDatagram(resp_.to_string().c_str(), dg.senderAddress(), dg.senderPort());
                }
            }
        }
    }
}
