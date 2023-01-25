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

#ifndef BASE_APPLICATION_RECORD_H
#define BASE_APPLICATION_RECORD_H

#include <string>

namespace litelockr {

class BaseApplicationRecord {
public:
    [[nodiscard]] const std::wstring& path() const {
        return path_;
    }

    [[nodiscard]] bool findByName() const {
        return findByName_;
    }

    [[nodiscard]] bool findByAutomationId() const {
        return findByAutomationId_;
    }

    [[nodiscard]] const std::wstring& buttonAutomationId() const {
        return buttonAutomationId_;
    }

    [[nodiscard]] bool searchExecutableInAutomationId() const {
        return searchExecutableInAutomationId_;
    }

    virtual void fromJSON(const std::wstring& jsonString);
    [[nodiscard]] std::wstring toJSON() const;

    virtual void fromPath(const std::wstring& path);

    void reset();

protected:
    std::wstring path_;
    bool findByName_ = false;
    bool findByAutomationId_ = false;
    std::wstring buttonNamePattern_;
    std::wstring buttonAutomationId_;
    bool searchExecutableInAutomationId_ = false;

    constexpr static auto Path = L"path";
    constexpr static auto FindByName = L"findByName";
    constexpr static auto FindByAutomationId = L"findByAutomationId";
    constexpr static auto ButtonNamePattern = L"buttonNamePattern";
    constexpr static auto ButtonAutomationId = L"buttonAutomationId";
    constexpr static auto SearchExecutableInAutomationId = L"searchExecutableInAutomationId";
};

} // namespace litelockr

#endif // BASE_APPLICATION_RECORD_H
