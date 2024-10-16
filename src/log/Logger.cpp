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

#include "Logger.h"

#include <fstream>
#include <filesystem>

#include <windows.h>

#define PLOG_OMIT_LOG_DEFINES

#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include "app/Version.h"
#include "sys/Process.h"


namespace plog {
class CustomTxtFormatter {
public:
    static util::nstring header() {
        return {};
    }

    static util::nstring format(const Record& record) {
        tm t{};
        util::localtime_s(&t, &record.getTime().time);

        static std::vector<const char *> months{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
                                                "Nov", "Dec"};
        assert(t.tm_mon < 12);

        util::nostringstream ss;
        ss << PLOG_NSTR("[") << severityToString(record.getSeverity())[0] << PLOG_NSTR("] "); // gets a first character
        ss << months[t.tm_mon] << PLOG_NSTR(" ") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday
           << PLOG_NSTR(" ");
        ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":")
           << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0'))
           << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3)
           << static_cast<int> (record.getTime().millitm) << PLOG_NSTR(" ");
        ss << std::setfill(PLOG_NSTR(' '));
        ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
        ss << record.getMessage() << PLOG_NSTR("\n");

        return ss.str();
    }
};
} // namespace plog

namespace litelockr {

namespace fs = std::filesystem;

constexpr static auto LOG_FILENAME = "LiteLockr.log";
Log::Severity Log::maxSeverity_ = Log::Severity::None;

void Log::initialize(Log::Severity maxSeverity) {
    maxSeverity_ = maxSeverity;

    if (maxSeverity > Log::Severity::None) {
        fs::path logPath = Process::modulePath();
        logPath /= LOG_FILENAME;

        if (std::wofstream ofs(logPath); ofs) { // create or rewrite the file
            SeverityName severityName;
            auto severity = severityName.getName(maxSeverity);

            ofs << std::wstring(80, L'*') << std::endl;
            ofs << APP_NAME << L" Version " << APP_VERSION << std::endl;
            ofs << L"Process ID: " << GetCurrentProcessId() << std::endl;
            ofs << L"Path to executable: " << GetCommandLine() << std::endl;
            ofs << L"Log severity: " << severity << L" [" << severity[0] << L"]" << std::endl;
            ofs << std::wstring(80, L'*') << std::endl;
        } else {
            std::wstring msg = L"Could not create a log file \"";
            msg += logPath.c_str();
            msg += L"\"";
            MessageBox(nullptr, msg.c_str(), APP_NAME, MB_OK | MB_ICONERROR);
        }

        plog::init<plog::CustomTxtFormatter>(static_cast<plog::Severity>(maxSeverity), logPath.c_str(), 0, 0);
    }
}

void Log::print(Severity severity, const wchar_t *format, ...) {
    if (maxSeverity_ >= severity) {
        wchar_t *str = nullptr;
        va_list ap;

        va_start(ap, format);
        int len = plog::util::vaswprintf(&str, format, ap);
        static_cast<void>(len);
        va_end(ap);

        PLOG(static_cast<plog::Severity>(severity)) << str;
        free(str);
    }
}

#ifdef LOG_PROFILER
std::chrono::high_resolution_clock::time_point Log::_started;

void Log::_start() {
    _started = std::chrono::high_resolution_clock::now();
}

void Log::_end(const wchar_t* str) {
    using namespace std::chrono;

    auto end = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(end - _started);
    debug(L"[%s] time = %f seconds", str, time_span.count());
}
#endif // LOG_PROFILER

Log::Severity Log::SeverityName::getValue(std::wstring_view name) const {
    for (const auto& [value, str]: map_) {
        if (std::wstring_view(str) == name) {
            return value;
        }
    }
    return Log::Severity::None;
}

std::wstring Log::SeverityName::getName(Log::Severity value) {
    auto it = map_.find(value);
    if (it != map_.end()) {
        return it->second;
    }
    return L"";
}

} // namespace litelockr
