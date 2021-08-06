#pragma once

#include "common/glm.h"

ENG_DISABLE_WARNINGS()
#include <glm/gtx/hash.hpp>
ENG_ENABLE_WARNINGS()

#include <set>

namespace engine
{
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
}
