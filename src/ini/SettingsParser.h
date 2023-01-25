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

#ifndef SETTINGS_PARSER_H
#define SETTINGS_PARSER_H

#include <simpleini/SimpleIni.h>
#include "ini/Property.h"

namespace litelockr {

class SettingsParser {
public:
    explicit SettingsParser(const std::wstring& path);

    template<class T>
    void loadProperty(T& property, bool save) {
        if (save && !property.saveToIni()) {
            return;
        }

        auto section = property.section();
        auto key = property.key();

        if (save) {
            std::wstringstream buf;
            property.invokeSerializer(property.value_, buf);
            ini_.SetValue(section, key, buf.str().c_str(), nullptr, true);
        } else {
            const auto val = ini_.GetValue(section, key);
            if (val) {
                std::wstringstream buf;
                buf << val;
                property.invokeDeserializer(property.value_, buf);
            } else {
                property.reset();
            }
        }
    }

    template<class T>
    void loadListProperty(T& property, bool save) {
        if (save && !property.saveToIni()) {
            return;
        }

        auto section = property.section();
        auto key = property.key();

        if (save) {
            ini_.Delete(property.section(), property.key(), true);

            for (auto&& val: property.value_) {
                std::wstringstream buf;
                property.invokeSerializer(val, buf);
                ini_.SetValue(property.section(), property.key(), buf.str().c_str(), nullptr, false);
            }
        } else {
            CSimpleIniW::TNamesDepend vals;
            ini_.GetAllValues(section, key, vals);

            vals.sort(CSimpleIniW::Entry::LoadOrder());

            property.value_.clear();

            CSimpleIniW::TNamesDepend::const_iterator it;
            for (it = vals.begin(); it != vals.end(); ++it) {
                std::wstring result;
                std::wstringstream buf;
                buf << it->pItem;
                property.invokeDeserializer(result, buf);
                property.value_.push_back(result);
            }
        }
    }

    void loadProperties(const PropertyVec& properties, bool save = false) {
        for (auto&& property: properties) {
            switch (property.get().type()) {
                case PropertyType::Bool: {
                    auto& prop = static_cast<BoolProperty&>(property.get()); // downcast
                    loadProperty(prop, save);
                    break;
                }
                case PropertyType::Long: {
                    auto& prop = static_cast<LongProperty&>(property.get()); // downcast
                    loadProperty(prop, save);
                    break;
                }
                case PropertyType::String: {
                    auto& prop = static_cast<StringProperty&>(property.get()); // downcast
                    loadProperty(prop, save);
                    break;
                }
                case PropertyType::StringList: {
                    auto& prop = static_cast<StringListProperty&>(property.get()); // downcast
                    loadListProperty(prop, save);
                    break;
                }
                default:
                    assert(false);
                    break;
            }
        }
    }

    void saveProperties(const PropertyVec& properties) {
        loadProperties(properties, true);
    }

private:
    CSimpleIniW ini_;
    std::wstring path_;

public:
    [[nodiscard]] bool fileExists() const;
    [[nodiscard]] bool loadFile();
    [[nodiscard]] bool saveFile() const;
};

} // namespace litelockr

#endif // SETTINGS_PARSER_H
