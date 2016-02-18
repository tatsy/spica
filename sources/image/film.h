#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FILM_H_
#define _SPICA_FILM_H_

#include <string>
#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "../filter/filter.h"

namespace spica {

class Film {
public:
    Film(const Point2i& resolution, const Rect& cropWindow,
         std::unique_ptr<Filter> filter, double diagonal,
         const std::string& filename, double scale);



};  // class Film

}  // namespace spica

#endif  // _SPICA_FILM_H_
