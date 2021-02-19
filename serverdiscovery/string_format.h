#pragma once
#include <memory>
#include <string>
#include <stdio.h>

namespace rl
{

namespace str
{
#if __cplusplus >= 201703L
    namespace details
    {

        template <typename T>
        auto convert(T&& t)
        {
            if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value) {
                return std::forward<T>(t).c_str();
            } else {
                return std::forward<T>(t);
            }
        }

        template <typename... Args>
        std::string stringFormatInternal(const std::string& format, Args&&... args)
        {
            size_t size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1;
            if (size <= 0) {
                return {};
            }
            std::unique_ptr<char[]> buf(new char[size]);
            snprintf(buf.get(), size, format.c_str(), args...);
            return std::string(buf.get(), buf.get() + size - 1);
        }
    }

    template <typename... Args>
    inline std::string format(std::string fmt, Args&&... args)
    {
        return details::stringFormatInternal(fmt, details::convert(std::forward<Args>(args))...);
    }
#else
    template <typename... Args>
    inline std::string format(std::string format, Args&&... args)
    {
        size_t size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1;
        if (size <= 0) {
            return {};
        }
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
#endif
}
}
