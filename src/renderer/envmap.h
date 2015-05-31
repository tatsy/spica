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

#include "../utils/color.h"

namespace spica {

    class SPICA_ENVMAP_DLL Envmap {
    private:
        int _numPhi;
        int _numTheta;
        Color* _table;

    public:
        Envmap();
        Envmap(const int numPhi, const int numTheta);
        Envmap(const Envmap& envmap);
        
        ~Envmap();

        Envmap& operator=(const Envmap& envmap);
        const Color& operator()(const double phi, const double theta) const;

        void resize(const int nPhi, const int nTheta);

        void set(const int phi, const int theta, const Color& color);

        void clearColor(const Color& color);

    private:
        void release();

    };

}

#endif  // _SPICA_ENVMAP_H_
