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

#include "AppConfigSet.h"

#include "lock/apps/ApplicationFrameHostCfg.h"
#include "lock/apps/ExplorerCfg.h"
#include "lock/apps/LiteLockrCfg.h"
#include "lock/apps/ShellExperienceHostCfg.h"
#include "sys/Executable.h"
#include "sys/StringUtils.h"

namespace litelockr {

AppConfigSet::AppConfigSet() {
    add<ApplicationFrameHostCfg>();
    add<ExplorerCfg>();
    add<LiteLockrCfg>();
    add<ShellExperienceHostCfg>();
}

bool AppConfigSet::shouldSkipApp(const std::wstring& executable) const {
    if (auto cfg = map_.find(executable); cfg != map_.end()) {
        return cfg->second->shouldSkipApp();
    }
    return false;
}

bool AppConfigSet::shouldSkipApp(HWND hWnd) const {
    DWORD processId;
    if (GetWindowThreadProcessId(hWnd, &processId)) {
        auto executable = StringUtils::toUpperCase(Executable::getExecutableFileByPID(processId));
        return shouldSkipApp(executable);
    }
    return false;
}

HWND AppConfigSet::findRootWindow(HWND hWnd) const {
    for (const auto& [_, cfg]: map_) {
        if (cfg->findRootWindow(hWnd)) {
            return hWnd;
        }
    }
    return hWnd;
}

} // namespace litelockr