/************************************************************************//**
 * @file    StringsPlPL.cpp
 * @brief   波兰语翻译表
 * @details 所有 UI 文本的波兰语翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsPlPL[] = {
    /* ITEM_NAME            */ L"Asystent DeepSeek",
    /* COMMAND_NAME         */ L"Asystent salda DeepSeek",
    /* PLUGIN_DESCRIPTION   */ L"Wtyczka DeepSeek DeskBand",
    /* DLG_TITLE            */ L"Ustawienia DeepSeek",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Nieprawidłowy format klucza: musi zaczynać się od sk- i zawierać 32 małe litery i cyfry",
    /* DLG_BTN_TEST_API     */ L"Testuj API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Przetestuj przed zapisaniem",
    /* DLG_LABEL_INTERVAL   */ L"Interwał aktualizacji (s):",
    /* DLG_LABEL_TIMEOUT    */ L"Limit czasu żądania (s):",
    /* DLG_LABEL_HISTORY_COUNT */ L"Liczba wpisów:",
    /* DLG_LABEL_HISTORY    */ L"Historia salda:",
    /* DLG_COL_TIME         */ L"Czas",
    /* DLG_COL_AVAILABLE    */ L"Dostępne",
    /* DLG_COL_TOTAL        */ L"Saldo całkowite",
    /* DLG_COL_GRANTED      */ L"Saldo przyznane",
    /* DLG_COL_TOPPED_UP    */ L"Saldo doładowane",
    /* DLG_COL_CURRENCY     */ L"Waluta",
    /* DLG_BTN_OK           */ L"OK(&O)",
    /* DLG_BTN_CANCEL       */ L"Anuluj(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Wyczyść historię(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Automatyczne odświeżanie",
    /* STATUS_TESTING       */ L"Testowanie API...",
    /* STATUS_TEST_PASSED   */ L"Test API pomyślny",
    /* STATUS_TEST_FAILED   */ L"Test API nieudany",
    /* BOOL_YES             */ L"Tak",
    /* BOOL_NO              */ L"Nie",
    /* MSG_TEST_SUCCESS_TITLE*/ L"Test API Pomyślny",
    /* MSG_TEST_SUCCESS_BODY */ L"Test API pomyślny!\n\nKonto dostępne: %s\nSaldo całkowite: %s %s\nSaldo przyznane: %s %s\nSaldo doładowane: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"Test API Nieudany",
    /* MSG_TEST_FAILED_BODY */ L"Test API nieudany!\n\nBłąd: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Potwierdź wyczyszczenie",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Czy na pewno chcesz wyczyścić całą historię?\n\nTej operacji nie można cofnąć.",
    /* MSG_INVALID_KEY_TITLE */ L"Nieprawidłowy format",
    /* MSG_INVALID_KEY_BODY */ L"Nieprawidłowy format klucza API. Sprawdź i spróbuj ponownie.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Błąd parametru",
    /* MSG_INTERVAL_ERROR_BODY*/ L"Interwał aktualizacji musi być większy niż limit czasu żądania,\nw przeciwnym razie nowe żądanie rozpocznie się przed zakończeniem poprzedniego.",
    /* DLG_LABEL_LANGUAGE   */ L"Język:",
    /* COMBO_LANGUAGE_AUTO  */ L"Automatyczny",
};
