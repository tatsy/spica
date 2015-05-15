#ifndef _SPICA_RANDOM_H_
#define _SPICA_RANDOM_H_

#include "../utils/common.h"
#include "random_base.h"
#include "twister.h"
#include "primary_sampler.h"

namespace spica {

    enum RandType {
        RAND_TWISTER = 0x01,
        RAND_PRIMARY_SPACE = 0x02
    };

    class Random {
    private:
        RandomBase* _rng;
        int* _numCopies;
        RandType _type;

    public:
        explicit Random(int seed = -1, RandType type = RAND_TWISTER)
            : _rng(NULL)
            , _numCopies(NULL)
            , _type(type)
        {
            _numCopies = new int(0);
            switch (type) {
            case RAND_TWISTER:
                _rng = new Twister(seed);
                break;

            case RAND_PRIMARY_SPACE:
                _rng = NULL;
                break;

            default:
                msg_assert(false, "Unknown random type");
                break;
            }
        }

        ~Random() 
        {
            release();
        }

        Random(const Random& rand)
            : _rng(NULL)
            , _numCopies(NULL)
            , _type()
        {
            operator=(rand);
        }

        Random& operator=(const Random& rand) {
            release();
            this->_rng = rand._rng;
            this->_numCopies = rand._numCopies;
            this->_type = rand._type;
            (*_numCopies) += 1;
        }

        int nextInt(const int n) const {
            return _rng->nextInt(n);
        }

        double nextReal() const {
            return _rng->nextReal();
        }

    private:

        void release() {
            if (_numCopies != NULL) {
                if (*_numCopies == 0) {
                    delete _rng;
                    delete _numCopies;
                    _rng = NULL;
                    _numCopies = NULL;
                } else {
                    (*_numCopies) -= 1;
                }
            }   
        }
    };

}

#endif  // _SPICA_RANDOM_H_
