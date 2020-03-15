/*!
 *  @brief     创建windows中程序奔溃dump文件
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.02.26
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef WINDUMPFILE_H
#define WINDUMPFILE_H

#include <Windows.h>
#include<DbgHelp.h>
#pragma comment(lib,"DbgHelp.lib")

void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
    HANDLE hDumpFile = CreateFile((LPCWSTR)lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    CloseHandle(hDumpFile);
}

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    CreateDumpFile(L"Test.dmp",pException);
    return EXCEPTION_EXECUTE_HANDLER;
}

#endif // WINDUMPFILE_H
