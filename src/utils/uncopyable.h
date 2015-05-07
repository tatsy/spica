#ifndef _SPICA_UNCOPYABLE_H_
#define _SPICA_UNCOPYABLE_H_

namespace spica {

    class Uncopyable {
    public:
        Uncopyable() {}

    private:
        Uncopyable(const Uncopyable& inst) {}
        Uncopyable& operator=(const Uncopyable& inst) {}
    };

}

#endif  // _SPICA_UNCOPYABLE_H_
