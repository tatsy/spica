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
class SPICA_EXPORTS alignas(128) MemoryArena : Uncopyable {
public:
    MemoryArena(size_t blockSize = 262144);
    MemoryArena(MemoryArena&& arena) noexcept;
    virtual ~MemoryArena();

    MemoryArena& operator=(MemoryArena&& arena) = delete;

    void* allocate(size_t nBytes);

    template <class T>
    T* allocate(size_t n = 1, bool runConstructor = true) {
        T* ret = (T*)allocate(n * sizeof(T));
        if (runConstructor) {
            for (size_t i = 0; i < n; i++) {
                new (&ret[i]) T();
            }
        }
        return ret;
    }

    template <class T, class... Args>
    T* allocate(const Args&... args) {
        T* ret = (T*)allocate(sizeof(T));
        new (ret) T(args...);
        return ret;
    }

    void reset();

    size_t totalAllocated() const;

private:
    // Private fields
    const size_t blockSize_;
    size_t currentBlockPos_ = 0;
    size_t currentAllocSize_ = 0;
    unsigned char* currentBlock_ = nullptr;
    std::list<std::pair<size_t, unsigned char*>> usedBlocks_, availableBlocks_;
};  // class MemoryArena

}  // namespace spica

#endif  // _SPICA_MEMORY_H_
