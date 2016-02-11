#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RANDOM_BASE_H_
#define _SPICA_RANDOM_BASE_H_

#include <vector>

#include "../core/common.h"

namespace spica {

    /** Interface class for random number generator.
     *  @ingroup random_module
     */
    class SPICA_EXPORTS RandomInterface {
    public:
        virtual double get1D() = 0;
    };

}  // namespace spica

#endif  // _SPICA_RANDOM_BASE_H_
