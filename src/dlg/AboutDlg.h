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

#ifndef ABOUT_DLG_H
#define ABOUT_DLG_H

#include "gui/Dialog.h"

namespace litelockr {

class AboutDlg: public Dialog {
public:
    AboutDlg();

    void show();
    void destroy();

protected:
    BOOL onInitDialog() override;
    void onShowDialog() override {}

    LRESULT onSysLink(int /*wParam*/, LPNMHDR lParam, BOOL& bHandled);
    LRESULT onEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
    LRESULT onCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    void localizeDialog();
    void centerLogoIcon();
    void centerSysLink(int id);

    HFONT hNameFont_ = nullptr;
    HFONT hFont_ = nullptr;
    HFONT hCopyrightFont_ = nullptr;
    HBRUSH hbrBackground_ = nullptr;

    constexpr static COLORREF BACKGROUND_COLOR = RGB(240, 240, 240);
    constexpr static COLORREF TEXT_COLOR = RGB(35, 35, 35);
    constexpr static COLORREF COPYRIGHT_COLOR = RGB(80, 80, 80);
};

} // namespace litelockr

#endif // ABOUT_DLG_H
