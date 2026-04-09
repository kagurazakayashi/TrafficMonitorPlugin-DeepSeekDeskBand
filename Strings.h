/************************************************************************//**
 * @file    Strings.h
 * @brief   多语言字符串管理
 * @details 定义支持的语言枚举和字符串键枚举，提供根据系统语言自动获取
 *          对应翻译的接口。支持简体中文、繁体中文、日语、英语四种语言。
 ****************************************************************************/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/** @brief 支持的语言列表 */
enum class Language
{
    Auto,               ///< 自动检测（根据系统语言）
    ChineseSimplified,  ///< 简体中文（大陆风格）
    ChineseTraditional, ///< 繁体中文（台湾风格）
    Japanese,           ///< 日语
    English,            ///< 英语
    German,             ///< 德语
    Hebrew,             ///< 希伯来语
    Hungarian,          ///< 匈牙利语
    Italian,            ///< 意大利语
    Polish,             ///< 波兰语
    PortugueseBrazilian,///< 葡萄牙语（巴西）
    Russian,            ///< 俄语
    Turkish,            ///< 土耳其语
};

/** @brief 本地化字符串键，对应 UI 中的每一处文本 */
enum class StringKey
{
    // ---- 显示项目 ----
    ITEM_NAME,              ///< 右键菜单显示项目名称

    // ---- 右键菜单命令 ----
    COMMAND_NAME,           ///< 右键菜单项名称

    // ---- 插件描述 ----
    PLUGIN_DESCRIPTION,     ///< 插件功能描述

    // ---- 设置对话框 ----
    DLG_TITLE,              ///< 窗口标题
    DLG_API_LABEL,          ///< API 密钥标签
    DLG_API_HINT,           ///< API 格式校验提示
    DLG_BTN_TEST_API,       ///< 测试 API 按钮
    DLG_STATUS_UNTESTED,    ///< 未测试时状态文本
    DLG_LABEL_INTERVAL,     ///< 更新间隔标签
    DLG_LABEL_TIMEOUT,      ///< 请求超时标签
    DLG_LABEL_HISTORY_COUNT,///< 历史记录数量标签
    DLG_LABEL_HISTORY,      ///< 历史记录标签
    DLG_COL_TIME,           ///< ListView 列：时间
    DLG_COL_AVAILABLE,      ///< ListView 列：可用
    DLG_COL_TOTAL,          ///< ListView 列：总余额
    DLG_COL_GRANTED,        ///< ListView 列：赠送余额
    DLG_COL_TOPPED_UP,      ///< ListView 列：充值余额
    DLG_COL_CURRENCY,       ///< ListView 列：币种
    DLG_BTN_OK,             ///< 确定按钮
    DLG_BTN_CANCEL,         ///< 取消按钮
    DLG_BTN_CLEAR_HISTORY,  ///< 清除历史按钮
    DLG_CHECK_AUTO_REFRESH, ///< 自动刷新复选框

    // ---- 状态文本 ----
    STATUS_TESTING,         ///< 正在测试 API
    STATUS_TEST_PASSED,     ///< API 测试通过
    STATUS_TEST_FAILED,     ///< API 测试失败

    // ---- ListView 布尔值 ----
    BOOL_YES,               ///< "是"
    BOOL_NO,                ///< "否"

    // ---- 消息对话框 ----
    MSG_TEST_SUCCESS_TITLE, ///< 测试成功对话框标题
    MSG_TEST_SUCCESS_BODY,  ///< 测试成功对话框内容（%s 占位符）
    MSG_TEST_FAILED_TITLE,  ///< 测试失败对话框标题
    MSG_TEST_FAILED_BODY,   ///< 测试失败对话框内容（%s 占位符）
    MSG_CONFIRM_CLEAR_TITLE,///< 确认清除对话框标题
    MSG_CONFIRM_CLEAR_BODY, ///< 确认清除对话框内容
    MSG_INVALID_KEY_TITLE,  ///< 密钥格式错误对话框标题
    MSG_INVALID_KEY_BODY,   ///< 密钥格式错误对话框内容
    MSG_INTERVAL_ERROR_TITLE,///< 参数错误对话框标题
    MSG_INTERVAL_ERROR_BODY,///< 参数错误对话框内容

    // ---- 语言设置 ----
    DLG_LABEL_LANGUAGE,     ///< 显示语言标签
    COMBO_LANGUAGE_AUTO,    ///< 语言下拉框"自动"项
};

/**
 * @brief 初始化多语言系统，检测系统语言
 * @note  应在 DLL_PROCESS_ATTACH 中尽早调用
 */
void Strings_Init();

/**
 * @brief 手动设置语言（覆盖自动检测）
 * @param lang 目标语言，Language::Auto 表示恢复自动检测
 */
void Strings_SetLanguage(Language lang);

/**
 * @brief 获取当前配置的语言（含自动检测结果）
 * @return 当前生效的语言枚举值
 */
Language Strings_GetEffectiveLanguage();

/**
 * @brief 获取用户配置的语言偏好（可能为 Auto）
 * @return 配置的语言枚举值
 */
Language Strings_GetConfiguredLanguage();

/**
 * @brief 根据键获取当前语言的字符串
 * @param key 字符串键
 * @return 本地化后的宽字符串指针（静态生命周期）
 */
const wchar_t* Strings_Get(StringKey key);
