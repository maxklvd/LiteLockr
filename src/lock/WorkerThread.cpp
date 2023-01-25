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

#include "WorkerThread.h"

#include <dwmapi.h>
#include <windowsx.h>
#include "app/AppMessages.h"
#include "app/NotificationArea.h"
#include "ini/SettingsData.h"
#include "log/Logger.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

#include "res/Resources.h"

namespace litelockr {

std::mutex WorkerThread::mtx_;
std::condition_variable WorkerThread::condVar_;
bool WorkerThread::readyFlag_{false};

thread_local WorkerThreadOptions WorkerThread::options_;

void WorkerThread::startThread() {
    assert(GetCurrentThreadId() == Process::hookThreadId());

    assert(!readyFlag_);
    assert(Process::workerThreadId() == 0);

    const auto& settings = SettingsData::instance();

    WorkerThreadOptions options;
    options.minimizeByDoubleClick = settings.minimizeByDoubleClick.value();
    options.minimizeByCtrlDoubleClick = settings.minimizeByCtrlDoubleClick.value();
    options.minimizeByCaptionButton = settings.minimizeByCaptionButton.value();

    std::thread thr(WorkerThread::threadProc, options);

    // wait for the WorkerThread
    {
        std::unique_lock<std::mutex> lk(mtx_);
        condVar_.wait(lk, [] { return readyFlag_; });
    }

    LOG_DEBUG(L"[WorkerThread] The thread has started work");
    thr.detach();
}

void WorkerThread::stopThread() {
    assert(GetCurrentThreadId() == Process::hookThreadId());
    assert(Process::workerThreadId());

    if (Process::workerThreadId()) {
        LOG_DEBUG(L"[WorkerThread] The thread is stopping");
        PostThreadMessage(Process::workerThreadId(), WM_QUIT, 0, 0);

        // wait for the WorkerThread
        {
            std::unique_lock<std::mutex> lk(mtx_);
            condVar_.wait(lk, [] { return !readyFlag_; });
        }

        LOG_DEBUG(L"[WorkerThread] The thread has stopped");
    }
}

void WorkerThread::threadProc(WorkerThreadOptions opt) {
    Process::setWorkerThreadId(GetCurrentThreadId());
    options_ = opt;

    MSG msg;
    PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    // WorkerThread is ready now
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
        if (msg.message == WMU_WT_PROCESS_CAPTION_BUTTON) {
            bool dblClick = (msg.wParam & PCB_DBL_CLICK) > 0;
            bool ctrlPressed = (msg.wParam & PCB_CTRL_PRESSED) > 0;
            POINT pt{GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};
            processCaptionButton(pt, dblClick, ctrlPressed);
        } else if (msg.message == WMU_WT_PROCESS_SYSTRAY_ICON) {
            processSystemTrayIcon(msg.wParam, msg.lParam);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // reset the ready flag
    {
        std::scoped_lock<std::mutex> lock{mtx_};
        assert(readyFlag_);
        readyFlag_ = false;
    }
    condVar_.notify_one();
    Process::setWorkerThreadId(0);
}


bool WorkerThread::isMenu(HWND hWnd) {
    constexpr int BUF_SIZE = 256;
    wchar_t buf[BUF_SIZE] = {0};

    if (GetClassName(hWnd, buf, BUF_SIZE)) {
        if (!wcscmp(buf, L"#32768")) { // The class for a menu
            return true;
        }
    }
    return false;
}

void WorkerThread::processCaptionButton(POINT pt, bool dblClick, bool ctrlPressed) {
    if (options_.minimizeByDoubleClick ||
        options_.minimizeByCtrlDoubleClick ||
        options_.minimizeByCaptionButton) {

        HWND _hWnd = WindowFromPoint(pt);
        HWND hWnd = GetAncestor(_hWnd, GA_ROOT);

        if (hWnd) {
            //
            // Check that either Double Click or Ctrl + Double Click has occurred
            //
            if (dblClick && (
                    (options_.minimizeByDoubleClick && !ctrlPressed) ||
                    (options_.minimizeByCtrlDoubleClick && ctrlPressed)
            )) {
                if (isMenu(hWnd)) {
                    PostMessage(hWnd, WM_SHOWWINDOW, SW_HIDE, 0L);
                } else {
                    PostMessage(hWnd, WM_CANCELMODE, 0, 0L);
                    PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0L);
                }
                return;
            }

            int button = findByTitleBarInfo(pt, hWnd);
            if (!button) {
                button = findByHitTest(pt, hWnd);
            }

            switch (button) {
                case HTCLOSE:
                case HTMINBUTTON:
                    if (options_.minimizeByCaptionButton) {
                        PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0L);
                    }
                    break;
                default:
                    // nothing to do
                    break;
            }
        }
    }
}

bool isElementVisible(DWORD state, const RECT& rect) {
    if (state & STATE_SYSTEM_INVISIBLE) {
        return false;
    }

    return !Rectangle::empty(rect);
}

int WorkerThread::findByTitleBarInfo(POINT pt, HWND hWnd) {
    TITLEBARINFOEX tbi = {sizeof(tbi)};

    if (SendMessage(hWnd, WM_GETTITLEBARINFOEX, 0, reinterpret_cast<LPARAM>(&tbi))) {
        //
        // Check the TitleBar visibility
        //
        if (!isElementVisible(tbi.rgstate[0], tbi.rcTitleBar)) {
            return 0;
        }

        const int IDX_MINIMIZE = 2;
        const int IDX_CLOSE = 5;

        int CY_SIZE = GetSystemMetrics(SM_CYSIZE); // minimal button height

        //
        // 'Close' button
        //
        auto& rcClose = tbi.rgrect[IDX_CLOSE];
        if (isElementVisible(tbi.rgstate[IDX_CLOSE], rcClose)) {
            // fix
            if (Rectangle::height(rcClose) < CY_SIZE) {
                rcClose.top = rcClose.bottom - CY_SIZE;
            }

            if (Rectangle::contains(rcClose, pt)) {
                return HTCLOSE;
            }
        }

        //
        // 'Minimize' button
        //
        auto& rcMinimize = tbi.rgrect[IDX_MINIMIZE];
        if (isElementVisible(tbi.rgstate[IDX_MINIMIZE], rcMinimize)) {
            // fix
            if (Rectangle::height(rcMinimize) < CY_SIZE) {
                rcMinimize.top = rcMinimize.bottom - CY_SIZE;
            }

            if (Rectangle::contains(rcMinimize, pt)) {
                return HTMINBUTTON;
            }
        }
    }
    return 0;
}

int WorkerThread::findByHitTest(POINT pt, HWND hWnd) {
    LRESULT button = SendMessage(hWnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
    if (button == HTCLOSE || button == HTMINBUTTON || button == HTSYSMENU) {
        return static_cast<int>(button);
    }
    return 0;
}

void WorkerThread::processSystemTrayIcon(WPARAM wParam, LPARAM lParam) {
    assert(wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP);

    POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    RECT iconRect = NotificationArea::getIconRect();

    LOG_DEBUG(L"[WorkerThread] processSystemTrayIcon, x=%d y=%d iconRec=%s",
              pt.x, pt.y, Rectangle::toString(iconRect).c_str());

    if (!Rectangle::empty(iconRect) && Rectangle::contains(iconRect, pt)) {
        switch (wParam) {
            case WM_LBUTTONUP:
                LOG_DEBUG(L"[WorkerThread] send WMU_TRAY_ICON_NOTIFY NIN_SELECT");
                PostMessage(Process::mainWindow(), WMU_TRAY_ICON_NOTIFY, 0, NIN_SELECT);
                break;
            case WM_RBUTTONUP:
                LOG_DEBUG(L"[WorkerThread] send WMU_TRAY_ICON_NOTIFY WM_CONTEXTMENU");
                PostMessage(Process::mainWindow(), WMU_TRAY_ICON_NOTIFY, 0, WM_CONTEXTMENU);
                break;
            default:
                assert(false);
                break;
        }
    }
}

} // namespace litelockr
