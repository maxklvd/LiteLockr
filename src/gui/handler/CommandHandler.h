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

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <functional>

namespace litelockr {

// member function pointers

template<typename C>
using CommandHandlerPtrVV = void (C::*)(); // VV = void(void)

template<typename C>
using CommandHandlerPtrVWH = void (C::*)(WORD, HWND);

template<typename C>
using CommandHandlerPtrLWWHB = LRESULT (C::*)(WORD, WORD, HWND, BOOL&);

//--------------------------------------------------------------
using CommandCallback = std::function<LRESULT(WORD, WORD, HWND, BOOL&)>;


struct CommandHandlerContainer {
    std::map<WORD, CommandCallback> callbacks;

    template<typename F>
    requires std::invocable<F>
    void addHandler(WORD id, F callback) {
        callbacks[id] = [callback](WORD, WORD, HWND, BOOL&) -> LRESULT {
            std::invoke(callback);
            return 0L;
        };
    }

    template<typename F>
    requires std::invocable<F, WORD, HWND>
    void addHandler(WORD id, F callback) {
        callbacks[id] = [callback](WORD wNotifyCode, WORD, HWND hWndCtl, BOOL&) -> LRESULT {
            std::invoke(callback, wNotifyCode, hWndCtl);
            return 0L;
        };
    }

    template<typename F>
    requires std::invocable<F, WORD, WORD, HWND, BOOL&>
    void addHandler(WORD id, F callback) {
        callbacks[id] = [callback](WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) -> LRESULT {
            return std::invoke(callback, wNotifyCode, wID, hWndCtl, bHandled);
        };
    }

    template<typename C>
    void addHandler(WORD id, C *object, CommandHandlerPtrVV<C> callback) {
        callbacks[id] = [object, callback](WORD, WORD, HWND, BOOL&) -> LRESULT {
            std::invoke(callback, object);
            return 0L;
        };
    }

    template<typename C>
    void addHandler(WORD id, C *object, CommandHandlerPtrVWH<C> callback) {
        callbacks[id] = [object, callback](WORD wNotifyCode, WORD, HWND hWndCtl, BOOL&) -> LRESULT {
            std::invoke(callback, object, wNotifyCode, hWndCtl);
            return 0L;
        };
    }

    template<typename C>
    void addHandler(WORD id, C *object, CommandHandlerPtrLWWHB<C> callback) {
        callbacks[id] = [object, callback](WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) -> LRESULT {
            return std::invoke(callback, object, wNotifyCode, wID, hWndCtl, bHandled);
        };
    }
};


} // namespace litelockr

#endif // COMMAND_HANDLER_H
