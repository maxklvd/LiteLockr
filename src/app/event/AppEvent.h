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

#ifndef APP_EVENT_H
#define APP_EVENT_H

#include <string>
#include <functional>

#include "sys/BaseEvent.h"

namespace litelockr {

class AppEvent: public BaseEvent {
public:
    enum EventId {
        NONE = 0,
        EXIT,
        EXIT_COMMAND,
        MENU,
        SETTINGS,
        CLOSE_SETTINGS,
        HELP,
        ABOUT,
        LICENSE_INFORMATION,
        LOCK,
        LOCK_NOW,
        CANCEL_LOCKING,
        UNLOCK,
        LOCK_COMMAND,
        LOCK_NOW_COMMAND,
        UNLOCK_COMMAND,
        KEYBOARD,
        MOUSE,
        HIDE,
        SHOW,
        HIDE_COMMAND,
        SHOW_COMMAND,
        SHOW_LOCK_INFORMATION,
        LOCK_INFORMATION_ENDED,
        WEBSITE,
        WEBSITE_GITHUB,
    };

    enum SourceId {
        SRC_NONE = 0,
        SRC_HOTKEY = 1,
    };

    unsigned int uid{++genId_};
    int source = SRC_NONE;
    int data = 0;

    explicit AppEvent(int id) : BaseEvent(id), source(SRC_NONE) {}

    AppEvent(int id, int src) : BaseEvent(id), source(src) {}

    [[nodiscard]] std::wstring toString() const;

private:
    static unsigned int genId_;
};

class AppEvents: public BaseEvents<AppEvent> {
public:
    static void send(AppEvent event) {
        instance().sendEvent(event);
    }

    static void send(int eventId) {
        instance().sendEvent(AppEvent(eventId));
    }

    static void remove(AppEvent event) {
        instance().removeEvent(event);
    }

    void registerHandler(std::function<void(const AppEvent&)>&& ptr) {
        processPtr_ = std::move(ptr);
    }

protected:
    void process(AppEvent event) override {
        if (processPtr_) {
            processPtr_(event);
        }
    }

private:
    std::function<void(const AppEvent&)> processPtr_;

//
// Singleton implementation
//
public:
    //
    // Returns the object instance
    //
    static AppEvents& instance() {
        static AppEvents instance_;
        return instance_;
    }

    AppEvents(const AppEvents&) = delete;
    AppEvents& operator=(const AppEvents&) = delete;

private:
    AppEvents() = default;
};

} // namespace litelockr

#endif // APP_EVENT_H
