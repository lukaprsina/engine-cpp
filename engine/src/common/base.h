#pragma once

#define ENG_ASSERT(condition) assert(condition)

#define BIT(x) (1 << x)

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
    using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;
}