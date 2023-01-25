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

#ifndef NOTIFY_HANDLER_H
#define NOTIFY_HANDLER_H

#include <functional>

namespace litelockr {

// member function pointers

template<typename C>
using NotifyHandlerPtrVV = void (C::*)(); // VV = void(void)

template<typename C>
using NotifyHandlerPtrLL = LRESULT (C::*)(LPNMHDR);

template<typename C>
using NotifyHandlerPtrLILB = LRESULT (C::*)(int, LPNMHDR, BOOL&);

//--------------------------------------------------------------
using NotifyCallback = std::function<LRESULT(int, LPNMHDR, BOOL&)>;


struct NotifyHandlerContainer {
    std::map<UINT, NotifyCallback> callbacks;

    template<typename F>
    requires std::invocable<F>
    void addHandler(UINT id, F callback) {
        callbacks[id] = [callback](int, LPNMHDR, BOOL&) -> LRESULT {
            std::invoke(callback);
            return 0L;
        };
    }

    template<typename F>
    requires std::invocable<F, LPNMHDR>
    void addHandler(UINT id, F callback) {
        callbacks[id] = [callback](int, LPNMHDR lParam, BOOL&) -> LRESULT {
            return std::invoke(callback, lParam);
        };
    }

    template<typename F>
    requires std::invocable<F, int, LPNMHDR, BOOL&>
    void addHandler(UINT id, F callback) {
        callbacks[id] = [callback](int wParam, LPNMHDR lParam, BOOL& bHandled) -> LRESULT {
            return std::invoke(callback, wParam, lParam, bHandled);
        };
    }

    template<typename C>
    void addHandler(UINT id, C *object, NotifyHandlerPtrVV<C> callback) {
        callbacks[id] = [object, callback](int, LPNMHDR, BOOL&) -> LRESULT {
            std::invoke(callback, object);
            return 0L;
        };
    }

    template<typename C>
    void addHandler(UINT id, C *object, NotifyHandlerPtrLL<C> callback) {
        callbacks[id] = [object, callback](int, LPNMHDR lParam, BOOL&) -> LRESULT {
            return std::invoke(callback, object, lParam);
        };
    }

    template<typename C>
    void addHandler(UINT id, C *object, NotifyHandlerPtrLILB<C> callback) {
        callbacks[id] = [object, callback](int wParam, LPNMHDR lParam, BOOL& bHandled) -> LRESULT {
            return std::invoke(callback, object, wParam, lParam, bHandled);
        };
    }
};


} // namespace litelockr

#endif // NOTIFY_HANDLER_H
