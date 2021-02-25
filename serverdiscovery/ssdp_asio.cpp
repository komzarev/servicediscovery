#include "ssdp_asio.hpp"

using namespace boost::asio::ip;

bool ssdp::asio::Server::start(const std::string& name, const std::string& details, boost::asio::io_service& io_service)
{
    if (port_.empty()) {
        return false;
    }

    udp::resolver resolver(io_service);
    udp::resolver::query query(host_name(), "");
    udp::resolver::iterator it = resolver.resolve(query);
    auto multicast_addr = address::from_string("239.255.255.250");
    udp::endpoint endpoint(boost::asio::ip::address_v4::any(), 1900);

    while (it != udp::resolver::iterator()) {
        auto addr = (it++)->endpoint();
        if (addr.address().is_v4() && !addr.address().is_loopback()) {
            auto socket = new UdpSocket(io_service);
            socket->open(endpoint.protocol());
            socket->bind(addr);
            socket->set_option(udp::socket::reuse_address(true));
            socket->set_option(multicast::join_group(multicast_addr));
            start_receive(socket);
            sockets_.emplace_back(socket);
        }
    }

    //            socket_->bind(QHostAddress::AnyIPv4, 1900, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    resp_.servername = name;
    resp_.serverdetails = details;

    return true;
}

void ssdp::asio::Server::readPendingDatagrams(const boost::system::error_code& error, const size_t bytes_recived, udp::endpoint addr_from, UdpSocket* socket)
{
    if (!error || error == boost::asio::error::message_size) {
        auto data = std::string(std::begin(data_), std::begin(data_) + bytes_recived);
        if (auto req = Request::from_string(data.data())) {
            if (resp_.matchRequest(req.value())) {
                resp_.location = socket->local_endpoint().address().to_string() + ":" + port_;
                socket->async_send_to(boost::asio::buffer(resp_.location), addr_from, [](const boost::system::error_code&, std::size_t) {

                });
            }
        }
    }
}

void ssdp::asio::Server::start_receive(udp::socket* socket)
{
    socket->async_receive_from(boost::asio::buffer(data_, MAX_DATA_LEN), remote_endpoint_, [this, socket](const boost::system::error_code& error, const size_t bytes_recived) {
        readPendingDatagrams(error, bytes_recived, remote_endpoint_, socket);
        start_receive(socket);
    });
}

bool ssdp::asio::Client::sent(const std::string& type, const std::string& name, const std::string& details)
{
    Request req(type, name, details);

    std::shared_ptr<std::string> message(new std::string(req.to_string()));
    udp::endpoint endpoint(address_v4::from_string("239.255.255.250"), 1900);
    try {
        socket_.send_to(boost::asio::buffer(*message), endpoint);
    }
    catch (boost::system::system_error& err) {
        std::cout << err.code().value() << '\n';
        std::cout << err.code().category().name() << '\n';
        std::cout << err.what() << '\n';
        return false;
    }
    return true;
}
