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

    using UdpSocket = boost::asio::ip::udp::socket;
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
                s->close();
            }
        }

        void setServicePort(const std::string& port)
        {
            port_ = port;
        }

    private:
        void readPendingDatagrams(const boost::system::error_code& error, const size_t bytes_recived, boost::asio::ip::udp::endpoint addr_from, UdpSocket* socket);

        void start_receive(boost::asio::ip::udp::socket* socket);

        UdpSockets sockets_;
        Response resp_;
        std::string port_;
        const static int MAX_DATA_LEN = 1024;
        char data_[MAX_DATA_LEN];
        boost::asio::ip::udp::endpoint remote_endpoint_;
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
            auto list = findAllServers(type, name, details, timeout_ms);
            if (!list.empty()) {
                ret = list[0].socketString;
            }
            return ret;
        }

        std::vector<ServerInfo> findAllServers(const std::string& type, const std::string& name, const std::string& details, int timeout_ms = 500)
        {
            std::vector<ServerInfo> ret;
            if (!sent(type, name, details)) {
                return ret;
            }

            boost::asio::ip::udp::endpoint sender_endpoint;

            timer_.expires_after(std::chrono::milliseconds(timeout_ms));

            timer_.async_wait([this](const boost::system::error_code& error) {
                if (!error) {
                    socket_.cancel();
                    //                    socket_.close();
                    isRunning = false;
                }
            });

            socket_.async_receive(boost::asio::buffer(data_, MAX_DATA_LEN), [this, &ret](const boost::system::error_code& error, const size_t bytes_recived) {
                if (!error || error == boost::asio::error::message_size) {
                    auto res = Response::from_string(std::string(std::begin(data_), std::begin(data_) + bytes_recived));
                    if (res.has_value()) {
                        ServerInfo si;
                        si.socketString = res->location;
                        si.name = res->servername;
                        si.type = res->servertype;
                        si.details = res->serverdetails;
                        ret.push_back(si);
                    }
                }
            });

            isRunning = true;
            while (isRunning) { //socket_.is_open() always true
                io_service_.run_one();
            }
            return ret;
        }

    private:
        bool sent(const std::string& type, const std::string& name, const std::string& details);

        bool isRunning = false;
        const static int MAX_DATA_LEN = 1024;
        char data_[MAX_DATA_LEN];
        boost::asio::io_service io_service_;
        boost::asio::steady_timer timer_;
        boost::asio::ip::udp::socket socket_;
    };
}
}

#endif // SSDP_ASIO_HPP
