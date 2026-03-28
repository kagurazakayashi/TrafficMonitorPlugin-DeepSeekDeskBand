/************************************************************************//**
 * @file    framework.h
 * @brief   Windows 头文件封装
 * @details 所有依赖 Windows API 的源文件通过包含此文件来引入必要的 Windows 头文件。
 *          使用 WIN32_LEAN_AND_MEAN 宏可以减少编译时间。
 ****************************************************************************/
#pragma once

// 排除不常用的 Windows API 以加快编译速度
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
