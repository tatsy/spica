#ifndef _SPICA_BSSRDF_H_
#define _SPICA_BSSRDF_H_

namespace spica {

    class BSSRDF {
    private:
        double _sigmaA;
        double _sigmapS;
        double _eta;
        double _maxError;

    public:
        BSSRDF()
            : _sigmaA(0.0)
            , _sigmapS(0.0)
            , _eta(0.0)
            , _maxError(0.0)
        {
        }

        BSSRDF(double sigmaA, double sigmapS, double eta = 1.3, double maxError = 0.01)
            : _sigmaA(sigmaA)
            , _sigmapS(sigmapS)
            , _eta(eta)
            , _maxError(maxError)
        {
        }

        BSSRDF(const BSSRDF& bssrdf)
            : _sigmaA(0.0)
            , _sigmapS(0.0)
            , _eta(0.0)
            , _maxError(0.0)
        {
            this->operator=(bssrdf);
        }

        BSSRDF& operator=(const BSSRDF& bssrdf) {
            this->_sigmaA = bssrdf._sigmaA;
            this->_sigmapS = bssrdf._sigmapS;
            this->_eta = bssrdf._eta;
            this->_maxError = bssrdf._maxError;
            return *this;
        }

        inline double sigma_a() const { return _sigmaA; }
        inline double sigmap_s() const { return _sigmapS; }
        inline double eta() const { return _eta; }
        inline double maxError() const { return _maxError; }
    };

}

#endif  // _SPICA_BSSRDF_H_
