/************************************************************************//**
 * @file    Logger.h
 * @brief   增强日志模块（头文件实现，单文件包含即可使用）
 * @details 提供分级日志输出，支持：
 *          - 通过 DSDB_LOG_LEVEL 宏控制日志级别
 *          - 通过 DSDB_LOG_FILE_PATH 宏控制是否写入文件（空字符串=禁用文件输出）
 *          - 始终通过 OutputDebugStringW 输出到调试器
 *          - 线程安全（使用 InitOnceExecuteOnce 确保初始化安全）
 *          - 自动附加时间戳、日志级别标签、源文件位置
 *
 *          使用示例：
 *            Logger_Debug(L"变量 x=%d", x);
 *            Logger_Info(L"服务已启动");
 *            Logger_Warn(L"配置项 %s 无效，使用默认值", key);
 *            Logger_Error(L"操作失败，错误码=%lu", GetLastError());
 ****************************************************************************/
#pragma once
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <cwchar>
#include <string>
#include "Config.h"

/** @brief 日志级别常量 */
#define LOG_LEVEL_DEBUG    0   ///< 调试信息（最详细）
#define LOG_LEVEL_INFO     1   ///< 常规信息
#define LOG_LEVEL_WARN     2   ///< 警告
#define LOG_LEVEL_ERROR    3   ///< 错误
#define LOG_LEVEL_OFF      4   ///< 禁用所有日志

// ============================================================
// 内部实现辅助
// ============================================================

/** @brief 日志模块内部状态 */
struct LoggerState
{
    CRITICAL_SECTION cs;        ///< 写锁临界区
    bool             initialized; ///< 是否已初始化
    std::wstring    filePath;   ///< 日志文件完整路径
    bool             fileEnabled; ///< 文件输出是否启用
};

/** @brief 获取日志模块内部状态（线程安全单例） */
inline LoggerState& Logger_GetState()
{
    static LoggerState s_state = {};
    return s_state;
}

/** @brief 一次性初始化日志模块（线程安全，使用 Windows InitOnce） */
inline void Logger_EnsureInit()
{
    LoggerState& state = Logger_GetState();
    if (state.initialized)
        return;

    // 双重检查加锁模式——EnterCriticalSection 作为屏障
    // 注意：在 DLL_PROCESS_ATTACH 中不应调用可能加载其他 DLL 的函数，
    // 因此初始化延迟到首次调用 Logger_* 时执行。
    static volatile LONG s_initGuard = 0;
    if (InterlockedCompareExchange(&s_initGuard, 1, 0) != 0)
    {
        // 其他线程正在初始化，自旋等待
        while (!state.initialized)
            Sleep(0);
        return;
    }

    InitializeCriticalSection(&state.cs);

    // 检查是否启用文件日志
    const wchar_t* cfgPath = DSDB_LOG_FILE_PATH;
    if (cfgPath && cfgPath[0] != L'\0')
    {
        state.filePath = cfgPath;
        state.fileEnabled = true;

        // 创建日志目录（如果不存在）
        std::wstring dir = state.filePath;
        size_t lastSep = dir.find_last_of(L"\\/");
        if (lastSep != std::wstring::npos)
        {
            dir = dir.substr(0, lastSep);
            // 确保父目录存在（不递归，简单创建）
            CreateDirectoryW(dir.c_str(), nullptr);
        }
    }
    else
    {
        state.fileEnabled = false;
    }

    state.initialized = true;
    InterlockedExchange(&s_initGuard, 0);

    // 写入日志文件头
    if (state.fileEnabled)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        wchar_t header[256];
        swprintf_s(header, L"========================================\r\n"
            L"  DeepSeekDeskBand 日志\r\n"
            L"  开始时间: %04d-%02d-%02d %02d:%02d:%02d\r\n"
            L"  日志级别: %d\r\n"
            L"========================================\r\n",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond,
            DSDB_LOG_LEVEL);
        FILE* fp = nullptr;
        _wfopen_s(&fp, state.filePath.c_str(), L"w, ccs=UTF-8");
        if (fp)
        {
            fputws(header, fp);
            fclose(fp);
        }
    }
}

/** @brief 核心日志输出函数（内部使用） */
inline void Logger_Write(int level, const wchar_t* levelLabel,
    const wchar_t* srcFile, int srcLine, const wchar_t* format, va_list args)
{
    // 确保初始化
    Logger_EnsureInit();

    LoggerState& state = Logger_GetState();

    // ---- 格式化时间戳 ----
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timestamp[32];
    swprintf_s(timestamp, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    // ---- 格式化消息 ----
    wchar_t msg[2048];
    _vsnwprintf_s(msg, _TRUNCATE, format, args);

    // ---- 提取文件名（仅文件名，不含路径） ----
    const wchar_t* fileName = srcFile;
    if (fileName)
    {
        const wchar_t* lastSep = wcsrchr(fileName, L'\\');
        if (lastSep)
            fileName = lastSep + 1;
        const wchar_t* lastSep2 = wcsrchr(fileName, L'/');
        if (lastSep2)
            fileName = lastSep2 + 1;
    }
    else
    {
        fileName = L"?";
    }

    // ---- 组装完整行 ----
    wchar_t buf[2400];
    swprintf_s(buf, L"[%s][%s][%s:%d] %s\r\n",
        timestamp, levelLabel, fileName, srcLine, msg);

    // ---- 输出到调试器 ----
    OutputDebugStringW(buf);

    // ---- 输出到日志文件 ----
    if (state.fileEnabled)
    {
        EnterCriticalSection(&state.cs);
        FILE* fp = nullptr;
        _wfopen_s(&fp, state.filePath.c_str(), L"a, ccs=UTF-8");
        if (fp)
        {
            fputws(buf, fp);
            fclose(fp);
        }
        LeaveCriticalSection(&state.cs);
    }
}

/** @brief 清理日志模块（在 DLL_PROCESS_DETACH 中调用） */
inline void Logger_Shutdown()
{
    LoggerState& state = Logger_GetState();
    if (state.initialized)
    {
        DeleteCriticalSection(&state.cs);
        state.initialized = false;
    }
}

// ============================================================
// 日志辅助函数（处理 va_list 转换，避免 MSVC va_start 引用类型断言）
// ============================================================

/**
 * @brief 日志写入辅助函数（可变参数版本）
 * @details 此函数作为 Logger_Write 的包装器，接收可变参数后通过 va_list 转发。
 *          使用此包装器而非在宏中直接调用 va_start 的原因：
 *          在宏展开上下文中，字符串字面量的 decltype 为引用数组类型，
 *          MSVC 14.50+ 的 va_start 强化检查会触发 static_assert 编译错误。
 *          通过此中间函数，format 参数的类型强制退化为 const wchar_t*。
 */
inline void Logger_WriteHelper(int level, const wchar_t* levelLabel,
    const wchar_t* srcFile, int srcLine, const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    Logger_Write(level, levelLabel, srcFile, srcLine, format, args);
    va_end(args);
}

// ============================================================
// 公开的日志宏
// ============================================================

/** @brief 辅助宏：将窄字符串转为宽字符串（用于 __FILE__ 转换） */
#define LOG_WIDEN2(x) L ## x
#define LOG_WIDEN(x)  LOG_WIDEN2(x)

/** @brief 调试级别日志（DSDB_LOG_LEVEL <= 0 时输出） */
#if DSDB_LOG_LEVEL <= LOG_LEVEL_DEBUG
#define Logger_Debug(format, ...) \
    Logger_WriteHelper(LOG_LEVEL_DEBUG, L"DEBG", LOG_WIDEN(__FILE__), __LINE__, format, ##__VA_ARGS__)
#else
#define Logger_Debug(format, ...) ((void)0)
#endif

/** @brief 信息级别日志（DSDB_LOG_LEVEL <= 1 时输出） */
#if DSDB_LOG_LEVEL <= LOG_LEVEL_INFO
#define Logger_Info(format, ...) \
    Logger_WriteHelper(LOG_LEVEL_INFO, L"INFO", LOG_WIDEN(__FILE__), __LINE__, format, ##__VA_ARGS__)
#else
#define Logger_Info(format, ...) ((void)0)
#endif

/** @brief 警告级别日志（DSDB_LOG_LEVEL <= 2 时输出） */
#if DSDB_LOG_LEVEL <= LOG_LEVEL_WARN
#define Logger_Warn(format, ...) \
    Logger_WriteHelper(LOG_LEVEL_WARN, L"WARN", LOG_WIDEN(__FILE__), __LINE__, format, ##__VA_ARGS__)
#else
#define Logger_Warn(format, ...) ((void)0)
#endif

/** @brief 错误级别日志（DSDB_LOG_LEVEL <= 3 时输出） */
#if DSDB_LOG_LEVEL <= LOG_LEVEL_ERROR
#define Logger_Error(format, ...) \
    Logger_WriteHelper(LOG_LEVEL_ERROR, L"ERR ", LOG_WIDEN(__FILE__), __LINE__, format, ##__VA_ARGS__)
#else
#define Logger_Error(format, ...) ((void)0)
#endif

/**
 * @brief 无条件日志（向后兼容旧代码）
 * @note  新代码应使用 Logger_Debug/Info/Warn/Error 代替
 */
#define Logger_Log(format, ...) \
    Logger_WriteHelper(LOG_LEVEL_INFO, L"LOG ", LOG_WIDEN(__FILE__), __LINE__, format, ##__VA_ARGS__)
