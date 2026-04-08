/************************************************************************//**
 * @file    Strings.cpp
 * @brief   多语言字符串管理实现
 * @details 在 DLL 加载时通过 GetUserDefaultUILanguage() 检测系统语言，
 *          根据检测结果定位到对应的翻译表，Strings_Get() 返回当前语言的文本。
 ****************************************************************************/
#include "Strings.h"

/** @brief 当前用户配置的语言（Auto 表示自动检测） */
static Language g_language = Language::Auto;

/** @brief 系统检测到的语言（由 Strings_Init 设置，后续不变） */
static Language g_systemLanguage = Language::English;

/**
 * @brief 检测系统 UI 语言并设置全局语言变量
 * @note  简体中文通过 SUBLANGID 区分：SUBLANG_CHINESE_SIMPLIFIED (0x02)
 *         繁体中文取所有非简体的 LANG_CHINESE 子语言
 */
void Strings_Init()
{
    LANGID langId = GetUserDefaultUILanguage();
    WORD primary  = PRIMARYLANGID(langId);
    WORD sub      = SUBLANGID(langId);

    if (primary == LANG_CHINESE)
    {
        if (sub == SUBLANG_CHINESE_SIMPLIFIED)
            g_systemLanguage = Language::ChineseSimplified;
        else
            g_systemLanguage = Language::ChineseTraditional;
    }
    else if (primary == LANG_JAPANESE)
    {
        g_systemLanguage = Language::Japanese;
    }
    else
    {
        g_systemLanguage = Language::English;
    }
}

/**
 * @brief 解析实际生效的语言（Auto 时返回系统检测语言）
 */
static Language ResolveLanguage()
{
    if (g_language == Language::Auto)
        return g_systemLanguage;
    return g_language;
}

void Strings_SetLanguage(Language lang)
{
    g_language = lang;
}

Language Strings_GetEffectiveLanguage()
{
    return ResolveLanguage();
}

Language Strings_GetConfiguredLanguage()
{
    return g_language;
}

// ---- 翻译表（定义在独立的语言文件中） ----
extern const wchar_t* kStringsZhCN[];
extern const wchar_t* kStringsZhTW[];
extern const wchar_t* kStringsJaJP[];
extern const wchar_t* kStringsEnUS[];

/**
 * @brief 根据当前语言和字符串键返回对应文本
 * @param key 字符串键
 * @return 本地化后的宽字符串指针（指向编译期常量数据）
 */
const wchar_t* Strings_Get(StringKey key)
{
    int idx = static_cast<int>(key);
    switch (ResolveLanguage())
    {
    case Language::ChineseSimplified:  return kStringsZhCN[idx];
    case Language::ChineseTraditional: return kStringsZhTW[idx];
    case Language::Japanese:           return kStringsJaJP[idx];
    case Language::English:
    default:                           return kStringsEnUS[idx];
    }
}
