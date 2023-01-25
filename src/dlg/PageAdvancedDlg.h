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

#ifndef PAGE_ADVANCED_DLG_H
#define PAGE_ADVANCED_DLG_H

#include "gui/Dialog.h"
#include "gui/ControlAccessor.h"
#include "gui/Tooltip.h"
#include "ini/SettingsData.h"

#include "res/Resources.h"

namespace litelockr {

class PageAdvancedDlg: public Dialog {
public:
    PageAdvancedDlg();

protected:
    BOOL onInitDialog() override;
    void update() override;
    void onShowDialog() override {}
    void enableControls();
    void onEventInterception(WORD wNotifyCode, HWND hWnd);

private:
    void localizeDialog();
    bool save();

    friend class SettingsDlg;

    SettingsData& settings_{SettingsData::instance()};

    ControlAccessor controlAccessor_{hWnd};
    Tooltip hideTrayIconToolip_;
    Tooltip lockWhenIdleToolip_;

    class InitialValues {
    public:
        void save();
        void revert() const;

    private:
        decltype(settings_.showNotifications)::Type showNotifications_{};
        decltype(settings_.hideTrayIconWhenLocked)::Type hideTrayIconWhenLocked_{};
        decltype(settings_.unlockOnCtrlAltDel)::Type unlockOnCtrlAltDel_{};
        decltype(settings_.delayBeforeLocking)::Type delayBeforeLocking_{};
        decltype(settings_.lockWhenIdle)::Type lockWhenIdle_{};
        decltype(settings_.eventInterception)::Type eventInterception_{};
    } initialValues_;
};

} // namespace litelockr

#endif // PAGE_ADVANCED_DLG_H
