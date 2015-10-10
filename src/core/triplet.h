#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIPLET_H_
#define _SPICA_TRIPLET_H_

namespace spica {

    template <class T, class Enable = void>
    class Triplet_;

    template <class T>
    class Triplet_<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
    private:
        std::array<T, 3> _data;

    public:
        Triplet_()
            : _data() {
        }

        Triplet_(T i, T j, T k)
            : _data() {
            _data[0] = i;
            _data[1] = j;
            _data[2] = k;
        }

        Triplet_(const Triplet_<T>& triplet)
            : _data() {
            this->operator=(triplet);
        }

        ~Triplet_() {
        }

        Triplet_& operator=(const Triplet_<T>& triplet) {
            this->_data = triplet._data;
            return *this;
        }

        T operator[](int i) const {
            Assertion(0 <= i && i <= 2, "access index out of bounds!");
            return _data[i];
        }    
    };

    using Triplet = Triplet_<int>;

}  // namespace spica

#endif  // _SPICA_TRIPLET_H_
