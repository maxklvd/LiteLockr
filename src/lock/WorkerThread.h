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

#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <condition_variable>
#include <mutex>
#include <thread>

#include <windows.h>

constexpr UINT WMU_WT_PROCESS_CAPTION_BUTTON = WM_APP + 200;
constexpr UINT WMU_WT_PROCESS_SYSTRAY_ICON = WM_APP + 201;

namespace litelockr {

struct WorkerThreadOptions {
    bool minimizeByDoubleClick = false;
    bool minimizeByCtrlDoubleClick = false;
    bool minimizeByCaptionButton = false;
};

class WorkerThread {
public:
    static void startThread();
    static void stopThread();

    constexpr static auto PCB_DBL_CLICK = 1;
    constexpr static auto PCB_CTRL_PRESSED = 2;

private:
    static void threadProc(WorkerThreadOptions opt);

    static void processCaptionButton(POINT pt, bool dblClick, bool ctrlPressed);
    static void processSystemTrayIcon(WPARAM wParam, LPARAM lParam);
    static bool isMenu(HWND hWnd);
    static int findByTitleBarInfo(POINT pt, HWND hWnd);
    static int findByHitTest(POINT pt, HWND hWnd);

    static std::mutex mtx_;
    static std::condition_variable condVar_;
    static bool readyFlag_;

    static thread_local WorkerThreadOptions options_;
};

} // namespace litelockr

#endif // WORKER_THREAD_H
