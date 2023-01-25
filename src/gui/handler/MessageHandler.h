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

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <functional>

namespace litelockr {

// member function pointers

template<typename C>
using MessageHandlerPtrVV = void (C::*)(); // VV = void(void)

template<typename C>
using MessageHandlerPtrVWL = void (C::*)(WPARAM, LPARAM);

template<typename C>
using MessageHandlerPtrLUWLB = LRESULT (C::*)(UINT, WPARAM, LPARAM, BOOL&);

//--------------------------------------------------------------
using MessageCallback = std::function<LRESULT(UINT, WPARAM, LPARAM, BOOL&)>;

struct MessageHandlerContainer {
    std::map<UINT, MessageCallback> callbacks;

    template<typename F>
    requires std::invocable<F>
    void addHandler(UINT msg, F callback) {
        callbacks[msg] = [callback](UINT, WPARAM, LPARAM, BOOL&) -> LRESULT {
            std::invoke(callback);
            return 0L;
        };
    }

    template<typename F>
    requires std::invocable<F, WPARAM, LPARAM>
    void addHandler(UINT msg, F callback) {
        callbacks[msg] = [callback](UINT, WPARAM wParam, LPARAM lParam, BOOL&) -> LRESULT {
            std::invoke(callback, wParam, lParam);
            return 0L;
        };
    }

    template<typename F>
    requires std::invocable<F, UINT, WPARAM, LPARAM, BOOL&>
    void addHandler(UINT msg, F callback) {
        callbacks[msg] = [callback](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) -> LRESULT {
            return std::invoke(callback, uMsg, wParam, lParam, bHandled);
        };
    }

    template<typename C>
    void addHandler(UINT msg, C *object, MessageHandlerPtrVV<C> callback) {
        callbacks[msg] = [object, callback](UINT, WPARAM, LPARAM, BOOL&) -> LRESULT {
            std::invoke(callback, object);
            return 0L;
        };
    }

    template<typename C>
    void addHandler(UINT msg, C *object, MessageHandlerPtrVWL<C> callback) {
        callbacks[msg] = [object, callback](UINT, WPARAM wParam, LPARAM lParam, BOOL&) -> LRESULT {
            std::invoke(callback, object, wParam, lParam);
            return 0L;
        };
    }

    template<typename C>
    void addHandler(UINT msg, C *object, MessageHandlerPtrLUWLB<C> callback) {
        callbacks[msg] = [object, callback](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) -> LRESULT {
            return std::invoke(callback, object, uMsg, wParam, lParam, bHandled);
        };
    }
};


} // namespace litelockr

#endif // MESSAGE_HANDLER_H
