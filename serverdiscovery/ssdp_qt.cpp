#include "ssdp_qt.hpp"
#include "ssdp_message.hpp"
#include <QDeadlineTimer>
#include <QNetworkInterface>
#include <QThread>

using namespace ssdp::qt;

void Logger::info(const QString& msg)
{
    if (isDebugMode_) {
        qInfo() << "[SSDP][INFO]: " << msg;
    }
}

void Logger::error(const QString& msg)
{
    if (isDebugMode_) {
        qWarning() << "[SSDP][ERROR]: " << msg;
    }
}

void Client::updateInterfaces_()
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
                        bool bindRes = socket->bind(ip.ip(), 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

                        if (bindRes) {
                            log.info("Join interface: " + ip.ip().toString());
                            sockets.emplace_back(socket);
                        } else {
                            log.error("Join interface: " + ip.ip().toString() + " Failed!");
                            delete socket;
                        }
                        break;
                    }
                }
            }
        }
    }
}

Client::Client(QObject* parent)
    : QObject(parent)
{
}

bool Client::isLocal(const QString& socketString)
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

void Client::setDebugMode(bool isDebug)
{
    log.setDebugMode(isDebug);
}

bool Client::checkRequest(const Client::ServerRequestInfo& req)
{
    if (!req.ipmask.isEmpty()) {
        return req.ipmask.contains("*") && !req.ipmask.contains("**");
    }
    return true;
}

optional<Client::ServerInfo> Client::resolve(const Client::ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime)
{
    ServerInfo ret;
    bool hasResult = resolve_(server, maxServerWaitTime, [&ret](ServerInfo& si) -> bool {
        ret = si;
        return false;
    });

    if (hasResult) {
        return ret;
    }
    return nullopt;
}

QList<Client::ServerInfo> Client::resolveAll(const Client::ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime)
{
    QList<ServerInfo> ret;
    resolve_(server, maxServerWaitTime, [&ret](ServerInfo& si) -> bool {
        ret.push_back(si);
        return true;
    });

    if (!ret.isEmpty()) {
        std::sort(std::begin(ret), std::end(ret), [](auto rhs, auto lhs) {
            return rhs < lhs;
        });
    }
    return ret;
}

bool Client::startResolveAsync(const Client::ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime)
{
    if (!Client::checkRequest(server)) {
        return false;
    }
    updateInterfaces_();
    bool ret = sent_(server);
    maxServerWaitTime_ = maxServerWaitTime;
    timer_.reset(new QTimer());
    timer_->setInterval(60);
    timer_->callOnTimeout([this, server] {
        maxServerWaitTime_ -= timer_->intervalAsDuration();
        if (maxServerWaitTime_.count() > 0 && isRunning_.load()) {

            for (auto& s : sockets) {
                if (s->hasPendingDatagrams()) {
                    auto dg = s->receiveDatagram();
                    auto ba = dg.data();
                    auto res = Response::from_string(ba.data());
                    if (res.has_value()) {
                        ServerInfo si;
                        si.socketString = QString::fromStdString(res->location);
                        log.info("Get replay from: " + si.socketString);

                        if (!server.ipmask.isEmpty() && !isIpMatchedToMask(si.socketString, server.ipmask)) {
                            log.info(si.socketString + " was filtred by mask.");
                            continue;
                        }

                        si.name = QString::fromStdString(res->servername);
                        si.type = QString::fromStdString(res->servertype);
                        si.details = QString::fromStdString(res->serverdetails);
                        si.isLocal = isLocal(si.socketString);
                        emit serverFound(si);
                    }
                }
            }
        }
    });
    return ret;
}

bool Client::resolve_(const Client::ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime, std::function<bool(Client::ServerInfo&)> func)
{
    if (!Client::checkRequest(server)) {
        return false;
    }

    updateInterfaces_();
    if (!sent_(server)) {
        return false;
    }
    QDeadlineTimer deadline(maxServerWaitTime);
    bool wasFound = false;
    while (!deadline.hasExpired() && isRunning_.load()) {

        for (auto& s : sockets) {
            if (s->hasPendingDatagrams()) {
                auto dg = s->receiveDatagram();
                auto ba = dg.data();
                auto res = Response::from_string(ba.data());
                if (res.has_value()) {
                    ServerInfo si;
                    si.socketString = QString::fromStdString(res->location);
                    log.info("Get replay from: " + si.socketString);

                    if (!server.ipmask.isEmpty() && !isIpMatchedToMask(si.socketString, server.ipmask)) {
                        log.info(si.socketString + " was filtred by mask.");
                        continue;
                    }

                    si.name = QString::fromStdString(res->servername);
                    si.type = QString::fromStdString(res->servertype);
                    si.details = QString::fromStdString(res->serverdetails);
                    si.isLocal = isLocal(si.socketString);
                    wasFound = true;
                    if (!func(si)) {
                        return true;
                    }
                }
            }
        }

        QThread::msleep(60);
    }

#ifdef __QNXNTO__
    if (ret.isEmpty()) {
        qWarning("[SSDP][ERROR]: Can't find server. Check firewall ruls on Windows for mpnet-server!");
    }

#endif

    return wasFound;
}

bool Client::sent_(const Client::ServerRequestInfo& server)
{
    if (!sent(server.serviceType, server.serviceName, server.serviceDetails)) {
#ifdef __QNXNTO__
        qWarning("[SSDP][ERROR]: Can't send request, route entry is missed? Add this command to the start script: \n\n"
                 "\t\troute add 239.0.0.0/8 192.168.50.255\n\n"
                 "\t\tAnd then restart. Also check firewall ruls on Windows for mpnet-server!\n");
#endif

        log.error("All sent attempts FAILED");
        return false;
    }

    return true;
}

bool Client::isIpMatchedToMask(const QString& ip, const QString& mask)
{
    int j = 0;
    for (int i = 0; i < ip.size(); ++i) {
        if (mask[j] != '*') {
            while (i < ip.size() && ip[i] != '.') {
                ++i;
            }
            ++j;
        }

        if (j >= mask.size() || i >= ip.size()) {
            return true;
        }

        if (ip[i] != mask[j]) {
            return false;
        }
    }

    return true;
}

bool Client::isRunning()
{
    return isRunning_.load();
}

void Client::stopResolve()
{
    isRunning_.store(false);
}

bool Client::sent(const QString& type, const QString& name, const QString& details)
{
    Request req(type.toLatin1().data(), name.toLatin1().data(), details.toLatin1().data());
    auto str = req.to_string();

    bool ret = false;
    for (auto& s : sockets) {
        bool fail = -1 == s->writeDatagram(str.c_str(), str.size(), QHostAddress("239.255.255.250"), 1900);
        if (log.isDebugMode()) {
            if (fail) {
                log.error("Sent request to: " + s->localAddress().toString() + " FAILED: " + s->errorString());
                log.error("Error: " + QVariant::fromValue(s->error()).toString());
                log.error("Socket state: " + QVariant::fromValue(s->state()).toString());
            } else {
                log.info("Sent request to: " + s->localAddress().toString());
            }
        }
        // only if all fails, return false
        ret |= !fail;
    }

    return ret;
}

void Server::updateInterfacesList()
{
#ifndef _WIN32
    if (socket_ != nullptr) {
        socket_->close();
        delete socket_;
        socket_ = nullptr;
        joinedInterfaces_.clear();
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
                log.info("Join interface: " + iface.humanReadableName());
                socket_->joinMulticastGroup(groupAddress, iface);
            }
        }
    }
}

void Server::readPendingDatagrams()
{
    while (socket_->hasPendingDatagrams()) {
        auto datagram = socket_->receiveDatagram();
        processDatagram(datagram);
    }
}

Server::Server(const QString& type, QObject* parent)
    : QObject(parent)
{
    resp_.reset(new Response());
    resp_->servertype = type.toLatin1().data();
}

Server::~Server()
{
    stop();
}

void Server::setDebugMode(bool isDebug)
{
    log.setDebugMode(isDebug);
}

bool Server::start(const QString& name, const QString& details)
{
    if (port_.isEmpty()) {
        log.error("Service port was not set");
        return false;
    }

    resp_->servername = name.toLatin1().data();
    resp_->serverdetails = details.toLatin1().data();

    updateInterfacesList();

    connect(socket_, &QUdpSocket::readyRead, this, &Server::readPendingDatagrams);

    updateInterfaceListTimer_.setInterval(10000);

    connect(&updateInterfaceListTimer_, &QTimer::timeout, this, [this] {
        updateInterfacesList();
    });

    updateInterfaceListTimer_.start();
    return true;
}

void Server::stop()
{
    updateInterfaceListTimer_.stop();
    socket_->close();
    delete socket_;
    socket_ = nullptr;
}

void Server::processDatagram(const QNetworkDatagram& dg)
{
    auto data = dg.data();
    if (auto req = Request::from_string(data.data())) {
        if (resp_->matchRequest(req.value())) {
            auto iface = QNetworkInterface::interfaceFromIndex(dg.interfaceIndex());
            auto ips = iface.addressEntries();
            if (ips.empty()) {
                log.error(iface.name() + " has no entries");
                return;
            }

            for (const auto& ip : qAsConst(ips)) {
                if (ip.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    auto str = ip.ip().toString() + ":" + port_;
                    auto tmp = *resp_;
                    tmp.location = str.toLatin1().data();
                    log.info("Sent answer to: " + dg.senderAddress().toString() + ":" + QString::number(dg.senderPort()));
                    socket_->writeDatagram(tmp.to_string().c_str(), dg.senderAddress(), dg.senderPort());
                    break;
                }
            }
        }
    }
}

bool Client::ServerInfo::operator<(const Client::ServerInfo& other) const
{
    if (isLocal != other.isLocal) {
        return isLocal && !other.isLocal;
    }

    return socketString < other.socketString;
}
