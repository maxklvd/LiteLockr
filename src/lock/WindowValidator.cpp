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

#include "WindowValidator.h"

#include "gui/WindowUtils.h"
#include "lock/apps/ExplorerCfg.h"
#include "log/Logger.h"
#include "sys/Executable.h"
#include "sys/Process.h"
#include "sys/StringUtils.h"

namespace litelockr {

bool WindowValidator::findAppByPid(DWORD processId, std::wstring& exeFile) {
    auto name = appByPidCache_.get(processId);
    if (name) {
        exeFile = name.get();
        return true;
    }

    exeFile = Executable::getExecutableFileByPID(processId);
    if (!exeFile.empty()) {
        appByPidCache_.insert(processId, exeFile);
        return true;
    }
    return false;
}

bool WindowValidator::isAllowed(HWND hWnd) {
    assert(GetCurrentThreadId() == Process::hookThreadId());

    assert(hWnd);
    if (hWnd == nullptr) {
        return false;
    }

    hWnd = appConfigSet_.findRootWindow(hWnd);

    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);

    if (processId == 0) {
        return false;
    }

    if (lastUsedValue_.pid == processId && lastUsedValue_.hWnd == hWnd) {
        return lastUsedValue_.allowed; // cached value
    }

    lastUsedValue_ = {.pid = processId, .hWnd = hWnd};

    if (processId && processId == Process::processId()) {
        lastUsedValue_.allowed = true;
        return true;
    }

    if (dontLockProcessId_ != INVALID_PROCESS_ID && dontLockProcessId_ == processId) {
        lastUsedValue_.allowed = true;
        return true;
    }

    if (containsInAppList(processId)) {
        lastUsedValue_.allowed = true;
        return true;
    }

    if (containsInClassList(processId, hWnd)) {
        lastUsedValue_.allowed = true;
        return true;
    }
    return false;
}

bool WindowValidator::isAllowedInfo(HWND hWnd, std::wstring& exeName) {
    assert(GetCurrentThreadId() == Process::mainThreadId());
    assert(hWnd);

    if (hWnd == nullptr) {
        return false;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(appConfigSet_.findRootWindow(hWnd), &processId);

    if (processId == 0) {
        return false;
    }

    //
    // Check the processId
    //
    if (!appSet_.empty()) {
        std::wstring origExe;
        if (findAppByPid(processId, origExe)) {
            std::wstring exe(origExe);
            StringUtils::toUpperCase(exe);

            if (appSet_.find(exe) != appSet_.end()) {
                exeName = origExe;
                return true;
            }
        }
    }
    return false;
}

bool WindowValidator::containsInAppList(DWORD processId) {
    assert(GetCurrentThreadId() != Process::mainThreadId());

    //
    // Is the processId already checked?
    //
    auto allowed = allowedByAppCache_.get(processId);
    if (allowed) {
        return allowed.value();
    }

    //
    // Check the processId
    //
    if (!appSet_.empty()) {
        std::wstring exe;
        if (findAppByPid(processId, exe)) {
            StringUtils::toUpperCase(exe);

            if (appSet_.find(exe) != appSet_.end()) {
                allowedByAppCache_.put(processId, true);
                return true;
            }
        }
    }

    allowedByAppCache_.put(processId, false);
    return false;
}

bool WindowValidator::containsInClassList(DWORD processId, HWND hWnd) {
    assert(GetCurrentThreadId() != Process::mainThreadId());

    PidHwndKey processIdWnd(processId, hWnd);
    //
    // Are the processId and hWnd already checked?
    //
    auto allowed = allowedByClassCache_.get(processIdWnd);
    if (allowed) {
        return allowed.value();
    }

    if (!classSet_.empty()) {
        std::wstring app;
        if (findAppByPid(processId, app)) {
            constexpr int BUF_SIZE = 256;
            wchar_t buf[BUF_SIZE] = {0};

            if (GetClassName(hWnd, buf, BUF_SIZE)) {
                std::wstring appClassPair = app;
                appClassPair += L':';
                appClassPair += buf;

                StringUtils::toUpperCase(appClassPair);

                if (classSet_.find(appClassPair) != classSet_.end()) {
                    allowedByClassCache_.put(processIdWnd, true);
                    return true;
                }
            }
        }
    }

    allowedByClassCache_.put(processIdWnd, false);
    return false;
}

void WindowValidator::clearAll() {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    appSet_.clear();
    classSet_.clear();

    allowedByAppCache_.clear();
    allowedByClassCache_.clear();
    appByPidCache_.clear();

    lastUsedValue_ = {};

    dontLockProcessId_ = INVALID_PROCESS_ID;
}

void WindowValidator::addAllowedApp(const std::wstring& app) {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    std::wstring exe = StringUtils::toUpperCase(Executable::getFileName(app));

    LOG_DEBUG(L"[WindowValidator] allowed app: %s", exe.c_str());
    appSet_.insert(exe);
}

void WindowValidator::addAllowedWindowClass(const std::wstring& windowClass) {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    std::wstring str = StringUtils::toUpperCase(windowClass);
    LOG_DEBUG(L"[WindowValidator] allowed window class: %s", str.c_str());
    classSet_.insert(str);
}

bool WindowValidator::isExplorer(HWND hWnd) {
    auto cached = isExplorerCache_.get(hWnd);
    if (cached) {
        return cached.value();
    }

    bool value = ExplorerCfg::isExplorer(hWnd);
    isExplorerCache_.put(hWnd, value);
    return value;
}

bool WindowValidator::isFullScreenWindow(HWND hWnd) {
    auto cached = isFullScreenWindowCache_.get(hWnd);
    if (cached) {
        return cached.value();
    }

    bool value = false;
    if (!isExplorer(hWnd) &&
        WindowUtils::isFullScreenWindow(hWnd)) {
        value = true;
    }

    isFullScreenWindowCache_.put(hWnd, value);
    return value;
}

} // namespace litelockr
