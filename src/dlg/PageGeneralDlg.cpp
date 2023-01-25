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

#include "PageGeneralDlg.h"

#include <algorithm>

#include <Uxtheme.h>
#include "app/event/AppEvent.h"
#include "app/HotkeyHandler.h"
#include "app/Version.h"
#include "gui/WindowUtils.h"
#include "lock/MousePositionValidator.h"
#include "lock/taskbar/TaskbarButtonDetector.h"
#include "sys/Executable.h"
#include "sys/Process.h"
#include "sys/StringUtils.h"

namespace litelockr {

void PageGeneralDlg::localizeDialog() {
    setLocalizedText(IDC_LOCK_UNLOCK_HOTKEY, MessageId::LockUnlockHotkey);
    setLocalizedText(IDC_DONT_LOCK_APPS_LABEL, MessageId::DontLockApps);
    setLocalizedText(IDC_DONT_LOCK_CURRENT_APP, MessageId::DontLockCurrentApp);
    setLocalizedText(IDC_DONT_LOCK_CURRENT_APP_HINT_1, MessageId::DontLockCurrentAppHint1);
    setLocalizedText(IDC_DONT_LOCK_CURRENT_APP_HINT_2, MessageId::DontLockCurrentAppHint2);
    setLocalizedText(IDC_PREVENT_UNLOCKING_INPUT, MessageId::PreventUnlockingInput);
    setLocalizedText(IDC_START_WITH_WINDOWS, MessageId::StartWithWindows);
    setLocalizedText(IDC_LOCK_ON_STARTUP, MessageId::LockOnStartup);
    setLocalizedText(IDC_USE_PIN, MessageId::Use4DigitPIN);
    setLocalizedText(IDC_SET_PIN, settings_.pin.value().empty() ? MessageId::SetUpPIN : MessageId::ChangeYourPIN);
    setLocalizedText(IDC_ADD, MessageId::Add);
    setLocalizedText(IDC_PROPERTIES, MessageId::Properties);
    setLocalizedText(IDC_REMOVE, MessageId::Remove);
    setLocalizedText(IDC_CHECK_ALL, MessageId::CheckAll);
}

bool PageGeneralDlg::save() {
    controlAccessor_.readValue(IDC_DONT_LOCK_CURRENT_APP, settings_.dontLockCurrentApp);
    controlAccessor_.readValue(IDC_PREVENT_UNLOCKING_INPUT, settings_.preventUnlockingInput);
    controlAccessor_.readValue(IDC_USE_PIN, settings_.usePin);
    controlAccessor_.readValue(IDC_START_WITH_WINDOWS, settings_.startWithWindows);
    controlAccessor_.readValue(IDC_LOCK_ON_STARTUP, settings_.lockOnStartup);

    auto hotkey = static_cast<WORD>(SendDlgItemMessage(hWnd, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0L));
    settings_.hotkey.setValue(HotkeyHandler::toString(hotkey));

    std::wstring errMessage;
    if (!HotkeyHandler::instance().setHotkey(hotkey, errMessage)) {
        MessageBox(Process::mainWindow(), errMessage.c_str(), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
        return false;
    }
    return true;
}

void PageGeneralDlg::setHotkeyValue() {
    HWND hk = dlgItem(IDC_HOTKEY);
    WORD hotkey = HotkeyHandler::fromString(settings_.hotkey.value());
    SendMessage(hk, HKM_SETHOTKEY, hotkey, 0L);
    SendMessage(hk, HKM_SETRULES, HKCOMB_NONE | HKCOMB_S, MAKELPARAM(HOTKEYF_CONTROL | HOTKEYF_ALT, 0));
}

BOOL PageGeneralDlg::onInitDialog() {
    localizeDialog();
    initialValues_.save();
    EnableThemeDialogTexture(hWnd, ETDT_ENABLETAB);

    auto hIcon = reinterpret_cast<HICON>(LoadImageW(Process::moduleInstance(),
                                                    MAKEINTRESOURCEW(IDI_APP_ICON),
                                                    IMAGE_ICON,
                                                    GetSystemMetrics(SM_CXSMICON),
                                                    GetSystemMetrics(SM_CYSMICON),
                                                    0));
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));

    update();
    return FALSE;
}

void PageGeneralDlg::update() {
    refreshAllowedAppList();

    controlAccessor_.writeValue(IDC_DONT_LOCK_CURRENT_APP, settings_.dontLockCurrentApp);
    controlAccessor_.writeValue(IDC_PREVENT_UNLOCKING_INPUT, settings_.preventUnlockingInput);
    controlAccessor_.writeValue(IDC_USE_PIN, settings_.usePin);
    controlAccessor_.writeValue(IDC_START_WITH_WINDOWS, settings_.startWithWindows);
    controlAccessor_.writeValue(IDC_LOCK_ON_STARTUP, settings_.lockOnStartup);
    controlAccessor_.writeValue(IDC_SHOW_NOTIFICATIONS, settings_.showNotifications);

    setHotkeyValue();

    enableControls();

    EnableWindow(dlgItem(IDC_DONT_LOCK_CURRENT_APP_HINT_1), FALSE);
    EnableWindow(dlgItem(IDC_DONT_LOCK_CURRENT_APP_HINT_2), FALSE);
}

void PageGeneralDlg::enableControls() {
    bool preventUnlockingInput = false;
    bool usePin = false;
    controlAccessor_.readValue(IDC_PREVENT_UNLOCKING_INPUT, preventUnlockingInput);
    controlAccessor_.readValue(IDC_USE_PIN, usePin);

    EnableWindow(dlgItem(IDC_USE_PIN), preventUnlockingInput);
    EnableWindow(dlgItem(IDC_SET_PIN), preventUnlockingInput && usePin);

    EnableWindow(dlgItem(IDC_CHECK_ALL), !allowedAppsIdx_.empty());
    EnableWindow(dlgItem(IDC_PROPERTIES), !allowedAppsIdx_.empty());
    EnableWindow(dlgItem(IDC_REMOVE), !allowedAppsIdx_.empty());
}

void PageGeneralDlg::refreshAllowedAppList(int selectItem) {
    HWND hWndList = dlgItem(IDC_DONT_LOCK_APPS);
    SendMessage(hWndList, LB_RESETCONTENT, 0, 0);

    int idx = 0;
    allowedAppsIdx_.clear();
    for (const auto& app: settings_.getAllowedApps()) {
        if (app.persistent) {
            SendMessage(hWndList, LB_INSERTSTRING, static_cast<WPARAM>(-1),
                        reinterpret_cast<LPARAM>(app.path().c_str()));
            allowedAppsIdx_.push_back(idx);
        }
        idx++;
    }
    if (selectItem >= 0) {
        SendMessage(hWndList, LB_SETCURSEL, selectItem, 0L);
    }
    enableControls();
}


//
void PageGeneralDlg::onAdd() {
    if (isCheckRunning_) {
        return;
    }

    wchar_t file[MAX_PATH] = L"";

    OPENFILENAME ofn = {sizeof(ofn)};
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = L"(*.exe) Windows Executable\0*.EXE\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        int idx = settings_.addAllowedApp(ofn.lpstrFile);
        if (idx >= 0) {
            detectAutomationId(idx);
            refreshAllowedAppList(idx);
        }
    }
}

void PageGeneralDlg::onAddFromList(int dropDownItemId) {
    if (isCheckRunning_) {
        return;
    }

    auto it = dropDownItems_.find(dropDownItemId);
    if (it != dropDownItems_.end()) {
        auto& path = it->second.first;
        auto& title = it->second.second;

        int idx = settings_.addAllowedApp(path);
        if (idx >= 0) {
            addTitleIntoAppNames(idx, title);
            detectAutomationId(idx);
            refreshAllowedAppList(idx);
        }
    }
}

void PageGeneralDlg::addTitleIntoAppNames(int idx, const std::wstring& title) {
    if (!title.empty()) {
        auto upper = StringUtils::toUpperCase(title);
        auto& app = settings_.getAllowedApps()[idx];
        auto& names = app.detectedAppNames_;

        for (const auto& name: names) {
            if (upper == StringUtils::toUpperCase(name)) {
                return; // the value already exists
            }
        }
        names.push_back(title);
    }
}

void PageGeneralDlg::detectAutomationId(int appIdx) {
    auto& app = settings_.getAllowedApps()[appIdx];

    auto aid = TaskbarButtonDetector::detectAutomationId(app);
    if (!aid.empty()) {
        app.buttonAutomationId_ = aid;
        app.findByAutomationId_ = true;
    }
}

LRESULT PageGeneralDlg::onCreateDropDownMenu(LPNMHDR lParam) {
    auto pDropDown = reinterpret_cast<LPNMBCDROPDOWN>(lParam);
    if (pDropDown->hdr.idFrom == IDC_ADD) {

        // Get screen coordinates of the button.
        POINT pt{pDropDown->rcButton.left, pDropDown->rcButton.bottom};
        ClientToScreen(pDropDown->hdr.hwndFrom, &pt);

        // Create a menu and add items.
        HMENU hSplitMenu = CreatePopupMenu();
        AppendMenu(hSplitMenu, MF_STRING | MF_DISABLED | MF_GRAYED, 6000,
                   Messages::get(MessageId::RunningApplications).c_str());
        AppendMenu(hSplitMenu, MF_SEPARATOR, 0, nullptr);

        std::unordered_set<std::wstring> alreadyAddedSet;
        for (const auto& app: settings_.getAllowedApps()) {
            alreadyAddedSet.insert(StringUtils::toUpperCase(app.path()));
        }

        std::vector<HWND> windowList;
        WindowUtils::findTopLevelWindows([this, &windowList](HWND hwnd) {
            hwnd = appConfigSet_.findRootWindow(hwnd);
            windowList.push_back(hwnd);
        });
        std::map<std::wstring, std::pair<std::wstring, std::wstring>> execMap; // executable -> (path, window title)

        for (const auto& hwnd: windowList) {
            DWORD processId = 0;

            GetWindowThreadProcessId(hwnd, &processId);
            if (processId) {
                auto path = Executable::getExecutablePathByPID(processId);
                if (!path.empty()) {
                    auto executable = StringUtils::toUpperCase(Executable::getFileName(path));
                    execMap[executable] = std::make_pair(path, WindowUtils::getWindowText(hwnd));
                }
            }
        }

        int idx = DROP_DOWN_ITEM_MIN;
        dropDownItems_.clear();

        for (const auto& [executable, pair]: execMap) {
            dropDownItems_[idx] = pair;

            if (appConfigSet_.shouldSkipApp(executable)) {
                //
                // skip the process
                //
                continue;
            }

            const auto& path = pair.first;
            const auto& title = pair.second;

            if (alreadyAddedSet.find(StringUtils::toUpperCase(path)) == alreadyAddedSet.end()) {
                auto text = Executable::getFileName(path); // name.exe
                if (!title.empty()) {
                    text.append(L" \u2013 ");        // -
                    text.append(title);                 // window title
                }
                AppendMenu(hSplitMenu, MF_STRING, idx, StringUtils::truncate(text, 80).c_str());
                idx++;
            }
        }

        if (idx == DROP_DOWN_ITEM_MIN) {
            AppendMenu(hSplitMenu, MF_STRING | MF_DISABLED | MF_GRAYED, 6001,
                       Messages::get(MessageId::NoAppsFound).c_str());
        }

        // Display the menu.
        TrackPopupMenu(hSplitMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hWnd, nullptr);
        return TRUE;
    }
    return FALSE;
}

void PageGeneralDlg::onProperties() {
    if (isCheckRunning_) {
        return;
    }

    HWND hWndList = dlgItem(IDC_DONT_LOCK_APPS);
    auto index = SendMessage(hWndList, LB_GETCURSEL, 0, 0L);
    if (index != LB_ERR) {
        assert(index >= 0 && index < static_cast<int>(allowedAppsIdx_.size()));
        int idx = allowedAppsIdx_[index];
        assert(idx < static_cast<int>(settings_.getAllowedApps().size()));

        auto& app = settings_.getAllowedApps()[idx];
        propertiesDlg_.show(hWnd, app);
    }
}

void PageGeneralDlg::onRemove() {
    if (isCheckRunning_) {
        return;
    }

    if (propertiesDlg_.isVisible()) {
        return; // don't allow to remove an item while Properties dialog is open
    }

    HWND hWndList = dlgItem(IDC_DONT_LOCK_APPS);
    auto index = SendMessage(hWndList, LB_GETCURSEL, 0, 0L);
    if (index != LB_ERR) {
        wchar_t app[MAX_PATH] = L"";
        SendMessage(hWndList, LB_GETTEXT, index, reinterpret_cast<LPARAM>(app));

        int idx = settings_.removeAllowedApp(app);
        if (idx >= 0) {
            refreshAllowedAppList(idx);
        }
    }
}

void PageGeneralDlg::onCheckAll(WORD /*wNotifyCode*/, HWND hWndCtrl) {
    EnableWindow(hWndCtrl, FALSE);
    isCheckRunning_ = true;

    AppEvent e(AppEvent::SHOW_LOCK_INFORMATION);
    e.data = MousePositionValidator::CHECK_ALL;
    AppEvents::send(e);
}

void PageGeneralDlg::onLockPreviewEnded() {
    EnableWindow(dlgItem(IDC_CHECK_ALL), TRUE);
    isCheckRunning_ = false;

    if (propertiesDlg_.isVisible()) {
        propertiesDlg_.onLockPreviewEnded();
    } else {
        SetForegroundWindow(GetParent(hWnd));
    }
}

void PageGeneralDlg::onSetPIN() {
    pinDlg_.show(hWnd);
    setLocalizedText(IDC_SET_PIN, settings_.pin.value().empty() ? MessageId::SetUpPIN : MessageId::ChangeYourPIN);
}

BOOL PageGeneralDlg::windowProc(HWND /*hWnd*/, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult) {
    if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) >= DROP_DOWN_ITEM_MIN && LOWORD(wParam) <= DROP_DOWN_ITEM_MAX) {
            onAddFromList(LOWORD(wParam));
            return TRUE;
        }
        if (HIWORD(wParam) == LBN_DBLCLK) {
            onProperties();
            return TRUE;
        }
    }
    return Window::windowProc(hWnd, uMsg, wParam, lParam, lResult);
}

PageGeneralDlg::PageGeneralDlg() : Dialog(IDD_PAGE_GENERAL) {
    onCommand(IDC_ADD, &PageGeneralDlg::onAdd);
    onCommand(IDC_PROPERTIES, &PageGeneralDlg::onProperties);
    onCommand(IDC_REMOVE, &PageGeneralDlg::onRemove);
    onCommand(IDC_CHECK_ALL, &PageGeneralDlg::onCheckAll);
    onCommand(IDC_SET_PIN, &PageGeneralDlg::onSetPIN);
    onCommand(IDC_PREVENT_UNLOCKING_INPUT, [this]() { enableControls(); });
    onCommand(IDC_USE_PIN, [this]() { enableControls(); });
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:26454)
#endif
    onNotify(BCN_DROPDOWN, &PageGeneralDlg::onCreateDropDownMenu);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

void PageGeneralDlg::InitialValues::save() {
    const auto& s = SettingsData::instance();

    hotkey_ = s.hotkey.value();
    allowedApp_ = s.allowedApp.value();
    dontLockCurrentApp_ = s.dontLockCurrentApp.value();
    preventUnlockingInput_ = s.preventUnlockingInput.value();
    usePin_ = s.usePin.value();
    pin_ = s.pin.value();
    startWithWindows_ = s.startWithWindows.value();
    lockOnStartup_ = s.lockOnStartup.value();
}

void PageGeneralDlg::InitialValues::revert() const {
    auto& s = SettingsData::instance();

    if (s.hotkey.value() != hotkey_) {
        s.hotkey.setValue(hotkey_);
        [[maybe_unused]] std::wstring _;
        HotkeyHandler::instance().setHotkey(HotkeyHandler::fromString(hotkey_), _); // skips an error message
    }

    s.allowedApp.setValue(allowedApp_);
    s.updateAllowedAppList();

    s.dontLockCurrentApp.setValue(dontLockCurrentApp_);
    s.preventUnlockingInput.setValue(preventUnlockingInput_);
    s.usePin.setValue(usePin_);
    s.pin.setValue(pin_);
    s.startWithWindows.setValue(startWithWindows_);
    s.lockOnStartup.setValue(lockOnStartup_);
}

} // namespace litelockr
