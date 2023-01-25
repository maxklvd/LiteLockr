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

#include "SettingsData.h"

#include <algorithm>
#include <sstream>

#include "app/StringConverter.h"
#include "app/Version.h"
#include "ini/SettingsParser.h"
#include "lang/Messages.h"
#include "lock/hook/InterceptionHook.h"
#include "log/Logger.h"
#include "sys/BinaryResource.h"
#include "sys/Executable.h"
#include "sys/Process.h"
#include "sys/StringUtils.h"

#include "res/Resources.h"

namespace litelockr {

SettingsData::SettingsData() : allProperties_{
        lockKeyboard,
        lockMouse,
        hotkey,
        allowedApp,
        dontLockCurrentApp,
        preventUnlockingInput,
        usePin,
        pin,
        startWithWindows,
        lockOnStartup,
        showNotifications,
        hideTrayIconWhenLocked,
        unlockOnCtrlAltDel,
        unlockOnSessionChange,
        delayBeforeLocking,
        lockWhenIdle,
        eventInterception,
        allowMouseMovement,
        playSounds,
        lightMode,
        language,
        minimizeByDoubleClick,
        minimizeByCtrlDoubleClick,
        minimizeByCaptionButton,
} {
    iniPath_ = Process::modulePath();
    iniPath_ += APP_NAME;
    iniPath_ += L".ini";

    //
    // PIN
    //
    pin.setSerializer([](const auto& val, std::wstringstream& out) {
        out << StringConverter::encodePin(val);
    });
    pin.setDeserializer([](auto& val, std::wstringstream& in) {
        val = StringConverter::decodePin(in.str());
    });
}

bool isPinValid(std::wstring_view pin) {
    //
    // 4-digits PIN
    //
    return pin.size() == 4 &&
           std::ranges::all_of(pin, [](auto c) { return c >= L'0' && c <= L'9'; });
}

void SettingsData::initialize() {
    languages_.initialize(); // loads all available languages from lang/*.ini

    SettingsParser ini(iniPath_);
    if (!ini.fileExists()) {
        reset();
        return;
    }

    load();
}

void SettingsData::reset() {
    FILE *f = _wfopen(iniPath_.c_str(), L"wb");
    assert(f);
    if (f) {
        fputs(reinterpret_cast<const char *>(Bin::data(IDB_LITELOCKR_INI)), f);
        fclose(f);
    }

    load();
}

bool SettingsData::load() {
    SettingsParser ini(iniPath_);
    if (!ini.loadFile()) {
        return false;
    }

    ini.loadProperties(allProperties_);
    updateAllowedAppList();

    validate();

    //
    // Lang & messages
    //
    Messages::instance().load(language.value());

    return true;
}

void SettingsData::save(HWND hwndParent) {
    SettingsParser ini(iniPath_);
    if (ini.loadFile()) {
        //
        // Updates Settings.AllowedApp
        //
        std::vector<std::wstring> apps;
        if (!allowedAppList_.empty()) {
            for (auto&& app: allowedAppList_) {
                if (app.persistent) {
                    apps.push_back(app.toJSON());
                }
            }
        }
        allowedApp.setValue(apps);

        ini.saveProperties(allProperties_);

        if (!ini.saveFile()) {
            if (hwndParent) {
                MessageBox(hwndParent, Messages::get(MessageId::UnableSaveSettings).c_str(),
                           Messages::get(MessageId::SaveSettings).c_str(), MB_OK | MB_ICONEXCLAMATION);
            }
        }

        setRegistryAutoRun(startWithWindows.value());
    }
}

void SettingsData::validate() {
    //
    // DelayBeforeLocking
    //
    int delay = delayBeforeLocking.value();
    delayBeforeLocking.setValue(std::clamp(delay, MIN_DELAY_BEFORE_LOCKING, MAX_DELAY_BEFORE_LOCKING));

    //
    // LockKeyboard & LockMouse
    //
    if (lockKeyboard.value() == 0 && lockMouse.value() == 0) {
        lockKeyboard.reset();
        lockMouse.reset();
    }

    //
    // LockWhenIdle
    //
    int time = lockWhenIdle.value();
    lockWhenIdle.setValue(std::clamp(time, MIN_LOCK_WHEN_IDLE, MAX_LOCK_WHEN_IDLE));

    //
    // PIN
    //
    if (!isPinValid(pin.value())) {
        pin.reset();
    }

    //
    // Hotkey
    //
    if (hotkey.value().empty()) {
        hotkey.setValue(HOTKEY_NONE);
    }

    if (!hotkeyExists()) {
        //
        // Reset the following options if the hotkey is not set
        //
        hideTrayIconWhenLocked.setValue(FALSE);
    }

    //
    // Autodetect UI language
    //
    if (language.value() == Languages::AUTODETECT) {
        auto langCode = Languages::getLocaleLanguage(); // e.g. 'en-US'
        LOG_DEBUG(L"Locale language: %s", langCode.c_str());
        if (languages_.languages().contains(langCode)) {
            language.setValue(langCode); // change the language
        } else {
            auto pos = langCode.find(L'-');
            if (pos != std::string::npos) {
                auto langOnly = langCode.substr(0, pos); // e.g. 'en'
                if (languages_.languages().contains(langOnly)) {
                    language.setValue(langOnly); // change the language
                }
            }
        }

        if (language.value() == Languages::AUTODETECT) { // the value was not changed
            language.setValue(Languages::DEFAULT_LANGUAGE);
            LOG_DEBUG(L"Default language will be used for UI");
        } else {
            LOG_DEBUG(L"'%s' language will be used for UI", language.value().c_str());
        }
    }

    //
    // Event Interception
    //
    switch (eventInterception.value()) {
        case EventInterception::NULL_HOOK:
        case EventInterception::GLOBAL_WINDOWS_HOOK:
            break;
        case EventInterception::INTERCEPTION_DRIVER:
            if (!InterceptionHook::isDriverInstalled()) {
                // Interception driver is not installed on this system
                eventInterception.reset();
            }
            break;
        default:
            // invalid value
            eventInterception.reset();
            break;
    }
}

void SettingsData::setRegistryAutoRun(BOOL enabled) {
    HKEY hKey = nullptr;
    LONG err;

    err = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                       0, KEY_ALL_ACCESS, &hKey);
    assert(hKey);
    if (err != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return;
    }

    if (enabled) {
        wchar_t pathToExe[MAX_PATH] = {0};
        GetModuleFileName(nullptr, pathToExe, MAX_PATH);

        std::wstring buf = L"\"";
        buf += pathToExe;
        buf += L"\" /autorun";

        DWORD sizeInBytes = buf.size() * sizeof(wchar_t);

        err = RegSetValueEx(hKey, APP_NAME, 0, REG_SZ, reinterpret_cast<CONST BYTE *>(buf.data()), sizeInBytes);
        assert(err == ERROR_SUCCESS);
    } else {
        RegDeleteValue(hKey, APP_NAME);
    }

    RegCloseKey(hKey);
}

bool SettingsData::allowedAppExists(const std::wstring& path) const {
    const auto& v = allowedAppList_;
    return (std::find_if(v.begin(), v.end(), [&path](const ApplicationRecord& app) {
        return app.path() == path;
    }) != v.end());
}

void SettingsData::updateAllowedAppList() {
    allowedAppList_.clear();
    for (auto&& json: allowedApp.value()) {
        ApplicationRecord application;
        application.fromJSON(json);
        if (!allowedAppExists(application.path())) {
            allowedAppList_.push_back(application);
        }
    }
}

int SettingsData::addAllowedApp(const std::wstring& path) {
    // adds if the path not exists
    if (!allowedAppExists(path)) {
        ApplicationRecord application;
        application.fromPath(path);
        allowedAppList_.push_back(application);
        return static_cast<int>(allowedAppList_.size()) - 1;
    }
    return -1;
}

int SettingsData::removeAllowedApp(const std::wstring& path) {
    int idx = 0;
    int removed = -1;

    auto& v = allowedAppList_;
    auto it = std::remove_if(v.begin(), v.end(), [&path, &idx, &removed](const ApplicationRecord& app) {
        bool toRemove = (app.path() == path);

        if (toRemove) {
            removed = idx;
        }
        idx++;
        return toRemove;
    });
    v.erase(it, v.end());

    return removed;
}

void SettingsData::setupCurrentApp(HWND hwndActive) {
    //
    // removes all non-persistent items
    //
    auto& v = allowedAppList_;
    auto it = std::remove_if(v.begin(), v.end(), [](const ApplicationRecord& app) {
        return !app.persistent;
    });
    v.erase(it, v.end());

    //
    // adds an active application into the allowed apps list
    //
    if (hwndActive && dontLockCurrentApp.value()) {
        DWORD processId;

        if (GetWindowThreadProcessId(hwndActive, &processId)) {
            auto path = Executable::getExecutablePathByPID(processId);
            if (!path.empty()) {
                int idx = addAllowedApp(path);
                if (idx >= 0) {
                    allowedAppList_[idx].persistent = false;
                }
            }
        }
    }
}

const LanguageMap& SettingsData::languages() const {
    return languages_.languages();
}

void SettingsData::printAllValues() const {
    std::wstringstream buf;
    printPropertyValues(buf, allProperties_);
    LOG_DEBUG(L"Loaded settings:\n%s", buf.str().c_str());
}

void SettingsData::printPropertyValues(std::wstringstream& buf, const PropertyVec& properties) const {
    std::wstring padding(4, L' ');

    for (auto&& property: properties) {
        buf << padding << property.get().section() << L"." << property.get().key() << L": ";

        switch (property.get().type()) {
            case PropertyType::Bool: {
                auto& prop = static_cast<BoolProperty&>(property.get()); // downcast
                buf << (prop.value() ? L"on" : L"off");
                break;
            }
            case PropertyType::Long: {
                auto& prop = static_cast<LongProperty&>(property.get()); // downcast
                if (prop.key() == lightMode.key()) {
                    buf << LightMode::getString(prop.value());
                } else if (prop.key() == eventInterception.key()) {
                    buf << EventInterception::getString(prop.value());
                } else {
                    buf << prop.value();
                }
                break;
            }
            case PropertyType::String: {
                auto& prop = static_cast<StringProperty&>(property.get()); // downcast
                if (prop.key() == pin.key()) {
                    buf << std::wstring(pin.value().size(), L'*');
                } else {
                    buf << prop.value();
                }
                break;
            }
            case PropertyType::StringList: {
                auto& prop = static_cast<StringListProperty&>(property.get()); // downcast
                if (prop.value().empty()) {
                    buf << L"[]";
                } else {
                    buf << L"[" << std::endl;
                    for (auto&& val: prop.value()) {
                        buf << padding << padding << val << std::endl;
                    }
                    buf << padding << L"]";
                }
                break;
            }
            default:
                assert(false);
                break;
        }
        buf << std::endl;
    }
}


bool SettingsData::hotkeyExists() const {
    const auto& hk = hotkey.value();
    return !hk.empty() && StringUtils::toUpperCase(hk) != HOTKEY_NONE;
}

} // namespace litelockr
