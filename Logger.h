/************************************************************************//**
 * @file    Logger.h
 * @brief   简易文件日志模块（头文件实现，单文件包含即可使用）
 * @details 将日志写入插件所在目录下的 DeepSeekDeskBand.log。
 *          同时通过 OutputDebugStringW 输出到调试器。
 ****************************************************************************/
#pragma once
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <cwchar>
#include <string>

/** @brief 获取当前模块所在目录路径 */
inline std::wstring Logger_GetModuleDir()
{
    wchar_t path[MAX_PATH];
    HMODULE hMod = nullptr;
    // 获取当前 DLL 的 HMODULE（通过传入本模块内的地址）
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&Logger_GetModuleDir),
        &hMod);
    GetModuleFileNameW(hMod, path, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash)
        *lastSlash = L'\0';
    return std::wstring(path);
}

/** @brief 记录一条日志（线程安全，自动追加时间戳和换行） */
inline void Logger_Log(const wchar_t* format, ...)
{
    static CRITICAL_SECTION s_cs;
    static bool s_init = false;
    if (!s_init)
    {
        InitializeCriticalSection(&s_cs);
        s_init = true;
    }

    EnterCriticalSection(&s_cs);

    // ---- 格式化时间戳 ----
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timestamp[32];
    swprintf_s(timestamp, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    // ---- 格式化消息 ----
    va_list args;
    va_start(args, format);
    wchar_t msg[2048];
    _vsnwprintf_s(msg, _TRUNCATE, format, args);
    va_end(args);

    // ---- 组装完整行 ----
    wchar_t line[2200];
    swprintf_s(line, L"[%s] %s\r\n", timestamp, msg);

    // ---- 输出到调试器 ----
    OutputDebugStringW(line);

    // ---- 输出到日志文件 ----
    std::wstring logPath = Logger_GetModuleDir() + L"\\DeepSeekDeskBand.log";
    FILE* fp = nullptr;
    _wfopen_s(&fp, logPath.c_str(), L"a, ccs=UTF-8");
    if (fp)
    {
        fputws(line, fp);
        fclose(fp);
    }

    LeaveCriticalSection(&s_cs);
}
