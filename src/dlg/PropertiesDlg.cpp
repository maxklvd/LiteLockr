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

#include "PropertiesDlg.h"

#include <future>

#include <windowsx.h>
#include "app/event/AppEvent.h"
#include "ini/SettingsData.h"
#include "lock/MousePositionValidator.h"
#include "lock/taskbar/TaskbarButtonDetector.h"
#include "lock/uia/UIAutomationHelper.h"
#include "sys/Executable.h"
#include "sys/StringUtils.h"

#include "res/Resources.h"

namespace litelockr {

constexpr UINT WMU_NAME_COMBO_CHANGED = WM_APP + 100;

PropertiesDlg::PropertiesDlg() : Dialog(IDD_PROPERTIES) {
    onCommand(IDOK, &PropertiesDlg::onOK);
    onCommand(IDCANCEL, &PropertiesDlg::onCancel);
    onCommand(IDC_FIND_BY_AUTOMATIONID_CHECK, [this]() {
        bool enabled = Button_GetCheck(dlgItem(IDC_FIND_BY_AUTOMATIONID_CHECK));
        enableAutomationIdGroup(enabled);
        if (enabled) {
            showAutomationIdPreview();
        }
    });
    onCommand(IDC_FIND_BY_NAME_CHECK, [this]() {
        bool enabled = Button_GetCheck(dlgItem(IDC_FIND_BY_NAME_CHECK));
        enableNameGroup(enabled);
        if (enabled) {
            showNamePreview();
        }
    });
    onCommand(IDC_SEARCH_EXECUTABLE_IN_AUTOMATIONID, &PropertiesDlg::onSearchExecutableInAutomationId);
    onCommand(IDC_AUTOMATIONID_PREV, &PropertiesDlg::onAutomationIdPrev);
    onCommand(IDC_AUTOMATIONID_NEXT, &PropertiesDlg::onAutomationIdNext);
    onCommand(IDC_NAME_COMBO, &PropertiesDlg::onNameCombo);
    onCommand(IDC_NAME_PREV, &PropertiesDlg::onNamePrev);
    onCommand(IDC_NAME_NEXT, &PropertiesDlg::onNameNext);
    onCommand(IDC_CHECK, &PropertiesDlg::onCheckButton);
    onMessage(WMU_NAME_COMBO_CHANGED, &PropertiesDlg::onNameComboChanged);
    onMessage(WM_TIMER, &PropertiesDlg::onTimer);
}

BOOL PropertiesDlg::onInitDialog() {
    localizeDialog();

    HWND hWndNameCombo = dlgItem(IDC_NAME_COMBO);
    COMBOBOXINFO info = {sizeof(info)};
    if (GetComboBoxInfo(hWndNameCombo, &info)) {
        assert(info.hwndItem);
        hWndNameEditor_ = info.hwndItem;
    }

    const auto& app = appRecord();

    Button_SetCheck(dlgItem(IDC_FIND_BY_AUTOMATIONID_CHECK), app.findByAutomationId());
    Edit_SetText(dlgItem(IDC_AUTOMATIONID_TEXT), app.buttonAutomationId().c_str());
    Button_SetCheck(dlgItem(IDC_SEARCH_EXECUTABLE_IN_AUTOMATIONID), app.searchExecutableInAutomationId());

    appName_ = app.buttonNamePattern();
    detectedAppNames_.clear();
    detectedAppNameSet_.clear();

    //
    // Fill up the combobox
    //
    ComboBox_AddString(hWndNameCombo, Messages::get(MessageId::DetectAutomatically).c_str());
    for (const auto& name: app.detectedAppNames()) {
        ComboBox_AddString(hWndNameCombo, name.c_str());
        detectedAppNames_.append(L"\u00A0'");
        detectedAppNames_.append(name);
        detectedAppNames_.append(L"'");

        detectedAppNameSet_.insert(StringUtils::prepareSearchString(name));
    }

    Button_SetCheck(dlgItem(IDC_FIND_BY_NAME_CHECK), app.findByName());

    bool autoDetected = app.buttonNamePattern().empty();
    if (autoDetected) {
        ComboBox_SetCurSel(hWndNameCombo, DETECT_AUTOMATICALLY_IDX);
        ComboBox_SetText(hWndNameCombo, detectedAppNames_.c_str());
    } else {
        ComboBox_SetText(hWndNameCombo, appName_.c_str());
    }

    enableAutomationIdGroup(app.findByAutomationId());
    enableNameGroup(app.findByName());

    SetTimer(hWnd, TIMER_ID, 200, nullptr);

    return TRUE;
}

void PropertiesDlg::onOK() {
    saveAppRecord();

    KillTimer(hWnd, TIMER_ID);
    buttonPreview_._destroy();

    destroyDialog();
}

void PropertiesDlg::onCancel() {
    appRecord() = initialAppRecord_;

    KillTimer(hWnd, TIMER_ID);
    buttonPreview_._destroy();
    destroyDialog();
}

void PropertiesDlg::enableAutomationIdGroup(bool enabled) {
    EnableWindow(dlgItem(IDC_AUTOMATIONID_TEXT), enabled);
    EnableWindow(dlgItem(IDC_AUTOMATIONID_PREV), enabled);
    EnableWindow(dlgItem(IDC_AUTOMATIONID_NEXT), enabled);
    EnableWindow(dlgItem(IDC_SEARCH_EXECUTABLE_IN_AUTOMATIONID), enabled);
}

void PropertiesDlg::enableNameGroup(bool enabled) {
    HWND hWndCombo = dlgItem(IDC_NAME_COMBO);
    EnableWindow(hWndCombo, enabled);
    EnableWindow(dlgItem(IDC_NAME_PREV), enabled);
    EnableWindow(dlgItem(IDC_NAME_NEXT), enabled);

    Edit_SetReadOnly(hWndNameEditor_, isAutoDetectSelected());
    ComboBox_SetEditSel(hWndCombo, -1, -1); // drops a text selection
}

bool PropertiesDlg::isAutoDetectSelected() {
    HWND hWndCombo = dlgItem(IDC_NAME_COMBO);
    constexpr int BUF_SIZE = 256;
    wchar_t buf[BUF_SIZE] = {0};
    ComboBox_GetText(hWndCombo, buf, BUF_SIZE);
    return detectedAppNames_ == buf;
}

void PropertiesDlg::localizeDialog() {
    auto title = Executable::getFileName(appRecord().path());
    title.append(L" ");
    title.append(Messages::get(MessageId::Properties));
    SetWindowText(hWnd, title.c_str());

    setLocalizedText(IDC_TASKBAR_BUT_LABEL, MessageId::TaskbarButton);
    setLocalizedText(IDC_FIND_BY_NAME_CHECK, MessageId::FindByName);
    setLocalizedText(IDC_FIND_BY_AUTOMATIONID_CHECK, MessageId::FindByAutomationId);
    setLocalizedText(IDC_SEARCH_EXECUTABLE_IN_AUTOMATIONID, MessageId::SearchForExecutableName);
    setLocalizedText(IDC_CHECK, MessageId::Check);
    setLocalizedText(IDOK, MessageId::OK);
    setLocalizedText(IDCANCEL, MessageId::Cancel);
}

void PropertiesDlg::onSearchExecutableInAutomationId(WORD /*wNotifyCode*/, HWND hWndCtrl) {
    if (Button_GetCheck(hWndCtrl)) {
        auto result = TaskbarButtonDetector::findByExecutableInAutomationId(appRecord().exeName());
        buttonPreview_.show(result, PROP_AUTOMATION_ID);
    }
}

void PropertiesDlg::onNameCombo(WORD wNotifyCode, HWND hWndCtrl) {
    switch (wNotifyCode) {
        case CBN_EDITCHANGE:
            debounceDelay_.setEnabled(true);
            break;
        case CBN_SELCHANGE:
            PostMessage(hWnd, WMU_NAME_COMBO_CHANGED, 0, 0L);
            break;
        case CBN_SETFOCUS:
            ComboBox_SetEditSel(hWndCtrl, -1, -1); // drops a text selection
            break;
        default:
            // nothing to do
            break;
    }
}

void PropertiesDlg::onAutomationIdPrev() {
    auto propOpt = buttonEnumeration_.prev();
    if (propOpt) {
        const auto& prop = propOpt->get();
        Edit_SetText(dlgItem(IDC_AUTOMATIONID_TEXT), prop.automationId.c_str());
        buttonPreview_.show(prop, PROP_AUTOMATION_ID);
        return;
    }

    Edit_SetText(dlgItem(IDC_AUTOMATIONID_TEXT), L"");
    buttonPreview_._destroy();
}


void PropertiesDlg::onAutomationIdNext() {
    auto propOpt = buttonEnumeration_.next();
    if (propOpt) {
        const auto& prop = propOpt->get();
        Edit_SetText(dlgItem(IDC_AUTOMATIONID_TEXT), prop.automationId.c_str());
        buttonPreview_.show(prop, PROP_AUTOMATION_ID);
        return;
    }

    Edit_SetText(dlgItem(IDC_AUTOMATIONID_TEXT), L"");
}

void PropertiesDlg::onNamePrev() {
    Edit_SetReadOnly(hWndNameEditor_, false);

    auto propOpt = buttonEnumeration_.prev();
    if (propOpt) {
        const auto& prop = propOpt->get();
        ComboBox_SetText(dlgItem(IDC_NAME_COMBO), prop.name.c_str());
        buttonPreview_.show(prop, PROP_NAME);
        return;
    }

    ComboBox_SetText(dlgItem(IDC_NAME_COMBO), L"");
    buttonPreview_._destroy();
}

void PropertiesDlg::onNameNext() {
    Edit_SetReadOnly(hWndNameEditor_, false);

    auto propOpt = buttonEnumeration_.next();
    if (propOpt) {
        const auto& prop = propOpt->get();
        ComboBox_SetText(dlgItem(IDC_NAME_COMBO), prop.name.c_str());
        buttonPreview_.show(prop, PROP_NAME);
        return;
    }

    ComboBox_SetText(dlgItem(IDC_NAME_COMBO), L"");
}

void PropertiesDlg::onCheckButton(WORD /*wNotifyCode*/, HWND hWndCtrl) {
    EnableWindow(hWndCtrl, FALSE);

    saveAppRecord();

    auto& settings = SettingsData::instance();
    for (auto& app: settings.getAllowedApps()) {
        app.enabled = (app == appRecord());
    }

    buttonPreview_._destroy();

    AppEvent e(AppEvent::SHOW_LOCK_INFORMATION);
    e.data = MousePositionValidator::CHECK_APP;
    AppEvents::send(e);
}

void PropertiesDlg::show(HWND hWndParent, ApplicationRecord& appRecord) {
    if (isVisible()) {
        SetActiveWindow(hWnd);
    } else {
        assert(hWndParent);
        appRecord_ = appRecord;
        initialAppRecord_ = appRecord;
        buttonEnumeration_.reset();
        showDialog(hWndParent);
    }
}

void PropertiesDlg::onLockPreviewEnded() {
    auto& settings = SettingsData::instance();
    for (auto& app: settings.getAllowedApps()) {
        app.enabled = true;
    }
    EnableWindow(dlgItem(IDC_CHECK), TRUE);
    SetForegroundWindow(hWnd);
}

void PropertiesDlg::onNameComboChanged() {
    HWND hWndCtrl = dlgItem(IDC_NAME_COMBO);
    if (ComboBox_GetCurSel(hWndCtrl) == DETECT_AUTOMATICALLY_IDX) {
        ComboBox_SetText(hWndCtrl, detectedAppNames_.c_str());
    }
    enableNameGroup(true);
    showNamePreview();
}

void PropertiesDlg::onTimer() {
    buttonPreview_.tick();

    if (debounceDelay_.timeout()) {
        debounceDelay_.setEnabled(false);
        PostMessage(hWnd, WMU_NAME_COMBO_CHANGED, 0, 0L);
    }
}

void PropertiesDlg::saveAppRecord() {
    //
    // save Find By AutomationId option
    //
    appRecord().findByAutomationId_ = Button_GetCheck(dlgItem(IDC_FIND_BY_AUTOMATIONID_CHECK));

    //
    // save AutomationId value
    //
    constexpr int BUF_SIZE = 256;
    wchar_t buf[BUF_SIZE] = {0};
    Edit_GetText(dlgItem(IDC_AUTOMATIONID_TEXT), buf, BUF_SIZE);
    appRecord().buttonAutomationId_ = buf;

    appRecord().searchExecutableInAutomationId_ = Button_GetCheck(dlgItem(IDC_SEARCH_EXECUTABLE_IN_AUTOMATIONID));

    //
    // save Find By Name option
    //
    appRecord().findByName_ = Button_GetCheck(dlgItem(IDC_FIND_BY_NAME_CHECK));

    //
    // save Name value
    //
    buf[0] = L'\0';
    ComboBox_GetText(dlgItem(IDC_NAME_COMBO), buf, BUF_SIZE);
    if (detectedAppNames_ == buf) { // auto-detected names
        appRecord().buttonNamePattern_ = L"";
    } else {
        appRecord().buttonNamePattern_ = buf;
        if (appRecord().buttonNamePattern_.empty()) {
            ComboBox_SetText(dlgItem(IDC_NAME_COMBO), detectedAppNames_.c_str());
            Edit_SetReadOnly(hWndNameEditor_, true);
        }
    }
}

void PropertiesDlg::showAutomationIdPreview() {
    constexpr int BUF_SIZE = 256;
    wchar_t buf[BUF_SIZE] = {0};
    Edit_GetText(dlgItem(IDC_AUTOMATIONID_TEXT), buf, BUF_SIZE);

    std::wstring id = buf;
    if (!id.empty()) {
        auto result = TaskbarButtonDetector::findByAutomationId(id);
        buttonPreview_.show(result, PROP_AUTOMATION_ID);
    }
}

void PropertiesDlg::showNamePreview() {
    HWND hWndCtrl = dlgItem(IDC_NAME_COMBO);

    std::unordered_set<std::wstring> names;
    if (ComboBox_GetCurSel(hWndCtrl) == DETECT_AUTOMATICALLY_IDX) {
        names = detectedAppNameSet_;
    } else {
        constexpr int BUF_SIZE = 256;
        wchar_t buf[BUF_SIZE] = {0};
        ComboBox_GetText(hWndCtrl, buf, BUF_SIZE);

        auto upper = StringUtils::prepareSearchString(buf);
        if (!upper.empty()) {
            names.insert(upper);
        }
    }

    if (!names.empty()) {
        auto future = std::async(std::launch::async, [&names]() {
            UIAutomationHelper uia;
            return uia.findTaskbarButton(PROP_NAME, names, STARTS_WITH | ENDS_WITH);
        });
        auto result = future.get();
        buttonPreview_.show(result, PROP_NAME);
    }
}

} // namespace litelockr
