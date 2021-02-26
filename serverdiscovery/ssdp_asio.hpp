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

    //    using UdpSocket = boost::asio::ip::udp::socket;

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

        Client()
            : timer_(io_service_)
            , socket_(io_service_)
        {
            socket_.open(boost::asio::ip::udp::v4());
        }

        ~Client()
        {
            socket_.close();
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
        std::vector<ServerInfo> findAllServers_(const std::string& type, const std::string& name, const std::string& details, int timeout_ms, bool onlyOne)
        {
            std::vector<ServerInfo> ret;
            if (!sent(type, name, details)) {
                return ret;
            }

            timer_.expires_after(std::chrono::milliseconds(timeout_ms));
            timer_.async_wait([this](const boost::system::error_code& error) {
                if (!error) {
                    socket_.cancel();
                    //                    socket_.close();
                    isRunning = false;
                }
            });

            startRecieve_(ret, onlyOne);
            isRunning = true;

            while (isRunning) { //socket_.is_open() always true
                io_service_.run_one();
            }
            return ret;
        }

        void startRecieve_(std::vector<ServerInfo>& ret, bool onlyOne = false)
        {
            socket_.async_receive(boost::asio::buffer(buffer_), [this, &ret, onlyOne](const boost::system::error_code& error, const size_t bytes_recived) {
                if (!error || error == boost::asio::error::message_size) {
                    auto res = Response::from_string(std::string(std::begin(buffer_), std::begin(buffer_) + bytes_recived));
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
                            startRecieve_(ret, onlyOne);
                        }
                    }
                }
            });
        }
        bool sent(const std::string& type, const std::string& name, const std::string& details);

        std::array<char, 1024> buffer_;
        bool isRunning = false;
        boost::asio::io_service io_service_;
        boost::asio::steady_timer timer_;
        boost::asio::ip::udp::socket socket_;
    };
}
}

#endif // SSDP_ASIO_HPP
