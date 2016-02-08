#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BIRATERAL_H_
#define _SPICA_BIRATERAL_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {

    void SPICA_EXPORTS birateral(const Image& src, Image* dst, double sigma_s, double sigma_r);

}  // namespace spica

#endif  // _SPICA_BIRATERAL_H_
