/************************************************************************//**
 * @file    StringsDeDE.cpp
 * @brief   德语翻译表
 * @details 所有 UI 文本的德语翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsDeDE[] = {
    /* ITEM_NAME            */ L"DeepSeek-Assistent",
    /* COMMAND_NAME         */ L"DeepSeek-Guthabenassistent",
    /* PLUGIN_DESCRIPTION   */ L"DeepSeek DeskBand-Plugin",
    /* DLG_TITLE            */ L"DeepSeek-Einstellungen",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Ungültiges Schlüsselformat: muss mit sk- beginnen, gefolgt von 32 Kleinbuchstaben und Ziffern",
    /* DLG_BTN_TEST_API     */ L"API testen(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Bitte vor dem Speichern testen",
    /* DLG_LABEL_INTERVAL   */ L"Aktualisierungsintervall (s):",
    /* DLG_LABEL_TIMEOUT    */ L"Anforderungs-Timeout (s):",
    /* DLG_LABEL_HISTORY_COUNT */ L"Verlaufsanzahl:",
    /* DLG_LABEL_HISTORY    */ L"Guthabenverlauf:",
    /* DLG_COL_TIME         */ L"Zeit",
    /* DLG_COL_AVAILABLE    */ L"Verfügbar",
    /* DLG_COL_TOTAL        */ L"Gesamtguthaben",
    /* DLG_COL_GRANTED      */ L"Gewährtes Guthaben",
    /* DLG_COL_TOPPED_UP    */ L"Aufgeladenes Guthaben",
    /* DLG_COL_CURRENCY     */ L"Währung",
    /* DLG_BTN_OK           */ L"OK(&O)",
    /* DLG_BTN_CANCEL       */ L"Abbrechen(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Verlauf löschen(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Autom. aktualisieren",
    /* STATUS_TESTING       */ L"API wird getestet...",
    /* STATUS_TEST_PASSED   */ L"API-Test erfolgreich",
    /* STATUS_TEST_FAILED   */ L"API-Test fehlgeschlagen",
    /* BOOL_YES             */ L"Ja",
    /* BOOL_NO              */ L"Nein",
    /* MSG_TEST_SUCCESS_TITLE*/ L"API-Test erfolgreich",
    /* MSG_TEST_SUCCESS_BODY */ L"API-Test erfolgreich!\n\nKonto verfügbar: %s\nGesamtguthaben: %s %s\nGewährtes Guthaben: %s %s\nAufgeladenes Guthaben: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"API-Test fehlgeschlagen",
    /* MSG_TEST_FAILED_BODY */ L"API-Test fehlgeschlagen!\n\nFehler: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Löschen bestätigen",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Möchten Sie wirklich den gesamten Verlauf löschen?\n\nDiese Aktion kann nicht rückgängig gemacht werden.",
    /* MSG_INVALID_KEY_TITLE */ L"Ungültiges Format",
    /* MSG_INVALID_KEY_BODY */ L"Ungültiges API-Schlüsselformat. Bitte überprüfen und erneut versuchen.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Parameterfehler",
    /* MSG_INTERVAL_ERROR_BODY*/ L"Das Aktualisierungsintervall muss größer als das Anforderungs-Timeout sein,\nandernfalls wird eine neue Anforderung gestartet, bevor die vorherige abgeschlossen ist.",
    /* DLG_LABEL_LANGUAGE   */ L"Sprache:",
    /* COMBO_LANGUAGE_AUTO  */ L"Automatisch",
};
