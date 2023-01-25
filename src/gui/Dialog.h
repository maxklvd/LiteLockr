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

#ifndef DIALOG_H
#define DIALOG_H

#include <vector>

#include "gui/Window.h"
#include "lang/Messages.h"

namespace litelockr {

class Dialog: public Window {
public:
    explicit Dialog(int dialogId) : dialogId_(dialogId) {}

    HWND create(HWND hWndParent = nullptr);
    void showDialog(HWND hWndParent);
    void destroyDialog();
    static bool isDialogMessage(LPMSG msg);

    virtual void update() {}

    auto isVisible() { return IsWindowVisible(hWnd); }

    auto dlgItem(int id) { return GetDlgItem(hWnd, id); }

    static INT_PTR CALLBACK dialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    void setLocalizedText(int dlgItemId, MessageId messageId) {
        SetDlgItemText(hWnd, dlgItemId, Messages::get(messageId).c_str());
    }

    virtual BOOL onInitDialog() { return TRUE; }
    virtual void onShowDialog() {}
    virtual void onDestroyDialog() {}

private:
    static void addDialog(HWND hDlg);

    int dialogId_;
    static std::vector<HWND> dlgHandles_;
};


} // namespace litelockr

#endif // DIALOG_H
