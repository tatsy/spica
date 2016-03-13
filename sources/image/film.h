#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FILM_H_
#define _SPICA_FILM_H_

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/uncopyable.h"
#include "../core/point2d.h"

#include "../image/image.h"
#include "../filter/filter.h"

namespace spica {

class SPICA_EXPORTS Film : public Uncopyable {
public:
    // Public methods
    Film(const Point2i& resolution, std::unique_ptr<Filter>&& filter,
         const std::string& filename,
         std::unique_ptr<std::function<void(const Image&)>>&& callback = nullptr);

    ~Film() = default;

    inline Point2i resolution() const { return resolution_; }
    inline double aspect() const { 
        return static_cast<double>(resolution_.x()) / resolution_.y();
    }

    void save(int id = 0) const;
    void setImage(const Image& image);
    void addPixel(const Point2i& pixel, const Point2d& pInPixel,
                  const Spectrum& color);

    inline void setSaveCallback(std::unique_ptr<std::function<void(const Image&)>>&& callback) {
        this->saveCallback_ = std::move(callback);        
    }

private:
    // Private methods
    Point2i resolution_;
    std::unique_ptr<Filter> filter_;
    std::string filename_;
    Image image_;
    std::vector<std::vector<double>> weights_;
    std::unique_ptr<std::function<void(const Image&)>> saveCallback_;

};  // class Film

}  // namespace spica

#endif  // _SPICA_FILM_H_
