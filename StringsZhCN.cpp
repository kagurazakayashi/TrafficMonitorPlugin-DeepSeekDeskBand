/************************************************************************//**
 * @file    StringsZhCN.cpp
 * @brief   简体中文翻译表
 * @details 所有 UI 文本的简体中文翻译，采用大陆风格用语。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsZhCN[] = {
    /* ITEM_NAME            */ L"DeepSeek 助手",
    /* COMMAND_NAME         */ L"DeepSeek余额助手",
    /* PLUGIN_DESCRIPTION   */ L"DeepSeek 桌面助手插件",
    /* DLG_TITLE            */ L"DeepSeek 设置",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"密钥格式错误：必须以 sk- 开头，后跟32位小写字母和数字",
    /* DLG_BTN_TEST_API     */ L"测试API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< 请先测试再保存",
    /* DLG_LABEL_INTERVAL   */ L"更新间隔（秒）:",
    /* DLG_LABEL_TIMEOUT    */ L"请求超时（秒）:",
    /* DLG_LABEL_HISTORY_COUNT */ L"历史记录数量:",
    /* DLG_LABEL_HISTORY    */ L"历史变化记录:",
    /* DLG_COL_TIME         */ L"时间",
    /* DLG_COL_AVAILABLE    */ L"可用",
    /* DLG_COL_TOTAL        */ L"总余额",
    /* DLG_COL_GRANTED      */ L"赠送余额",
    /* DLG_COL_TOPPED_UP    */ L"充值余额",
    /* DLG_COL_CURRENCY     */ L"币种",
    /* DLG_BTN_OK           */ L"确定(&O)",
    /* DLG_BTN_CANCEL       */ L"取消(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"清除历史(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"自动刷新",
    /* STATUS_TESTING       */ L"正在测试 API...",
    /* STATUS_TEST_PASSED   */ L"API 测试通过",
    /* STATUS_TEST_FAILED   */ L"API 测试失败",
    /* BOOL_YES             */ L"是",
    /* BOOL_NO              */ L"否",
    /* MSG_TEST_SUCCESS_TITLE*/ L"API 测试成功",
    /* MSG_TEST_SUCCESS_BODY */ L"API 测试通过！\n\n账户可用：%s\n总余额：%s %s\n赠送余额：%s %s\n充值余额：%s %s",
    /* MSG_TEST_FAILED_TITLE */ L"API 测试失败",
    /* MSG_TEST_FAILED_BODY */ L"API 测试失败！\n\n错误信息：%s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"确认清除",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"确定要清除所有历史记录吗？\n\n此操作不可撤销。",
    /* MSG_INVALID_KEY_TITLE */ L"格式错误",
    /* MSG_INVALID_KEY_BODY */ L"API 密钥格式无效，请检查后重试。",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"参数错误",
    /* MSG_INTERVAL_ERROR_BODY*/ L"更新间隔必须大于请求超时时间，\n否则上一个请求未完成就会发起下一个请求。",
    /* DLG_LABEL_LANGUAGE   */ L"显示语言:",
    /* COMBO_LANGUAGE_AUTO  */ L"自动",
};
