#define SPICA_API_EXPORT
#include "random_sampler.h"

#include "random.h"
#include "halton.h"

namespace spica {

    RandomSampler::RandomSampler()
        : _rng{} {
    }

    RandomSampler::~RandomSampler() {
    }

    void RandomSampler::request(Stack<double>* rands, int n) {
        if (_rng) {
            _rng->request(rands, n);
        }
    }

    RandomSampler::RandomSampler(RandomSampler&& rs)
        : _rng{std::move(rs._rng)} {    
    }

    RandomSampler& RandomSampler::operator=(RandomSampler&& rs) {
        this->_rng = std::move(rs._rng);
        return *this;
    }

    RandomSampler RandomSampler::useMersenne(unsigned int seed) {
        RandomSampler rnd;
        rnd._rng = std::make_unique<Random>(seed);
        return std::move(rnd);
    }

    RandomSampler RandomSampler::useHalton(int dim, bool isPermute, unsigned int seed) {
        RandomSampler rnd;
        rnd._rng = std::make_unique<Halton>(dim, isPermute, seed);
        return std::move(rnd);
    }

}  // namespace spica
