#ifndef _SPICA_RANDOM_QUEUE_H_
#define _SPICA_RANDOM_QUEUE_H_

#include <cstring>
#include <algorithm>

#include "../random/random.h"

namespace spica {

    template <class Ty>
    class RandomQueue {
    private:
        static const size_t initSize;
        static const Random rng;
        
        size_t _size;
        size_t _pos;
        Ty* _data;
    
    public:
        RandomQueue()
            : _size(0)
            , _pos(0)
            , _data(NULL)
        {
            _size = initSize;
            _data = new Ty[initSize];
        }

        ~RandomQueue()
        {
            delete[] _data;
        }

        RandomQueue(const RandomQueue& que)
            : _size(0)
            , _pos(0)
            , _data(NULL)
        {
            opearator=(que);
        }

        RandomQueue& operator=(const RandomQueue& que) {
            delete[] _data;

            _size = que._size;
            _pos = que._pos;
            _data = new Ty[_size];
            memcpy(_data, que._data, sizeof(Ty) * _size);
            
            return *this;
        }

        void push(const Ty& ty) {
            _data[_pos++] = ty;
            if (_pos == _size) {
                Ty* temp = new Ty[_size * 2];
                memcpy(temp, _data, sizeof(Ty) * _size);

                delete[] _data;
                _size *= 2;
                _data = temp;
            }
        }

        Ty pop() {
            msg_assert(_pos >= 0, "Queue is empty !!");
            int r = rng.nextInt(_pos);
            _pos--;

            std::swap(_data[r], _data[_pos]);
            return _data[_pos];
        }

        bool empty() const {
            return _pos == 0;
        }

        size_t size() const {
            return _pos;
        }
    
    };

    template <class Ty>
    const size_t RandomQueue<Ty>::initSize = 1024;

    template <class Ty>
    const Random RandomQueue<Ty>::rng = Random();

}

#endif  // _SPICA_RANDOM_QUEUE_H_
