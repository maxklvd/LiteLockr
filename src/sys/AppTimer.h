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

#ifndef APP_TIMER_H
#define APP_TIMER_H

#include <vector>

#include "sys/AppClock.h"

namespace litelockr {

class AppTimerListener {
public:
    virtual void timeout() = 0;
    virtual ~AppTimerListener() = default;
};

template<class T>
class DefaultAppTimerListener: public AppTimerListener {
public:
    void timeout() override {
        instance_.tick();
    }

    T& window() { return instance_; }

    const T& window() const { return instance_; }

private:
    T instance_;
};

class AppTimer {
public:
    AppTimer() = default;

    void addListener(AppTimerListener& listener);
    void fireTimeout() const;

    static void setAnimationActiveFor(int milliseconds) {
        isAnimationActive_ = true;

        auto expiryTime = AppClock::now() + std::chrono::milliseconds(milliseconds);
        if (animationExpiryTime_ < expiryTime) {
            animationExpiryTime_ = expiryTime;
        }
    }

    static bool isAnimationActive() {
        return isAnimationActive_;
    }

private:
    using AppTimerListenerVec = std::vector<AppTimerListener *>;
    AppTimerListenerVec listeners_;

    static bool isAnimationActive_;
    static AppClock::TimePoint animationExpiryTime_;
};

} // namespace litelockr

#endif // APP_TIMER_H
