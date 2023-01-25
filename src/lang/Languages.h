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

#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <map>
#include <string>
#include <utility>

namespace litelockr {

using LanguageMap = std::map<std::wstring, std::pair<std::wstring, std::wstring>, std::less<>>;

class Languages {
public:
    static constexpr auto LANGUAGE_DIR = L"lang";
    static constexpr auto INI_EXT = L".ini";

    static constexpr auto AUTODETECT = L"auto";
    static constexpr auto DEFAULT_LANGUAGE = L"en";

    static constexpr auto SECTION_LANGUAGE = L"Language";
    static constexpr auto SECTION_MESSAGES = L"Messages";

    static constexpr auto LANGUAGE_NAME = L"LanguageName";
    static constexpr auto ENGLISH_LANGUAGE_NAME = L"EnglishLanguageName";

    void initialize();

    [[nodiscard]] const LanguageMap& languages() const {
        return languages_;
    }

    static std::wstring getLocaleLanguage();

private:
    LanguageMap languages_;
};

} // namespace litelockr

#endif // LANGUAGES_H
