#pragma once
#include <functional>

namespace Test {

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class KeyValueStore {
    public:
        virtual Value value_for(Key const& key, Value const& default_value = Value()) const = 0;
        virtual void add_or_update_mapping(Key const& key, Value const& value) = 0;
        virtual void remove_mapping(Key const& key) = 0;
        virtual ~KeyValueStore() {}
};

}
