#include "ssdp_asio.hpp"

struct with_separator
{
    with_separator(std::string sep)
        : sep(std::move(sep))
    {
    }

    std::string sep;
};

struct separated_stream
{
    separated_stream(std::ostream& stream, std::string sep)
        : _stream(stream)
        , _sep(std::move(sep))
        , _first(true)
    {
    }

    template <class Rhs>
    separated_stream& operator<<(Rhs&& rhs)
    {
        if (_first)
            _first = false;
        else
            _stream << _sep;

        _stream << std::forward<Rhs>(rhs);
        return *this;
    }

    separated_stream& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        manip(_stream);
        return *this;
    }

private:
    std::ostream& _stream;
    std::string _sep;
    bool _first;
};

separated_stream operator<<(std::ostream& stream, with_separator wsep)
{
    return separated_stream(stream, std::move(wsep.sep));
}

int main(int argc, char* argv[])
{
    ssdp::asio::Client client;
    client.setDebugMode(true);
    while (true) {
        auto list = client.resolve("mpnet_server", "", "", 5000);
        if (list.empty()) {
            std::cout << "No servers\n";
        } else {

            for (auto l : list) {
                std::cout << with_separator(" ") << l.type << l.name << l.details << l.socketString << '\n';
            }

            std::cout << "======\n";
        }
#ifdef _WIN32
        ::Sleep(1000);
#else
        sleep(1);
#endif
    }
    return 0;
}
