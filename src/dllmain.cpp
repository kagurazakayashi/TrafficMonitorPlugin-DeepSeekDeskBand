/************************************************************************//**
 * @file    dllmain.cpp
 * @brief   DLL 入口点
 * @details 实现标准的 DllMain 函数，作为动态库的入口点。
 *          DLL_PROCESS_ATTACH 时初始化多语言系统。
 *          DLL_PROCESS_DETACH 时清理日志模块。
 ****************************************************************************/
#include "framework.h"
#include "Strings.h"
#include "Logger.h"

/**
 * @brief DLL 入口点函数
 * @param hModule           当前 DLL 模块的句柄
 * @param ul_reason_for_call 调用原因（加载/卸载/线程创建/线程销毁）
 * @param lpReserved         保留参数
 * @return 始终返回 TRUE
 * @note   DLL_PROCESS_ATTACH 中不应调用可能加载其他 DLL 的复杂操作，
 *          因此仅做轻量级初始化（Strings_Init 仅设置语言检测标志位）。
 *          Logger 的初始化延迟到首次调用 Logger_* 宏时执行。
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:   // DLL 被加载时，检测系统语言
        Strings_Init();
        break;

    case DLL_PROCESS_DETACH:   // DLL 被卸载时，清理日志模块
        Logger_Shutdown();
        break;

    case DLL_THREAD_ATTACH:    // 新线程创建时
    case DLL_THREAD_DETACH:    // 线程退出时
        break;
    }
    return TRUE;
}
