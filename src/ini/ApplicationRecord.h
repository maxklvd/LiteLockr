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

#ifndef APPLICATION_RECORD_H
#define APPLICATION_RECORD_H

#include <vector>

#include "ini/BaseApplicationRecord.h"

namespace litelockr {

class ApplicationRecord: public BaseApplicationRecord {
public:
    bool persistent = true;
    bool enabled = true;

    ApplicationRecord() = default;

    [[nodiscard]] const std::wstring& exeName() const {
        return exeName_;
    }

    [[nodiscard]] const std::wstring& buttonNamePattern() const {
        return buttonNamePattern_;
    }

    [[nodiscard]] const std::vector<std::wstring>& detectedAppNames() const {
        return detectedAppNames_;
    }

    [[nodiscard]] std::vector<std::wstring> getButtonNames() const {
        if (buttonNamePattern_.empty()) {
            return detectedAppNames_;
        } else {
            return {buttonNamePattern_};
        }
    }

    void fromJSON(const std::wstring& jsonString) override;
    void fromPath(const std::wstring& path) override;

    bool operator==(const ApplicationRecord& other) const {
        return path_ == other.path_;
    }

private:
    std::wstring exeName_;
    std::vector<std::wstring> detectedAppNames_;

    void detectAppNames(const std::wstring& path);

    friend class PropertiesDlg;

    friend class PageGeneralDlg;

    friend class TaskbarButtonDetector;
};

using ApplicationRecordVec = std::vector<ApplicationRecord>;

} // namespace litelockr

#endif // APPLICATION_RECORD_H
