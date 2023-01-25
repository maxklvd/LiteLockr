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

#ifndef PAGE_GENERAL_DLG_H
#define PAGE_GENERAL_DLG_H

#include <string>
#include <vector>
#include <unordered_map>

#include "dlg/PinDlg.h"
#include "dlg/PropertiesDlg.h"
#include "gui/Dialog.h"
#include "gui/ControlAccessor.h"
#include "ini/SettingsData.h"
#include "lock/config/AppConfigSet.h"

#include "res/Resources.h"

namespace litelockr {

class PageGeneralDlg: public Dialog {
public:
    PageGeneralDlg();

    void onLockPreviewEnded();

protected:
    BOOL windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult) override;
    BOOL onInitDialog() override;
    void update() override;
    void onShowDialog() override {}
    void enableControls();
    void refreshAllowedAppList(int selectItem = -1);

    void onAdd();
    void onAddFromList(int dropDownItemId);
    LRESULT onCreateDropDownMenu(LPNMHDR lParam);
    void onProperties();
    void onRemove();

    void onCheckAll(WORD /*wNotifyCode*/, HWND hWndCtrl);
    void onSetPIN();

private:
    void setHotkeyValue();
    void localizeDialog();
    void addTitleIntoAppNames(int idx, const std::wstring& title);
    void detectAutomationId(int appIdx);
    bool save();

    enum {
        DROP_DOWN_ITEM_MIN = 6100,
        DROP_DOWN_ITEM_MAX = 6199,
    };
    std::unordered_map<int, std::pair<std::wstring, std::wstring>> dropDownItems_; // item id -> (path,  title)
    std::vector<int> allowedAppsIdx_;

    PinDlg pinDlg_;
    PropertiesDlg propertiesDlg_;
    bool isCheckRunning_ = false;

    friend class SettingsDlg;

    SettingsData& settings_{SettingsData::instance()};
    ControlAccessor controlAccessor_{hWnd};
    AppConfigSet appConfigSet_;

    class InitialValues {
    public:
        void save();
        void revert() const;

    private:
        decltype(settings_.hotkey)::Type hotkey_{};
        decltype(settings_.allowedApp)::Type allowedApp_{};
        decltype(settings_.dontLockCurrentApp)::Type dontLockCurrentApp_{};
        decltype(settings_.preventUnlockingInput)::Type preventUnlockingInput_{};
        decltype(settings_.usePin)::Type usePin_{};
        decltype(settings_.pin)::Type pin_{};
        decltype(settings_.startWithWindows)::Type startWithWindows_{};
        decltype(settings_.lockOnStartup)::Type lockOnStartup_{};
    } initialValues_;
};

} // namespace litelockr

#endif // PAGE_GENERAL_DLG_H
