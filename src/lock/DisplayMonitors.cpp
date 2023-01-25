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

#include "DisplayMonitors.h"

#include <cassert>

#include "log/Logger.h"
#include "sys/Rectangle.h"

namespace litelockr {

MonitorInfo DisplayMonitors::monitorInfo_;
std::mutex DisplayMonitors::mtx_;

BOOL CALLBACK DisplayMonitors::enumDisplayMonitorProc(HMONITOR hMonitor, HDC /* hdc */, LPRECT /*lpRMonitor*/,
                                                      LPARAM dwData) {
    assert(dwData);

    auto& mi = *reinterpret_cast<MonitorInfo *>(dwData);
    MONITORINFOEX mix = {sizeof(mix)};

    if (GetMonitorInfo(hMonitor, &mix)) {
        LOG_DEBUG(L"Monitor: %s%s %s WorkArea: %s",
                  mix.szDevice,
                  (mix.dwFlags & MONITORINFOF_PRIMARY ? L" [primary]" : L""),
                  Rectangle::toString(mix.rcMonitor).c_str(),
                  Rectangle::toString(mix.rcWork).c_str());

        mi.hMonitors[hMonitor] = mi.numMonitors;
        mi.numMonitors++;
        mi.monitors.push_back(mix.rcMonitor);
        mi.workAreas.push_back(mix.rcWork);
        UnionRect(&mi.unionMonitor, &mi.unionMonitor, &mix.rcMonitor);
    }
    return TRUE;
}

void DisplayMonitors::update() {
    MonitorInfo mi{};
    mi.primaryMonitor = {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    EnumDisplayMonitors(nullptr, nullptr, enumDisplayMonitorProc, reinterpret_cast<LPARAM>(&mi));

    //
    // Updates monitorInfo_
    //
    {
        std::scoped_lock<std::mutex> lock{mtx_};
        monitorInfo_ = mi;
    }
}

void DisplayMonitors::get(MonitorInfo& monitorInfo) {
    std::scoped_lock<std::mutex> lock{mtx_};
    monitorInfo = monitorInfo_;
}

int DisplayMonitors::numMonitors() {
    std::scoped_lock<std::mutex> lock{mtx_};
    return monitorInfo_.numMonitors;
}

} // namespace litelockr
