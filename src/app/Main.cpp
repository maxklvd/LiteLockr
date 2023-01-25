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

#include <windows.h>
#include <CommCtrl.h>
#include <crtdbg.h>
#include <VersionHelpers.h>
#include "app/AppMessages.h"
#include "app/HotkeyHandler.h"
#include "app/event/AppEventHandler.h"
#include "gui/WindowUtils.h"
#include "log/Logger.h"
#include "sys/BinaryResource.h"
#include "sys/MiniDump.h"

namespace litelockr {

void doMessageLoop(AppTimer& appTimer) {
    using namespace std::chrono_literals;

    TimeCounter counter(32ms);

    constexpr UINT MESSAGE_LOOP_WAKEUP_TIMER = 1000;
    SetTimer(nullptr, MESSAGE_LOOP_WAKEUP_TIMER, 150, nullptr);

    for (;;) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return;
            }

            if (!Dialog::isDialogMessage(&msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (counter.timeout()) {
            appTimer.fireTimeout();
        }

        if (AppTimer::isAnimationActive()) {
            // use PeekMessage for smooth animation
            Sleep(1); // sleeps for the smallest time interval to avoid high CPU usage
        } else {
            WaitMessage();
        }
    }
}

HANDLE addFontFromResource(int resourceId) {
    DWORD nFonts = 0;
    HANDLE hFont = AddFontMemResourceEx(
            (PVOID) Bin::data(resourceId),
            Bin::size(resourceId),
            nullptr,
            &nFonts
    );
    assert(nFonts == 1);
    return hFont;
}

int winMain() {
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    //process cmd options such as '/lock', '/locknow', '/unlock'
    AppParameters::initialize();
    assert(AppParameters::hasInitialized());

    Process::initialize();
    Log::initialize(AppParameters::log);

    MiniDump::initialize();
    MiniDump::setTerminateHandler();

    if (!IsWindows8OrGreater()) {
        MessageBox(nullptr, L"This program requires Windows 8 or newer.", APP_NAME, MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }

    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        return -1;
    }

    // check if the application is already running
    CreateMutex(nullptr, FALSE, Process::APP_MUTEX);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hWnd = FindWindow(FlyoutWindow::WINDOW_CLASS, nullptr);
        if (hWnd) {
            if (AppParameters::lock) {
                // /lock command
                SendMessage(hWnd, WMU_LOCK_COMMAND, 0, 0L);
            } else if (AppParameters::lockNow) {
                // /locknow command
                SendMessage(hWnd, WMU_LOCK_NOW_COMMAND, 0, 0L);
            } else if (AppParameters::unlock) {
                // /unlock command
                SendMessage(hWnd, WMU_UNLOCK_COMMAND, 0, 0L);
            } else if (AppParameters::hide) {
                // /hide command
                SendMessage(hWnd, WMU_HIDE_COMMAND, 0, 0L);
            } else if (AppParameters::show) {
                // /show command
                SendMessage(hWnd, WMU_SHOW_COMMAND, 0, 0L);
            } else if (AppParameters::exit) {
                // /exit command
                SendMessage(hWnd, WMU_EXIT_COMMAND, 0, 0L);
            } else {
                // show yourself
                ShowWindow(hWnd, SW_SHOWNORMAL);
                FlyoutView::setForeground(hWnd);
                SendMessage(hWnd, WMU_SHOW_YOURSELF, 0, 0L);
            }
        }
        return -1;
    }

    if (AppParameters::exit) {
        // /exit command
        return -1;
    }

    WindowUtils::registerDefaultClass();

    INITCOMMONCONTROLSEX iccx = {sizeof(iccx)};
    iccx.dwICC = ICC_LINK_CLASS | ICC_HOTKEY_CLASS;
    InitCommonControlsEx(&iccx);

    HANDLE hDigitFont = addFontFromResource(IDB_FONT_DIGITS);
    HANDLE hSymbolFont = addFontFromResource(IDB_FONT_SYMBOLS);

    auto& settings = SettingsData::instance();
    settings.initialize();
    if (Log::enabled(Log::Severity::Debug)) {
        settings.printAllValues();
    }

    SettingsData::setRegistryAutoRun(settings.startWithWindows.value());

    FlyoutAppTimerListener flyoutAppTimerListener;
    flyoutAppTimerListener.window().create();
    if (!NotificationArea::addIcon(LockIcon::Unlocked)) {
        assert(false);
        LOG_ERROR(L"Cannot add a tray notification icon");
        return -1;
    }

    AppEventHandler appEventHandler(flyoutAppTimerListener.window());
    appEventHandler.initialize();

    AppEvents::instance().registerHandler([&appEventHandler](const AppEvent& event) {
        appEventHandler.handleEvent(event);
    });

    AppTimer appTimer;
    appTimer.addListener(flyoutAppTimerListener);
    appTimer.addListener(AppEvents::instance());
    appTimer.addListener(appEventHandler);

    flyoutAppTimerListener.window().startup();

    //
    // Hot key initialization
    //
    HotkeyHandler::instance().initialize(SettingsData::instance().hotkey.value());

    timeBeginPeriod(1);

    doMessageLoop(appTimer);

    timeEndPeriod(1);

    if (hDigitFont) {
        RemoveFontMemResourceEx(hDigitFont);
    }

    if (hSymbolFont) {
        RemoveFontMemResourceEx(hSymbolFont);
    }

    return 0;
}

} // namespace litelockr

int APIENTRY WinMain(_In_ HINSTANCE /*hInstance*/,
                     _In_opt_ HINSTANCE /*hPrevInstance*/,
                     _In_ LPSTR    /*lpCmdLine*/,
                     _In_ int       /*nCmdShow*/) {
    return litelockr::winMain();
}
