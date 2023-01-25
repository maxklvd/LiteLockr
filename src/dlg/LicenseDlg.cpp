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

#include "LicenseDlg.h"

#include <windowsx.h>
#include "app/Version.h"
#include "sys/BinaryResource.h"
#include "sys/Process.h"

#include "res/Resources.h"

namespace litelockr {

LicenseDlg::LicenseDlg() : Dialog(IDD_LICENSE) {
    onCommand(IDCANCEL, [this]() {
        destroyDialog();
    });
}

void LicenseDlg::localizeDialog() {
    auto title = Messages::get(MessageId::LicenseInformation);
    title += L" - ";
    title += APP_NAME;
    SetWindowText(hWnd, title.c_str());

    setLocalizedText(IDCANCEL, MessageId::Close);
}

BOOL LicenseDlg::onInitDialog() {
    localizeDialog();

    auto text = reinterpret_cast<const char *>(Bin::data(IDB_LICENSE));
    auto size = Bin::size(IDB_LICENSE);
    std::wstring license(size + 1, L'\0');
    mbstowcs(&license[0], text, size);

    HWND hWndEdit = dlgItem(IDC_LICENSE_TEXT);
    SetFocus(hWndEdit);
    Edit_SetText(hWndEdit, license.c_str());
    Edit_SetSel(hWndEdit, static_cast<WPARAM>(-1), static_cast<LPARAM>(-1));
    return FALSE;
}

void LicenseDlg::show() {
    if (isVisible()) {
        SetActiveWindow(hWnd);
    } else {
        showDialog(Process::mainWindow());
    }
}

void LicenseDlg::destroy() {
    destroyDialog();
}

} // namespace litelockr
