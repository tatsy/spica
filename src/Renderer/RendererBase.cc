#define SPICA_RENDERER_BASE_EXPORT
#include "RendererBase.h"

namespace spica {

    const Color RendererBase::backgroundColor = Color();
    const int   RendererBase::maxDepth        = 5;
    const int   RendererBase::depthLimit      = 64;

    RendererBase::RendererBase(int width, int height, int samples, int supsamples)
        : _width(width)
        , _height(height)
        , _samplePerPixel(samples)
        , _supsamplePerAxis(supsamples)
    {
    }

    RendererBase::RendererBase(const RendererBase& renderer)
        : _width(renderer._width)
        , _height(renderer._height)
        , _samplePerPixel(renderer._samplePerPixel)
        , _supsamplePerAxis(renderer._supsamplePerAxis)
    {
    }

    RendererBase::~RendererBase()
    {
    }

    RendererBase& RendererBase::operator=(const RendererBase& renderer) {
        this->_width = renderer._width;
        this->_height = renderer._height;
        this->_samplePerPixel = renderer._samplePerPixel;
        this->_supsamplePerAxis = renderer._supsamplePerAxis;
        return *this;
    }

}
