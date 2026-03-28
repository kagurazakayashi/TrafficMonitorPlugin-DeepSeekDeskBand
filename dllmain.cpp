/************************************************************************//**
 * @file    dllmain.cpp
 * @brief   DLL 入口点
 * @details 实现标准的 DllMain 函数，作为动态库的入口点。
 *          当前为空实现，所有插件逻辑在 TMPluginGetInstance() 中完成。
 ****************************************************************************/
#include "framework.h"

/**
 * @brief DLL 入口点函数
 * @param hModule           当前 DLL 模块的句柄
 * @param ul_reason_for_call 调用原因（加载/卸载/线程创建/线程销毁）
 * @param lpReserved         保留参数
 * @return 始终返回 TRUE
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:   // DLL 被加载时
    case DLL_THREAD_ATTACH:    // 新线程创建时
    case DLL_THREAD_DETACH:    // 线程退出时
    case DLL_PROCESS_DETACH:   // DLL 被卸载时
        break;
    }
    return TRUE;
}
