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

#ifndef FLYOUT_DIALOGS_H
#define FLYOUT_DIALOGS_H

#include "dlg/AboutDlg.h"
#include "dlg/LicenseDlg.h"
#include "dlg/SettingsDlg.h"
#include "lock/ui/LockPreviewWnd.h"

namespace litelockr {

class FlyoutDialogs {
public:
    FlyoutDialogs() = default;

    AboutDlg& aboutDlg() { return aboutDlg_; }
    LicenseDlg& licenseDlg() { return licenseDlg_; }
    SettingsDlg& settingsDlg() { return settingsDlg_; }
    LockPreviewWnd& lockPreviewWnd() { return lockPreviewWnd_; }

private:
    AboutDlg aboutDlg_;
    LicenseDlg licenseDlg_;
    SettingsDlg settingsDlg_;
    LockPreviewWnd lockPreviewWnd_;
};

} // namespace litelockr

#endif // FLYOUT_DIALOGS_H
