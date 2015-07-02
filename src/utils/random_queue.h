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
        
        size_t _size;
        size_t _pos;
        Ty* _data;
        Random _rng;
    
    public:
        explicit RandomQueue(int seed = -1)
            : _size(0)
            , _pos(0)
            , _data(NULL)
            , _rng(seed)
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
            operator=(que);
        }

        RandomQueue& operator=(const RandomQueue& que) {
            delete[] _data;

            _size = que._size;
            _pos = que._pos;
            _data = new Ty[_size];
            memcpy((void*)_data, (void*)que._data, sizeof(Ty) * _size);
            
            return *this;
        }

        void push(const Ty& ty) {
            _data[_pos++] = ty;
            if (_pos == _size) {
                Ty* temp = new Ty[_size * 2];
                memcpy((void*)temp, (void*)_data, sizeof(Ty) * _size);

                delete[] _data;
                _size *= 2;
                _data = temp;
            }
        }

        Ty pop() {
            msg_assert(_pos >= 0, "Queue is empty !!");
            int r = _rng.nextInt(_pos);
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

}

#endif  // _SPICA_RANDOM_QUEUE_H_
