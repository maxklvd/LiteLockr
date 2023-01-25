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

#ifndef NULL_HOOK_H
#define NULL_HOOK_H

#include <condition_variable>
#include <mutex>

#include "lock/hook/AbstractHook.h"

namespace litelockr {

//
// Note: set EventInterception=-1 in the LiteLockr.ini if you want to use this hook
//

class NullHook: public AbstractHook {
public:
    void initialize(const HookOptions& options) override;

    void dispose() override { /* nothing to dispose, it's a dummy hook */ }

    bool canBeUsed() override { return true; }

    void start() override;
    void stop() override;

private:
    std::mutex mtx_;
    std::condition_variable condVar_;
    static bool stopFlag_;
};

} // namespace litelockr

#endif // NULL_HOOK_H
