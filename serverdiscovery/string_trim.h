#ifndef stringtrim_h__
#define stringtrim_h__

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <iostream>

namespace rl
{
namespace str
{
    inline void oneline(std::string& s)
    {
        auto it = s.find("\r");
        while (it != std::string::npos) {
            s.replace(it, 1, " ");
            it = s.find("\r");
        }

        it = s.find("\n");
        while (it != std::string::npos) {
            s.replace(it, 1, " ");
            it = s.find("\n");
        }
    }

    inline std::string onelined(std::string s)
    {
        oneline(s);
        return s;
    }

    // trim from start (in place)
    inline void ltrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    }

    // trim from end (in place)
    inline void rtrim(std::string& s)
    {

        auto it = std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); });

        s.erase(it.base(), s.end());
    }

    // trim from both ends (in place)
    inline void trim(std::string& s)
    {
        ltrim(s);
        rtrim(s);
    }

    // trim from start (copying)
    inline std::string ltrimmed(std::string s)
    {
        ltrim(s);
        return s;
    }

    // trim from end (copying)
    inline std::string rtrimmed(std::string s)
    {
        rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    inline std::string trimmed(std::string s)
    {
        trim(s);
        return s;
    }

}
}

#endif // stringtrim_h__
