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

    class Logger
    {
    public:
        template <typename T, typename... Args>
        void info(T&& msg, Args&&... args)
        {

            if (isDebugMode_) {
                std::cout << "[SSDP][INFO]: " << std::forward<T>(msg);
                print_(args...);
                std::cout << '\n';
            }
        }
        template <typename T, typename... Args>
        void error(T&& msg, Args&&... args)
        {
            if (isDebugMode_) {
                std::cout << "[SSDP][ERROR]: " << std::forward<T>(msg);
                print_(args...);
                std::cout << '\n';
            }
        }
        void setDebugMode(bool isDebug)
        {
            isDebugMode_ = isDebug;
        }
        bool isDebugMode() const
        {
            return isDebugMode_;
        }

    private:
        void print_()
        {
        }

        template <typename T, typename... Args>
        void print_(T t, Args... args)
        {
            std::cout << t;
            print_(args...);
        }

        bool isDebugMode_ = false;
    };

    using UdpSockets = std::vector<std::unique_ptr<UdpSocket>>;

    class Server
    {

    public:
        Server(std::string type);

        ~Server();

        bool start(std::string name, std::string details, boost::asio::io_service& io_service);

        void stop();

        void setServicePort(std::string port)
        {
            port_ = std::move(port);
        }

        void setDebugMode(bool isDebug)
        {
            log_.setDebugMode(isDebug);
        }

        void updateInterfacesList(boost::asio::io_service& io_service);

    private:
        void readPendingDatagrams(const boost::system::error_code& error, const size_t bytes_recived, UdpSocket* socket);

        void startReceive(UdpSocket* socket);

        Logger log_;
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
            bool isLocal;
            bool operator<(const ServerInfo& other) const;
        };

        Client();

        ~Client();

        void setDebugMode(bool isDebug)
        {
            log_.setDebugMode(isDebug);
        }

        std::vector<ServerInfo> resolve(const std::string& serviceType, const std::string& serviceName, const std::string& serviceDetails, uint32_t timeout_ms = 5000);

        bool isLocal(const std::string& socketString);

    private:
        void updateInterfaces_();
        void startRecieve_(std::vector<ServerInfo>& ret, UdpSocket* socket);
        bool sent(const std::string& type, const std::string& name, const std::string& details);

        bool isRunning = false;
        boost::asio::io_service io_service_;
        boost::asio::steady_timer timer_;
        UdpSockets sockets_;
        std::vector<std::string> joinedInterfaces_;
        Logger log_;
    };
}
}

#endif // SSDP_ASIO_HPP
