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

#include "Dialog.h"

#include "log/Logger.h"
#include "sys/Process.h"

namespace litelockr {

std::vector<HWND> Dialog::dlgHandles_;

void Dialog::addDialog(HWND hDlg) {
    assert(hDlg);
    if (hDlg) {
        dlgHandles_.push_back(hDlg);
    }
}

HWND Dialog::create(HWND hWndParent) {
    HWND hDlg = CreateDialogParam(Process::moduleInstance(), MAKEINTRESOURCE(dialogId_),
                                  hWndParent, dialogProc, reinterpret_cast<LPARAM>(this));
    if (hDlg) {
        addDialog(hDlg);
        return hDlg;
    }

    assert(false);
    LOG_ERROR(L"Could not create a dialog (id=0x%04x, error=%d)", dialogId_, GetLastError());
    return nullptr;
}

void Dialog::showDialog(HWND hWndParent) {
    HWND hDlg = create(hWndParent);
    onShowDialog();
    ShowWindow(hDlg, SW_SHOW);
}

void Dialog::destroyDialog() {
    assert(hWnd);
    onDestroyDialog();
    destroyWindow();
}

bool Dialog::isDialogMessage(LPMSG msg) {
    if (!dlgHandles_.empty()) {
        for (HWND hDlg: dlgHandles_) {
            if (IsDialogMessage(hDlg, msg)) {
                return true;
            }
        }
    }
    return false;
}

INT_PTR Dialog::dialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
        auto dlgPtr = reinterpret_cast<Dialog *>(lParam);
        if (dlgPtr) {
            dlgPtr->attach(hWnd);
            return dlgPtr->onInitDialog();
        }
        return FALSE;
    }

    auto wndPtr = getInstance(hWnd);
    if (wndPtr) {
        LRESULT lResult = 0;
        BOOL bRet = wndPtr->windowProc(hWnd, uMsg, wParam, lParam, lResult);
        if (bRet) {
            switch (uMsg) {
                case WM_COMPAREITEM:
                case WM_VKEYTOITEM:
                case WM_CHARTOITEM:
                case WM_QUERYDRAGICON:
                case WM_CTLCOLORMSGBOX:
                case WM_CTLCOLOREDIT:
                case WM_CTLCOLORLISTBOX:
                case WM_CTLCOLORBTN:
                case WM_CTLCOLORDLG:
                case WM_CTLCOLORSCROLLBAR:
                case WM_CTLCOLORSTATIC:
                    // return directly
                    bRet = static_cast<BOOL>(lResult);
                    break;
                default:
                    // return in DWL_MSGRESULT
                    SetWindowLongPtr(hWnd, DWLP_MSGRESULT, lResult);
                    break;
            }
        }
        return bRet;
    }
    return FALSE;
}


} // namespace litelockr
