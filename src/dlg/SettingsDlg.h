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

#ifndef SETTINGS_DLG_H
#define SETTINGS_DLG_H

#include <vector>

#include "dlg/PageGeneralDlg.h"
#include "dlg/PageAdvancedDlg.h"
#include "dlg/PageLanguageDlg.h"
#include "gui/Dialog.h"
#include "ini/SettingsData.h"

#include "res/Resources.h"

namespace litelockr {

class SettingsDlg: public Dialog {
public:
    SettingsDlg();

    void show();
    void destroy();
    void onLockPreviewEnded();

protected:
    BOOL onInitDialog() override;
    void onShowDialog() override {}
    void onOK();
    void onCancel();
    void onWarning();
    LRESULT onReset(LPNMHDR lParam);
    LRESULT onTabChanging(LPNMHDR /*lParam*/);
    LRESULT onTabChange(LPNMHDR /*lParam*/);
    void onLocalizeDialog();
    LRESULT onSetCursor(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

private:
    void localizeDialog();

    bool save() {
        return pageGeneral_.save() &&
               pageAdvanced_.save() &&
               pageLanguage_.save();
    }

    enum {
        PAGE_GENERAL = 0,
        PAGE_ADVANCED = 1,
        PAGE_LANGUAGE = 2,
    };
    void showPage(int pageIdx);
    void updatePage(int pageIdx);

    PageGeneralDlg pageGeneral_;
    PageAdvancedDlg pageAdvanced_;
    PageLanguageDlg pageLanguage_;

    std::vector<std::reference_wrapper<Dialog>> pages_ = {pageGeneral_, pageAdvanced_, pageLanguage_};
    RECT rcTab_{};
    int currentPageIdx_ = -1;

    SettingsData& settings_{SettingsData::instance()};

    std::wstring warnings_;
    void updateWarnings();
};

} // namespace litelockr

#endif // SETTINGS_DLG_H
