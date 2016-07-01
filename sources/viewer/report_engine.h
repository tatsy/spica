#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_REPORT_ENGINE_H_
#define _SPICA_REPORT_ENGINE_H_

#include "../../include/spica.h"

#include <QtCore/qobject.h>
#include <QtGui/qimage.h>
#include <boost/property_tree/ptree.hpp>

namespace spica {

class ReportEngine : public QObject, public Engine {
    Q_OBJECT

public:
    ReportEngine()
        : Engine{} {
    }

    virtual ~ReportEngine() {
    }

    bool parse_film(const boost::property_tree::ptree& xml,
                    Film** film) const override {
        int width = 0;
        int height = 0;
        std::unique_ptr<Filter> filter = nullptr;
    
        // Parse width and height
        for (const auto& props : xml.get_child("")) {
            if (props.first != "integer") continue;
            const std::string name =
                props.second.get<std::string>("<xmlattr>.name");
            if (name == "width") {
                width = props.second.get<int>("<xmlattr>.value");
            } else if (name == "height") {
                height = props.second.get<int>("<xmlattr>.value");
            }
        }

        // Parse rfilter
        const std::string filtType = xml.get<std::string>("rfilter.<xmlattr>.type");
        if (filtType == "box") {
            filter = std::make_unique<BoxFilter>(Vector2d(0.5, 0.5));
        } else if (filtType == "tent") {
            filter = std::make_unique<TentFilter>(Vector2d(0.5, 0.5));
        } else if (filtType == "gaussian") {
            filter = std::make_unique<GaussianFilter>(Vector2d(0.5, 0.5), 2.0);
        } else {
            fprintf(stderr, "Unknown filter type \"%s\" is specified!!", filtType.c_str());
            return false;
        }


        auto callback = std::make_unique<std::function<void(const Image&)>>([&](const Image& image) {
            QImage qimage(image.width(), image.height(), QImage::Format_ARGB32_Premultiplied);
            for (int y = 0; y < image.height(); y++) {
                for (int x = 0; x < image.width(); x++) {
                    RGBSpectrum rgb = image(x, y).toRGB();
                    int r = std::max(0, std::min((int)(rgb.red() * 255.0), 255));
                    int g = std::max(0, std::min((int)(rgb.green() * 255.0), 255));
                    int b = std::max(0, std::min((int)(rgb.blue() * 255.0), 255));
                    qimage.setPixel(x, y, qRgb(r, g, b));
                }
            }
            emit imageSaved(qimage);
        });

        *film = new Film(Point2i(width, height), std::move(filter),
                         option_.outfile, std::move(callback));

        return true;
    }



signals:
    void imageSaved(const QImage& image) const;
};

}  // namespace spica

#endif  // _SPICA_REPORT_ENGINE_H_
