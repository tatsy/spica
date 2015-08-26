#define SPICA_RANDOM_SAMPLER_EXPORT
#include "random_sampler.h"

namespace spica {

    RandomSampler::RandomSampler()
        : _rng()
    {
    }

    RandomSampler::~RandomSampler()
    {
    }

    void RandomSampler::request(Stack<double>* rands, int n) {
        if (_rng) {
            _rng->request(rands, n);
        }
    }

    RandomSampler::RandomSampler(RandomSampler&& rs)
        : _rng(std::move(rs._rng))
    {    
    }

    RandomSampler& RandomSampler::operator=(RandomSampler&& rs) {
        this->_rng = std::move(rs._rng);
        return *this;
    }

}  // namespace spica
