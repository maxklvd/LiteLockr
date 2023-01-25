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

#ifndef PROPERTY_H
#define PROPERTY_H

#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>

namespace litelockr {

enum class PropertyType {
    None,
    Bool,
    Long,
    String,
    StringList,
};

class Property {
public:
    Property(PropertyType type, const wchar_t *section, const wchar_t *key)
            : type_(type),
              section_(section),
              key_(key) {}

    virtual ~Property() = default;

    [[nodiscard]] PropertyType type() const {
        assert(type_ != PropertyType::None);
        return type_;
    }

    [[nodiscard]] const wchar_t *section() const {
        assert(section_);
        return section_;
    }

    [[nodiscard]] const wchar_t *key() const {
        assert(key_);
        return key_;
    }

    virtual void reset() = 0;

protected:
    PropertyType type_;
    const wchar_t *section_;
    const wchar_t *key_;
};

using PropertyVec = std::vector<std::reference_wrapper<Property>>;

template<class T, typename VT = T>
class AbstractProperty: public Property {
public:
    using Type = T;
    using ValueType = VT;

    AbstractProperty(PropertyType type, const wchar_t *section, const wchar_t *key, T defaultValue, bool saveToIni)
            : Property(type, section, key),
              defaultValue_(defaultValue),
              value_(defaultValue),
              saveToIni_(saveToIni) {}

    [[nodiscard]] const T& value() const {
        return value_;
    }

    void setValue(T value) {
        value_ = value;
    }

    [[nodiscard]] const T& defaultValue() const {
        return defaultValue_;
    }

    void reset() override {
        value_ = defaultValue_;
    }

    [[maybe_unused]] [[nodiscard]] bool saveToIni() const {
        return saveToIni_;
    }

    using PropertySerializer = std::function<void(const ValueType& val, std::wstringstream& out)>;
    using PropertyDeserializer = std::function<void(ValueType& val, std::wstringstream& in)>;

    void setSerializer(PropertySerializer&& serializer) {
        serializer_ = std::move(serializer);
    }

    void setDeserializer(PropertyDeserializer&& deserializer) {
        deserializer_ = std::move(deserializer);
    }

protected:
    virtual void serialize(const ValueType& val, std::wstringstream& out) = 0;
    virtual void deserialize(ValueType& val, std::wstringstream& in) = 0;

private:
    [[maybe_unused]] void invokeSerializer(const ValueType& val, std::wstringstream& out) {
        if (serializer_) {
            serializer_(val, out);
        } else {
            serialize(val, out);
        }
    }

    [[maybe_unused]] void invokeDeserializer(ValueType& val, std::wstringstream& in) {
        if (deserializer_) {
            deserializer_(val, in);
        } else {
            deserialize(val, in);
        }
    }

protected:
    T value_;
    T defaultValue_;
    PropertySerializer serializer_;
    PropertyDeserializer deserializer_;
    bool saveToIni_ = true;

    friend class SettingsParser;
};

template<class T>
struct PropertyDescriptor {
    const wchar_t *section = nullptr;
    const wchar_t *key = nullptr;
    T defaultValue;
    bool saveToIni = true;
};


class BoolProperty: public AbstractProperty<bool> {
public:
    constexpr static auto YES = L"yes";
    constexpr static auto NO = L"no";

    explicit BoolProperty(const PropertyDescriptor<bool>& descriptor)
            : AbstractProperty(
            PropertyType::Bool,
            descriptor.section,
            descriptor.key,
            descriptor.defaultValue,
            descriptor.saveToIni) {}

    void serialize(const bool& val, std::wstringstream& out) override {
        // by default
        out << (val ? YES : NO);
    }

    void deserialize(bool& val, std::wstringstream& in) override {
        // by default
        switch (in.str()[0]) {
            case L'Y':   // YES
            case L'y':   // yes
            case L'T':   // TRUE
            case L't':   // true
            case L'1':   // 1
                val = true;
                break;

            case L'N':   // NO
            case L'n':   // no
            case L'F':   // FALSE
            case L'f':   // false
            case L'0':   // 0
                val = false;
                break;

            default:
                val = defaultValue_;
                break;
        }
    }
};

class LongProperty: public AbstractProperty<long> {
public:
    explicit LongProperty(const PropertyDescriptor<long>& descriptor)
            : AbstractProperty(
            PropertyType::Long,
            descriptor.section,
            descriptor.key,
            descriptor.defaultValue,
            descriptor.saveToIni) {}

    void serialize(const long& val, std::wstringstream& out) override {
        out << val;
    }

    void deserialize(long& val, std::wstringstream& in) override {
        in >> val;
    }
};

class StringProperty: public AbstractProperty<std::wstring> {
public:
    explicit StringProperty(const PropertyDescriptor<std::wstring>& descriptor)
            : AbstractProperty(
            PropertyType::String,
            descriptor.section,
            descriptor.key,
            descriptor.defaultValue,
            descriptor.saveToIni) {}

    void serialize(const std::wstring& val, std::wstringstream& out) override {
        // default serializer
        out << val;
    }

    void deserialize(std::wstring& val, std::wstringstream& in) override {
        // default deserializer
        val = in.str();
    }
};

class StringListProperty: public AbstractProperty<std::vector<std::wstring>, std::wstring> {
public:
    explicit StringListProperty(const PropertyDescriptor<std::vector<std::wstring>>& descriptor)
            : AbstractProperty(
            PropertyType::StringList,
            descriptor.section,
            descriptor.key,
            descriptor.defaultValue,
            descriptor.saveToIni) {}

    void serialize(const std::wstring& val, std::wstringstream& out) override {
        // default serializer
        out << val;
    }

    void deserialize(std::wstring& val, std::wstringstream& in) override {
        // default deserializer
        val = in.str();
    }
};

} // namespace litelockr

#endif // PROPERTY_H
