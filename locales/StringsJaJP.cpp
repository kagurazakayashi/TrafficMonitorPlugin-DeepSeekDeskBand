/************************************************************************//**
 * @file    StringsJaJP.cpp
 * @brief   日语翻译表
 * @details 所有 UI 文本的日语翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsJaJP[] = {
    /* ITEM_NAME            */ L"DeepSeek 残高アシスタント",
    /* COMMAND_NAME         */ L"DeepSeek 残高アシスタント",
    /* PLUGIN_DESCRIPTION   */ L"TrafficMonitor のタスクバーウィンドウに DeepSeek 公式サイトの残高情報を表示し、残高の変化履歴を記録できます。",
    /* PLUGIN_AUTHOR        */ L"神楽坂雅詩",
    /* DLG_TITLE            */ L"DeepSeek 設定",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"APIキー形式エラー：sk-で始まり32桁の小文字英数字が必要です",
    /* DLG_BTN_TEST_API     */ L"APIテスト(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< テストしてから保存してください",
    /* DLG_LABEL_INTERVAL   */ L"更新間隔（秒）:",
    /* DLG_LABEL_TIMEOUT    */ L"リクエストタイムアウト（秒）:",
    /* DLG_LABEL_HISTORY_COUNT */ L"履歴件数:",
    /* DLG_LABEL_HISTORY    */ L"残高変動履歴:",
    /* DLG_COL_TIME         */ L"日時",
    /* DLG_COL_AVAILABLE    */ L"有効",
    /* DLG_COL_TOTAL        */ L"合計残高",
    /* DLG_COL_GRANTED      */ L"付与残高",
    /* DLG_COL_TOPPED_UP    */ L"チャージ残高",
    /* DLG_COL_CURRENCY     */ L"通貨",
    /* DLG_BTN_OK           */ L"OK(&O)",
    /* DLG_BTN_CANCEL       */ L"キャンセル(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"履歴消去(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"自動更新",
    /* STATUS_TESTING       */ L"APIテスト中...",
    /* STATUS_TEST_PASSED   */ L"APIテスト成功",
    /* STATUS_TEST_FAILED   */ L"APIテスト失敗",
    /* BOOL_YES             */ L"はい",
    /* BOOL_NO              */ L"いいえ",
    /* MSG_TEST_SUCCESS_TITLE*/ L"APIテスト成功",
    /* MSG_TEST_SUCCESS_BODY */ L"APIテスト成功！\n\nアカウント有効：%s\n合計残高：%s %s\n付与残高：%s %s\nチャージ残高：%s %s",
    /* MSG_TEST_FAILED_TITLE */ L"APIテスト失敗",
    /* MSG_TEST_FAILED_BODY */ L"APIテスト失敗！\n\nエラー情報：%s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"消去確認",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"すべての履歴を消去しますか？\n\nこの操作は元に戻せません。",
    /* MSG_INVALID_KEY_TITLE */ L"形式エラー",
    /* MSG_INVALID_KEY_BODY */ L"APIキーの形式が無効です。確認してから再試行してください。",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"パラメータエラー",
    /* MSG_INTERVAL_ERROR_BODY*/ L"更新間隔はリクエストタイムアウトより大きくする必要があります。\nそうしないと前のリクエストが完了する前に次のリクエストが開始されます。",
    /* DLG_LABEL_LANGUAGE   */ L"表示言語:",
    /* COMBO_LANGUAGE_AUTO  */ L"自動",
};
