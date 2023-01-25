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

#include "HotkeyHandler.h"

#include <cassert>
#include <iomanip>
#include <sstream>
#include <vector>

#include <CommCtrl.h>
#include "app/Version.h"
#include "lang/Messages.h"
#include "sys/StringUtils.h"
#include "sys/Process.h"

namespace litelockr {

WORD HotkeyHandler::fromString(const std::wstring& hotkey) {
    std::wstring buf(hotkey);

    // removes spaces
    std::wstring::iterator end_pos = std::remove(buf.begin(), buf.end(), L' ');
    buf.erase(end_pos, buf.end());

    StringUtils::toUpperCase(buf);

    if (buf.empty() || buf == NONE) {
        return 0;
    }

    bool ctrl = false, shift = false, alt = false, ext = false;
    wchar_t vkCode = L'\0';

    std::vector<std::wstring> strings;
    std::wistringstream f(buf);
    std::wstring s;
    while (getline(f, s, L'+')) {
        if (s == L"CTRL") {
            ctrl = true;
        } else if (s == L"SHIFT") {
            shift = true;
        } else if (s == L"ALT") {
            alt = true;
        } else if (s.size() == 1) {
            wchar_t c = s[0];
            if (isDigit(c) || isLetter(c)) {
                vkCode = c;
            }
            break;
        } else if (s.size() == 6) {
            if (s[0] == L'0' && s[1] == L'X' &&
                isDigit16(s[2]) && isDigit16(s[3]) &&
                isDigit16(s[4]) && isDigit16(s[5])) {
                auto key = static_cast<WORD>(std::wcstol(s.c_str(), nullptr, 16));
                vkCode = static_cast<wchar_t>(LOBYTE(key));
                ext = HIBYTE(key) != 0;
            }
            break;
        }
    }

    if (vkCode == L'\0') {
        return 0;
    }

    BYTE mods = 0;
    if (ctrl) {
        mods |= HOTKEYF_CONTROL;
    }
    if (shift) {
        mods |= HOTKEYF_SHIFT;
    }
    if (alt) {
        mods |= HOTKEYF_ALT;
    }
    if (ext) {
        mods |= HOTKEYF_EXT;
    }

    return MAKEWORD(vkCode, mods);
}

std::wstring HotkeyHandler::toString(WORD hotkey) {
    auto vkCode = LOBYTE(hotkey);
    auto mods = HIBYTE(hotkey);

    if (vkCode == 255) { // VK_none_
        vkCode = 0;
    }

    std::wostringstream oss;

    if (vkCode > 0) {
        if (mods & HOTKEYF_CONTROL) {
            oss << L"Ctrl+";
        }
        if (mods & HOTKEYF_SHIFT) {
            oss << L"Shift+";
        }
        if (mods & HOTKEYF_ALT) {
            oss << L"Alt+";
        }

        if (isDigit(vkCode) || isLetter(vkCode)) {
            oss << static_cast<wchar_t>(vkCode);
        } else {
            int ext = (mods & HOTKEYF_EXT) ? 1 : 0;
            int val = vkCode;
            oss << L"0x" << std::setfill(L'0') << std::setw(2) << ext;
            oss << std::uppercase << std::setfill(L'0') << std::setw(2) << std::hex << val;
        }
    } else {
        oss << NONE;
    }

    return oss.str();
}

void HotkeyHandler::initialize(const std::wstring& hotkeyStr) {
    assert(!hasInitialized_);

    hotkeyAtom_ = GlobalAddAtom(HOTKEY_ATOM);

    WORD hotkey = fromString(hotkeyStr);
    std::wstring errMessage;
    if (!setHotkey(hotkey, errMessage)) {
        assert(Process::mainWindow());
        MessageBox(Process::mainWindow(), errMessage.c_str(), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
    }

    hasInitialized_ = true;
}

void HotkeyHandler::free() const {
    assert(hasInitialized_);
    assert(hotkeyAtom_ != 0);
    GlobalDeleteAtom(hotkeyAtom_);
}

bool HotkeyHandler::setHotkey(WORD hotkey, std::wstring& errMessage) {
    auto vkCode = LOBYTE(hotkey);
    auto mods = HIBYTE(hotkey);

    if (vkCode == 255) { // VK_none_
        vkCode = 0;
    }

    ctrl_ = alt_ = shift_ = false;
    vkCode_ = 0;
    hotkeyFormatted_ = L"";
    errMessage = L"";

    // unregister the key
    HWND hWnd = Process::mainWindow();
    assert(hWnd);
    UnregisterHotKey(hWnd, hotkeyAtom_);

    if (vkCode > 0) {
        vkCode_ = vkCode;

        UINT fsModifiers = 0x4000; // MOD_NOREPEAT
        if (mods & HOTKEYF_CONTROL) {
            ctrl_ = true;
            fsModifiers |= MOD_CONTROL;
            hotkeyFormatted_ += L"Ctrl + ";
        }
        if (mods & HOTKEYF_SHIFT) {
            shift_ = true;
            fsModifiers |= MOD_SHIFT;
            hotkeyFormatted_ += L"Shift + ";
        }
        if (mods & HOTKEYF_ALT) {
            alt_ = true;
            fsModifiers |= MOD_ALT;
            hotkeyFormatted_ += L"Alt + ";
        }
        bool ext = (mods & HOTKEYF_EXT) != 0;

        std::wstring keyStr;
        if (virtualKeyToString(vkCode_, ext, keyStr)) {
            hotkeyFormatted_ += keyStr;
        } else {
            hotkeyFormatted_ = L"";
        }

        // register the key
        assert(hotkeyAtom_ != 0);
        BOOL result = RegisterHotKey(hWnd, hotkeyAtom_, fsModifiers, vkCode_);
        if (!result) {
            errMessage = Messages::get(MessageId::UnableRegisterHotkey, hotkeyFormatted_);

            ctrl_ = alt_ = shift_ = false;
            vkCode_ = 0;
            hotkeyFormatted_ = L"";
            return false;
        }
    }

    return true;
}

bool HotkeyHandler::virtualKeyToString(UINT virtualKey, bool isExt, std::wstring& result) {
    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

    if (isExt) {
        scanCode |= KF_EXTENDED; // set extended bit
    }

    assert(scanCode != 0);
    if (scanCode > 0) {
        constexpr int BUF_SIZE = 256;
        wchar_t buf[BUF_SIZE] = {0};

        int ret = GetKeyNameText(static_cast<LONG>(scanCode) << 16, buf, BUF_SIZE);
        assert(ret != 0);
        if (ret) {
            result = buf;
            return true;
        }
    }
    return false;
}

unsigned HotkeyHandler::hotkeyId() const {
    assert(hotkeyAtom_ != 0);
    return hotkeyAtom_;
}

} // namespace litelockr
