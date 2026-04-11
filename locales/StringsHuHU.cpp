/************************************************************************//**
 * @file    StringsHuHU.cpp
 * @brief   匈牙利语翻译表
 * @details 所有 UI 文本的匈牙利语翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsHuHU[] = {
    /* ITEM_NAME            */ L"DeepSeek Egyenleg Asszisztens",
    /* COMMAND_NAME         */ L"DeepSeek Egyenleg Asszisztens",
    /* PLUGIN_DESCRIPTION   */ L"Megjeleníti a DeepSeek egyenleginformációit a TrafficMonitor tálcaablakában, egyenlegváltozási előzményekkel.",
    /* PLUGIN_AUTHOR        */ L"KagurazakaYashi(Miyabi)",
    /* DLG_TITLE            */ L"DeepSeek Beállítások",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Érvénytelen kulcsformátum: sk- kezdetű, majd 32 kisbetű és számjegy szükséges",
    /* DLG_BTN_TEST_API     */ L"API tesztelése(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Mentés előtt tesztelje",
    /* DLG_LABEL_INTERVAL   */ L"Frissítési időköz (mp):",
    /* DLG_LABEL_TIMEOUT    */ L"Kérés időtúllépés (mp):",
    /* DLG_LABEL_HISTORY_COUNT */ L"Előzmények száma:",
    /* DLG_LABEL_HISTORY    */ L"Egyenleg előzmények:",
    /* DLG_COL_TIME         */ L"Idő",
    /* DLG_COL_AVAILABLE    */ L"Elérhető",
    /* DLG_COL_TOTAL        */ L"Teljes egyenleg",
    /* DLG_COL_GRANTED      */ L"Adományozott egyenleg",
    /* DLG_COL_TOPPED_UP    */ L"Feltöltött egyenleg",
    /* DLG_COL_CURRENCY     */ L"Pénznem",
    /* DLG_BTN_OK           */ L"OK(&O)",
    /* DLG_BTN_CANCEL       */ L"Mégse(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Előzmények törlése(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Automatikus frissítés",
    /* STATUS_TESTING       */ L"API tesztelése...",
    /* STATUS_TEST_PASSED   */ L"API teszt sikeres",
    /* STATUS_TEST_FAILED   */ L"API teszt sikertelen",
    /* BOOL_YES             */ L"Igen",
    /* BOOL_NO              */ L"Nem",
    /* MSG_TEST_SUCCESS_TITLE*/ L"API Teszt Sikeres",
    /* MSG_TEST_SUCCESS_BODY */ L"API teszt sikeres!\n\nFiók elérhető: %s\nTeljes egyenleg: %s %s\nAdományozott egyenleg: %s %s\nFeltöltött egyenleg: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"API Teszt Sikertelen",
    /* MSG_TEST_FAILED_BODY */ L"API teszt sikertelen!\n\nHiba: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Törlés megerősítése",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Biztosan törli az összes előzményt?\n\nEz a művelet nem vonható vissza.",
    /* MSG_INVALID_KEY_TITLE */ L"Érvénytelen formátum",
    /* MSG_INVALID_KEY_BODY */ L"Érvénytelen API kulcs formátum. Ellenőrizze és próbálja újra.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Paraméterhiba",
    /* MSG_INTERVAL_ERROR_BODY*/ L"A frissítési időköznek nagyobbnak kell lennie a kérés időtúllépésénél,\nkülönben új kérés indul a befejezés előtt.",
    /* DLG_LABEL_LANGUAGE   */ L"Nyelv:",
    /* COMBO_LANGUAGE_AUTO  */ L"Automatikus",
};
