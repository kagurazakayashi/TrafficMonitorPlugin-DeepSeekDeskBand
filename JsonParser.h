/************************************************************************//**
 * @file    JsonParser.h
 * @brief   简易 JSON 解析工具声明
 * @details 提供不依赖第三方库的轻量 JSON 键值提取功能。
 *          支持提取布尔值和字符串值，适用于响应体结构简单的 API。
 ****************************************************************************/
#pragma once
#include <string>

/**
 * @brief 在 JSON 字符串中查找指定键，提取其布尔值（true/false）
 * @param json  以 null 结尾的 UTF-16 JSON 字符串
 * @param key   要查找的键（不含外层引号）
 * @param out   输出：提取到的布尔值
 * @return 找到并成功解析返回 true，键不存在或值格式错误返回 false
 */
bool JsonExtractBool(const wchar_t* json, const wchar_t* key, bool& out);

/**
 * @brief 在 JSON 字符串中查找指定键，提取其字符串值（不含外层引号）
 * @param json  以 null 结尾的 UTF-16 JSON 字符串
 * @param key   要查找的键（不含外层引号）
 * @param out   输出：提取到的字符串值（不含引号）
 * @return 找到并成功提取返回 true，键不存在或值格式错误返回 false
 */
bool JsonExtractString(const wchar_t* json, const wchar_t* key, std::wstring& out);
