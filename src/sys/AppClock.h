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

#ifndef APP_CLOCK_H
#define APP_CLOCK_H

#include <chrono>

namespace litelockr {

template<typename C>
class BaseClock {
public:
    using Clock = C;
    using Duration = typename C::duration;
    using TimePoint = typename C::time_point;

    static TimePoint now() { return Clock::now(); }
};

class AppClock: public BaseClock<std::chrono::steady_clock> {
};

class TimeCounter {
public:
    TimeCounter() = default;
    explicit TimeCounter(AppClock::Duration periodTime);
    explicit TimeCounter(AppClock::Duration periodTime, bool enabled);
    TimeCounter(const TimeCounter&) = delete;
    TimeCounter& operator=(const TimeCounter&) = delete;

    void create(AppClock::Duration period);
    void destroy();
    void reset();
    bool timeout();
    [[nodiscard]] AppClock::Duration elapsed() const;
    [[nodiscard]] AppClock::Duration period() const;
    void setEnabled(bool enabled);

private:
    AppClock::Duration period_{};
    AppClock::Duration elapsed_{};
    AppClock::TimePoint lastTime_;
    bool hasInitialized_ = false;
    bool enabled_ = true;
};

class OneShotTimeCounter {
public:
    OneShotTimeCounter() = default;

    explicit OneShotTimeCounter(AppClock::Duration waitTime) : timeCounter_(waitTime) {}

    OneShotTimeCounter(const OneShotTimeCounter&) = delete;
    OneShotTimeCounter& operator=(const OneShotTimeCounter&) = delete;

    void create(AppClock::Duration waitTime);
    void recreate(AppClock::Duration waitTime);
    void reset();
    void destroy();
    bool timeout();

private:
    TimeCounter timeCounter_;
    bool finished_ = false;
};

} // namespace litelockr

#endif // APP_CLOCK_H
