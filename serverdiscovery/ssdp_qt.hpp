#ifndef QTSERVER_HPP
#define QTSERVER_HPP

#include <QDeadlineTimer>
#include <QElapsedTimer>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QObject>
#include <QTimer>

#include "ssdp_message.hpp"
#include <QUdpSocket>
#include <QtCore/qglobal.h>

namespace ssdp
{
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
        Server(const QString& type, QObject* parent = 0)
            : QObject(parent)
        {
            resp_.servertype = type.toLatin1().data();
        }

        ~Server()
        {
            stop();
        }

        void setDebugMode(bool isDebug)
        {
            log.setDebugMode(isDebug);
        }
        //************************************
        // Method:    start
        // FullName:  ssdp::qt::Server::start
        // Access:    public
        // Returns:   bool
        // Parameter: const QString & name, name of server can be empty
        // Parameter: const QString & details, detail of server in mpnet subnet can be empty
        // Launch server nonblocking and return false when server can't be started
        //************************************
        bool start(const QString& name, const QString& details);

        //************************************
        // Method:    stop
        // FullName:  ssdp::qt::Server::stop
        // Access:    public
        // Returns:   void
        // Stops server and delete data
        //************************************
        void stop();

        //************************************
        // Method:    setServicePort
        // FullName:  ssdp::qt::Server::setServicePort
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
        void readPendingDatagrams()
        {
            while (socket_->hasPendingDatagrams()) {
                auto datagram = socket_->receiveDatagram();
                processDatagram(datagram);
            }
        }

    private:
        void processDatagram(const QNetworkDatagram& dg);
        Response resp_;
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
        };

        Client(QObject* parent = 0);

        //************************************
        // Method:    findConnetionString
        // FullName:  ssdp::qt::Client::findConnetionString
        // Access:    public
        // Returns:   QString
        // Parameter: const QString & type
        // Parameter: const QString & name
        // Parameter: const QString & details
        // Parameter: uint32_t timeout_ms
        // Find connection string for first matched server, blocks for timeout_ms or until server found
        //************************************
        QString findConnetionString(const QString& type, const QString& name, const QString& details, uint32_t timeout_ms = 500);

        //************************************
        // Method:    findAllServers
        // FullName:  ssdp::qt::Client::findAllServers
        // Access:    public
        // Returns:   QList<ssdp::qt::Client::ServerInfo>
        // Parameter: const QString & type
        // Parameter: const QString & name
        // Parameter: const QString & details
        // Parameter: uint32_t timeout_ms
        // Returns all matched server which responded during timeout_ms, always blocks for timeout_ms
        //************************************
        QList<ServerInfo> findAllServers(const QString& type, const QString& name, const QString& details, uint32_t timeout_ms = 500);

        static bool isLocal(const QString& socketString);

        void setDebugMode(bool isDebug)
        {
            log.setDebugMode(isDebug);
        }

    private:
        void updateInterfaces_();
        QList<ServerInfo> findAllServers_(const QString& type, const QString& name, const QString& details, int timeout_ms, bool onlyOnce);
        bool sent(const QString& type, const QString& name, const QString& details);
        QStringList joinedInterfaces_;
        std::vector<std::unique_ptr<QUdpSocket>> sockets;
        Logger log;
    };
}
}

#endif // QTSERVER_HPP
