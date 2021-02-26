#include "ssdp_asio.hpp"

int main(int argc, char* argv[])
{
    boost::asio::io_service io_service;
    ssdp::asio::Server server("mpnet_server");
    server.setServicePort("5758");
    if (!server.start("asio", "57", io_service)) {
        std::cerr << "Fail to start";
        return -1;
    }

    return io_service.run();
}
