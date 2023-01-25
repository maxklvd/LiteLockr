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

#ifndef ABSTRACT_HOOK_H
#define ABSTRACT_HOOK_H

#include <atomic>

#include "lock/HookOptions.h"

namespace litelockr {

class AbstractHook {
public:
    virtual ~AbstractHook() = default;

    virtual void initialize(const HookOptions& options) = 0;
    virtual void dispose() = 0;
    virtual bool canBeUsed() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    static bool checkDisplayChanged();
    static void setDisplayChanged(bool changed);

    static unsigned long lastInputTime();

protected:
    static void initializeLastInputTime();
    static void updateLastInputTime();

private:
    static std::atomic<bool> displayChanged_;
    static std::atomic<unsigned long> lastInputTime_;
};

} // namespace litelockr

#endif // ABSTRACT_HOOK_H
