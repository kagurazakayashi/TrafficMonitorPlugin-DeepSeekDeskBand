/************************************************************************//**
 * @file    StringsItIT.cpp
 * @brief   意大利语翻译表
 * @details 所有 UI 文本的意大利语翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsItIT[] = {
    /* ITEM_NAME            */ L"Assistente Saldo DeepSeek",
    /* COMMAND_NAME         */ L"Assistente Saldo DeepSeek",
    /* PLUGIN_DESCRIPTION   */ L"Mostra le informazioni sul saldo DeepSeek nella finestra della barra delle applicazioni di TrafficMonitor, con cronologia delle variazioni del saldo.",
    /* PLUGIN_AUTHOR        */ L"KagurazakaYashi(Miyabi)",
    /* DLG_TITLE            */ L"Impostazioni DeepSeek",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Formato chiave non valido: deve iniziare con sk- seguito da 32 lettere minuscole e cifre",
    /* DLG_BTN_TEST_API     */ L"Testa API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Testare prima di salvare",
    /* DLG_LABEL_INTERVAL   */ L"Intervallo aggiornamento (s):",
    /* DLG_LABEL_TIMEOUT    */ L"Timeout richiesta (s):",
    /* DLG_LABEL_HISTORY_COUNT */ L"Numero cronologia:",
    /* DLG_LABEL_HISTORY    */ L"Cronologia saldo:",
    /* DLG_COL_TIME         */ L"Ora",
    /* DLG_COL_AVAILABLE    */ L"Disponibile",
    /* DLG_COL_TOTAL        */ L"Saldo totale",
    /* DLG_COL_GRANTED      */ L"Saldo concesso",
    /* DLG_COL_TOPPED_UP    */ L"Saldo ricaricato",
    /* DLG_COL_CURRENCY     */ L"Valuta",
    /* DLG_BTN_OK           */ L"OK(&O)",
    /* DLG_BTN_CANCEL       */ L"Annulla(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Cancella cronologia(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Aggiornamento automatico",
    /* STATUS_TESTING       */ L"Test API in corso...",
    /* STATUS_TEST_PASSED   */ L"Test API riuscito",
    /* STATUS_TEST_FAILED   */ L"Test API fallito",
    /* BOOL_YES             */ L"Sì",
    /* BOOL_NO              */ L"No",
    /* MSG_TEST_SUCCESS_TITLE*/ L"Test API Riuscito",
    /* MSG_TEST_SUCCESS_BODY */ L"Test API riuscito!\n\nAccount disponibile: %s\nSaldo totale: %s %s\nSaldo concesso: %s %s\nSaldo ricaricato: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"Test API Fallito",
    /* MSG_TEST_FAILED_BODY */ L"Test API fallito!\n\nErrore: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Conferma cancellazione",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Eliminare tutta la cronologia?\n\nQuesta azione non può essere annullata.",
    /* MSG_INVALID_KEY_TITLE */ L"Formato non valido",
    /* MSG_INVALID_KEY_BODY */ L"Formato chiave API non valido. Verificare e riprovare.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Errore parametro",
    /* MSG_INTERVAL_ERROR_BODY*/ L"L'intervallo di aggiornamento deve essere superiore al timeout,\naltrimenti una nuova richiesta partirà prima del completamento della precedente.",
    /* DLG_LABEL_LANGUAGE   */ L"Lingua:",
    /* COMBO_LANGUAGE_AUTO  */ L"Automatico",
};
