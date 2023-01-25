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

#ifndef PROPERTIES_DLG_H
#define PROPERTIES_DLG_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>

#include "gui/Dialog.h"
#include "ini/ApplicationRecord.h"
#include "lock/taskbar/TaskbarButtonEnumeration.h"
#include "lock/ui/ButtonPreview.h"
#include "sys/AppClock.h"

namespace litelockr {

class PropertiesDlg: public Dialog {
public:
    PropertiesDlg();

    void show(HWND hWndParent, ApplicationRecord& appRecord);
    void onLockPreviewEnded();

protected:
    constexpr static int DETECT_AUTOMATICALLY_IDX = 0;

    BOOL onInitDialog() override;
    void onShowDialog() override {}
    void onOK();
    void onCancel();
    void onSearchExecutableInAutomationId(WORD /*wNotifyCode*/, HWND hWndCtrl);
    void onAutomationIdPrev();
    void onAutomationIdNext();
    void onNameCombo(WORD wNotifyCode, HWND hWndCtrl);
    void onNamePrev();
    void onNameNext();
    void onCheckButton(WORD /*wNotifyCode*/, HWND hWndCtrl);
    void onNameComboChanged();
    void onTimer();

private:
    void localizeDialog();
    void enableAutomationIdGroup(bool enabled);
    void enableNameGroup(bool enabled);
    bool isAutoDetectSelected();
    void saveAppRecord();

    void showAutomationIdPreview();
    void showNamePreview();

    std::optional<std::reference_wrapper<ApplicationRecord>> appRecord_{};

    ApplicationRecord& appRecord() {
        assert(appRecord_.has_value());
        return appRecord_.value();
    }

    ApplicationRecord initialAppRecord_;

    std::wstring appName_;
    std::wstring detectedAppNames_;
    std::unordered_set<std::wstring> detectedAppNameSet_;

    TaskbarButtonEnumeration buttonEnumeration_;
    ButtonPreview buttonPreview_;

    TimeCounter debounceDelay_{std::chrono::seconds(1), false};
    HWND hWndNameEditor_ = nullptr;

    constexpr static int TIMER_ID = 1;
};

} // namespace litelockr

#endif // PROPERTIES_DLG_H
