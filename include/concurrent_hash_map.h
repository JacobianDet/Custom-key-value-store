#pragma once
#include <algorithm>
#include <functional>
#include <utility>
#include <memory>
#include <list>
#include <iostream>
#include <vector>
#include "key_value_store.h"
#include "read_write_lock.h"
#include "read_write_raii_wrapper.h"
#include "memory_pool.h"
#include "pool_allocator.h"

namespace Test {

static constexpr int POOL_NUM = 256;

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class ConcurrentHashMap : public KeyValueStore<Key, Value, Hash> {
  private:
	class bucket_type {
		private:
			using bucket_value = std::pair<Key, Value>;
            using alloc_type = PoolAllocator<bucket_value>; 
            using mem_pool = alloc_type::pool_type;
			using bucket_data = std::list<bucket_value, alloc_type>;
			using bucket_iterator = typename bucket_data::iterator;
			using const_bucket_iterator = typename bucket_data::const_iterator;
            mem_pool pool_elem;
			bucket_data data;
			mutable RWLock mutex_rwlock;

			const_bucket_iterator find_entry_for(Key const& key) const {
				return std::find_if(data.begin(), data.end(),
					   [&](bucket_value const& item) { return item.first == key; });
			}

			bucket_iterator find_entry_for(Key const& key) {
				return std::find_if(data.begin(), data.end(),
					   [&](bucket_value const& item) { return item.first == key; });
			}

		public:
            bucket_type() : pool_elem(POOL_NUM, POOL_NUM), data(alloc_type(pool_elem)) {}
 
			Value value_for(Key const& key, Value const& default_value) const {
				RAII_RWLock<RWLock> mutex_rwlock_lk(mutex_rwlock, 0);
				const_bucket_iterator found_entry = find_entry_for(key);
				return (found_entry == data.end()) ? default_value : found_entry->second;
			}
			
           void add_or_update_mapping(Key const& key, Value const& value) { 	
				RAII_RWLock<RWLock> mutex_rwlock_lk(mutex_rwlock, 1);
				bucket_iterator const found_entry = find_entry_for(key);
				if(found_entry == data.end()) {
					data.emplace_back(bucket_value(key, value));
				} else {
					found_entry->second = value;
				}
			}

			void remove_mapping(Key const& key) {
				RAII_RWLock<RWLock> mutex_rwlock_lk(mutex_rwlock, 1);
				bucket_iterator const found_entry = find_entry_for(key);
                if(found_entry != data.end()) {
					data.erase(found_entry);
				}
			}
	};

    using val_type = std::unique_ptr<bucket_type>;
    using alloc_type = PoolAllocator<val_type>;
    using mem_pool = alloc_type::pool_type;
    mem_pool vec_pool_elem;
	std::vector<val_type, alloc_type> buckets;
	Hash hasher;

	bucket_type& get_bucket(Key const& key) const {
		std::size_t const bucket_index = hasher(key) % buckets.size();
		return *buckets[bucket_index];
	}

  public:
	using key_type = Key;
	using mapped_type = Value;
	using hash_type = Hash;

	ConcurrentHashMap(unsigned num_buckets = 19, Hash const& hasher_ = Hash()) :
					 vec_pool_elem(POOL_NUM, POOL_NUM), buckets(num_buckets, alloc_type(vec_pool_elem)), 
                     hasher(hasher_) {
		for(unsigned i=0; i<num_buckets; ++i) {
			buckets[i].reset(new bucket_type);
		}
	}

	ConcurrentHashMap(ConcurrentHashMap const& other) = delete;
	ConcurrentHashMap& operator=(ConcurrentHashMap const& other) = delete;

	Value value_for(Key const& key, Value const& default_value = Value()) const override {
		return get_bucket(key).value_for(key, default_value);
	}

    void print_value(Key const& key, Value const& default_value = Value()) const {
        std::cout << value_for(key, default_value) << std::endl;
    }

	void add_or_update_mapping(Key const& key, Value const& value) override {
		get_bucket(key).add_or_update_mapping(key, value);
	}

	void remove_mapping(Key const& key) override {
		get_bucket(key).remove_mapping(key);
	}
};

}
