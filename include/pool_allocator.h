#pragma once
#include <mutex>
#include <cstddef>
#include <memory>
#include "memory_pool.h"

namespace Test {

template <typename T>
class PoolAllocator {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pool_type = MemoryPool;

        MemoryPool& pool;

        template <typename U>
        constexpr PoolAllocator(const PoolAllocator<U> &other) noexcept : pool(other.pool) {} 

        PoolAllocator(MemoryPool& pool_) : pool(pool_) {}

        [[nodiscard]] constexpr T* allocate(size_type n) {
            return static_cast<T*>(pool.allocate(n * sizeof(T)));
        }

        void deallocate(T* p, size_type n) {
            pool.deallocate(p);
        }
};

}
