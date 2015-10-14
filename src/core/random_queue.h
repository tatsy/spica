#ifndef _SPICA_RANDOM_QUEUE_H_
#define _SPICA_RANDOM_QUEUE_H_

#include <memory>
#include <algorithm>

#include "../random/random.h"

namespace spica {

    /** Queue of random-order popping.
     */
    template <class T>
    class RandomQueue {
    private:
        static const size_t initSize;
        
        size_t _size;
        size_t _pos;
        Random _rng;
        std::unique_ptr<T[]> _data;
    
    public:
        /** The RandomQueue constructor.
         */
        explicit RandomQueue(unsigned int seed = 0)
            : _size{initSize}
            , _pos{0}
            , _rng{seed}
            , _data{nullptr} {
            _data = std::make_unique<T[]>(initSize);
        }

        /** The RandomQueue destructor.
         */
        ~RandomQueue() {
        }

        /** The RandomQueue constructor (copy).
         */
        RandomQueue(const RandomQueue& que)
            : _size{0}
            , _pos{0}
            , _rng{}
            , _data{nullptr} {
            this->operator=(que);
        }

        /** Assignment operator.
         */
        RandomQueue& operator=(const RandomQueue& que) {
            if (this == &que) return *this;

            _data.reset();

            _size = que._size;
            _pos  = que._pos;
            _data = std::make_unique<T[]>(que._size);
            std::copy(que._data.get(), que._data.get() + que._size, _data.get());
            
            return *this;
        }

        /** Push a new value.
         */
        void push(const T& ty) {
            _data[_pos++] = ty;
            if (_pos == _size) {
                auto temp = std::make_unique<T[]>(_size * 2);
                std::copy(_data.get(), _data.get() + _size, temp.get());

                _size *= 2;
                _data.reset();
                _data = std::move(temp);
            }
        }

        /** Pop the random item in the queue.
         */
        T pop() {
            Assertion(_pos >= 0, "Queue is empty !!");
            const int r = _rng.nextInt(_pos);
            _pos--;

            std::swap(_data[r], _data[_pos]);
            return _data[_pos];
        }

        /** Check if the queue is empty.
         */
        bool empty() const {
            return _pos == 0;
        }

        /** Return the size of the queue.
         */
        size_t size() const {
            return _pos;
        }
    };

    template <class Ty>
    const size_t RandomQueue<Ty>::initSize = 1024;

}  // namespace spica

#endif  // _SPICA_RANDOM_QUEUE_H_
