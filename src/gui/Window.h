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

#ifndef WINDOW_H
#define WINDOW_H

#include <cassert>
#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <variant>

#include <windows.h>
#include "gui/handler/MessageHandler.h"
#include "gui/handler/CommandHandler.h"
#include "gui/handler/NotifyHandler.h"

namespace litelockr {

class Window;

template<typename T>
concept WindowT = (std::derived_from<T, Window>);

class Window {
public:
    Window() = default;
    virtual ~Window() = default;

    Window(const Window&) = delete;
    const Window& operator=(const Window&) = delete;

    [[nodiscard]] bool isNull() const { return hWnd == nullptr; }

    void destroyWindow();

    HWND hWnd = nullptr;
    constexpr static auto DEFAULT_CLASS = L"LLDefaultClass";

protected:
    static Window *getInstance(HWND hWnd);
    static LRESULT CALLBACK windowProcStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual BOOL windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult);

    void attach(HWND hWnd);
    HWND detach();

    template<typename F>
    requires std::invocable<F>
    void onMessage(UINT uMsg, F callback) {
        messageHandlers_.addHandler(uMsg, callback);
    }

    template<typename F>
    requires std::invocable<F, WPARAM, LPARAM>
    void onMessage(UINT uMsg, F callback) {
        messageHandlers_.addHandler(uMsg, callback);
    }

    template<typename F>
    requires std::invocable<F, UINT, WPARAM, LPARAM, BOOL&>
    void onMessage(UINT uMsg, F callback) {
        messageHandlers_.addHandler(uMsg, callback);
    }

    template<WindowT C, class R>
    void onMessage(UINT uMsg, R C::*callback) {
        messageHandlers_.addHandler(uMsg, static_cast<C *>(this), callback);
    }

    template<typename F>
    requires std::invocable<F>
    void onCommand(WORD id, F callback) {
        commandHandlers_.addHandler(id, callback);
    }

    template<typename F>
    requires std::invocable<F, WORD, HWND>
    void onCommand(WORD id, F callback) {
        commandHandlers_.addHandler(id, callback);
    }

    template<typename F>
    requires std::invocable<F, WORD, WORD, HWND, BOOL&>
    void onCommand(WORD id, F callback) {
        commandHandlers_.addHandler(id, callback);
    }

    template<WindowT C, class R>
    void onCommand(WORD id, R C::*callback) {
        commandHandlers_.addHandler(id, static_cast<C *>(this), callback);
    }


    template<typename F>
    requires std::invocable<F>
    void onNotify(UINT id, F callback) {
        notifyHandlers_.addHandler(id, callback);
    }

    template<typename F>
    requires std::invocable<F, LPNMHDR>
    void onNotify(UINT id, F callback) {
        notifyHandlers_.addHandler(id, callback);
    }

    template<typename F>
    requires std::invocable<F, int, LPNMHDR, BOOL&>
    void onNotify(UINT id, F callback) {
        notifyHandlers_.addHandler(id, callback);
    }

    template<WindowT C, class R>
    void onNotify(UINT id, R C::*callback) {
        notifyHandlers_.addHandler(id, static_cast<C *>(this), callback);
    }

private:
    MessageHandlerContainer messageHandlers_;
    CommandHandlerContainer commandHandlers_;
    NotifyHandlerContainer notifyHandlers_;

    friend class Dialog;

    friend class WindowUtils;
};

} // namespace litelockr

#endif /* WINDOW_H */
