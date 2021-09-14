#ifndef SERVERDISCOVERY_HPP
#define SERVERDISCOVERY_HPP

#include "string_find.h"
#include "string_format.h"
#include "string_trim.h"
#if __cplusplus < 201703L
#include "tl/optional.hpp"
using tl::nullopt;
using tl::optional;
#else
#include <optional>
using std::nullopt;
using std::optional;
#endif
#include <memory>
#include <stdexcept>
#include <string>

namespace ssdp
{

class Request
{
public:
    std::string to_string() const
    {
        static const char format[] = "M-SEARCH * HTTP/1.1\n"
                                     "HOST: 239.255.255.250:1900\n"
                                     "MAN: \"ssdp:discover\"\n"
                                     "MX: 200\n"
                                     "ST: urn:%s:%s:%s\n"
                                     "USER-AGENT: 007 UPnP/1.1\n";

        return rl::str::format(format, servertype.c_str(), servername.c_str(), serverdetails.c_str());
    }

    Request(std::string servertypeT, std::string servernameT, std::string serverdetailsT)
        : servertype(std::move(servertypeT))
        , servername(std::move(servernameT))
        , serverdetails(std::move(serverdetailsT))
    {
    }

    Request() = default;

    static optional<Request> from_string(const std::string& str)
    {
        using namespace rl::str;

        Request ret;
        if (str.rfind("M-SEARCH", 0) != 0 || str.find("MAN: \"ssdp:discover\"") == std::string::npos) {
            return nullopt;
        }

        auto urnpos = position_after(str, "ST: urn:");
        ret.servertype = get_until(str, ":", urnpos);
        ret.servername = get_until(str, ":", urnpos);
        ret.serverdetails = get_until(str, "\n", urnpos);
        if (urnpos == std::string::npos) {
            return nullopt;
        }

        trim(ret.servername);
        trim(ret.servertype);
        trim(ret.serverdetails);

        return ret;
    }

    std::string servertype;
    std::string servername;
    std::string serverdetails;
};

class Response
{
public:
    std::string to_string() const
    {
        static const char format[] = "HTTP/1.1 200 OK\n"
                                     "SERVER: %s UPnP/1.1\n"
                                     "CACHE-CONTROL: max-age=1200\n"
                                     "LOCATION: http://%s\n"
                                     "NTS: ssdp:alive \n"
                                     "ST: urn:%s:%s:%s\n"
                                     "Content-Length: 0\n";

        return rl::str::format(format, servertype.c_str(), location.c_str(), servertype.c_str(), servername.c_str(), serverdetails.c_str());
    }

    Response(std::string servertypeT, std::string servernameT, std::string serverdetailsT, std::string locationT)
        : servertype(std::move(servertypeT))
        , servername(std::move(servernameT))
        , serverdetails(std::move(serverdetailsT))
        , location(std::move(locationT))
    {
    }

    Response() = default;

    static optional<Response> from_string(const std::string& str)
    {
        using namespace rl::str;

        Response ret;
        if (str.rfind("HTTP/1.1 200 OK", 0) != 0 || str.find("NTS: ssdp:alive") == std::string::npos) {
            return nullopt;
        }

        auto urnpos = position_after(str, "ST: urn:");
        ret.servertype = get_until(str, ":", urnpos);
        ret.servername = get_until(str, ":", urnpos);
        ret.serverdetails = get_until(str, "\n", urnpos);
        if (urnpos == std::string::npos) {
            return nullopt;
        }

        urnpos = position_after(str, "LOCATION: http://");
        ret.location = get_until(str, "\n", urnpos);
        if (urnpos == std::string::npos) {
            return nullopt;
        }

        trim(ret.servername);
        trim(ret.servertype);
        trim(ret.serverdetails);
        trim(ret.location);

        return ret;
    }

    bool matchRequest(const Request& req)
    {
        return req.servertype == servertype
            && (req.servername.empty() || req.servername == servername)
            && (req.serverdetails.empty() || req.serverdetails == serverdetails);
    }

    std::string servertype;
    std::string servername;
    std::string serverdetails;
    std::string location;
};
}

#endif // SERVERDISCOVERY_HPP
