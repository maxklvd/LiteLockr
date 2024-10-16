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

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <unordered_map>

namespace litelockr {

#define LOG_FATAL(...) if (Log::enabled(Log::Severity::Fatal)) Log::fatal(__VA_ARGS__)
#define LOG_ERROR(...) if (Log::enabled(Log::Severity::Error)) Log::error(__VA_ARGS__)
#define LOG_DEBUG(...) if (Log::enabled(Log::Severity::Debug)) Log::debug(__VA_ARGS__)
#define LOG_WARNING(...) if (Log::enabled(Log::Severity::Warning)) Log::warning(__VA_ARGS__)
#define LOG_VERBOSE(...) if (Log::enabled(Log::Severity::Verbose)) Log::verbose(__VA_ARGS__)

class Log {
public:
    enum class Severity {
        None = 0,
        Fatal = 1,
        Error = 2,
        Warning = 3,
        Info = 4,
        Debug = 5,
        Verbose = 6,
        All = 7,
    };

    class SeverityName {
    public:
        Severity getValue(std::wstring_view name) const;
        std::wstring getName(Severity value);

    private:
        std::unordered_map<Severity, std::wstring> map_{
                {Severity::None,    L"NONE"},
                {Severity::Fatal,   L"FATAL"},
                {Severity::Error,   L"ERROR"},
                {Severity::Warning, L"WARNING"},
                {Severity::Info,    L"INFO"},
                {Severity::Debug,   L"DEBUG"},
                {Severity::Verbose, L"VERBOSE"},
                {Severity::All,     L"ALL"},
        };
    };

    static void initialize(Severity maxSeverity);

    static bool enabled(Severity severity) {
        return severity <= maxSeverity_;
    }

    template<typename ...Args>
    static void fatal(const wchar_t *format, Args... args) {
        print(Severity::Fatal, format, args...);
    }

    template<typename ...Args>
    static void error(const wchar_t *format, Args... args) {
        print(Severity::Error, format, args...);
    }

    template<typename ...Args>
    static void warning(const wchar_t *format, Args... args) {
        print(Severity::Warning, format, args...);
    }

    template<typename ...Args>
    static void debug(const wchar_t *format, Args... args) {
        print(Severity::Debug, format, args...);
    }

    template<typename ...Args>
    static void verbose(const wchar_t *format, Args... args) {
        print(Severity::Verbose, format, args...);
    }

#define LOG_PROFILE_START Log::_start
#define LOG_PROFILE_END Log::_end
#ifdef LOG_PROFILER
    static std::chrono::high_resolution_clock::time_point _started;
    static void _start();
    static void _end(const wchar_t* str = L"");
#endif // LOG_PROFILER

private:
    static void print(Severity severity, const wchar_t *format, ...);

    static Severity maxSeverity_;
};

} // namespace litelockr

#endif // LOGGER_H
