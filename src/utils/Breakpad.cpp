/*
 * Copyright 2010-2013 Bluecherry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QString>
#include <QDesktopServices>
#include <QFileInfo>

#ifdef Q_OS_WIN
/* Platform-specific code for Windows */
#include "client/windows/handler/exception_handler.h"
#include <windows.h>
using namespace google_breakpad;

static const wchar_t *executablePath = 0;

bool breakpadDumpCallback(const wchar_t *dump_path, const wchar_t *minidump_id, void *, EXCEPTION_POINTERS *,
                          MDRawAssertionInfo *, bool succeeded)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    wchar_t cmdline[MAX_PATH];
    wcscpy_s(cmdline, executablePath);
    wcscat_s(cmdline, L" --crash \"");
    if (succeeded)
    {
        wcscat_s(cmdline, dump_path);
        wcscat_s(cmdline, L"/");
        wcscat_s(cmdline, minidump_id);
        wcscat_s(cmdline, L".dmp\"");
    }

    if (!CreateProcessW(executablePath, cmdline, 0, 0, FALSE, 0, 0, 0, &si, &pi))
        return false;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

#ifndef QT_NO_DEBUG
    return false;
#else
    return true;
#endif
}

void initBreakpad()
{
    /* Store the path of the executable in case we crash; it might not be safe to do this later. */
    wchar_t buf[MAX_PATH];
    DWORD sz = GetModuleFileNameW(0, buf, MAX_PATH);
    executablePath = new wchar_t[sz+1];
    wmemcpy_s((wchar_t*)executablePath, sz+1, buf, sz+1);

    QString tmp = QDesktopServices::storageLocation(QDesktopServices::TempLocation);

    new ExceptionHandler(std::wstring(reinterpret_cast<const wchar_t*>(tmp.utf16()), tmp.size()), 0,
                         &breakpadDumpCallback, 0, ExceptionHandler::HANDLER_ALL);
}

#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
/* Shared code for Mac OS X and Linux */
#include <limits.h>
static QByteArray executablePath;

bool breakpadDumpCallback(const char *dump_path, const char *minidump_id, void *, bool succeeded)
{
    char param[] = "--crash";

    char dumpFile[PATH_MAX];
    if (succeeded)
    {
        strcpy(dumpFile, dump_path);
        strcat(dumpFile, "/");
        strcat(dumpFile, minidump_id);
        strcat(dumpFile, ".dmp");
    }

    char * const args[] = { (char*)executablePath.constData(), param, succeeded ? dumpFile : 0, 0 };

    execv(executablePath.constData(), args);
#ifndef QT_NO_DEBUG
    return false;
#else
    return true;
#endif
}

#if defined(Q_OS_MAC)
/* Platform-specific code for Mac OS X */
#include "client/mac/handler/exception_handler.h"
#include <mach-o/dyld.h>

void initBreakpad()
{
    quint32 size = 0;
    _NSGetExecutablePath(0, &size);
    executablePath.resize(++size);
    _NSGetExecutablePath(executablePath.data(), &size);
    executablePath.resize(size);

    QString tmp = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
    new google_breakpad::ExceptionHandler(tmp.toStdString(), 0, &breakpadDumpCallback, 0, true, 0);
}

#else
/* Platform-specific code for Linux */
#include "client/linux/handler/exception_handler.h"
using namespace google_breakpad;

void initBreakpad()
{
    QFileInfo fi(QLatin1String("/proc/self/exe"));
    executablePath = fi.symLinkTarget().toLocal8Bit();

    QString tmp = QDesktopServices::storageLocation(QDesktopServices::TempLocation);

    new ExceptionHandler(tmp.toStdString(), 0, &breakpadDumpCallback, 0, true);
}

#endif /* !defined(Q_OS_MAC) */
#endif /* defined(Q_OS_MAC) || defined(Q_OS_LINUX) */
