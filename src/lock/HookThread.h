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

#ifndef HOOK_THREAD_H
#define HOOK_THREAD_H

#include <condition_variable>
#include <functional>
#include <mutex>

#include "lock/hook/AbstractHook.h"
#include "lock/ui/AppSelection.h"

namespace litelockr {

class HookThread {
public:
    HookThread() = delete;

    static void startLocking();
    static void stopLocking();
    static bool isLocked();

    using LockStateChangedFunc = std::function<void(bool isLocked)>;

    static void setLockStateListener(LockStateChangedFunc&& func) {
        onLockStateChangedFunc_ = std::move(func);
    }

    static void initializeCurrentApp(HWND hwndActive);

    static AppSelection& currentAppSelection() {
        static AppSelection currentAppSelection_;
        return currentAppSelection_;
    }

private:
    static void threadProc(HookOptions opt);

    static std::mutex mtx_;
    static std::condition_variable condVar_;
    static bool readyFlag_;

    thread_local static HookOptions options_;
    static std::unique_ptr<AbstractHook> hook_;

    static LockStateChangedFunc onLockStateChangedFunc_;
};

} // namespace litelockr

#endif // HOOK_THREAD_H