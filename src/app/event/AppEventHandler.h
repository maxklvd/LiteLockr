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

#ifndef APP_EVENT_HANDLER_H
#define APP_EVENT_HANDLER_H

#include <unordered_map>

#include "BaseEventHandler.h"

namespace litelockr {

class AppEventHandler: public BaseEventHandler {
public:
    explicit AppEventHandler(FlyoutWindow& window) : BaseEventHandler(window) {}

    void initialize();
    void handleEvent(const AppEvent& event);

protected:
    void exit(const AppEvent& event);
    void exitCommand(const AppEvent& event);
    void menuBar(const AppEvent& event);
    void settings(const AppEvent& event);
    void closeSettings(const AppEvent& event);
    void help(const AppEvent& event);
    void about(const AppEvent& event);
    void website(const AppEvent& event);
    void websiteGithub(const AppEvent& event);
    void licenseInformation(const AppEvent& event);
    void keyboard(const AppEvent& event);
    void mouse(const AppEvent& event);
    void show(const AppEvent& event);
    void hide(const AppEvent& event);
    void showLockInformation(const AppEvent& event);
    void lockInformationEnded(const AppEvent& event);

    void lock(const AppEvent& event);
    void lockCommand(const AppEvent& event);
    void lockNow(const AppEvent& event);
    void lockNowCommand(const AppEvent& event);
    void cancelLocking(const AppEvent& event);
    void unlock(const AppEvent& event);
    void unlockCommand(const AppEvent& event);

private:
    using EventHandlerPtr = void (AppEventHandler::*)(const AppEvent& event);
    void setEventHandler(int eventId, EventHandlerPtr handler);

    std::unordered_map<int, EventHandlerPtr> eventHandlers_;
};

} // namespace litelockr

#endif // APP_EVENT_HANDLER_H