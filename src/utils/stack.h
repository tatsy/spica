#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QUEUE_H_
#define _SPICA_QUEUE_H_

#include <cstdlib>
#include <cstring>

#include "common.h"

namespace spica {

    template <class T>
    class Stack {
    private:
        static const int kInitSize = 512;

        int _pos;
        int _size;
        T* _data;

    public:
        Stack()
            : _pos(0)
            , _size(kInitSize)
            , _data(new T[kInitSize])
        {
        }

        Stack(const Stack<T>& stk)
            : _pos(0)
            , _size(0)
            , _data(NULL)
        {
            this->operator=(stk);
        }

        Stack(Stack<T>&& stk)
            : _pos(0)
            , _size(0)
            , _data(NULL)
        {
            this->operator=(std::move(stk));
        }

        ~Stack()
        {
            delete[] _data;
        }

        Stack<T>& operator=(const Stack<T>& stk) {
            release();

            this->_pos = stk._pos;
            this->_size = stk._size;
            this->_data = new T[stk._size];
            memcpy(_data, stk._data, sizeof(T) * stk._size);

            return *this;
        }

        Stack<T>& operator=(Stack<T>&& stk) {
            release();

            this->_pos = stk._pos;
            this->_size = stk._size;
            this->_data = stk._data;
            stk._data = nullptr;

            return *this;
        }

        void push(const T& v) {
            if (_pos == _size) {
                T* tmp = new T[_size * 2];
                memcpy(tmp, _data, sizeof(T) * _size);
                delete[] _data;
                _data = tmp;
                _size = _size * 2;
            } 
            
            _data[_pos++] = v;
        }

        T pop() {
            Assertion(_pos > 0, "No item remains!!");
            return std::move(_data[--_pos]);
        }

        void clear() {
            _pos = 0;
        }

    private:
        void release() {
            _pos = 0;
            _size = 0;
            delete[] _data;
            _data = NULL;
        }

    };

}  // namespace spica

#endif  // _SPICA_QUEUE_H_

