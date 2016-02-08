#ifndef _SPICA_UNCOPYABLE_H_
#define _SPICA_UNCOPYABLE_H_


#if defined(_WIN32) || defined(__WIN32__)
    #define SPICA_UNCOPYABLE_DLL __declspec(dllexport)
#else
    #define SPICA_UNCOPYABLE_DLL
#endif

namespace spica {

    /*! Interface class which forbids copy and assignment
     */
    class SPICA_UNCOPYABLE_DLL Uncopyable {
    public:
        Uncopyable() {}
        ~Uncopyable() {}

    private:
        Uncopyable(const Uncopyable&) = delete;
        Uncopyable& operator=(const Uncopyable&) = delete;
    };

}

#endif  // _SPICA_UNCOPYABLE_H_
