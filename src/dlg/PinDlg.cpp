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

#include "PinDlg.h"

#include "app/Version.h"
#include "ini/SettingsData.h"

#include "res/Resources.h"

namespace litelockr {

PinDlg::PinDlg() : Dialog(IDD_PIN) {
    onCommand(IDC_NEW_PIN, &PinDlg::onNewPin);
    onCommand(IDC_CONFIRM_PIN, &PinDlg::onConfirmPin);
    onCommand(IDOK, &PinDlg::onOk);
    onCommand(IDCANCEL, [this]() { destroyDialog(); });
}

BOOL PinDlg::onInitDialog() {
    localizeDialog();

    SendDlgItemMessage(hWnd, IDC_NEW_PIN, EM_SETLIMITTEXT, MAX_PIN_LENGTH, 0);
    SendDlgItemMessage(hWnd, IDC_CONFIRM_PIN, EM_SETLIMITTEXT, MAX_PIN_LENGTH, 0);

    EnableWindow(dlgItem(IDC_CONFIRM_PIN), false);
    EnableWindow(dlgItem(IDOK), false);
    return TRUE;
}

void PinDlg::localizeDialog() {
    std::wstring buf;

    if (SettingsData::instance().pin.value().empty()) {
        buf = Messages::get(MessageId::SetUpPIN);
    } else {
        buf = Messages::get(MessageId::ChangeYourPIN);
    }

    buf += L" - ";
    buf += APP_NAME;
    SetWindowText(hWnd, buf.c_str());

    setLocalizedText(IDC_ENTER_PIN_LABEL, MessageId::EnterPIN);
    setLocalizedText(IDC_CONFIRM_PIN_LABEL, MessageId::ConfirmPIN);

    setLocalizedText(IDOK, MessageId::OK);
    setLocalizedText(IDCANCEL, MessageId::Cancel);
}

void PinDlg::onNewPin(WORD wNotifyCode, HWND hWndCtrl) {
    if (wNotifyCode == EN_CHANGE) {
        BOOL enabled = (GetWindowTextLength(hWndCtrl) == MAX_PIN_LENGTH);
        EnableWindow(dlgItem(IDC_CONFIRM_PIN), enabled);
    }
}

void PinDlg::onConfirmPin(WORD wNotifyCode, HWND hWndCtrl) {
    if (wNotifyCode == EN_CHANGE) {
        BOOL enabled = FALSE;
        if (GetWindowTextLength(hWndCtrl) == MAX_PIN_LENGTH) { // 4 digits
            wchar_t buf1[MAX_PIN_LENGTH + 1] = {0};
            wchar_t buf2[MAX_PIN_LENGTH + 1] = {0};
            GetWindowText(dlgItem(IDC_NEW_PIN), buf1, MAX_PIN_LENGTH + 1);
            GetWindowText(hWndCtrl, buf2, MAX_PIN_LENGTH + 1);

            if (!_wcsicmp(buf1, buf2)) {
                enabled = TRUE;
            }
        }
        EnableWindow(dlgItem(IDOK), enabled);
    }
}

void PinDlg::onOk() {
    HWND hWndPin = dlgItem(IDC_NEW_PIN);
    if (GetWindowTextLength(hWndPin) == MAX_PIN_LENGTH) { // 4 digits
        wchar_t buf[MAX_PIN_LENGTH + 1] = {0};
        GetWindowText(hWndPin, buf, MAX_PIN_LENGTH + 1);

        std::wstring code(buf);
        SettingsData::instance().pin.setValue(code);
    } else {
        assert(false);
    }
    destroyDialog();
}

void PinDlg::show(HWND hWndParent) {
    if (isVisible()) {
        SetActiveWindow(hWnd);
    } else {
        assert(hWndParent);
        showDialog(hWndParent);
    }
}

} // namespace litelockr
