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

#ifndef BASE_EVENT_HANDLER_H
#define BASE_EVENT_HANDLER_H

#include "AppEvent.h"
#include "app/FlyoutWindow.h"
#include "sys/AppTimer.h"

namespace litelockr {

class BaseEventHandler: public AppTimerListener {
public:
    explicit BaseEventHandler(FlyoutWindow& window) : window_(window) {}

    void appExit() const;
    void closeApplication() const;
    void timeout() override;
    [[nodiscard]] bool isCheckRequired(int appEventId) const;
    bool checkAccess(const AppEvent& event);

protected:
    [[nodiscard]] bool canHandleEvent(const AppEvent& event) const;

    static bool isIdle();

    void clickKeyboard();
    void clickMouse();
    void startLocking(bool lockNow);
    void stopLocking(bool needRedraw = true);
    void clickLock(bool lockNow);
    void clickUnlock();
    static void clickHelp();

    void appHide(bool animation = false);
    void appShow(bool animation = false);

    [[nodiscard]] FlyoutWindow& window() const;
    FlyoutModel& model();
    [[nodiscard]] const FlyoutModel& model() const;
    [[nodiscard]] FlyoutView& view() const;

    static SettingsData& appSettings() {
        return SettingsData::instance();
    }

    [[nodiscard]] bool hidden() const {
        return !model().visible();
    }

    [[nodiscard]] int lockState() const {
        return model().lockState();
    }

    FlyoutWindow& window_;

private:
    bool lockWhenIdleFired_ = false;
    TimeCounter checkInterval_{std::chrono::seconds(1)};
};

} // namespace litelockr

#endif // BASE_EVENT_HANDLER_H




