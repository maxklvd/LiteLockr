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

#include "PageLanguageDlg.h"

#include <Uxtheme.h>
#include "app/AppMessages.h"
#include "sys/Process.h"

#include "res/Resources.h"

namespace litelockr {

PageLanguageDlg::PageLanguageDlg() : Dialog(IDD_PAGE_LANGUAGE) {
    onCommand(IDC_LANGUAGE, &PageLanguageDlg::onLanguage);
}

void PageLanguageDlg::localizeDialog() {
    setLocalizedText(IDC_LANGUAGE_LABEL, MessageId::Language);
}

BOOL PageLanguageDlg::onInitDialog() {
    localizeDialog();
    initialValues_.save();
    initialValues_.hWndSettings = GetParent(hWnd);
    EnableThemeDialogTexture(hWnd, ETDT_ENABLETAB);

    update();

    return TRUE;
}

void PageLanguageDlg::update() {
    HWND hWndLang = dlgItem(IDC_LANGUAGE);
    int selectedIdx = 0, idx = 0;
    langCodes_.clear();
    SendMessage(hWndLang, CB_RESETCONTENT, 0, 0L);

    for (const auto& [code, names]: settings_.languages()) {
        std::wstring lang = names.first;
        lang += L" (" + names.second + L")";

        if (code == settings_.language.value()) {
            selectedIdx = idx;
        }

        SendMessage(hWndLang, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(lang.c_str()));
        idx++;
        langCodes_.push_back(code);
    }

    SendMessage(hWndLang, CB_SETCURSEL, selectedIdx, 0L);
}

void PageLanguageDlg::onLanguage(WORD wNotifyCode, HWND hWndCtrl) {
    if (wNotifyCode == CBN_SELCHANGE) {
        unsigned idx = SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0L);

        assert(idx < langCodes_.size());
        if (idx < langCodes_.size()) {
            const auto& code = langCodes_[idx];

            settings_.language.setValue(code);
            Messages::instance().load(code);
            PostMessage(GetParent(this->hWnd), WMU_LOCALIZE_DIALOG, 0, 0L);
            PostMessage(Process::mainWindow(), WMU_TRAY_ICON_UPDATE, 0, 0L);
        }
    }
}

void PageLanguageDlg::InitialValues::save() {
    const auto& s = SettingsData::instance();

    language_ = s.language.value();
}

void PageLanguageDlg::InitialValues::revert() {
    auto& s = SettingsData::instance();

    if (s.language.value() != language_) {
        s.language.setValue(language_);
        Messages::instance().load(language_);
        PostMessage(hWndSettings, WMU_LOCALIZE_DIALOG, 0, 0L);
    }
}

} // namespace litelockr
