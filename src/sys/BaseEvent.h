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

#ifndef BASE_EVENT_H
#define BASE_EVENT_H

#include <algorithm>
#include <deque>
#include <mutex>

#include "sys/AppTimer.h"

namespace litelockr {

class BaseEvent {
public:
    int id;

    explicit BaseEvent(int _id) : id(_id) {}

    bool operator==(const BaseEvent&) const = default;
};

template<class T>
class EventQueue {
public:
    void push(T event) {
        std::scoped_lock<std::mutex> lock(mtx_);

        queue_.erase(std::remove(queue_.begin(), queue_.end(), event), queue_.end());

        queue_.push_back(event);
    }

    T pop() {
        std::scoped_lock<std::mutex> lock(mtx_);

        T event = queue_.front();
        queue_.pop_front();
        return event;
    }

    bool empty() const {
        std::scoped_lock<std::mutex> lock(mtx_);

        return queue_.empty();
    }

    void remove(T event) {
        std::scoped_lock<std::mutex> lock(mtx_);

        queue_.erase(std::remove(queue_.begin(), queue_.end(), event), queue_.end());
    }

private:
    std::deque<T> queue_;
    mutable std::mutex mtx_;
};


template<class T>
class BaseEvents: public AppTimerListener {
public:
    void timeout() override {
        if (!queue_.empty()) {
            process(queue_.pop());
        }
    }

protected:
    void sendEvent(T event) { queue_.push(event); }

    void removeEvent(T event) { queue_.remove(event); }

    virtual void process(T event) = 0;

private:
    EventQueue<T> queue_;
};

} // namespace litelockr

#endif // BASE_EVENT_H
