#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FILM_H_
#define _SPICA_FILM_H_

#include <string>
#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/point2d.h"

#include "../filter/filter.h"

namespace spica {

class Film {
public:
    // Public methods
    Film();
    Film(const Point2i& resolution, std::unique_ptr<Filter> filter,
         const std::string& filename);
    Film(const Film& film) = default;

    ~Film() = default;

    Film& operator=(const Film&) = default;

    inline Point2i resolution() const { return resolution_; }

private:
    // Private methods
    Point2i resolution_;
    Filter* filter_;
    std::string filename_;


};  // class Film

}  // namespace spica

#endif  // _SPICA_FILM_H_
