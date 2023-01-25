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

#ifndef WINDOW_VALIDATOR_H
#define WINDOW_VALIDATOR_H

#include <unordered_set>

#include <windows.h>
#include "lock/ExpiringCache.h"
#include "lock/config/AppConfigSet.h"

namespace litelockr {

class WindowValidator {
public:
    WindowValidator() = default;

    bool isAllowed(HWND hWnd);
    bool isAllowedInfo(HWND hWnd, std::wstring& exeName);
    void clearAll();
    void addAllowedApp(const std::wstring& app);
    void addAllowedWindowClass(const std::wstring& windowClass);
    bool isExplorer(HWND hWnd);
    bool isFullScreenWindow(HWND hWnd);

    const AppConfigSet& getAppConfigSet() const { return appConfigSet_; }

private:
    ExpiringCache<HWND, bool> isExplorerCache_{std::chrono::minutes(5)};
    ExpiringCache<HWND, bool> isFullScreenWindowCache_{std::chrono::seconds(1)};

    boost::compute::detail::lru_cache<DWORD, std::wstring> appByPidCache_{1024};
    bool findAppByPid(DWORD processId, std::wstring& exeFile);

    ExpiringCache<DWORD, bool> allowedByAppCache_{std::chrono::minutes(5)};

    using PidHwndKey = std::pair<DWORD, HWND>;
    ExpiringCache<PidHwndKey, bool> allowedByClassCache_{std::chrono::minutes(5)};

    std::unordered_set<std::wstring> appSet_;
    std::unordered_set<std::wstring> classSet_;

    constexpr static auto INVALID_PROCESS_ID = std::numeric_limits<unsigned long>::max();
    DWORD dontLockProcessId_ = INVALID_PROCESS_ID;

    AppConfigSet appConfigSet_;

    struct {
        DWORD pid = 0;
        HWND hWnd = nullptr;
        bool allowed = false;
    } lastUsedValue_;

    bool containsInAppList(DWORD processId);
    bool containsInClassList(DWORD processId, HWND hWnd);
};


} // namespace litelockr

#endif // WINDOW_VALIDATOR_H
