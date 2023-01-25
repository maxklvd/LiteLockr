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

#ifndef SETTINGS_DATA_H
#define SETTINGS_DATA_H

#include <string>
#include <vector>

#include <windows.h>
#include "ini/ApplicationRecord.h"
#include "ini/Property.h"
#include "lang/Languages.h"
#include "lock/hook/EventInterception.h"

namespace litelockr {

class SettingsData {
public:
    //
    // INI section names
    //
    constexpr static auto SETTINGS = L"Settings";
    constexpr static auto LOCKED_APP = L"LockedApp";

    //
    // [Settings] section
    //
    BoolProperty lockKeyboard{{SETTINGS, L"LockKeyboard", true}};                       // default: ON
    BoolProperty lockMouse{{SETTINGS, L"LockMouse", true}};                             // default: ON
    StringProperty hotkey{{SETTINGS, L"Hotkey", L"Ctrl+Alt+B"}};                        // default: Ctrl+Alt+B
    StringListProperty allowedApp{{SETTINGS, L"AllowedApp", {}}};                       // default: empty
    BoolProperty dontLockCurrentApp{{SETTINGS, L"DontLockCurrentApp", true}};           // default: ON
    BoolProperty preventUnlockingInput{{SETTINGS, L"PreventUnlockingInput", true}};     // default: ON
    BoolProperty usePin{{SETTINGS, L"UsePIN", false}};                                  // default: OFF
    StringProperty pin{{SETTINGS, L"PIN", L""}};                                        // default: empty
    BoolProperty startWithWindows{{SETTINGS, L"StartWithWindows", false}};              // default: OFF
    BoolProperty lockOnStartup{{SETTINGS, L"LockOnStartup", false}};                    // default: OFF
    BoolProperty showNotifications{{SETTINGS, L"ShowNotifications", true}};             // default: ON
    BoolProperty hideTrayIconWhenLocked{{SETTINGS, L"HideTrayIconWhenLocked", false}};  // default: OFF
    BoolProperty unlockOnCtrlAltDel{{SETTINGS, L"UnlockOnCtrlAltDel", true}};           // default: ON
    BoolProperty unlockOnSessionChange{{SETTINGS, L"UnlockOnSessionChange", false}};    // default: OFF
    LongProperty delayBeforeLocking{{SETTINGS, L"DelayBeforeLocking", 5}};              // default: 5 seconds
    LongProperty lockWhenIdle{{SETTINGS, L"LockWhenIdle", 0}};                          // default: OFF
    LongProperty eventInterception{{SETTINGS, L"EventInterception",                     // default: 0
                                    EventInterception::GLOBAL_WINDOWS_HOOK}};
    BoolProperty allowMouseMovement{{SETTINGS, L"AllowMouseMovement", true}};           // default: ON
    BoolProperty playSounds{{SETTINGS, L"PlaySounds", true}};                           // default: ON
    LongProperty lightMode{{SETTINGS, L"LightMode", 0}};                                // default: auto
    StringProperty language{{SETTINGS, L"Language", Languages::AUTODETECT}};            // default: auto

    //
    // [LockedApp] section
    //
    BoolProperty minimizeByDoubleClick{{LOCKED_APP, L"MinimizeByDoubleClick", true,     // default: ON
                                        false}};
    BoolProperty minimizeByCtrlDoubleClick{{LOCKED_APP, L"MinimizeByCtrlDoubleClick",   // default: OFF
                                            false, false}};
    BoolProperty minimizeByCaptionButton{{LOCKED_APP, L"MinimizeByCaptionButton",       // default: ON
                                          true, false}};

    void initialize();
    void reset();
    bool load();
    void save(HWND hwndParent);
    void validate();

    static void setRegistryAutoRun(BOOL enabled);

    [[nodiscard]] bool pinEnabled() const {
        return usePin.value() && !pin.value().empty();
    }

    static constexpr auto HOTKEY_NONE = L"NONE";

    [[nodiscard]] bool hotkeyExists() const;

    constexpr static int MIN_DELAY_BEFORE_LOCKING = 0;  // 0 - off
    constexpr static int MAX_DELAY_BEFORE_LOCKING = 30; // 30 seconds

    constexpr static int MIN_LOCK_WHEN_IDLE = 0;        // 0 - off
    constexpr static int MAX_LOCK_WHEN_IDLE = 3 * 60;   // 3 hours

    struct LightMode {
        enum {
            AUTO = 0,
            ON = 1,
            OFF = 2,
        };

        constexpr static auto getString(int value) {
            switch (value) {
                case AUTO:
                    return L"auto";
                case ON:
                    return L"on";
                default:
                    return L"off";
            }
        }
    };

    //
    // Allowed apps
    //
private:
    ApplicationRecordVec allowedAppList_;
public:
    [[nodiscard]] bool allowedAppExists(const std::wstring& path) const;
    void updateAllowedAppList();
    int addAllowedApp(const std::wstring& path);
    int removeAllowedApp(const std::wstring& path);

    ApplicationRecordVec& getAllowedApps() { return allowedAppList_; }

    [[nodiscard]] const ApplicationRecordVec& getAllowedApps() const { return allowedAppList_; }

    void setupCurrentApp(HWND hwndActive);

    //
    // All available languages
    //
private:
    Languages languages_;
public:
    [[nodiscard]] const LanguageMap& languages() const;

    //
    // Prints all values to .log file
    //
    void printAllValues() const;

private:
    void printPropertyValues(std::wstringstream& buf, const PropertyVec& properties) const;

//
// Singleton implementation
//
public:
    static SettingsData& instance() {
        static SettingsData _instance;
        return _instance;
    }

    SettingsData(const SettingsData&) = delete;
    SettingsData& operator=(const SettingsData&) = delete;

private:
    PropertyVec allProperties_;
    std::wstring iniPath_;

    SettingsData();
    ~SettingsData() = default;
};

} // namespace litelockr

#endif // SETTINGS_DATA_H
