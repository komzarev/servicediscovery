#ifndef SSDP_ASIO_HPP
#define SSDP_ASIO_HPP

#include "boost/asio.hpp"
#include "boost/asio/system_timer.hpp"
#include "ssdp_message.hpp"
#include <memory>
#include <vector>

namespace ssdp
{
namespace asio
{

    class UdpSocket
    {
    public:
        UdpSocket(boost::asio::io_service& io_service)
            : socket(io_service)
        {
        }

        boost::asio::ip::udp::socket socket;
        boost::asio::ip::udp::endpoint remote_endpoint;
        std::array<char, 1024> buffer;
    };

    using UdpSockets = std::vector<std::unique_ptr<UdpSocket>>;

    class Server
    {

    public:
        Server(const std::string& type)
        {
            resp_.servertype = type;
        }

        bool start(const std::string& name, const std::string& details, boost::asio::io_service& io_service);

        void stop()
        {
            for (auto& s : sockets_) {
                s->socket.close();
            }
        }

        void setServicePort(const std::string& port)
        {
            port_ = port;
        }

    private:
        void readPendingDatagrams(const boost::system::error_code& error, const size_t bytes_recived, UdpSocket* socket);

        void start_receive(UdpSocket* socket);

        UdpSockets sockets_;
        Response resp_;
        std::string port_;
    };

    class Client
    {

    public:
        struct ServerInfo
        {
            std::string type;
            std::string name;
            std::string details;
            std::string socketString;
        };

        Client();

        ~Client()
        {
            for (auto& s : sockets_) {
                s->socket.close();
            }
        }

        std::string findConnetionString(const std::string& type, const std::string& name, const std::string& details, int timeout_ms = 500)
        {
            std::string ret;
            auto list = findAllServers_(type, name, details, timeout_ms, true);
            if (!list.empty()) {
                ret = list[0].socketString;
            }
            return ret;
        }

        std::vector<ServerInfo> findAllServers(const std::string& type, const std::string& name, const std::string& details, int timeout_ms = 500)
        {
            return findAllServers_(type, name, details, timeout_ms, false);
        }

    private:
        std::vector<ServerInfo> findAllServers_(const std::string& type, const std::string& name, const std::string& details, int timeout_ms, bool onlyOne);

        void startRecieve_(std::vector<ServerInfo>& ret, bool onlyOne, UdpSocket* socket);
        bool sent(const std::string& type, const std::string& name, const std::string& details);

        bool isRunning = false;
        boost::asio::io_service io_service_;
        boost::asio::steady_timer timer_;
        UdpSockets sockets_;
    };
}
}

#endif // SSDP_ASIO_HPP
