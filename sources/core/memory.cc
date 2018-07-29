#define SPICA_API_EXPORT
#include "memory.h"

#include <cstdlib>
#include <cstddef>
#include <algorithm>

namespace spica {

MemoryArena::MemoryArena(size_t blockSize)
    : blockSize_{ blockSize } {
}

MemoryArena::~MemoryArena() {
    align_free(currentBlock_);

    for (auto& block : usedBlocks_) {
        align_free(block.second);
    }

    for (auto& block : availableBlocks_) {
        align_free(block.second);
    }
}

void* MemoryArena::allocBytes(size_t nBytes) {
    const uint32_t align = alignof(std::max_align_t);
    nBytes = (nBytes + align - 1) & ~(align - 1);
    if (currentBlockPos_ + nBytes > currentAllocSize_) {
        if (currentBlock_) {
            usedBlocks_.emplace_back(currentAllocSize_, currentBlock_);
            currentBlock_ = nullptr;
            currentAllocSize_ = 0;
        }

        for (auto iter = availableBlocks_.begin();
            iter != availableBlocks_.end(); ++iter) {
            if (iter->first >= nBytes) {
                currentAllocSize_ = iter->first;
                currentBlock_ = iter->second;
                availableBlocks_.erase(iter);
                break;
            }
        }

        if (!currentBlock_) {
            currentAllocSize_ = std::max(nBytes, blockSize_);
            currentBlock_ = (uint8_t*)align_alloc(currentAllocSize_, 64);
        }
        currentBlockPos_ = 0;
    }
    void* ret = currentBlock_ + currentBlockPos_;
    currentBlockPos_ += nBytes;
    return ret;
}

void MemoryArena::reset() {
    currentBlockPos_ = 0;
    availableBlocks_.splice(availableBlocks_.begin(), usedBlocks_);
}

size_t MemoryArena::totalAllocated() const {
    size_t total = currentAllocSize_;
    for (const auto &block : usedBlocks_) total += block.first;
    for (const auto &block : availableBlocks_) total += block.first;
    return total;
}

}  // namespace spica