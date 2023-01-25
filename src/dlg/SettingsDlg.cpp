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

#include "SettingsDlg.h"

#include <CommCtrl.h>
#include "app/AppMessages.h"
#include "app/Version.h"
#include "gui/WindowUtils.h"
#include "sys/Process.h"

namespace litelockr {

SettingsDlg::SettingsDlg() : Dialog(IDD_SETTINGS) {
    onCommand(IDOK, &SettingsDlg::onOK);
    onCommand(IDCANCEL, &SettingsDlg::onCancel);
    onCommand(MAKEWPARAM(IDC_WARNING_ICON, STN_CLICKED), &SettingsDlg::onWarning);
    onMessage(WMU_LOCALIZE_DIALOG, &SettingsDlg::onLocalizeDialog);
    onMessage(WM_SETCURSOR, &SettingsDlg::onSetCursor);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:26454)
#endif
    onNotify(NM_CLICK, &SettingsDlg::onReset);
    onNotify(NM_RETURN, &SettingsDlg::onReset);
    onNotify(TCN_SELCHANGING, &SettingsDlg::onTabChanging);
    onNotify(TCN_SELCHANGE, &SettingsDlg::onTabChange);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

void SettingsDlg::localizeDialog() {
    auto title = Messages::get(MessageId::Settings);
    title += L" - ";
    title += APP_NAME;
    SetWindowText(hWnd, title.c_str());

    auto general = Messages::get(MessageId::General);
    auto advanced = Messages::get(MessageId::Advanced);
    auto language = Messages::get(MessageId::Language);

    HWND hWndTab = dlgItem(IDC_TAB_SETTINGS);
    TCITEM tie{
            .mask = TCIF_TEXT,
            .pszText = const_cast<wchar_t *>(general.c_str()),
    };
    TabCtrl_SetItem(hWndTab, 0, &tie);

    tie.pszText = const_cast<wchar_t *>(advanced.c_str());
    TabCtrl_SetItem(hWndTab, 1, &tie);

    tie.pszText = const_cast<wchar_t *>(language.c_str());
    TabCtrl_SetItem(hWndTab, 2, &tie);

    std::wstring reset(L"<a>");
    reset.append(Messages::get(MessageId::Reset));
    reset.append(L"</a>");
    SetDlgItemText(hWnd, IDC_RESET, reset.c_str());

    setLocalizedText(IDOK, MessageId::OK);
    setLocalizedText(IDCANCEL, MessageId::Cancel);
}

BOOL SettingsDlg::onInitDialog() {
//	auto hIcon = static_cast<HICON>(LoadImageW(Process::moduleInstance(),
//		MAKEINTRESOURCEW(IDI_APP_ICON),
//		IMAGE_ICON,
//		GetSystemMetrics(SM_CXSMICON),
//		GetSystemMetrics(SM_CYSMICON),
//		0));
//	if (hIcon) {
//		SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
//	}
    //
    HICON hAppIcon = nullptr;
    LoadIconMetric(Process::moduleInstance(), MAKEINTRESOURCE(IDI_APP_ICON), LIM_SMALL, &hAppIcon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hAppIcon));

    HICON hWarningIcon = nullptr;
    LoadIconMetric(Process::moduleInstance(), MAKEINTRESOURCE(IDI_APP_WARNING), LIM_SMALL, &hWarningIcon);
    SendDlgItemMessage(hWnd, IDC_WARNING_ICON, STM_SETICON, reinterpret_cast<LPARAM>(hWarningIcon), 0);

    pageGeneral_.create(hWnd);
    pageAdvanced_.create(hWnd);
    pageLanguage_.create(hWnd);

    HWND hWndTab = dlgItem(IDC_TAB_SETTINGS);

    TCITEM tie{
            .mask = TCIF_TEXT | TCIF_IMAGE,
            .pszText = const_cast<wchar_t *>(L""),
            .iImage = -1,
    };
    TabCtrl_InsertItem(hWndTab, 0, &tie);
    TabCtrl_InsertItem(hWndTab, 1, &tie);
    TabCtrl_InsertItem(hWndTab, 2, &tie);

    GetClientRect(hWndTab, &rcTab_);
    MapWindowPoints(hWndTab, hWnd, reinterpret_cast<LPPOINT>(&rcTab_), 2);
    TabCtrl_AdjustRect(hWndTab, FALSE, &rcTab_);

    SetWindowLongPtr(pageGeneral_.hWnd, GWL_STYLE, GetWindowLongPtr(pageGeneral_.hWnd, GWL_STYLE) | WS_TABSTOP);
    SetWindowLongPtr(pageAdvanced_.hWnd, GWL_STYLE, GetWindowLongPtr(pageAdvanced_.hWnd, GWL_STYLE) | WS_TABSTOP);
    SetWindowLongPtr(pageLanguage_.hWnd, GWL_STYLE, GetWindowLongPtr(pageLanguage_.hWnd, GWL_STYLE) | WS_TABSTOP);

    int x = rcTab_.left, y = rcTab_.top,
            w = rcTab_.right - rcTab_.left,
            h = rcTab_.bottom - rcTab_.top;
    SetWindowPos(pageGeneral_.hWnd, HWND_TOP, x, y, w, h, 0);
    SetWindowPos(pageAdvanced_.hWnd, HWND_TOP, x, y, w, h, 0);
    SetWindowPos(pageLanguage_.hWnd, HWND_TOP, x, y, w, h, 0);

    localizeDialog();

    showPage(PAGE_GENERAL);
    updateWarnings();

    Process::setSettingsDialogOpen(true);
    SetFocus(dlgItem(IDOK));
    return FALSE;
}

void SettingsDlg::showPage(int pageIdx) {
    if (currentPageIdx_ >= 0) {
        Dialog& currentPage = pages_[currentPageIdx_];
        ShowWindow(currentPage.hWnd, SW_HIDE);
    }

    if (pageIdx >= PAGE_GENERAL && pageIdx <= PAGE_LANGUAGE) {
        Dialog& page = pages_[pageIdx];
        page.update();
        ShowWindow(page.hWnd, SW_SHOW);
        currentPageIdx_ = pageIdx;
    }
}

void SettingsDlg::updatePage(int pageIdx) {
    if (pageIdx >= PAGE_GENERAL && pageIdx <= PAGE_LANGUAGE) {
        Dialog& page = pages_[pageIdx];
        page.update();
    }
}

void SettingsDlg::onOK() {
    if (save()) {
        settings_.validate();
        settings_.save(hWnd);
        destroy();
    }
}

void SettingsDlg::onCancel() {
    pageGeneral_.initialValues_.revert();
    pageAdvanced_.initialValues_.revert();
    pageLanguage_.initialValues_.revert();
    PostMessage(Process::mainWindow(), WMU_TRAY_ICON_UPDATE, 0, 0L);
    destroy();
}

LRESULT SettingsDlg::onReset(LPNMHDR lParam) {
    auto pNMLink = reinterpret_cast<PNMLINK>(lParam);
    if (pNMLink && pNMLink->hdr.idFrom == IDC_RESET) {
        int answer = MessageBox(hWnd, Messages::get(MessageId::ResetSettingsMessage).c_str(),
                                Messages::get(MessageId::ResetSettings).c_str(), MB_YESNO | MB_ICONQUESTION);
        if (answer == IDYES) {
            settings_.reset();

            updatePage(PAGE_GENERAL);
            updatePage(PAGE_ADVANCED);
            updatePage(PAGE_LANGUAGE);
            PostMessage(hWnd, WMU_LOCALIZE_DIALOG, 0, 0L);
            PostMessage(Process::mainWindow(), WMU_TRAY_ICON_UPDATE, 0, 0L);
        }
    }
    return 0L;
}

LRESULT SettingsDlg::onTabChanging(LPNMHDR /*lParam*/) {
    if (save()) {
        return FALSE; // allow
    }
    return TRUE; // prevent
}

LRESULT SettingsDlg::onTabChange(LPNMHDR /*lParam*/) {
    settings_.validate();

    HWND hWndTab = dlgItem(IDC_TAB_SETTINGS);
    auto idx = TabCtrl_GetCurSel(hWndTab);
    showPage(idx);
    return 0L;
}

void SettingsDlg::onLocalizeDialog() {
    localizeDialog();
    pageGeneral_.localizeDialog();
    pageAdvanced_.localizeDialog();
    pageLanguage_.localizeDialog();

    SetWindowPos(dlgItem(IDC_WARNING_ICON), HWND_TOP, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE);
}

void SettingsDlg::show() {
    if (isVisible()) {
        SetActiveWindow(hWnd);
    } else {
        assert(Process::mainWindow());
        showDialog(Process::mainWindow());
    }
}

void SettingsDlg::destroy() {
    destroyDialog();
    Process::setSettingsDialogOpen(false);
}

void SettingsDlg::onLockPreviewEnded() {
    if (isVisible()) {
        pageGeneral_.onLockPreviewEnded();
    }
}

void SettingsDlg::onWarning() {
    if (!warnings_.empty()) {
        std::wstring caption = Messages::get(MessageId::Warnings);
        caption += L" - ";
        caption += +APP_NAME;

        MSGBOXPARAMS mbp = {sizeof(mbp)};
        mbp.hwndOwner = hWnd;
        mbp.hInstance = Process::moduleInstance();
        mbp.lpszCaption = caption.c_str();
        mbp.lpszText = warnings_.c_str();
        mbp.lpszIcon = MAKEINTRESOURCE(IDI_APP_WARNING);
        mbp.dwStyle = MB_OK | MB_USERICON;
        MessageBoxIndirect(&mbp);
    }
}


LRESULT SettingsDlg::onSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
    POINT pt = {0};
    RECT rc = {0};
    GetCursorPos(&pt);
    auto hItem = dlgItem(IDC_WARNING_ICON);
    if (IsWindowVisible(hItem)) {
        GetWindowRect(dlgItem(IDC_WARNING_ICON), &rc);
        SetCursor(LoadCursor(nullptr, PtInRect(&rc, pt) ? IDC_HAND : IDC_ARROW));
    } else {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
    }
    bHandled = TRUE;
    return TRUE;
}

void SettingsDlg::updateWarnings() {
    warnings_.clear();

    if (WindowUtils::isAutoHideTaskbar()) {
        warnings_ = Messages::get(MessageId::AutoHideTaskbarUnsupported);
    }

    ShowWindow(dlgItem(IDC_WARNING_ICON), warnings_.empty() ? SW_HIDE : SW_SHOW);
}


} // namespace litelockr
