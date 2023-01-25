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

#include "AboutDlg.h"

#include <cassert>

#include <CommCtrl.h>
#include "app/event/AppEvent.h"
#include "app/Version.h"
#include "gfx/BitmapContext.h"
#include "gui/WindowUtils.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

#include "res/Resources.h"

namespace litelockr {

AboutDlg::AboutDlg() : Dialog(IDD_ABOUT) {
    onMessage(WM_ERASEBKGND, &AboutDlg::onEraseBackground);
    onMessage(WM_CTLCOLORSTATIC, &AboutDlg::onCtlColorStatic);
    onCommand(IDOK, [this]() { destroyDialog(); });
    onCommand(IDCANCEL, [this]() { destroyDialog(); });
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:26454)
#endif
    onNotify(NM_CLICK, &AboutDlg::onSysLink);
    onNotify(NM_RETURN, &AboutDlg::onSysLink);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

void AboutDlg::localizeDialog() {
    SetWindowText(hWnd, Messages::get(MessageId::About, APP_NAME).c_str());
    SetDlgItemText(hWnd, IDC_VERSION, APP_VERSION);

    std::wstring licenseInformation{L"<a>"};
    licenseInformation.append(Messages::get(MessageId::LicenseInformation));
    licenseInformation.append(L"</a>");
    SetDlgItemText(hWnd, IDC_LICENSE_INFORMATION, licenseInformation.c_str());
}

BOOL AboutDlg::onInitDialog() {
    localizeDialog();
    auto scale = WindowUtils::getDpiScale();

    NONCLIENTMETRICS metrics = {};
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
    metrics.lfMessageFont.lfHeight = -std::lround(11 * scale);
    metrics.lfMessageFont.lfWeight = FW_NORMAL;
    hFont_ = CreateFontIndirect(&metrics.lfMessageFont);

    metrics.lfMessageFont.lfHeight = -std::lround(16 * scale);
    metrics.lfMessageFont.lfWeight = FW_BOLD;
    hNameFont_ = CreateFontIndirect(&metrics.lfMessageFont);

    metrics.lfMessageFont.lfHeight = -std::lround(10 * scale);
    metrics.lfMessageFont.lfWeight = FW_NORMAL;
    hCopyrightFont_ = CreateFontIndirect(&metrics.lfMessageFont);

    assert(hFont_);
    assert(hNameFont_);
    assert(hCopyrightFont_);

    hbrBackground_ = CreateSolidBrush(BACKGROUND_COLOR);

    SendDlgItemMessage(hWnd, IDC_NAME, WM_SETFONT, (WPARAM) hNameFont_, TRUE);
    SendDlgItemMessage(hWnd, IDC_VERSION, WM_SETFONT, (WPARAM) hFont_, TRUE);
    SendDlgItemMessage(hWnd, IDC_WEBSITE, WM_SETFONT, (WPARAM) hFont_, TRUE);
    SendDlgItemMessage(hWnd, IDC_WEBSITE_GITHUB, WM_SETFONT, (WPARAM) hFont_, TRUE);
    SendDlgItemMessage(hWnd, IDC_LICENSE_INFORMATION, WM_SETFONT, (WPARAM) hFont_, TRUE);
    SendDlgItemMessage(hWnd, IDC_COPYRIGHT, WM_SETFONT, (WPARAM) hCopyrightFont_, TRUE);

    if (scale >= 1.2f) {
        auto hIcon96 = reinterpret_cast<HICON>(LoadImageW(Process::moduleInstance(),
                                                          MAKEINTRESOURCEW(IDI_APP_ICON_96),
                                                          IMAGE_ICON, 96, 96, 0));
        if (hIcon96) {
            SendDlgItemMessage(hWnd, IDC_LOGO_ICON, STM_SETICON, (WPARAM) hIcon96, 0);
        }
    }

    centerLogoIcon();
    centerSysLink(IDC_WEBSITE);
    centerSysLink(IDC_WEBSITE_GITHUB);
    centerSysLink(IDC_LICENSE_INFORMATION);

    return TRUE;
}

LRESULT AboutDlg::onSysLink(int /*wParam*/, LPNMHDR lParam, BOOL& /*bHandled*/) {
    auto pNMLink = reinterpret_cast<PNMLINK>(lParam);
    if (pNMLink) {
        switch (pNMLink->hdr.idFrom) {
            case IDC_WEBSITE:
                AppEvents::send(AppEvent::WEBSITE);
                break;
            case IDC_WEBSITE_GITHUB:
                AppEvents::send(AppEvent::WEBSITE_GITHUB);
                break;
            case IDC_LICENSE_INFORMATION:
                AppEvents::send(AppEvent::LICENSE_INFORMATION);
                break;
            default:
                break;
        }
    }
    return 0L;
}


void AboutDlg::show() {
    if (isVisible()) {
        SetActiveWindow(hWnd);
    } else {
        showDialog(Process::mainWindow());
    }
}

void AboutDlg::destroy() {
    DeleteObject(hNameFont_);
    DeleteObject(hFont_);
    DeleteObject(hCopyrightFont_);
    DeleteObject(hbrBackground_);
    destroyDialog();
}

void AboutDlg::centerLogoIcon() {
    HWND hLogo = GetDlgItem(hWnd, IDC_LOGO_ICON);
    HWND hName = GetDlgItem(hWnd, IDC_NAME);
    assert(hLogo);

    RECT windowRc;
    RECT logoRc;
    RECT nameRc;
    GetClientRect(hWnd, &windowRc);
    GetWindowRect(hLogo, &logoRc);
    GetWindowRect(hName, &nameRc);

    POINT logoPt = {logoRc.left, logoRc.top};
    POINT namePt = {nameRc.left, nameRc.top};

    namePt.y += std::lround(5 * WindowUtils::getDpiScale());

    ScreenToClient(hWnd, &logoPt);
    ScreenToClient(hWnd, &namePt);

    logoPt.x = (Rectangle::width(windowRc) - Rectangle::width(logoRc)) / 2;
    logoPt.y = (namePt.y - Rectangle::height(logoRc)) / 2;
    SetWindowPos(hLogo, nullptr, logoPt.x, logoPt.y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void AboutDlg::centerSysLink(int id) {
    HWND hItem = GetDlgItem(hWnd, id);
    assert(hItem);
    auto hFont = reinterpret_cast<HFONT>(SendMessage(hItem, WM_GETFONT, 0, 0L));

    constexpr int BUF_SIZE = 256;
    wchar_t buf[BUF_SIZE] = {0};
    GetWindowText(hItem, buf, BUF_SIZE);

    std::wstring href{buf};
    assert(href.substr(0, 3) == L"<a>");
    assert(href.substr(href.size() - 4, 4) == L"</a>");

    if (href.size() > 7) { // <a></a>
        auto text = href.substr(3, href.size() - 7); // removes "<a>" and "</a>" from the string
        auto textSize = BitmapContext::textSize(text.c_str(), hFont, hItem);

        RECT windowRc;
        RECT itemRc;
        GetClientRect(hWnd, &windowRc);
        GetWindowRect(hItem, &itemRc);
        POINT itemPt = {itemRc.left, itemRc.top};
        ScreenToClient(hWnd, &itemPt);

        itemPt.x = (Rectangle::width(windowRc) - textSize.cx) / 2;
        SetWindowPos(hItem, nullptr, itemPt.x, itemPt.y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
    }
}

LRESULT AboutDlg::onEraseBackground(UINT, WPARAM wParam, LPARAM, BOOL&/*bHandled*/) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    FillRect((HDC) wParam, &rc, hbrBackground_);
    return TRUE;
}

LRESULT AboutDlg::onCtlColorStatic(UINT, WPARAM wParam, LPARAM lParam, BOOL&/*bHandled*/) {
    COLORREF textColor;
    DWORD id = GetDlgCtrlID((HWND) lParam);

    switch (id) {
        case IDC_NAME:
        case IDC_VERSION:
            textColor = TEXT_COLOR;
            break;
        case IDC_COPYRIGHT:
            textColor = COPYRIGHT_COLOR;
            break;
        default:
            textColor = 0;
            break;
    }

    if (textColor) {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetTextColor(hdc, textColor);
        SetBkColor(hdc, BACKGROUND_COLOR);
        return reinterpret_cast<INT_PTR>(hbrBackground_);
    }
    return FALSE;
}

} // namespace litelockr
