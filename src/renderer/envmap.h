#ifndef _SPICA_ENVMAP_H_
#define _SPICA_ENVMAP_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_ENVMAP_EXPORT
        #define SPICA_ENVMAP_DLL __declspec(dllexport)
    #else
        #define SPICA_ENVMAP_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_ENVMAP_DLL
#endif

#include <vector>

#include "../utils/color.h"
#include "../utils/image.h"
#include "../random/random.h"

namespace spica {

    class Photon;

    class SPICA_ENVMAP_DLL Envmap {
    private:
        Image _image;
        Image _importance;
        std::vector<double> _pdf;
        std::vector<double> _cdf;
        static const int IMPORTANCE_MAP_SIZE = 64;

    public:
        Envmap();
        Envmap(const std::string& filename);

        void resize(int width, int height);
        void clearColor(const Color& color);

        Color sampleFromDir(const Vector3& dir) const;
        Photon samplePhoton(RandomSeq& rseq, const int numPhotons) const;

    private:
        void createImportanceMap();

    };

}

#endif  // _SPICA_ENVMAP_H_
