#ifndef QT_SSDP_SERVER_HPP
#define QT_SSDP_SERVER_HPP

#include <QNetworkDatagram>
#include <QTimer>
#include <QUdpSocket>
#include <memory>
#include <vector>
#if __cplusplus < 201703L
#include "tl/optional.hpp"
using tl::nullopt;
using tl::optional;
#else
#include <optional>
using std::nullopt;
using std::optional;
#endif
namespace ssdp
{
class Response;
namespace qt
{
    class Logger
    {
    public:
        void info(const QString& msg);
        void error(const QString& msg);
        void setDebugMode(bool isDebug)
        {
            isDebugMode_ = isDebug;
        }
        bool isDebugMode() const
        {
            return isDebugMode_;
        }

    private:
        bool isDebugMode_ = false;
    };

    class Server : public QObject
    {
        Q_OBJECT

    public:
        Server(const QString& type, QObject* parent = 0);

        ~Server();

        void setDebugMode(bool isDebug);
        //************************************
        // Method:    start
        // FullName:  Server::start
        // Access:    public
        // Returns:   bool
        // Parameter: const QString & name, name of server can be empty
        // Parameter: const QString & details, detail of server in mpnet subnet can be empty
        // Launch server nonblocking and return false when server can't be started
        //************************************
        bool start(const QString& name, const QString& details);

        //************************************
        // Method:    stop
        // FullName:  Server::stop
        // Access:    public
        // Returns:   void
        // Stops server and delete data
        //************************************
        void stop();

        //************************************
        // Method:    setServicePort
        // FullName:  Server::setServicePort
        // Access:    public
        // Returns:   void
        // Parameter: const QString & port
        // Set port on which target service was launched, the port is used in response.
        // Port must be set before start() called.
        //************************************
        void setServicePort(const QString& port)
        {
            port_ = port;
        }

        void updateInterfacesList();

    private slots:
        void readPendingDatagrams();

    private:
        void processDatagram(const QNetworkDatagram& dg);
        std::unique_ptr<Response> resp_;
        QUdpSocket* socket_ = nullptr;
        QString port_;
        QStringList joinedInterfaces_;
        QTimer updateInterfaceListTimer_;
        Logger log;
    };

    class Client : public QObject
    {
        Q_OBJECT

    public:
        struct ServerInfo
        {
            QString type;
            QString name;
            QString details;
            QString socketString;
            bool isLocal;
            std::chrono::milliseconds responseTime;

            bool operator<(const ServerInfo& other) const;
        };

        struct ServerRequestInfo
        {
            QString serviceType;
            QString serviceName;
            QString serviceDetails;
            QString ipmask;
        };

        Client(QObject* parent = 0);
        ~Client();
        void setDebugMode(bool isDebug);

        static bool isLocal(const QString& socketString);
        static bool isIpMatchedToMask(const QString& ip, const QString& mask);
        static bool checkRequest(const ServerRequestInfo& req);

        optional<ServerInfo> resolve(const ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime = std::chrono::seconds(5));
        QList<ServerInfo> resolveAll(const ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime = std::chrono::seconds(5));

        bool startResolveAsync(const ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime = std::chrono::seconds(5));
        bool isRunning();
        void stopResolve();
    signals:
        void serverFound(const ServerInfo& server);
        void maxServerTimeElapsed();

    private:
        bool resolve_(const ServerRequestInfo& server, std::chrono::milliseconds maxServerWaitTime, std::function<bool(ServerInfo& si)> func);
        bool sent_(const ServerRequestInfo& server);
        void updateInterfaces_();
        bool sent(const QString& type, const QString& name, const QString& details);
        QStringList joinedInterfaces_;
        std::vector<std::unique_ptr<QUdpSocket>> sockets;
        Logger log;
        std::atomic_bool isRunning_{ false };
        std::unique_ptr<QTimer> timer_;
        std::chrono::milliseconds maxServerWaitTime_;
    };
}

}

Q_DECLARE_METATYPE(ssdp::qt::Client::ServerInfo);
#endif // QT_SSDP_SERVER_HPP
