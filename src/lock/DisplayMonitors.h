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

#ifndef DISPLAY_MONITORS_H
#define DISPLAY_MONITORS_H

#include <map>
#include <mutex>
#include <vector>

#include <windows.h>

namespace litelockr {

struct MonitorInfo {
    int numMonitors = 0;
    SIZE primaryMonitor{};
    RECT unionMonitor{};
    std::map<HMONITOR, int> hMonitors;
    std::vector<RECT> monitors;
    std::vector<RECT> workAreas;
};

class DisplayMonitors {
public:
    static void update();
    static void get(MonitorInfo& monitorInfo);
    static int numMonitors();

private:
    static BOOL CALLBACK enumDisplayMonitorProc(HMONITOR hMonitor, HDC /* hdc */, LPRECT lpRMonitor, LPARAM dwData);

    static MonitorInfo monitorInfo_;
    static std::mutex mtx_;
};

} // namespace litelockr

#endif // DISPLAY_MONITORS_H
