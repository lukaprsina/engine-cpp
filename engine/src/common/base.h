#pragma once

// Crash if false
#define ENG_ASSERT(condition, ...) assert(condition)

#define ENG_BIT(x) (1 << x)

#define ENG_BIND_EVENT_FN(fn) [this](auto &&...args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG)
#define ENG_DEBUG
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

ENG_DISABLE_WARNINGS()
#include <glm/gtx/hash.hpp>
ENG_ENABLE_WARNINGS()

#include <set>
#include <ostream>
#include <sstream>

namespace engine
{
    template <class T>
    uint32_t ToUint32_t(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "T must be numeric");

        if (static_cast<uintmax_t>(value) > static_cast<uintmax_t>(std::numeric_limits<uint32_t>::max()))
        {
            throw std::runtime_error("ToUint32_t() failed, value is too big to be converted to uint32_t");
        }

        return static_cast<uint32_t>(value);
    }

    template <typename T>
    inline std::vector<uint8_t> ToBytes(const T &value)
    {
        return std::vector<uint8_t>{reinterpret_cast<const uint8_t *>(&value),
                                    reinterpret_cast<const uint8_t *>(&value) + sizeof(T)};
    }

    template <class T>
    inline std::string ToString(const T &value)
    {
        std::stringstream ss;
        ss << std::fixed << value;
        return ss.str();
    }

    template <typename T>
    inline void Read(std::istringstream &is, T &value)
    {
        is.read(reinterpret_cast<char *>(&value), sizeof(T));
    }

    inline void Read(std::istringstream &is, std::string &value)
    {
        std::size_t size;
        Read(is, size);
        value.resize(size);
        is.read(const_cast<char *>(value.data()), size);
    }

    template <class T>
    inline void Read(std::istringstream &is, std::set<T> &value)
    {
        std::size_t size;
        Read(is, size);
        for (uint32_t i = 0; i < size; i++)
        {
            T item;
            is.read(reinterpret_cast<char *>(&item), sizeof(T));
            value.insert(std::move(item));
        }
    }

    template <class T>
    inline void Read(std::istringstream &is, std::vector<T> &value)
    {
        std::size_t size;
        Read(is, size);
        value.resize(size);
        is.read(reinterpret_cast<char *>(value.data()), value.size() * sizeof(T));
    }

    template <class T, class S>
    inline void Read(std::istringstream &is, std::map<T, S> &value)
    {
        std::size_t size;
        Read(is, size);

        for (uint32_t i = 0; i < size; i++)
        {
            std::pair<T, S> item;
            Read(is, item.first);
            Read(is, item.second);

            value.insert(std::move(item));
        }
    }

    template <class T, uint32_t N>
    inline void Read(std::istringstream &is, std::array<T, N> &value)
    {
        is.read(reinterpret_cast<char *>(value.data()), N * sizeof(T));
    }

    template <typename T, typename... Args>
    inline void Read(std::istringstream &is, T &first_arg, Args &...args)
    {
        Read(is, first_arg);

        Read(is, args...);
    }

    template <typename T>
    inline void Write(std::ostringstream &os, const T &value)
    {
        os.write(reinterpret_cast<const char *>(&value), sizeof(T));
    }

    inline void Write(std::ostringstream &os, const std::string &value)
    {
        Write(os, value.size());
        os.write(value.data(), value.size());
    }

    template <class T>
    inline void Write(std::ostringstream &os, const std::set<T> &value)
    {
        Write(os, value.size());
        for (const T &item : value)
        {
            os.write(reinterpret_cast<const char *>(&item), sizeof(T));
        }
    }

    template <class T>
    inline void Write(std::ostringstream &os, const std::vector<T> &value)
    {
        Write(os, value.size());
        os.write(reinterpret_cast<const char *>(value.data()), value.size() * sizeof(T));
    }

    template <class T, class S>
    inline void Write(std::ostringstream &os, const std::map<T, S> &value)
    {
        Write(os, value.size());

        for (const std::pair<T, S> &item : value)
        {
            Write(os, item.first);
            Write(os, item.second);
        }
    }

    template <class T, uint32_t N>
    inline void Write(std::ostringstream &os, const std::array<T, N> &value)
    {
        os.write(reinterpret_cast<const char *>(value.data()), N * sizeof(T));
    }

    template <typename T, typename... Args>
    inline void Write(std::ostringstream &os, const T &first_arg, const Args &...args)
    {
        Write(os, first_arg);

        Write(os, args...);
    }

    template <class T>
    inline void HashCombine(size_t &seed, const T &v)
    {
        std::hash<T> hasher;
        glm::detail::hash_combine(seed, hasher(v));
    }

    template <class T, class Y>
    struct TypeCast
    {
        Y operator()(T value) const noexcept
        {
            return static_cast<Y>(value);
        }
    };

    std::string ToSnakeCase(const std::string &text);

    template <class T>
    using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;
}