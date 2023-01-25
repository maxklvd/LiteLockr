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

#include "Messages.h"

#include <boost/algorithm/string/replace.hpp>
#include <simpleini/SimpleIni.h>
#include "lang/Languages.h"
#include "sys/Process.h"

namespace litelockr {

Messages::Messages() {
    using Id = MessageId;

    addMessage(Id::OK, L"OK", L"OK");
    addMessage(Id::Cancel, L"Cancel", L"Cancel");
    addMessage(Id::Close, L"Close", L"Close");

    addMessage(Id::Settings, L"Settings", L"Settings");
    addMessage(Id::General, L"General", L"General");
    addMessage(Id::Advanced, L"Advanced", L"Advanced");
    addMessage(Id::Language, L"Language", L"Language");
    addMessage(Id::LockUnlockHotkey, L"LockUnlockHotkey", L"Lock / Unlock hotkey");
    addMessage(Id::DontLockApps, L"DontLockApps", L"Don't lock the following apps");
    addMessage(Id::DontLockCurrentApp, L"DontLockCurrentApp", L"Don't lock the current application");
    addMessage(Id::DontLockCurrentAppHint1, L"DontLockCurrentAppHint1",
               L"The active application will remain unlocked after you press");
    addMessage(Id::DontLockCurrentAppHint2, L"DontLockCurrentAppHint2", L"the hot key");
    addMessage(Id::PreventUnlockingInput, L"PreventUnlockingInput",
               L"Prevent your child from unlocking input devices");
    addMessage(Id::StartWithWindows, L"StartWithWindows", L"Start with Windows");
    addMessage(Id::LockOnStartup, L"LockOnStartup", L"Lock on startup");
    addMessage(Id::Reset, L"Reset", L"Reset");
    addMessage(Id::ShowNotifications, L"ShowNotifications", L"Show notifications");
    addMessage(Id::HideTrayNotificationIconWhenLocked, L"HideTrayNotificationIconWhenLocked",
               L"Hide tray notification icon when locked");
    addMessage(Id::UnlockOnCtrlAltDel, L"UnlockOnCtrlAltDel", L"Unlock on Ctrl+Alt+Del");
    addMessage(Id::Use4DigitPIN, L"Use4DigitPIN", L"Use a 4-digit PIN");
    addMessage(Id::SetUpPIN, L"SetUpPIN", L"Set up a PIN");
    addMessage(Id::ChangeYourPIN, L"ChangeYourPIN", L"Change your PIN");
    addMessage(Id::Add, L"Add", L"Add");
    addMessage(Id::Remove, L"Remove", L"Remove");
    addMessage(Id::CheckAll, L"CheckAll", L"Check all");
    addMessage(Id::DelayBeforeLocking, L"DelayBeforeLocking", L"Delay before locking");
    addMessage(Id::Seconds, L"Seconds", L"seconds");
    addMessage(Id::AutomaticallyLockWhenIdle, L"AutomaticallyLockWhenIdle", L"Automatically lock when idle");
    addMessage(Id::Minutes, L"Minutes", L"minutes");
    addMessage(Id::EventInterception, L"EventInterception", L"Event interception");

    addMessage(Id::Warnings, L"Warnings", L"Warnings");
    addMessage(Id::AutoHideTaskbarUnsupported, L"AutoHideTaskbarUnsupported",
               L"The Windows taskbar with auto-hide enabled is not supported.");

    addMessage(Id::ResetSettings, L"ResetSettings", L"Reset settings");
    addMessage(Id::ResetSettingsMessage, L"ResetSettingsMessage",
               L"Are you sure you want to restore the default settings?");

    addMessage(Id::PIN, L"PIN", L"PIN");
    addMessage(Id::NewPIN, L"NewPIN", L"New PIN");
    addMessage(Id::EnterPIN, L"EnterPIN", L"Enter PIN");
    addMessage(Id::ConfirmPIN, L"ConfirmPIN", L"Confirm PIN");

    addMessage(Id::About, L"About", L"About {0}");
    addMessage(Id::Version, L"Version", L"Version {0}");
    addMessage(Id::LicenseInformation, L"LicenseInformation", L"License Information");

    addMessage(Id::ConfirmExit, L"ConfirmExit", L"Confirm Exit");
    addMessage(Id::ConfirmExitMessage, L"ConfirmExitMessage", L"Are you sure you want to exit {0}?");
    addMessage(Id::UnableRegisterHotkey, L"UnableRegisterHotkey",
               L"Unable to register Lock/Unlock hotkey '{0}'. Please select a different hotkey.");
    addMessage(Id::HotkeyRequired, L"HotkeyRequired", L"Lock/Unlock hotkey required");
    addMessage(Id::NotAvailableWhenUsingRDC, L"NotAvailableWhenUsingRDC",
               L"Not available when using Remote Desktop Connection");
    addMessage(Id::UnableSaveSettings, L"UnableSaveSettings", L"Unable to save the settings.");
    addMessage(Id::SaveSettings, L"SaveSettings", L"Save Settings");
    addMessage(Id::RunningApplications, L"RunningApplications", L"Running applications");
    addMessage(Id::NoAppsFound, L"NoAppsFound", L"No Apps Found");
    addMessage(Id::InstallInterceptionDriver, L"InstallInterceptionDriver",
               L"You need to install the Interception driver to use this option.");

    addMessage(Id::Locked, L"Locked", L"Locked");
    addMessage(Id::Unlocked, L"Unlocked", L"Unlocked");
    addMessage(Id::LockNotAvailable, L"LockNotAvailable", L"Lock not available");

    addMessage(Id::KeyboardLocked, L"KeyboardLocked", L"The keyboard has locked");
    addMessage(Id::KeyboardUnlocked, L"KeyboardUnlocked", L"The keyboard has unlocked");
    addMessage(Id::MouseLocked, L"MouseLocked", L"The mouse has locked");
    addMessage(Id::MouseUnlocked, L"MouseUnlocked", L"The mouse has unlocked");
    addMessage(Id::KeyboardAndMouseLocked, L"KeyboardAndMouseLocked", L"The keyboard and mouse have locked");
    addMessage(Id::KeyboardAndMouseUnlocked, L"KeyboardAndMouseUnlocked", L"The keyboard and mouse have unlocked");

    addMessage(Id::MenuCancel, L"MenuCancel", L"Cancel");
    addMessage(Id::MenuUnlock, L"MenuUnlock", L"Unlock");
    addMessage(Id::MenuLock, L"MenuLock", L"Lock");
    addMessage(Id::MenuSettings, L"MenuSettings", L"Settings");
    addMessage(Id::MenuHelp, L"MenuHelp", L"Help");
    addMessage(Id::MenuOnlineDocumentation, L"MenuOnlineDocumentation", L"Online Documentation");
    addMessage(Id::MenuWebSite, L"MenuWebSite", L"LiteLockr Web Site");
    addMessage(Id::MenuAbout, L"MenuAbout", L"About");
    addMessage(Id::MenuExit, L"MenuExit", L"Exit");

    addMessage(Id::Properties, L"Properties", L"Properties");
    addMessage(Id::TaskbarButton, L"TaskbarButton", L"Taskbar button");
    addMessage(Id::FindByName, L"FindByName", L"Find by name");
    addMessage(Id::FindByAutomationId, L"FindByAutomationId", L"Find by AutomationId");
    addMessage(Id::DetectAutomatically, L"DetectAutomatically", L"Detect automatically");
    addMessage(Id::SearchForExecutableName, L"SearchForExecutableName",
               L"Search for the executable name in AutomationId");
    addMessage(Id::Check, L"Check", L"Check");
}

void Messages::reset() {
    messages_.clear();
    for (const auto& [id, text]: defaultMessages_) {
        messages_[id] = text;
    }
}

bool Messages::load(std::wstring_view langCode) {
    // reset to defaults
    reset();

    std::wstring path(Process::modulePath());
    path.append(Languages::LANGUAGE_DIR);
    path.append(L"\\"); // file separator
    path.append(langCode);
    path.append(Languages::INI_EXT);

    CSimpleIniW ini;
    ini.SetUnicode();
    SI_Error rc = ini.LoadFile(path.c_str());
    if (rc < 0) {
        return false;
    }
    assert(rc == SI_OK);

    for (const auto& [id, name]: messageIds_) {
        messages_[id] = ini.GetValue(Languages::SECTION_MESSAGES, name.c_str(), defaultMessages_[id].c_str());
    }

    return true;
}

std::wstring Messages::get(MessageId id, const std::vector<std::wstring>& values) {
    std::wstring msg(get(id));

    boost::replace_all(msg, L"{EOL}", L"\n"); // End-of-Line

    int idx = 0;
    for (auto& value: values) {
        std::wstring s{L"{"};
        s += std::to_wstring(idx);
        s += L"}"; // {0}, {1}...

        boost::replace_all(msg, s, value);
        idx++;
    }
    return msg;
}

} // namespace litelockr
