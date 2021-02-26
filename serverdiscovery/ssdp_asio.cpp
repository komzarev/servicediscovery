#include "ssdp_asio.hpp"

using namespace boost::asio::ip;

bool ssdp::asio::Server::start(const std::string& name, const std::string& details, boost::asio::io_service& io_service)
{
    if (port_.empty()) {
        return false;
    }

    resp_.servername = name;
    resp_.serverdetails = details;

    udp::resolver resolver(io_service);
    udp::resolver::query query(host_name(), "");
    udp::resolver::iterator it = resolver.resolve(query);
    auto multicast_addr = address::from_string("239.255.255.250");

    while (it != udp::resolver::iterator()) {
        auto addr = (it++)->endpoint();
        if (addr.address().is_v4()) {

            try {
                auto socket = new UdpSocket(io_service);
                socket->socket.open(boost::asio::ip::udp::v4());
                socket->socket.set_option(udp::socket::reuse_address(true));
                socket->socket.bind(udp::endpoint(addr.address(), 1900));
                socket->socket.set_option(multicast::join_group(multicast_addr));
                std::cout << "Succesfully bind to: " << addr.address().to_v4().to_string() << "\n";
                sockets_.emplace_back(socket);
                start_receive(socket);
            }
            catch (boost::system::system_error& err) {
                std::cout << err.what() << '\n';
                return false;
            }
        }
    }

    return true;
}

void ssdp::asio::Server::readPendingDatagrams(const boost::system::error_code& error, const size_t bytes_recived, UdpSocket* socket)
{
    try {
        if (!error || error == boost::asio::error::message_size) {
            auto data = std::string(std::begin(socket->buffer), std::begin(socket->buffer) + bytes_recived);
            if (auto req = Request::from_string(data.data())) {
                if (resp_.matchRequest(req.value())) {
                    resp_.location = socket->socket.local_endpoint().address().to_string() + ":" + port_;
                    std::cout << "Answer to: " << socket->remote_endpoint.address().to_string() << "\n";
                    socket->socket.send_to(boost::asio::buffer(resp_.to_string()), socket->remote_endpoint);
                }
            }
        }
    }
    catch (boost::system::system_error& err) {
        std::cout << err.what() << '\n';
    }
}

void ssdp::asio::Server::start_receive(UdpSocket* socket)
{
    socket->socket.async_receive_from(boost::asio::buffer(socket->buffer), socket->remote_endpoint, [this, socket](const boost::system::error_code& error, const size_t bytes_recived) {
        readPendingDatagrams(error, bytes_recived, socket);
        start_receive(socket);
    });
}

ssdp::asio::Client::Client()
    : timer_(io_service_)
{
    udp::resolver resolver(io_service_);
    udp::resolver::query query(host_name(), "");
    udp::resolver::iterator it = resolver.resolve(query);

    while (it != udp::resolver::iterator()) {
        auto addr = (it++)->endpoint();
        if (addr.address().is_v4()) {

            try {
                auto socket = new UdpSocket(io_service_);
                socket->socket.open(boost::asio::ip::udp::v4());
                socket->socket.bind(udp::endpoint(addr.address(), 0));
                std::cout << "Succesfully bind to: " << addr.address().to_v4().to_string() << "\n";
                sockets_.emplace_back(socket);
            }
            catch (boost::system::system_error& err) {
                std::cout << err.what() << '\n';
            }
        }
    }
}

std::vector<ssdp::asio::Client::ServerInfo> ssdp::asio::Client::findAllServers_(const std::string& type, const std::string& name, const std::string& details, int timeout_ms, bool onlyOne)
{
    std::vector<ServerInfo> ret;
    if (!sent(type, name, details)) {
        return ret;
    }

    timer_.expires_after(std::chrono::milliseconds(timeout_ms));
    timer_.async_wait([this](const boost::system::error_code& error) {
        if (!error) {
            for (auto& s : sockets_) {
                s->socket.cancel();
            }
            //                    socket_.close();
            isRunning = false;
        }
    });

    for (auto& s : sockets_) {
        startRecieve_(ret, onlyOne, s.get());
    }
    isRunning = true;

    while (isRunning) { //socket_.is_open() always true
        io_service_.run_one();
    }
    return ret;
}

void ssdp::asio::Client::startRecieve_(std::vector<ssdp::asio::Client::ServerInfo>& ret, bool onlyOne, ssdp::asio::UdpSocket* socket)
{
    socket->socket.async_receive(boost::asio::buffer(socket->buffer), [this, &ret, onlyOne, socket](const boost::system::error_code& error, const size_t bytes_recived) {
        if (!error || error == boost::asio::error::message_size) {
            auto res = Response::from_string(std::string(std::begin(socket->buffer), std::begin(socket->buffer) + bytes_recived));
            if (res.has_value()) {
                ServerInfo si;
                si.socketString = res->location;
                si.name = res->servername;
                si.type = res->servertype;
                si.details = res->serverdetails;
                ret.push_back(si);
                if (onlyOne) {
                    isRunning = false;
                } else {
                    startRecieve_(ret, onlyOne, socket);
                }
            }
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
        std::cout << err.what() << '\n';
        return false;
    }
    return true;
}
