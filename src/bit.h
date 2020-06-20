/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#pragma once

#if __cplusplus >= 202002L
#include <bit>
using std::countl_zero;
using std::countr_zero;
#else
#include <limits>
#include <type_traits>

template<typename T>
int countl_zero(T v) noexcept
{
    using UT = typename std::make_unsigned<T>::type;
#if __GNUC__
    if (sizeof(T) == sizeof(long long))
        return __builtin_clzll(v);
    if (sizeof(T) == sizeof(long))
        return __builtin_clzl(v);
    /* if (sizeof(T) <= sizeof(int)) */
    constexpr auto usd = std::numeric_limits<unsigned>::digits;
    constexpr auto utd = std::numeric_limits<UT>::digits;
    const UT uv = v;
    return __builtin_clz(uv) - (usd - utd);
#elif _MSC_VER
    unsigned long r;
    if (sizeof(T) == 8) {
        _BitScanReverse64(&r, v);
        return std::numeric_limits<UT>::digits - 1 - r;
    }
    /* if (sizeof(T) <= 4) */
    constexpr auto utd = std::numeric_limits<UT>::digits;
    const UT uv = v;
    _BitScanReverse(&r, uv);
    return utd - 1 - r;

#else
#error "No bit ops support for your compiler."
#endif
}

template<typename T>
int countr_zero(T v) noexcept
{
    using UT = typename std::make_unsigned<T>::type;
#if __GNUC__
    if (sizeof(T) == sizeof(long long))
        return __builtin_ctzll(v);
    if (sizeof(T) == sizeof(long))
        return __builtin_ctzl(v);
    UT uv = v;
    return __builtin_ctz(uv);
#elif _MSC_VER
    unsigned long r;
    if (sizeof(T) == 8) {
        _BitScanForward64(&r, v);
        return r;
    }
    /* if (sizeof(T) <= 4) */
    UT uv = v;
    _BitScanForward(&r, uv);
    return r;

#else
#error "No bit ops support for your compiler."
#endif
}
#endif
