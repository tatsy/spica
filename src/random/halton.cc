#define SPICA_HALTON_EXPORT
#include "halton.h"

#include <cstdlib>
#include <algorithm>

namespace spica {

    namespace {
    
        const int primes[168] = {
            2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
            101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193,
            197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307,
            311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421,
            431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547,
            557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
            661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797,
            809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929,
            937, 941, 947, 953, 967, 971, 977, 983, 991, 997
        };

        void suffle(int* p, int d, const Random& rng) {
            for (int i = 0; i < d; i++) {
                for (int j = i; j < d; j++) {
                    const int r = rng.nextInt(d - i);
                    std::swap(p[i], p[i + r]);
                }
            }
        }

        void permutation(int* p, int d, const Random& rng) {
            for (int i = 0; i < d; i++) {
                p[i] = i;
            }
            suffle(p, d, rng);
        }

    }

    Halton::Halton()
        : dims(0)
        , bases(NULL)
        , permute(NULL)
    {
    }

    Halton::~Halton()
    {
        delete[] bases;
        delete[] permute;
    }

    Halton::Halton(int dim, const Random& rng)
        : dims(dim)
        , bases(NULL)
        , permute(NULL)
    {
        msg_assert(dim < 168, "You cannot specify dimension over 168");

        bases = new int[dims];
        int sumBases = 0;
        for (int i = 0; i < dims; i++) {
            bases[i] = primes[i];
            sumBases += primes[i];
        }

        permute = new int[sumBases];
        int* p = permute;
        for (int i = 0; i < dims; i++) {
            permutation(p, bases[i], rng);
            p += bases[i];
        }
    }

    double Halton::nextReal(const int baseID, const int seqID) const {
        int* p = permute;
        for (int i = 0; i < baseID; i++) {
            p += bases[i];
        }
        return radicalInverse(seqID, bases[baseID], p);
    }

    double Halton::radicalInverse(int n, int base, const int* p) const {
        double val = 0.0;
        double invBase = 1.0 / base;
        double invBi = invBase;
        while (n > 0) {
            int d_i = p[n % base];
            val += d_i * invBi;
            invBi *= invBase;
            n /= base;
        }
        return val;
    }

}  // namespace spica
