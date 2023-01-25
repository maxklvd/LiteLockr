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

#include "WinEventThread.h"

#include <cassert>
#include <thread>

#include "app/User32.h"
#include "gui/WindowUtils.h"
#include "log/Logger.h"
#include "sys/Process.h"

namespace litelockr {

std::mutex WinEventThread::mtx_;
std::condition_variable WinEventThread::condVar_;
bool WinEventThread::readyFlag_{false};

std::atomic<bool> WinEventThread::taskbarChanged_{false};

thread_local HWINEVENTHOOK WinEventThread::winEventHook_{nullptr};

void WinEventThread::startThread() {
    assert(GetCurrentThreadId() == Process::hookThreadId());

    assert(!readyFlag_);
    assert(Process::winEventThreadId() == 0);
    resetTaskbarChanged();
    std::thread thr(WinEventThread::threadProc);

    // wait for the WinEventThread
    {
        std::unique_lock<std::mutex> lk(mtx_);
        condVar_.wait(lk, [] { return readyFlag_; });
    }

    LOG_DEBUG(L"[WinEventThread] The thread has started work");
    thr.detach();
}

void WinEventThread::stopThread() {
    assert(GetCurrentThreadId() == Process::hookThreadId());
    assert(Process::winEventThreadId());

    if (Process::winEventThreadId()) {
        LOG_DEBUG(L"[WinEventThread] The thread is stopping");
        PostThreadMessage(Process::winEventThreadId(), WM_QUIT, 0, 0);

        // wait for the WinEventThread
        {
            std::unique_lock<std::mutex> lk(mtx_);
            condVar_.wait(lk, [] { return !readyFlag_; });
        }

        LOG_DEBUG(L"[WinEventThread] The thread has stopped");
    }
}

void WinEventThread::threadProc() {
    Process::setWinEventThreadId(GetCurrentThreadId());

    [[maybe_unused]] HRESULT hr = CoInitialize(nullptr);
    assert(SUCCEEDED(hr));

    MSG msg;
    PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    //
    // Set WinEventHook
    //
    assert(winEventHook_ == nullptr);
    winEventHook_ = User32::setWinEventHook(
            EVENT_OBJECT_CREATE, EVENT_OBJECT_DESTROY,
            nullptr,
            winEventProc,
            0, 0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    // WinEventThread is ready now
    {
        std::scoped_lock<std::mutex> lock{mtx_};
        readyFlag_ = true;
    }
    condVar_.notify_one();
    //

    //
    // message loop
    //
    while (GetMessage(&msg, nullptr, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //
    // Unset WinEventHook
    //
    assert(winEventHook_);
    User32::unhookWinEvent(winEventHook_);
    winEventHook_ = nullptr;
    CoUninitialize();

    // reset the ready flag
    {
        std::scoped_lock<std::mutex> lock{mtx_};
        assert(readyFlag_);
        readyFlag_ = false;
    }
    condVar_.notify_one();
    Process::setWinEventThreadId(0);
}


bool WinEventThread::isTaskbarChanged() {
    return taskbarChanged_.load(std::memory_order_relaxed);
}

void WinEventThread::resetTaskbarChanged() {
    taskbarChanged_.store(false, std::memory_order_relaxed);
}

void CALLBACK WinEventThread::winEventProc(HWINEVENTHOOK /*hook*/, DWORD event, HWND hwnd,
                                           LONG idObject, LONG idChild,
                                           DWORD /*dwEventThread*/, DWORD /*dwmsEventTime*/) {
    if (event == EVENT_OBJECT_CREATE) {
        if (WindowUtils::isRunningApplications(hwnd)) {
            LOG_DEBUG(L"[WinEvent] EVENT_OBJECT_CREATE: hwnd:0x%x idObject:%d idChild:%d", hwnd, idObject, idChild);
            taskbarChanged_.store(true, std::memory_order_relaxed);
        }
    } else if (event == EVENT_OBJECT_DESTROY && WindowUtils::isRunningApplications(hwnd)) {
        LOG_DEBUG(L"[WinEvent] EVENT_OBJECT_DESTROY: hwnd:0x%x idObject:%d idChild:%d", hwnd, idObject, idChild);
        taskbarChanged_.store(true, std::memory_order_relaxed);
    }
}

} // namespace litelockr
