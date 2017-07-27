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
         const std::string& filename = "image",
         const std::shared_ptr<std::function<void(const Image&)>> &callback = nullptr);

    ~Film() = default;

    inline Point2i resolution() const { return resolution_; }
    inline double aspect() const {
        return static_cast<double>(resolution_.x()) / resolution_.y();
    }

    /**
     * Save the result by dividing sum(w * I) by sum(w).
     * This method is typically used for SamplerIntegrator, which take the same
     * number of samples for each pixel.
     *
     * @param[in] id: The ID used for naming the image file.
     */
    void save(int id = 0) const;

    /**
     * Save the result by computing, (sum(w * I) * ns / (sum(w)), where ns is
     * number of samples generated for a pixel.
     * This method is typically used for MLT-like algorithms, which take
     * different number of samples for each pixel.
     *
     * @param[in] scale: The parameter to scale pixel values.
     */
    void saveMLT(double scale, int id = 0) const;

    void setImage(const Image& image);
    void addPixel(const Point2i& pixel, const Point2d& pInPixel,
                  const Spectrum& color);
    void addPixel(const Point2d& pixel, const Spectrum& color);

    inline void setFilename(const std::string filename) {
        this->filename_ = filename;
    }

    inline void setSaveCallback(std::unique_ptr<std::function<void(const Image&)>>&& callback) {
        this->saveCallback_ = std::move(callback);
    }

protected:
    // Protected methods
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
