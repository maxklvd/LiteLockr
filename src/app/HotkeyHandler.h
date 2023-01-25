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

#ifndef HOTKEY_HANDLER_H
#define HOTKEY_HANDLER_H

#include <string>

#include <windows.h>

namespace litelockr {

class HotkeyHandler {
public:
    static constexpr auto NONE = L"NONE";

    static WORD fromString(const std::wstring& hotkey);
    static std::wstring toString(WORD hotkey);

    void initialize(const std::wstring& hotkeyStr);

    void free() const;

    [[nodiscard]] unsigned hotkeyId() const;

    bool setHotkey(WORD hotkey, std::wstring& errMessage);

    [[nodiscard]] const wchar_t *hotkeyFormatted() const {
        return hotkeyFormatted_.c_str();
    }

    [[nodiscard]] bool empty() const { return vkCode_ == 0; }

    [[nodiscard]] bool isCtrl() const { return ctrl_; }

    [[nodiscard]] bool isAlt() const { return alt_; }

    [[nodiscard]] bool isShift() const { return shift_; }

    [[nodiscard]] BYTE vkCode() const { return vkCode_; }

private:
    static bool isDigit16(wchar_t c) {
        return isDigit(c) || (c >= L'A' && c <= L'F');
    }

    static bool isDigit(wchar_t c) {
        return c >= L'0' && c <= L'9';
    }

    static bool isLetter(wchar_t c) {
        return c >= L'A' && c <= L'Z';
    }

    static bool virtualKeyToString(UINT virtualKey, bool isExt, std::wstring& result);

    bool ctrl_ = false;
    bool alt_ = false;
    bool shift_ = false;
    BYTE vkCode_ = 0;

    bool hasInitialized_ = false;

    constexpr static auto HOTKEY_ATOM = L"E05CBD73-1D53-4E4E-9E78-34230B8FBBCD";

    ATOM hotkeyAtom_ = 0;

    std::wstring hotkeyFormatted_;

//
// Singleton implementation
//
public:
    //
    // Returns the object instance
    //
    static HotkeyHandler& instance() {
        static HotkeyHandler instance_;
        return instance_;
    }

    HotkeyHandler(const HotkeyHandler&) = delete;
    HotkeyHandler& operator=(const HotkeyHandler&) = delete;

private:
    HotkeyHandler() = default;
};

} // namespace litelockr

#endif // HOTKEY_HANDLER_H
