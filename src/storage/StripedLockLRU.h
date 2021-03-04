#ifndef AFINA_STORAGE_STRIPED_LOCK_LRU_H
#define AFINA_STORAGE_STRIPED_LOCK_LRU_H

#include <mutex>
#include <condition_variable>
#include <vector>
#include <array>

#include "SimpleLRU.h"
#include <afina/Storage.h>

namespace Afina {
namespace Backend {
class StripedLockLRU : public Afina::Storage {
public:
    static StripedLockLRU* create_cash(std::size_t count, std::size_t max_size);
    StripedLockLRU(size_t count = 16, std::size_t max_shard_size = 1024) : _count(count)
    {
        for (std::size_t i = 0; i < _count; i++)
        {
            _cashes.emplace_back(new SimpleLRU(max_shard_size));
        }
    }
    bool Put(const std::string &key, const std::string &value) override {
        std::hash<std::string> _hash;
        return _cashes[_hash(key) % _count]->Put(key, value);
    }
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        std::hash<std::string> _hash;
        return _cashes[_hash(key) % _count]->PutIfAbsent(key, value);
    }
    bool Set(const std::string &key, const std::string &value) override {
        std::hash<std::string> _hash;
        return _cashes[_hash(key) % _count]->Set(key, value);
    }
    bool Delete(const std::string &key) override {
        std::hash<std::string> _hash;
        return _cashes[_hash(key) % _count]->Delete(key);
    }
    bool Get(const std::string &key, std::string &value) override {
        std::hash<std::string> _hash;
        return _cashes[_hash(key) % _count]->Get(key, value);
    }
    ~StripedLockLRU() {}

private:
    size_t _count;
    size_t _max_size;
    std::vector<std::unique_ptr<SimpleLRU>> _cashes;
};

} // end Backend
} // end Afina

#endif