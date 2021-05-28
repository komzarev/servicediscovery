#include "ssdp_asio.hpp"
#include "string_find.h"

using namespace boost::asio::ip;

ssdp::asio::Server::Server(std::string type)
{
    resp_.servertype = std::move(type);
}

ssdp::asio::Server::~Server()
{
    stop();
}

void ssdp::asio::Server::updateInterfacesList(boost::asio::io_service& io_service)
{
    udp::resolver resolver(io_service);
    udp::resolver::query query(host_name(), "");
    udp::resolver::iterator it = resolver.resolve(query);
    auto multicast_addr = address::from_string("239.255.255.250");

    while (it != udp::resolver::iterator()) {
        auto addr = (it++)->endpoint();
        if (addr.address().is_v4()) {

            try {
                auto socket = new UdpSocket(io_service);
                socket->socket.open(udp::v4());
                socket->socket.set_option(udp::socket::reuse_address(true));
                socket->socket.bind(udp::endpoint(addr.address(), 1900));
                socket->socket.set_option(multicast::join_group(multicast_addr));
                log_.info("Succesfully bind to: ", addr.address().to_v4().to_string());
                sockets_.emplace_back(socket);
                startReceive(socket);
            }
            catch (boost::system::system_error& err) {
                log_.error(err.what());
            }
        }
    }
}

bool ssdp::asio::Server::start(std::string name, std::string details, boost::asio::io_service& io_service)
{
    if (port_.empty()) {
        log_.error("Port number was not set");
        return false;
    }

    resp_.servername = std::move(name);
    resp_.serverdetails = std::move(details);

    updateInterfacesList(io_service);
    return true;
}

void ssdp::asio::Server::stop()
{
    for (auto& s : sockets_) {
        try {
            s->socket.close();
        }
        catch (boost::system::system_error& err) {
            log_.error(err.what());
        }
    }
    sockets_.clear();
}

void ssdp::asio::Server::readPendingDatagrams(const boost::system::error_code& error, const size_t bytes_recived, UdpSocket* socket)
{
    try {
        if (!error || error == boost::asio::error::message_size) {
            auto data = std::string(std::begin(socket->buffer), std::begin(socket->buffer) + bytes_recived);
            if (auto req = Request::from_string(data.data())) {
                if (resp_.matchRequest(req.value())) {
                    auto tmp = resp_;
                    tmp.location = socket->socket.local_endpoint().address().to_string() + ":" + port_;
                    log_.info("Answer to: ", socket->remote_endpoint.address().to_string());
                    socket->socket.send_to(boost::asio::buffer(tmp.to_string()), socket->remote_endpoint);
                }
            }
        }
    }
    catch (boost::system::system_error& err) {
        log_.error(err.what());
    }
}

void ssdp::asio::Server::startReceive(UdpSocket* socket)
{
    socket->socket.async_receive_from(
        boost::asio::buffer(socket->buffer), socket->remote_endpoint, [this, socket](const boost::system::error_code& error, const size_t bytes_recived) {
            readPendingDatagrams(error, bytes_recived, socket);
            startReceive(socket);
        });
}

ssdp::asio::Client::Client()
    : timer_(io_service_)
{
}

ssdp::asio::Client::~Client()
{

    for (auto& s : sockets_) {
        try {
            s->socket.close();
        }
        catch (boost::system::system_error& err) {
            log_.error(err.what());
        }
    }
}

std::vector<ssdp::asio::Client::ServerInfo>
ssdp::asio::Client::resolve(const std::string& serviceType, const std::string& serviceName, const std::string& serviceDetails, uint32_t timeout_ms)
{
    updateInterfaces_();

    std::vector<ServerInfo> ret;
    if (!sent(serviceType, serviceName, serviceDetails)) {
        return ret;
    }

    timer_.expires_after(std::chrono::milliseconds(timeout_ms));
    timer_.async_wait([this](const boost::system::error_code& error) {
        if (!error) {
            isRunning = false;
        }
    });

    for (auto& s : sockets_) {
        startRecieve_(ret, s.get());
    }
    isRunning = true;

    while (isRunning) {
        io_service_.run_one();
    }

    if (!ret.empty()) {
        std::sort(std::begin(ret), std::end(ret), [](auto rhs, auto lhs) {
            return rhs < lhs;
        });
    }
    return ret;
}

bool ssdp::asio::Client::isLocal(const std::string& socketString)
{
    auto tmp = rl::str::get_until(socketString, ":");
    if (tmp.empty()) {
        return false;
    }

    for (const auto& ip : joinedInterfaces_) {
        if (tmp == ip) {
            return true;
        }
    }

    return false;
}

void ssdp::asio::Client::updateInterfaces_()
{
    udp::resolver resolver(io_service_);
    udp::resolver::query query(host_name(), "");
    udp::resolver::iterator it = resolver.resolve(query);

    while (it != udp::resolver::iterator()) {
        auto addr = (it++)->endpoint();
        if (addr.address().is_v4()) {

            try {
                auto name = addr.address().to_string();

                bool hasName = std::find(joinedInterfaces_.begin(), joinedInterfaces_.end(), name) != joinedInterfaces_.end();
                if (!hasName) {
                    joinedInterfaces_.push_back(name);
                    log_.info("Try bind to: ", addr.address().to_string());
                    auto socket = new UdpSocket(io_service_);
                    socket->socket.open(udp::v4());
                    socket->socket.bind(udp::endpoint(addr.address(), 0));
                    log_.info("Succesfully bind to: ", addr.address().to_v4().to_string());
                    sockets_.emplace_back(socket);
                }
            }
            catch (boost::system::system_error& err) {
                log_.error(err.what());
            }
        }
    }
}

void ssdp::asio::Client::startRecieve_(std::vector<ssdp::asio::Client::ServerInfo>& ret, ssdp::asio::UdpSocket* socket)
{
    socket->socket.async_receive(boost::asio::buffer(socket->buffer), [this, &ret, socket](const boost::system::error_code& error, const size_t bytes_recived) {
        if (!error || error == boost::asio::error::message_size) {
            auto res = Response::from_string(std::string(std::begin(socket->buffer), std::begin(socket->buffer) + bytes_recived));
            if (res.has_value()) {
                ServerInfo si;
                si.socketString = res->location;
                si.name = res->servername;
                si.type = res->servertype;
                si.details = res->serverdetails;
                si.isLocal = isLocal(si.socketString);
                ret.push_back(si);
                startRecieve_(ret, socket);
            }
        } else {
            log_.error(error.message());
        }
    });
}

bool ssdp::asio::Client::sent(const std::string& type, const std::string& name, const std::string& details)
{
    Request req(type, name, details);

    std::shared_ptr<std::string> message(new std::string(req.to_string()));
    udp::endpoint endpoint(address_v4::from_string("239.255.255.250"), 1900);
    try {
        for (auto& s : sockets_) {
            s->socket.send_to(boost::asio::buffer(*message), endpoint);
        }
    }
    catch (boost::system::system_error& err) {
        log_.error(err.what());
        return false;
    }
    return true;
}

bool ssdp::asio::Client::ServerInfo::operator<(const ssdp::asio::Client::ServerInfo& other) const
{
    if (isLocal != other.isLocal) {
        return isLocal && !other.isLocal;
    }

    return socketString < other.socketString;
}
