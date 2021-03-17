#include "ssdp_qt.hpp"

#include <QThread>

void ssdp::qt::Client::updateInterfaces_()
{
    auto list = QNetworkInterface::allInterfaces();
    for (const auto& iface : qAsConst(list)) {
        auto flgs = iface.flags();
        if (flgs.testFlag(QNetworkInterface::CanMulticast) && flgs.testFlag(QNetworkInterface::IsRunning)) {

            auto ips = iface.addressEntries();
            if (ips.empty()) {
                continue;
            }

            auto iname = iface.name();
            if (!joinedInterfaces_.contains(iname)) {
                joinedInterfaces_.push_back(iname);
                for (const auto& ip : qAsConst(ips)) {
                    if (ip.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                        auto socket = new QUdpSocket(this);
                        socket->bind(ip.ip(), 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
                        sockets.emplace_back(socket);
                        break;
                    }
                }
            }
        }
    }
}

ssdp::qt::Client::Client(QObject* parent)
    : QObject(parent)
{
}

QString ssdp::qt::Client::findConnetionString(const QString& type, const QString& name, const QString& details, uint32_t timeout_ms)
{
    QString ret;
    auto list = findAllServers_(type, name, details, timeout_ms, true);
    if (!list.isEmpty()) {
        ret = list[0].socketString;
    }
    return ret;
}

bool ssdp::qt::Client::isLocal(const QString& socketString)
{
    auto tmp = socketString.split(":");
    if (tmp.size() != 2) {
        return false;
    }

    auto fromHost = QHostAddress(tmp[0]);

    auto list = QNetworkInterface::allInterfaces();
    for (const auto& iface : qAsConst(list)) {
        auto flgs = iface.flags();
        if (flgs.testFlag(QNetworkInterface::IsRunning)) {

            auto ips = iface.addressEntries();
            if (ips.empty()) {
                continue;
            }

            for (const auto& ip : qAsConst(ips)) {

                if (ip.ip() == fromHost) {
                    return true;
                }
            }
        }
    }

    return false;
}

QList<ssdp::qt::Client::ServerInfo> ssdp::qt::Client::findAllServers(const QString& type, const QString& name, const QString& details, uint32_t timeout_ms)
{
    return findAllServers_(type, name, details, timeout_ms, false);
}

QList<ssdp::qt::Client::ServerInfo> ssdp::qt::Client::findAllServers_(const QString& type, const QString& name, const QString& details, int timeout_ms, bool onlyOnce)
{
    updateInterfaces_();

    QList<ServerInfo> ret;
    if (!sent(type, name, details)) {
        return ret;
    }

    QDeadlineTimer deadline(timeout_ms);

    while (!deadline.hasExpired()) {

        for (auto& s : sockets) {
            if (s->hasPendingDatagrams()) {
                auto dg = s->receiveDatagram();
                auto ba = dg.data();
                auto res = Response::from_string(ba.data());
                if (res.has_value()) {
                    ServerInfo si;
                    si.socketString = QString::fromStdString(res->location);
                    si.name = QString::fromStdString(res->servername);
                    si.type = QString::fromStdString(res->servertype);
                    si.details = QString::fromStdString(res->serverdetails);
                    ret.push_back(si);
                    if (onlyOnce) {
                        return ret;
                    }
                }
            } else {
                QThread::msleep(60);
            }
        }
    }

    return ret;
}

bool ssdp::qt::Client::sent(const QString& type, const QString& name, const QString& details)
{
    Request req(type.toLatin1().data(), name.toLatin1().data(), details.toLatin1().data());
    auto str = req.to_string();

    for (auto& s : sockets) {
        s->writeDatagram(str.c_str(), str.size(), QHostAddress("239.255.255.250"), 1900);
    }

    return true;
}

void ssdp::qt::Server::updateInterfacesList()
{
#ifndef _WIN32
    if (socket_ != nullptr) {
        socket_->close();
        delete socket_;
        socket_ = nullptr;
    }
#endif

    if (socket_ == nullptr) {
        socket_ = new QUdpSocket(this);
        socket_->bind(QHostAddress::AnyIPv4, 1900, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        connect(socket_, &QUdpSocket::readyRead, this, &Server::readPendingDatagrams);
    }

    auto list = QNetworkInterface::allInterfaces();
    QHostAddress groupAddress("239.255.255.250");
    for (const auto& iface : qAsConst(list)) {
        auto flgs = iface.flags();
        if (flgs.testFlag(QNetworkInterface::CanMulticast) && flgs.testFlag(QNetworkInterface::IsRunning)) {
            auto iname = iface.name();
            if (!joinedInterfaces_.contains(iname)) {
                joinedInterfaces_.push_back(iname);
                if (isDebugMode_) {
                    qInfo() << "[SSDP][INFO]:  Join" << iface.name();
                }
                socket_->joinMulticastGroup(groupAddress, iface);
            }
        }
    }
}

bool ssdp::qt::Server::start(const QString& name, const QString& details)
{
    if (port_.isEmpty()) {
        return false;
    }

    updateInterfacesList();

    connect(socket_, &QUdpSocket::readyRead, this, &Server::readPendingDatagrams);

    resp_.servername = name.toLatin1().data();
    resp_.serverdetails = details.toLatin1().data();

    updateInterfaceListTimer_.setInterval(10000);

    connect(&updateInterfaceListTimer_, &QTimer::timeout, this, [this] {
        updateInterfacesList();
    });

    updateInterfaceListTimer_.start();
    return true;
}

void ssdp::qt::Server::stop()
{
    updateInterfaceListTimer_.stop();
    socket_->close();
    delete socket_;
    socket_ = nullptr;
}

void ssdp::qt::Server::processDatagram(const QNetworkDatagram& dg)
{
    auto data = dg.data();
    if (auto req = Request::from_string(data.data())) {
        if (resp_.matchRequest(req.value())) {
            auto iface = QNetworkInterface::interfaceFromIndex(dg.interfaceIndex());
            auto ips = iface.addressEntries();
            if (ips.empty()) {
                if (isDebugMode_) {
                    qWarning() << "[SSDP][ERROR]:  " << iface.name() << "has no entries";
                }
                return;
            }

            for (const auto& ip : qAsConst(ips)) {
                if (ip.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    auto str = ip.ip().toString() + ":" + port_;
                    auto tmp = resp_;
                    tmp.location = str.toLatin1().data();
                    if (isDebugMode_) {
                        qInfo() << "[SSDP][INFO]: Answer to:" << dg.senderAddress().toString();
                    }

                    socket_->writeDatagram(tmp.to_string().c_str(), dg.senderAddress(), dg.senderPort());
                    break;
                }
            }
        }
    }
}
