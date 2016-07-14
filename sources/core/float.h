#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FLOAT_H_
#define _SPICA_FLOAT_H_

#include <cmath>
#include <cstring>

inline decltype(auto) floatToBits(double d) {
    unsigned long long ret;
    memcpy(&ret, &d, sizeof(double));
    return ret;
}

inline decltype(auto) bitsToFloat(unsigned long long bits) {
    double ret;
    memcpy(&ret, &bits, sizeof(double));
    return ret;
}

inline decltype(auto) nextFloatUp(double d) {
    if (std::isinf(d) && d > 0.0) return d;
    if (d == -0.0f) d = 0.0f;
    auto ui = floatToBits(d);
    if (d > 0.0) {
        ui++;
    } else {
        ui--;
    }
    return bitsToFloat(ui);
}

inline decltype(auto) nextFloatDown(double d) {
    if (std::isinf(d) && d > 0.0) return d;
    if (d == -0.0f) d = 0.0f;
    auto ui = floatToBits(d);
    if (d > 0.0) {
        ui--;
    } else {
        ui++;
    }
    return bitsToFloat(ui);    
}

#endif  // _SPICA_FLOAT_H_
