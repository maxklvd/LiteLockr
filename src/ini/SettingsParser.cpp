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

#include "SettingsParser.h"

namespace litelockr {

SettingsParser::SettingsParser(const std::wstring& path) : path_(path) {
    ini_.SetUnicode();
    ini_.SetMultiKey();
}

bool SettingsParser::loadFile() {
    SI_Error rc = ini_.LoadFile(path_.c_str());
    assert(rc == SI_OK);
    return rc == SI_OK;
}

bool SettingsParser::saveFile() const {
    SI_Error rc = ini_.SaveFile(path_.c_str());
    assert(rc == SI_OK);
    return rc == SI_OK;
}

bool SettingsParser::fileExists() const {
    return _waccess(path_.c_str(), 0) == 0;
}

} // namespace litelockr
