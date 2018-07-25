#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FILM_H_
#define _SPICA_FILM_H_

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "core/core.hpp"
#include "core/common.h"
#include "core/uncopyable.h"
#include "core/point2d.h"
#include "core/image.h"
#include "core/filter.h"
#include "core/cobject.h"

namespace spica {

class SPICA_EXPORTS Film : public CObject, Uncopyable {
public:
    // Public methods
    Film(const Point2i& resolution,
         const std::shared_ptr<Filter> &filter,
         const std::string& filename,
         const std::shared_ptr<std::function<void(const Image&)>> &callback = nullptr);

    virtual ~Film() = default;

    inline Point2i resolution() const { return resolution_; }
    inline double aspect() const {
        return static_cast<double>(resolution_.x()) / resolution_.y();
    }

    /**
     * Save the result (scale * sum(w * I) * ns) / sum(w).
     * Typically, you need to set "scale = #sample^{-1}"
     */
    void save(int id, double scale) const;

    void setImage(const Image& image);
    void addPixel(const Point2i& pixel, const Point2d& pInPixel,
                  const Spectrum& color);
    void addPixel(const Point2d& pixel, const Spectrum& color);

    inline const std::string &filename() const {
        return filename_;
    }

    inline void setFilename(const std::string filename) {
        this->filename_ = filename;
    }

    inline void setSaveCallback(std::unique_ptr<std::function<void(const Image&)>>&& callback) {
        this->saveCallback_ = std::move(callback);
    }

    inline double weight(const Point2d &pixel) const {
        return filter_->evaluate(pixel);
    }

    virtual void saveImage(const std::string &filename,  const Image &image) const = 0;

private:
    // Private fields
    Point2i resolution_;
    std::shared_ptr<Filter> filter_;
    std::string filename_;
    Image image_;
    std::vector<std::vector<double>> weights_;
    std::vector<std::vector<int>> samples_;
    std::shared_ptr<std::function<void(const Image&)>> saveCallback_;

};  // class Film

}  // namespace spica

#endif  // _SPICA_FILM_H_
