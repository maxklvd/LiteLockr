/*
 * Copyright (C) 2020, 2022 Max Kolesnikov <maxklvd@gmail.com>
 *
 * This file is part of LiteLockr.
 *
 * LiteLockr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LiteLockr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LiteLockr.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXPIRING_CACHE_H
#define EXPIRING_CACHE_H

#include <chrono>
#include <optional>

#include <boost/compute/detail/lru_cache.hpp>

namespace litelockr {

template<class Key, class Value>
class ExpiringCache {
public:
    using clock = std::chrono::steady_clock;
    using duration = clock::duration;
    using time_point = clock::time_point;

    constexpr static duration DEFAULT_TTL = std::chrono::minutes(5);
    constexpr static size_t DEFAULT_CAPACITY = 1024;

    explicit ExpiringCache(duration ttl = DEFAULT_TTL, size_t capacity = DEFAULT_CAPACITY)
            : ttl_(ttl),
              lruCache_{capacity} {}

    std::optional<Value> get(Key key) {
        auto value = lruCache_.get(key);
        if (value) {
            const auto& entry = value.get();
            if (entry.expiryTime > clock::now()) {
                return entry.value;
            }
        }
        return {};
    }

    void put(Key key, Value value) {
        lruCache_.insert(key, {value, clock::now() + ttl_});
    }

    void clear() {
        lruCache_.clear();
    }

private:
    struct Entry {
        Value value{};
        time_point expiryTime{};
    };
    boost::compute::detail::lru_cache<Key, Entry> lruCache_;
    duration ttl_;
};

} // namespace litelockr

#endif // EXPIRING_CACHE_H
