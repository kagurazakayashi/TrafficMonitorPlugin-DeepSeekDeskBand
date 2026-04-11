/************************************************************************//**
 * @file    JsonParser.cpp
 * @brief   简易 JSON 解析工具实现
 * @details 实现轻量的 JSON 键值提取，忽略嵌套结构，仅支持顶层键查找。
 *          不依赖任何第三方 JSON 库，代码量极小。
 ****************************************************************************/
#include "JsonParser.h"
#include <cwchar>

bool JsonExtractBool(const wchar_t* json, const wchar_t* key, bool& out)
{
    if (!json || !key)
        return false;

    // 构造带引号的键字符串，如 L"\"is_available\""
    std::wstring searchKey = L"\"";
    searchKey += key;
    searchKey += L"\"";

    const wchar_t* pos = wcsstr(json, searchKey.c_str());
    if (!pos)
        return false;

    // 跳过键部分，找到 ":" 分隔符
    pos = pos + searchKey.size();
    while (*pos && (*pos == L':' || *pos == L' ' || *pos == L'\t' || *pos == L'\n' || *pos == L'\r'))
        pos++;

    if (wcsncmp(pos, L"true", 4) == 0)
    {
        out = true;
        return true;
    }
    if (wcsncmp(pos, L"false", 5) == 0)
    {
        out = false;
        return true;
    }
    return false;
}

bool JsonExtractString(const wchar_t* json, const wchar_t* key, std::wstring& out)
{
    if (!json || !key)
        return false;

    std::wstring searchKey = L"\"";
    searchKey += key;
    searchKey += L"\"";

    const wchar_t* pos = wcsstr(json, searchKey.c_str());
    if (!pos)
        return false;

    // 跳过键部分，找到 ":" 后的第一个 '"'
    pos = pos + searchKey.size();
    while (*pos && (*pos == L':' || *pos == L' ' || *pos == L'\t' || *pos == L'\n' || *pos == L'\r'))
        pos++;

    if (*pos != L'"')
        return false;

    pos++; // 跳过开引号
    const wchar_t* end = pos;
    while (*end && *end != L'"')
        end++;

    if (*end != L'"')
        return false;

    out.assign(pos, static_cast<size_t>(end - pos));
    return true;
}
