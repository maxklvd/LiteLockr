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

#ifndef PAGE_LANGUAGE_DLG_H
#define PAGE_LANGUAGE_DLG_H

#include <string>
#include <vector>

#include "gui/Dialog.h"
#include "ini/SettingsData.h"

namespace litelockr {

class PageLanguageDlg: public Dialog {
public:
    PageLanguageDlg();

protected:
    BOOL onInitDialog() override;
    void update() override;
    void onShowDialog() override {}
    void onLanguage(WORD wNotifyCode, HWND hWndCtrl);

private:
    void localizeDialog();

    [[nodiscard]] bool save() const { return true; }

    std::vector<std::wstring> langCodes_;

    friend class SettingsDlg;

    SettingsData& settings_{SettingsData::instance()};

    class InitialValues {
    public:
        void save();
        void revert();
        HWND hWndSettings = nullptr;

    private:
        decltype(settings_.language)::Type language_{};
    } initialValues_;
};

} // namespace litelockr

#endif // PAGE_LANGUAGE_DLG_H
