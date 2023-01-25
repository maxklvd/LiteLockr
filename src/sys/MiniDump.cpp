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

#include "MiniDump.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include <dbghelp.h>
#include "app/Version.h"
#include "log/Logger.h"
#include "sys/Process.h"

namespace litelockr {

void MiniDump::initialize() {
    SetUnhandledExceptionFilter(unhandledExceptionHandler);
}

void MiniDump::setTerminateHandler() {
    std::set_terminate(terminate);
}

LONG WINAPI MiniDump::unhandledExceptionHandler(PEXCEPTION_POINTERS exceptionPointers) {
    if (Log::enabled(Log::Severity::Fatal)) {
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        int yearShort = lt.wYear - (lt.wYear / 100) * 100;

        std::wstringstream path;
        path << Process::modulePath();
        path << APP_NAME;
        path << L"-CrashDump-v" APP_VERSION L"-";
        path << std::setfill(L'0')
             << std::setw(2) << yearShort // year
             << std::setw(2) << lt.wMonth // month
             << std::setw(2) << lt.wDay;  // day
        path << L".dmp";

        HANDLE hFile = CreateFile(path.str().c_str(),
                                  GENERIC_WRITE,
                                  0,
                                  nullptr,
                                  CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  nullptr);

        MINIDUMP_EXCEPTION_INFORMATION miniDumpInfo;
        miniDumpInfo.ThreadId = GetCurrentThreadId();
        miniDumpInfo.ExceptionPointers = exceptionPointers;
        miniDumpInfo.ClientPointers = TRUE;

        MiniDumpWriteDump(GetCurrentProcess(),
                          GetCurrentProcessId(),
                          hFile,
                          (MINIDUMP_TYPE) (MiniDumpWithFullMemory | MiniDumpWithHandleData),
                          &miniDumpInfo,
                          nullptr,
                          nullptr);

        CloseHandle(hFile);

        LOG_FATAL(L"[Fatal Error] Crash dump has been written to '%s'", path.str().c_str());
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void MiniDump::terminate() {
    RaiseException(0xE0000010, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

} // namespace litelockr

