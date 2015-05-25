#ifndef _SPICA_RANDOM_BASE_H_
#define _SPICA_RANDOM_BASE_H_

#if defined(_WIN32) || defined(__WIN32__)
#define SPICA_RANDOM_BASE_DLL __declspec(dllexport)
#else
#define SPICA_RANDOM_BASE_DLL
#endif

#include <vector>

#include "../utils/uncopyable.h"

namespace spica {

    // --------------------------------------------------
    // Random number sequence genrated by generator classes
    // --------------------------------------------------
    class RandomSeq : public Uncopyable {
    private:
        int _pos;
        int _size;
        double* _values;

    public:
        RandomSeq()
            : _pos(0)
            , _size(0)
            , _values(NULL)
        {
        }

        ~RandomSeq()
        {
            delete[] _values;
        }

        void resize(const int size) {
            if (_size != size) {
                delete[] _values;
                this->_size = size;
                _values = new double[_size];
            }
        }

        void set(int i, double val) {
            msg_assert(0 <= i && i < _size, "Sample index out of bounds!!");
            _values[i] = val;
        }

        void reset() {
            _pos = 0;
        }

        double next() {
            msg_assert(_pos < _size, "No more samples are stored...");
            return _values[_pos++];
        }

        void next(const int n, std::vector<double>* v) {
            v->resize(n);
            for (int i = 0; i < n; i++) {
               (*v)[i] = next(); 
            }
        }
    };

    // --------------------------------------------------
    // Interface class for random number generator
    // --------------------------------------------------
    class RandomBase {
    public:
        // Request specified amount of random numbers
        virtual void requestSamples(RandomSeq& randSeq, const int numRequested) = 0;
    };
};

#endif  // _SPICA_RANDOM_BASE_H_
