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

#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <vector>
#include <unordered_map>

namespace litelockr {

enum class MessageId {
    Null,

    OK,
    Cancel,
    Close,

    Settings,
    General,
    Advanced,
    Language,
    LockUnlockHotkey,
    DontLockApps,
    DontLockCurrentApp,
    DontLockCurrentAppHint1,
    DontLockCurrentAppHint2,
    PreventUnlockingInput,
    StartWithWindows,
    LockOnStartup,
    Reset,
    ShowNotifications,
    HideTrayNotificationIconWhenLocked,
    UnlockOnCtrlAltDel,
    Use4DigitPIN,
    SetUpPIN,
    ChangeYourPIN,
    Add,
    Remove,
    CheckAll,
    DelayBeforeLocking,
    Seconds,
    AutomaticallyLockWhenIdle,
    Minutes,
    EventInterception,

    Warnings,
    AutoHideTaskbarUnsupported,

    ResetSettings,
    ResetSettingsMessage,

    PIN,
    NewPIN,
    EnterPIN,
    ConfirmPIN,

    About,
    Version,
    LicenseInformation,

    ConfirmExit,
    ConfirmExitMessage,
    UnableRegisterHotkey,
    HotkeyRequired,
    NotAvailableWhenUsingRDC,
    UnableSaveSettings,
    SaveSettings,
    RunningApplications,
    NoAppsFound,
    InstallInterceptionDriver,

    Locked,
    Unlocked,
    LockNotAvailable,

    KeyboardLocked,
    KeyboardUnlocked,
    MouseLocked,
    MouseUnlocked,
    KeyboardAndMouseLocked,
    KeyboardAndMouseUnlocked,

    MenuCancel,
    MenuUnlock,
    MenuLock,
    MenuSettings,
    MenuHelp,
    MenuOnlineDocumentation,
    MenuWebSite,
    MenuAbout,
    MenuExit,

    Properties,
    TaskbarButton,
    FindByName,
    FindByAutomationId,
    DetectAutomatically,
    SearchForExecutableName,
    Check,
};

class Messages {
public:
    static std::wstring get(MessageId id) {
        return instance().messages_[id];
    }

    template<typename T = std::wstring, typename ...Args>
    static std::wstring get(MessageId id, Args... args) {
        std::vector<T> values = {
                T(std::forward<Args>(args))...
        };
        return get(id, values);
    }

    static std::wstring get(MessageId id, const std::vector<std::wstring>& values);
    void reset();
    bool load(std::wstring_view langCode);

private:
    std::unordered_map<MessageId, std::wstring> defaultMessages_;
    std::unordered_map<MessageId, std::wstring> messages_;
    std::unordered_map<MessageId, std::wstring> messageIds_;

    void addMessage(MessageId msgId, const std::wstring& strId, const std::wstring& defaultMessage) {
        messageIds_[msgId] = strId;
        defaultMessages_[msgId] = defaultMessage;
    }

//
// Singleton implementation
//
public:
    //
    // Returns the object instance
    //
    static Messages& instance() {
        static Messages _instance;
        return _instance;
    }

    Messages(const Messages&) = delete;
    Messages& operator=(const Messages&) = delete;

private:
    Messages();
};

} // namespace litelockr

#endif // MESSAGES_H
