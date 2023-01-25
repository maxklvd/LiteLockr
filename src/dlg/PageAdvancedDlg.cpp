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

#include "PageAdvancedDlg.h"

#include <Uxtheme.h>
#include "app/Version.h"
#include "lock/hook/InterceptionHook.h"
#include "sys/Process.h"

namespace litelockr {

PageAdvancedDlg::PageAdvancedDlg() : Dialog(IDD_PAGE_ADVANCED) {
    onCommand(IDC_DELAY_BEFORE_LOCKING_CHECK, [this]() { enableControls(); });
    onCommand(IDC_LOCK_WHEN_IDLE_CHECK, [this]() { enableControls(); });
    onCommand(IDC_EVENT_INTERCEPTION, &PageAdvancedDlg::onEventInterception);
}

void PageAdvancedDlg::localizeDialog() {
    setLocalizedText(IDC_DELAY_BEFORE_LOCKING_CHECK, MessageId::DelayBeforeLocking);
    setLocalizedText(IDC_DELAY_BEFORE_LOCKING_UNIT, MessageId::Seconds);
    setLocalizedText(IDC_SHOW_NOTIFICATIONS, MessageId::ShowNotifications);
    setLocalizedText(IDC_HIDE_TRAY_ICON_WHEN_LOCKED, MessageId::HideTrayNotificationIconWhenLocked);
    setLocalizedText(IDC_UNLOCK_ON_CTRL_ALT_DEL, MessageId::UnlockOnCtrlAltDel);
    setLocalizedText(IDC_LOCK_WHEN_IDLE_CHECK, MessageId::AutomaticallyLockWhenIdle);
    setLocalizedText(IDC_LOCK_WHEN_IDLE_UNIT, MessageId::Minutes);
    setLocalizedText(IDC_EVENT_INTERCEPTION_LABEL, MessageId::EventInterception);
}

BOOL PageAdvancedDlg::onInitDialog() {
    localizeDialog();
    initialValues_.save();
    EnableThemeDialogTexture(hWnd, ETDT_ENABLETAB);

    //
    // Sets the buddy for Delay before locking
    //
    auto hwndDBL = dlgItem(IDC_DELAY_BEFORE_LOCKING);
    auto hwndDBLSpin = dlgItem(IDC_DELAY_BEFORE_LOCKING_SPIN);
    SendMessage(hwndDBLSpin, UDM_SETBUDDY, (WPARAM) hwndDBL, 0L);
    SendMessage(hwndDBLSpin, UDM_SETRANGE, 0,
                MAKELONG(SettingsData::MAX_DELAY_BEFORE_LOCKING, SettingsData::MIN_DELAY_BEFORE_LOCKING + 1));

    //
    // Sets the buddy for Lock when idle
    //
    auto hwndLWI = dlgItem(IDC_LOCK_WHEN_IDLE);
    auto hwndLWISpin = dlgItem(IDC_LOCK_WHEN_IDLE_SPIN);
    SendMessage(hwndLWISpin, UDM_SETBUDDY, (WPARAM) hwndLWI, 0L);
    SendMessage(hwndLWISpin, UDM_SETRANGE, 0,
                MAKELONG(SettingsData::MAX_LOCK_WHEN_IDLE, SettingsData::MIN_LOCK_WHEN_IDLE + 1));

    hideTrayIconToolip_.create(hWnd);
    hideTrayIconToolip_.addRectangle(IDC_HIDE_TRAY_ICON_WHEN_LOCKED, Messages::get(MessageId::HotkeyRequired));
    hideTrayIconToolip_.activate(false);

    lockWhenIdleToolip_.create(hWnd);
    lockWhenIdleToolip_.addRectangle(IDC_LOCK_WHEN_IDLE_CHECK, Messages::get(MessageId::NotAvailableWhenUsingRDC));
    lockWhenIdleToolip_.activate(false);

    HWND hWndEventInterception = dlgItem(IDC_EVENT_INTERCEPTION);
    SendMessage(hWndEventInterception, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(EventInterception::GlobalWindowsHook));
    SendMessage(hWndEventInterception, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(EventInterception::InterceptionDriver));

    update();

    return TRUE;
}

void PageAdvancedDlg::update() {
    controlAccessor_.writeValue(IDC_SHOW_NOTIFICATIONS, settings_.showNotifications);
    controlAccessor_.writeValue(IDC_HIDE_TRAY_ICON_WHEN_LOCKED, settings_.hideTrayIconWhenLocked);
    controlAccessor_.writeValue(IDC_UNLOCK_ON_CTRL_ALT_DEL, settings_.unlockOnCtrlAltDel);

    hideTrayIconToolip_.updateTipText(Messages::get(MessageId::HotkeyRequired));
    lockWhenIdleToolip_.updateTipText(Messages::get(MessageId::NotAvailableWhenUsingRDC));

    //
    // Delay before locking
    //
    auto hwndDBLCheck = dlgItem(IDC_DELAY_BEFORE_LOCKING_CHECK);
    auto hwndDBLSpin = dlgItem(IDC_DELAY_BEFORE_LOCKING_SPIN);

    bool delayEnabled = true;
    auto delayVal = settings_.delayBeforeLocking.value();
    if (delayVal == 0) { // off
        delayEnabled = false;
        delayVal = settings_.delayBeforeLocking.defaultValue();
    }

    SetDlgItemInt(hWnd, IDC_DELAY_BEFORE_LOCKING, delayVal, FALSE);
    SendMessage(hwndDBLCheck, BM_SETCHECK, delayEnabled, 0L);
    SendMessage(hwndDBLSpin, UDM_SETPOS, 0L, MAKELONG(delayVal, 0));


    //
    // Lock when idle
    //
    auto hwndLWICheck = dlgItem(IDC_LOCK_WHEN_IDLE_CHECK);
    auto hwndLWISpin = dlgItem(IDC_LOCK_WHEN_IDLE_SPIN);

    bool lwiEnabled = true;
    auto lwiVal = settings_.lockWhenIdle.value();
    if (lwiVal == 0) { // off
        lwiEnabled = false;
        lwiVal = settings_.lockWhenIdle.defaultValue();
    }

    SetDlgItemInt(hWnd, IDC_LOCK_WHEN_IDLE, lwiVal, FALSE);
    SendMessage(hwndLWICheck, BM_SETCHECK, lwiEnabled, 0L);
    SendMessage(hwndLWISpin, UDM_SETPOS, 0L, MAKELONG(lwiVal, 0));

    SendDlgItemMessage(hWnd, IDC_EVENT_INTERCEPTION, CB_SETCURSEL, settings_.eventInterception.value(), 0L);

    enableControls();
}

void PageAdvancedDlg::onEventInterception(WORD wNotifyCode, HWND hWndCtrl) {
    if (wNotifyCode == CBN_SELCHANGE) {
        auto idx = static_cast<int>(SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0L));

        if (idx == EventInterception::INTERCEPTION_DRIVER) {
            InterceptionHook hook;
            if (!hook.canBeUsed()) {
                MessageBox(hWnd, Messages::get(MessageId::InstallInterceptionDriver).c_str(),
                           APP_NAME, MB_OK | MB_ICONEXCLAMATION);
                // restore the previous value
                SendMessage(hWndCtrl, CB_SETCURSEL, settings_.eventInterception.value(), 0L);
                return;
            }
        }
        settings_.eventInterception.setValue(idx);
    }
}

void PageAdvancedDlg::enableControls() {
    EnableWindow(dlgItem(IDC_HIDE_TRAY_ICON_WHEN_LOCKED), settings_.hotkeyExists());
    hideTrayIconToolip_.activate(!settings_.hotkeyExists());

    bool enabled = false;
    controlAccessor_.readValue(IDC_DELAY_BEFORE_LOCKING_CHECK, enabled);
    EnableWindow(dlgItem(IDC_DELAY_BEFORE_LOCKING), enabled);
    EnableWindow(dlgItem(IDC_DELAY_BEFORE_LOCKING_UNIT), enabled);

    controlAccessor_.readValue(IDC_LOCK_WHEN_IDLE_CHECK, enabled);
    bool remote = Process::isRemoteSession();
    EnableWindow(dlgItem(IDC_LOCK_WHEN_IDLE_CHECK), !remote);
    EnableWindow(dlgItem(IDC_LOCK_WHEN_IDLE), enabled && !remote);
    EnableWindow(dlgItem(IDC_LOCK_WHEN_IDLE_UNIT), enabled && !remote);
    lockWhenIdleToolip_.activate(remote);
}

bool PageAdvancedDlg::save() {
    controlAccessor_.readValue(IDC_SHOW_NOTIFICATIONS, settings_.showNotifications);
    controlAccessor_.readValue(IDC_HIDE_TRAY_ICON_WHEN_LOCKED, settings_.hideTrayIconWhenLocked);
    controlAccessor_.readValue(IDC_UNLOCK_ON_CTRL_ALT_DEL, settings_.unlockOnCtrlAltDel);

    //
    // Delay before locking
    //
    bool checked = false;
    int value = 0;
    controlAccessor_.readValue(IDC_DELAY_BEFORE_LOCKING_CHECK, checked);
    controlAccessor_.readValue(IDC_DELAY_BEFORE_LOCKING, value);
    settings_.delayBeforeLocking.setValue(checked ? value : 0);     // 0 = off

    //
    // Lock when idle
    //
    checked = false;
    value = 0;
    controlAccessor_.readValue(IDC_LOCK_WHEN_IDLE_CHECK, checked);
    controlAccessor_.readValue(IDC_LOCK_WHEN_IDLE, value);
    settings_.lockWhenIdle.setValue(checked ? value : 0);           // 0 = off

    return true;
}

void PageAdvancedDlg::InitialValues::save() {
    const auto& s = SettingsData::instance();

    showNotifications_ = s.showNotifications.value();
    hideTrayIconWhenLocked_ = s.hideTrayIconWhenLocked.value();
    unlockOnCtrlAltDel_ = s.unlockOnCtrlAltDel.value();
    delayBeforeLocking_ = s.delayBeforeLocking.value();
    lockWhenIdle_ = s.lockWhenIdle.value();
    eventInterception_ = s.eventInterception.value();
}

void PageAdvancedDlg::InitialValues::revert() const {
    auto& s = SettingsData::instance();

    s.showNotifications.setValue(showNotifications_);
    s.hideTrayIconWhenLocked.setValue(hideTrayIconWhenLocked_);
    s.unlockOnCtrlAltDel.setValue(unlockOnCtrlAltDel_);
    s.delayBeforeLocking.setValue(delayBeforeLocking_);
    s.lockWhenIdle.setValue(lockWhenIdle_);
    s.eventInterception.setValue(eventInterception_);
}

} // namespace litelockr
