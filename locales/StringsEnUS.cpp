/************************************************************************//**
 * @file    StringsEnUS.cpp
 * @brief   英语翻译表
 * @details 所有 UI 文本的英语翻译，作为默认回退语言。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsEnUS[] = {
    /* ITEM_NAME            */ L"DeepSeek Balance Assistant",
    /* COMMAND_NAME         */ L"DeepSeek Balance Assistant",
    /* PLUGIN_DESCRIPTION   */ L"Displays DeepSeek balance information in the TrafficMonitor taskbar window, with balance change history tracking.",
    /* PLUGIN_AUTHOR        */ L"KagurazakaYashi(Miyabi)",
    /* DLG_TITLE            */ L"DeepSeek Settings",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Invalid key format: must start with sk- followed by 32 lowercase letters and digits",
    /* DLG_BTN_TEST_API     */ L"Test API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Please test before saving",
    /* DLG_LABEL_INTERVAL   */ L"Update interval (s):",
    /* DLG_LABEL_TIMEOUT    */ L"Request timeout (s):",
    /* DLG_LABEL_HISTORY_COUNT */ L"History count:",
    /* DLG_LABEL_HISTORY    */ L"Balance history:",
    /* DLG_COL_TIME         */ L"Time",
    /* DLG_COL_AVAILABLE    */ L"Available",
    /* DLG_COL_TOTAL        */ L"Total Balance",
    /* DLG_COL_GRANTED      */ L"Granted Balance",
    /* DLG_COL_TOPPED_UP    */ L"Topped-up Balance",
    /* DLG_COL_CURRENCY     */ L"Currency",
    /* DLG_BTN_OK           */ L"OK(&O)",
    /* DLG_BTN_CANCEL       */ L"Cancel(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Clear History(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Auto Refresh",
    /* STATUS_TESTING       */ L"Testing API...",
    /* STATUS_TEST_PASSED   */ L"API test passed",
    /* STATUS_TEST_FAILED   */ L"API test failed",
    /* BOOL_YES             */ L"Yes",
    /* BOOL_NO              */ L"No",
    /* MSG_TEST_SUCCESS_TITLE*/ L"API Test Successful",
    /* MSG_TEST_SUCCESS_BODY */ L"API test passed!\n\nAccount available: %s\nTotal balance: %s %s\nGranted balance: %s %s\nTopped-up balance: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"API Test Failed",
    /* MSG_TEST_FAILED_BODY */ L"API test failed!\n\nError: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Confirm Clear",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Are you sure you want to clear all history?\n\nThis action cannot be undone.",
    /* MSG_INVALID_KEY_TITLE */ L"Invalid Format",
    /* MSG_INVALID_KEY_BODY */ L"Invalid API key format. Please check and try again.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Parameter Error",
    /* MSG_INTERVAL_ERROR_BODY*/ L"Update interval must be greater than request timeout,\notherwise a new request will start before the previous one completes.",
    /* DLG_LABEL_LANGUAGE   */ L"Language:",
    /* COMBO_LANGUAGE_AUTO  */ L"Auto",
};
