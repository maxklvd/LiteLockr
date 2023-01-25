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

#ifndef PIN_DLG_H
#define PIN_DLG_H

#include "gui/Dialog.h"

namespace litelockr {

class PinDlg: public Dialog {
public:
    PinDlg();
    void show(HWND hWndParent);

protected:
    BOOL onInitDialog() override;
    void onShowDialog() override {}
    void onNewPin(WORD wNotifyCode, HWND hWndCtrl);
    void onConfirmPin(WORD wNotifyCode, HWND hWndCtrl);
    void onOk();

private:
    constexpr static int MAX_PIN_LENGTH = 4;
    void localizeDialog();
};

} // namespace litelockr

#endif // PIN_DLG_H
