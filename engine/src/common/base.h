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

template <class T>
uint32_t to_u32(T value)
{
    static_assert(std::is_arithmetic<T>::value, "T must be numeric");

    if (static_cast<uintmax_t>(value) > static_cast<uintmax_t>(std::numeric_limits<uint32_t>::max()))
    {
        throw std::runtime_error("to_u32() failed, value is too big to be converted to uint32_t");
    }

    return static_cast<uint32_t>(value);
}