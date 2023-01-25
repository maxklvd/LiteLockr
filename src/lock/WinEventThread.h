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

#ifndef WIN_EVENT_THREAD_H
#define WIN_EVENT_THREAD_H

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <windows.h>

namespace litelockr {

class WinEventThread {
public:
    static void startThread();
    static void stopThread();

    static bool isTaskbarChanged();
    static void resetTaskbarChanged();

private:
    static void threadProc();

    static std::mutex mtx_;
    static std::condition_variable condVar_;
    static bool readyFlag_;

    static std::atomic<bool> taskbarChanged_;
    static thread_local HWINEVENTHOOK winEventHook_;

    static void CALLBACK winEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                                      LONG idObject, LONG idChild,
                                      DWORD dwEventThread, DWORD dwmsEventTime);
};

} // namespace litelockr

#endif // WIN_EVENT_THREAD_H
