#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MEMORY_H_
#define _SPICA_MEMORY_H_

#include <list>

#include "../core/common.h"
#include "../core/uncopyable.h"

namespace spica {

/**
 * Memory management class.
 * @details
 * This class only allocates the memory space only when
 * the currently-allocated (and empty) memory space is 
 * insufficient for assigning new instance to it.
 * In addition, this class does not have explicit memory deleter.
 * This deallocates the memory space when the lifetime is finished.
 */
class SPICA_EXPORTS MEM_ALIGN(64) MemoryArena : private Uncopyable {
public:
    explicit MemoryArena(size_t blockSize = 262144);
    MemoryArena(MemoryArena&& arena) noexcept;
    virtual ~MemoryArena();

    template <class T, class... Args>
    typename std::enable_if<!std::is_array<T>::value, T>::type*
    allocate(const Args&... args) {
        T *ret = (T*)allocBytes(sizeof(T));
        new (ret) T(args...);
        return ret;
    }

    template <class T>
    typename std::enable_if<std::is_array<T>::value, typename std::remove_extent<T>::type>::type*
    allocate(size_t size) {
        using Elem = typename std::remove_extent<T>::type;
        Elem *ret = (Elem*)allocBytes(sizeof(Elem) * size);
        for (size_t i = 0; i < size; i++) {
            new (&ret[i]) Elem();
        }
        return ret;
    }

    void reset();

    size_t totalAllocated() const;

private:
    // Private methods
    void* allocBytes(size_t nBytes);

    // Private fields
    const size_t blockSize_;
    size_t currentBlockPos_ = 0;
    size_t currentAllocSize_ = 0;
    uint8_t* currentBlock_ = nullptr;
    std::list<std::pair<size_t, uint8_t*>> usedBlocks_, availableBlocks_;
};  // class MemoryArena

}  // namespace spica

#endif  // _SPICA_MEMORY_H_
