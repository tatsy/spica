#ifndef _SPICA_AXIS_COMPARATOR_H_
#define _SPICA_AXIS_COMPARATOR_H_

#if defined(_WIN32) || defined(__WIN32__)
#define SPICA_AXIS_COMPARABLE_DLL __declspec(dllexport)
#else
#define SPICA_AXIS_COMPARABLE_DLL
#endif

namespace spica {

    // --------------------------------------------------
    // Interface class for sorting point along the specified axis
    // --------------------------------------------------
    class AxisComparable {
    public:
        AxisComparable() {}
        ~AxisComparable() {}
        virtual double get(int d) const = 0;
    };

}

#endif  // _SPICA_AXIS_COMPARATOR_H_
