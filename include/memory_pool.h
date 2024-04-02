#pragma once
#include <iostream>
#include <mutex>
#include <cassert>
#include <cstddef>

namespace Test {

class MemoryPool {
    private:
        struct memblk {
            memblk* next;
        };

        std::size_t block_size;
        unsigned block_cnt;
        unsigned char* pool;
        memblk* freelist;
        std::mutex pool_mutex;

        void initialize() {
            std::size_t pool_size = block_size * block_cnt;
            pool = static_cast<unsigned char*>(::operator new[](pool_size));
            
            freelist = reinterpret_cast<memblk*>(pool);
            memblk *current = freelist;

            for (unsigned i=0; i<block_cnt-1; ++i) {
                current->next = reinterpret_cast<memblk*>(pool + (i + 1) * block_size);
                current = current->next;
            }
            current->next = nullptr;
        }

        MemoryPool(const MemoryPool&) = delete;
        MemoryPool& operator=(const MemoryPool&) = delete;

    public:
        MemoryPool(std::size_t block_size_, unsigned block_cnt_) : block_size(block_size_),
                   block_cnt(block_cnt_), pool(nullptr), freelist(nullptr) {
            initialize();
        }

        ~MemoryPool() {
            ::operator delete[](pool);
        }

        void* allocate(std::size_t alloc_size) {
            std::lock_guard<std::mutex> pool_mutex_lk(pool_mutex);

            if (alloc_size > block_size)
                throw std::invalid_argument("Requested size is greater than block size");

            if (!freelist) {
                throw std::bad_alloc();
            }

            memblk *head = freelist;
            freelist = freelist->next;
            return head;
        }

        void deallocate(void *ptr) {
            std::lock_guard<std::mutex> pool_mutex_lk(pool_mutex);

            memblk *block = static_cast<memblk*>(ptr);
            block->next = freelist;
            freelist = block; 
        }
};

}
