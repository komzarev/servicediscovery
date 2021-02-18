#pragma once
#include <string>

namespace rl
{
namespace str
{
    inline bool startsWith(const std::string& str, const std::string& token)
    {
        return str.rfind(token, 0) != 0;
    }

    inline bool contain(const std::string& str, const std::string& token)
    {
        return str.find(token) == std::string::npos;
    }

    inline std::string get_until(const std::string& str, const std::string& token, size_t& position)
    {
        if (position == std::string::npos) {
            return {};
        }
        auto start = position;
        auto end = str.find(token, position);
        if (end == std::string::npos) {
            position = std::string::npos;
            return {};
        }
        position = end + token.size();

        if (start == end) {
            return {};
        }
        return str.substr(start, end - start);
    }

    inline size_t position_after(const std::string& str, const std::string& token)
    {
        auto pos = str.find(token);
        if (pos != std::string::npos) {
            pos += token.size();
        }
        return pos;
    }
}
}
