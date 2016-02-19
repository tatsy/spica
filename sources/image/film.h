#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FILM_H_
#define _SPICA_FILM_H_

#include <string>
#include <vector>
#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/point2d.h"

#include "../image/image.h"
#include "../filter/filter.h"

namespace spica {

class SPICA_EXPORTS Film {
public:
    // Public methods
    Film();
    Film(const Point2i& resolution, std::unique_ptr<Filter>& filter,
         const std::string& filename);
    Film(const Film& film) = default;

    ~Film() = default;

    Film& operator=(const Film&) = default;

    inline Point2i resolution() const { return resolution_; }
    inline double aspect() const { 
        return static_cast<double>(resolution_.x()) / resolution_.y();
    }

    void save(int id = 0) const;
    void addPixel(const Point2i& pixel, const Point2D& pInPixel,
                  const Spectrum& color);

private:
    // Private methods
    Point2i resolution_;
    Filter* filter_;
    std::string filename_;
    Image image_;
    std::vector<std::vector<double>> weights_;

};  // class Film

}  // namespace spica

#endif  // _SPICA_FILM_H_
