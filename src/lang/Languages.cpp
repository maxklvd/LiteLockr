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

#include "Languages.h"

#include <string>
#include <filesystem>

#include <boost/algorithm/string.hpp>
#include <simpleini/SimpleIni.h>
#include "lang/LanguageList.h"
#include "log/Logger.h"
#include "sys/Process.h"

namespace litelockr {

namespace fs = std::filesystem;

void Languages::initialize() {
    languages_.clear();
    languages_[L"en"] = {L"English", L"English"};

    fs::path langPath = Process::modulePath();
    langPath /= LANGUAGE_DIR;

    fs::directory_entry dirEntry{langPath};
    if (!dirEntry.exists()) {
        LOG_DEBUG(L"%s directory was not found: %s", LANGUAGE_DIR, langPath.c_str());
        return;
    }

    for (auto&& langCode: LanguageList::getSupportedLanguages()) {
        std::wstring iniPath = langPath;
        iniPath += fs::path::preferred_separator;
        iniPath += langCode;
        iniPath += INI_EXT;

        CSimpleIniW ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile(iniPath.c_str());
        if (rc == SI_OK) {
            std::wstring langName = ini.GetValue(SECTION_LANGUAGE, LANGUAGE_NAME, L"");
            std::wstring engLangName = ini.GetValue(SECTION_LANGUAGE, ENGLISH_LANGUAGE_NAME, L"");

            if (langName.empty()) {
                LOG_VERBOSE(L"Language: %s%s: no '%s' option", langCode.c_str(), INI_EXT, LANGUAGE_NAME);
            } else if (engLangName.empty()) {
                LOG_VERBOSE(L"Language: %s%s: no '%s' option", langCode.c_str(), INI_EXT, ENGLISH_LANGUAGE_NAME);
            } else {
                languages_[langCode] = {engLangName, langName};
                LOG_VERBOSE(L"Language: %s%s: '%s' (%s) was added", langCode.c_str(), INI_EXT,
                            engLangName.c_str(), langName.c_str());
            }
        } else {
            LOG_VERBOSE(L"Language: %s%s was not found", langCode.c_str(), INI_EXT);
        }
    }
}

std::wstring Languages::getLocaleLanguage() {
    constexpr int BUF_SIZE = LOCALE_NAME_MAX_LENGTH;
    wchar_t buf[BUF_SIZE] = {0};
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SNAME, buf, BUF_SIZE);
    return buf;
}

} // namespace litelockr
