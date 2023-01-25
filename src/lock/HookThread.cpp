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

#include "HookThread.h"

#include <thread>
#include <future>

#include "app/AppParameters.h"
#include "app/HotkeyHandler.h"
#include "ini/SettingsData.h"
#include "lock/HookData.h"
#include "lock/MouseFilter.h"
#include "lock/WorkerThread.h"
#include "lock/WinEventThread.h"
#include "lock/apps/ExplorerCfg.h"
#include "lock/hook/HookFactory.h"
#include "lock/uia/UIAutomationHelper.h"
#include "sys/Process.h"

namespace litelockr {

std::mutex HookThread::mtx_;
std::condition_variable HookThread::condVar_;
bool HookThread::readyFlag_{false};

thread_local HookOptions HookThread::options_;
std::unique_ptr<AbstractHook> HookThread::hook_;

std::function<void(bool isLocked)> HookThread::onLockStateChangedFunc_;


void HookThread::threadProc(HookOptions opt) {
    Process::setHookThreadId(GetCurrentThreadId());
    options_ = opt;

    WorkerThread::startThread();
    WinEventThread::startThread();

    assert(!hook_);
    hook_ = HookFactory::create(SettingsData::instance().eventInterception.value());
    hook_->initialize(options_);

    MSG msg;
    PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    // HookThread is ready now
    {
        std::scoped_lock<std::mutex> lock{mtx_};
        readyFlag_ = true;
    }
    if (onLockStateChangedFunc_) {
        onLockStateChangedFunc_(readyFlag_);
    }
    condVar_.notify_one();
    //

    hook_->start();
    hook_->dispose();
    hook_.reset();

    WinEventThread::stopThread();
    WorkerThread::stopThread();

    // reset the ready flag
    {
        std::scoped_lock<std::mutex> lock{mtx_};
        assert(readyFlag_);
        readyFlag_ = false;
    }
    if (onLockStateChangedFunc_) {
        onLockStateChangedFunc_(readyFlag_);
    }
    condVar_.notify_one();
    Process::setHookThreadId(0);
}

void HookThread::startLocking() {
    assert(GetCurrentThreadId() == Process::mainThreadId());
    if (HookThread::isLocked()) {
        return;
    }

    auto& settings = SettingsData::instance();
    auto& validator = HookData::windowValidator();
    const auto& appConfigSet = validator.getAppConfigSet();

    HWND hwndActive = Process::getActiveWindow();
    HWND hwndCurrentApp = appConfigSet.findRootWindow(hwndActive);
    if (appConfigSet.shouldSkipApp(hwndCurrentApp)) {
        hwndCurrentApp = nullptr;
    }
    settings.setupCurrentApp(hwndCurrentApp);

    validator.clearAll();
    for (const auto& app: settings.getAllowedApps()) {
        if (app.enabled) {
            validator.addAllowedApp(app.path());
        }
    }

    for (const auto& windowClass: ExplorerCfg::allowedWindowClasses()) {
        std::wstring str = ExplorerCfg::ExplorerExe;
        str += L":" + windowClass;
        validator.addAllowedWindowClass(str);
    }

    auto& hk = HotkeyHandler::instance();

    HookOptions options;
    options.lockKeyboard = settings.lockKeyboard.value();
    options.lockMouse = settings.lockMouse.value();
    options.unlockOnCtrlAltDel = settings.unlockOnCtrlAltDel.value();
    options.processCaptionButtons = settings.minimizeByDoubleClick.value() ||
                                    settings.minimizeByCtrlDoubleClick.value() ||
                                    settings.minimizeByCaptionButton.value();
    options.hotkey = {hk.vkCode(), hk.isCtrl(), hk.isAlt(), hk.isShift()};

    //
    assert(options.lockKeyboard || options.lockMouse);


    if (hwndCurrentApp && !(AppParameters::autoRun && AppParameters::lockNow)) {
        initializeCurrentApp(hwndActive);
    }
    if (options.lockMouse) {
        MouseFilter::updatePositionValidator(MousePositionValidatorTimer::UPDATE_IMMEDIATELY);
    }

    assert(!readyFlag_);
    assert(Process::hookThreadId() == 0);
    std::thread thr(HookThread::threadProc, options);

    // wait for the HookThread
    {
        std::unique_lock<std::mutex> lk(mtx_);
        condVar_.wait(lk, [] { return readyFlag_; });
    }

    LOG_DEBUG(L"[HookThread] The thread has started work");
    thr.detach();
}

void HookThread::stopLocking() {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    if (!HookThread::isLocked()) {
        return;
    }

    assert(Process::hookThreadId());
    if (hook_) {
        LOG_DEBUG(L"[HookThread] The thread is stopping");
        hook_->stop();

        // wait for the HookThread
        {
            std::unique_lock<std::mutex> lk(mtx_);
            condVar_.wait(lk, [] { return !readyFlag_; });
        }

        LOG_DEBUG(L"[HookThread] The thread has stopped");
    }
}

bool HookThread::isLocked() {
    assert(GetCurrentThreadId() == Process::mainThreadId());
    return readyFlag_;
}

void HookThread::initializeCurrentApp(HWND hwndActive) {
    Process::setCurrentAppAutomationId(L""); // reset id

    if (hwndActive) {
        currentAppSelection().show(hwndActive); // shows blue corners

        //
        // get an automation id of the current app
        //
        auto automationIdFuture = std::async(std::launch::async, []() {
            UIAutomationHelper uia;
            return uia.getAutomationIdOfActiveButton();
        });
        Process::setCurrentAppAutomationId(automationIdFuture.get());
        LOG_DEBUG(L"[CurrentApp] AutomationId = %s", Process::currentAppAutomationId().c_str());
    }
}

} // namespace litelockr
