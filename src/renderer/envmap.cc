#define SPICA_ENVMAP_EXPORT
#include "envmap.h"

#include <cstdlib>
#include <cstring>

#include "../utils/common.h"

namespace spica {

    Envmap::Envmap()
        : _numPhi(0)
        , _numTheta(0)
        , _table(NULL)
    {
    }

    Envmap::Envmap(const int numPhi, const int numTheta)
        : _numPhi(numPhi)
        , _numTheta(numTheta)
        , _table(new Color[numPhi * numTheta])
    {
    }

    Envmap::Envmap(const Envmap& envmap)
        : _numPhi(0)
        , _numTheta(0)
        , _table(NULL)
    {
        this->operator=(envmap);
    }

    Envmap::~Envmap()
    {
        release();
    }

    Envmap& Envmap::operator=(const Envmap& envmap) {
        release();

        this->_numPhi = envmap._numPhi;
        this->_numTheta = envmap._numTheta;
        this->_table = new Color[_numPhi * _numTheta];
        memcpy(_table, envmap._table, sizeof(Color) * _numPhi * _numTheta);

        return *this;
    }

    const Color& Envmap::operator()(const double phi, const double theta) const {
        msg_assert(0.0 <= phi && phi <= 2.0 * PI && 0.0 <= theta && theta <= PI, "Conditions, phi in [0, 2 * pi] and theta in [0, pi], should be met!!");
        const int p = static_cast<int>(phi / (2.0 * PI / _numPhi)) % _numPhi;
        const int t = static_cast<int>(theta / (PI / _numTheta)) % _numTheta;
        return _table[p * _numTheta + t];
    }

    void Envmap::resize(const int numPhi, const int numTheta) {
        release();

        this->_numPhi = numPhi;
        this->_numTheta = numTheta;
        this->_table = new Color[numPhi * numTheta];
    }

    void Envmap::set(const double phi, const double theta, const Color& color) {
        msg_assert(0.0 <= phi && phi <= 2.0 * PI && 0.0 <= theta && theta <= PI, "Conditions, phi in [0, 2 * pi] and theta in [0, pi], should be met!!");
        const int p = static_cast<int>(phi / (2.0 * PI / _numPhi)) % _numPhi;
        const int t = static_cast<int>(theta / (PI / _numTheta)) % _numTheta;
        _table[p * _numTheta + t] = color;
    }

    void Envmap::clearColor(const Color& color) {
        for (int i = 0; i < _numPhi; i++) {
            for (int j = 0; j < _numTheta; j++) {
                _table[i * _numTheta + j] = color;
            }
        }
    }

    void Envmap::release() {
        _numPhi = 0;
        _numTheta = 0;
        delete[] _table;
        _table = NULL;
    }

}  // namespace spica


