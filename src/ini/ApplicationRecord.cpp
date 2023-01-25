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

#include "ApplicationRecord.h"

#include <unordered_map>

#include "sys/StringUtils.h"
#include "sys/Executable.h"

namespace litelockr {

void ApplicationRecord::fromJSON(const std::wstring& jsonString) {
    BaseApplicationRecord::fromJSON(jsonString);
    exeName_ = Executable::getFileName(path_);
    detectAppNames(path_);
}

void ApplicationRecord::fromPath(const std::wstring& path) {
    BaseApplicationRecord::fromPath(path);
    exeName_ = Executable::getFileName(path_);
    detectAppNames(path_);
}

void ApplicationRecord::detectAppNames(const std::wstring& path) {
    std::unordered_map<std::wstring, std::wstring> map;
    std::wstring buf;

    auto addIfUnique = [&map](const std::wstring& val) {
        if (!val.empty()) {
            auto upper = StringUtils::toUpperCase(val);
            if (map.find(upper) == map.end()) {
                map[upper] = val;
            }
        }
    };

    //
    // File Description
    //
    addIfUnique(Executable::getFileDescription(path));

    //
    // Friendly Name
    //
    addIfUnique(Executable::getFriendlyName(path));

    //
    // Internal Name
    //
    addIfUnique(Executable::getInternalName(path));

    //
    // Product Name
    //
    addIfUnique(Executable::getProductName(path));

    //
    // Executable Name
    //
    addIfUnique(Executable::getExecutableName(path));

    detectedAppNames_.clear();
    for (const auto& [_, value]: map) {
        detectedAppNames_.push_back(value);
    }
}

} // namespace litelockr
