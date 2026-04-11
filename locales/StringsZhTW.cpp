/************************************************************************//**
 * @file    StringsZhTW.cpp
 * @brief   繁体中文翻译表
 * @details 所有 UI 文本的繁体中文翻译，采用台湾风格用语。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsZhTW[] = {
    /* ITEM_NAME            */ L"DeepSeek 餘額助手",
    /* COMMAND_NAME         */ L"DeepSeek 餘額助手",
    /* PLUGIN_DESCRIPTION   */ L"在 TrafficMonitor 的工作列視窗中顯示 DeepSeek 官網餘額資訊，支援記錄餘額變化歷史。",
    /* PLUGIN_AUTHOR        */ L"神楽坂雅詩",
    /* DLG_TITLE            */ L"DeepSeek 設定",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"金鑰格式錯誤：必須以 sk- 開頭，後接32位小寫字母和數字",
    /* DLG_BTN_TEST_API     */ L"測試API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< 請先測試再儲存",
    /* DLG_LABEL_INTERVAL   */ L"更新間隔（秒）:",
    /* DLG_LABEL_TIMEOUT    */ L"請求逾時（秒）:",
    /* DLG_LABEL_HISTORY_COUNT */ L"歷史記錄數量:",
    /* DLG_LABEL_HISTORY    */ L"歷史變化記錄:",
    /* DLG_COL_TIME         */ L"時間",
    /* DLG_COL_AVAILABLE    */ L"可用",
    /* DLG_COL_TOTAL        */ L"總餘額",
    /* DLG_COL_GRANTED      */ L"贈送餘額",
    /* DLG_COL_TOPPED_UP    */ L"儲值餘額",
    /* DLG_COL_CURRENCY     */ L"幣別",
    /* DLG_BTN_OK           */ L"確定(&O)",
    /* DLG_BTN_CANCEL       */ L"取消(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"清除歷史(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"自動重新整理",
    /* STATUS_TESTING       */ L"正在測試 API...",
    /* STATUS_TEST_PASSED   */ L"API 測試通過",
    /* STATUS_TEST_FAILED   */ L"API 測試失敗",
    /* BOOL_YES             */ L"是",
    /* BOOL_NO              */ L"否",
    /* MSG_TEST_SUCCESS_TITLE*/ L"API 測試成功",
    /* MSG_TEST_SUCCESS_BODY */ L"API 測試通過！\n\n帳戶可用：%s\n總餘額：%s %s\n贈送餘額：%s %s\n儲值餘額：%s %s",
    /* MSG_TEST_FAILED_TITLE */ L"API 測試失敗",
    /* MSG_TEST_FAILED_BODY */ L"API 測試失敗！\n\n錯誤資訊：%s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"確認清除",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"確定要清除所有歷史記錄嗎？\n\n此操作無法復原。",
    /* MSG_INVALID_KEY_TITLE */ L"格式錯誤",
    /* MSG_INVALID_KEY_BODY */ L"API 金鑰格式無效，請檢查後重試。",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"參數錯誤",
    /* MSG_INTERVAL_ERROR_BODY*/ L"更新間隔必須大於請求逾時時間，\n否則上一個請求未完成就會發起下一個請求。",
    /* DLG_LABEL_LANGUAGE   */ L"顯示語言:",
    /* COMBO_LANGUAGE_AUTO  */ L"自動",
};
