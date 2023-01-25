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

#include "Process.h"

#include <cassert>

namespace litelockr {

HINSTANCE Process::hModuleInstance_(nullptr);
HWND Process::hWndRoot_(nullptr);
DWORD Process::mainThreadId_(0);
DWORD Process::hookThreadId_(0);
DWORD Process::workerThreadId_(0);
DWORD Process::winEventThreadId_(0);
DWORD Process::processId_(0);
std::wstring Process::modulePath_;
bool Process::remoteSession_(false);
bool Process::isSettingsDialogOpen_(false);
HWND Process::activeWindow_(nullptr);
std::wstring Process::currentAppAutomationId_;
bool Process::lightMode_(false);

void Process::initialize() {
    assert(hModuleInstance_ == nullptr); // must be set only once
    hModuleInstance_ = GetModuleHandleA(nullptr);

    assert(mainThreadId_ == 0);
    mainThreadId_ = GetCurrentThreadId();

    assert(processId_ == 0);
    processId_ = GetCurrentProcessId();

    assert(modulePath_.empty());
    wchar_t buf[MAX_PATH] = {0};
    GetModuleFileName(nullptr, buf, MAX_PATH);
    size_t len = wcslen(buf);
    while (len--) {
        if (buf[len] == L'\\') {
            buf[len + 1] = 0;
            break;
        }
    }
    modulePath_ = buf;

    updateRemoteSession();
}

HINSTANCE Process::moduleInstance() {
    assert(hModuleInstance_);
    return hModuleInstance_;
}

void Process::setMainWindow(HWND hWnd) {
    assert(hWnd);
    hWndRoot_ = hWnd;
}

HWND Process::mainWindow() {
    assert(hWndRoot_);
    return hWndRoot_;
}

DWORD Process::mainThreadId() {
    assert(mainThreadId_ != 0);
    return mainThreadId_;
}

DWORD Process::processId() {
    assert(processId_ != 0);
    return processId_;
}

void Process::updateRemoteSession() {
    remoteSession_ = (GetSystemMetrics(SM_REMOTESESSION) != 0);
    if (remoteSession_) {
        ClipCursor(nullptr); // reset
    }
}


} // namespace litelockr
