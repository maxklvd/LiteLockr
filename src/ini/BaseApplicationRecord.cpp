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

#include "BaseApplicationRecord.h"

#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include "log/Logger.h"

namespace litelockr {

void BaseApplicationRecord::fromJSON(const std::wstring& jsonString) {
    reset();

    assert(jsonString.at(0) == L'{'); // is it a JSON?

    std::wstringstream buf;
    buf << jsonString;

    boost::property_tree::wptree json;
    try {
        boost::property_tree::read_json(buf, json);
        path_ = json.get<std::wstring>(Path);
        findByName_ = json.get<bool>(FindByName, true);
        findByAutomationId_ = json.get<bool>(FindByAutomationId, true);
        buttonNamePattern_ = json.get<std::wstring>(ButtonNamePattern, L"");
        buttonAutomationId_ = json.get<std::wstring>(ButtonAutomationId, L"");
        searchExecutableInAutomationId_ = json.get<bool>(SearchExecutableInAutomationId, true);
    } catch (...) {
        LOG_ERROR(L"JSON reading error occurred: %s", jsonString.c_str());
    }
}

std::wstring BaseApplicationRecord::toJSON() const {
    try {
        boost::property_tree::wptree json;
        json.put(Path, path_);
        json.put(FindByName, findByName_);
        json.put(FindByAutomationId, findByAutomationId_);
        json.put(ButtonNamePattern, buttonNamePattern_);
        json.put(ButtonAutomationId, buttonAutomationId_);
        json.put(SearchExecutableInAutomationId, searchExecutableInAutomationId_);

        std::wstringstream ss;
        boost::property_tree::write_json(ss, json, false);
        return ss.str();
    } catch (...) {
        LOG_ERROR(L"JSON writing error occurred");
    }
    return L"";
}

void BaseApplicationRecord::fromPath(const std::wstring& path) {
    reset();

    path_ = path;
    std::replace(path_.begin(), path_.end(), L'/', L'\\');
    path_.erase(std::remove(path_.begin(), path_.end(), L'"'), path_.end());
}

void BaseApplicationRecord::reset() {
    path_.clear();
    findByName_ = false;
    findByAutomationId_ = false;
    buttonNamePattern_.clear();
    buttonAutomationId_.clear();
    searchExecutableInAutomationId_ = false;
}

} // namespace litelockr
