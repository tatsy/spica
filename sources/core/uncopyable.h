#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_UNCOPYABLE_H_
#define _SPICA_UNCOPYABLE_H_

#include "core/common.h"

namespace spica {

/**
 * Interface class which forbids copy and assignment
 */
class SPICA_EXPORTS Uncopyable {
public:
    Uncopyable() = default;
    virtual ~Uncopyable() = default;

    Uncopyable(const Uncopyable&) = delete;
    Uncopyable& operator=(const Uncopyable&) = delete;
};

}  // namespace spica

#endif  // _SPICA_UNCOPYABLE_H_
