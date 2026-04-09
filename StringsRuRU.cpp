/************************************************************************//**
 * @file    StringsRuRU.cpp
 * @brief   俄语翻译表
 * @details 所有 UI 文本的俄语翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsRuRU[] = {
    /* ITEM_NAME            */ L"DeepSeek Ассистент",
    /* COMMAND_NAME         */ L"Ассистент баланса DeepSeek",
    /* PLUGIN_DESCRIPTION   */ L"Плагин DeepSeek DeskBand",
    /* DLG_TITLE            */ L"Настройки DeepSeek",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Неверный формат ключа: должен начинаться с sk- и содержать 32 строчные буквы и цифры",
    /* DLG_BTN_TEST_API     */ L"Тест API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Проверьте перед сохранением",
    /* DLG_LABEL_INTERVAL   */ L"Интервал обновления (с):",
    /* DLG_LABEL_TIMEOUT    */ L"Тайм-аут запроса (с):",
    /* DLG_LABEL_HISTORY_COUNT */ L"Кол-во записей:",
    /* DLG_LABEL_HISTORY    */ L"История баланса:",
    /* DLG_COL_TIME         */ L"Время",
    /* DLG_COL_AVAILABLE    */ L"Доступен",
    /* DLG_COL_TOTAL        */ L"Общий баланс",
    /* DLG_COL_GRANTED      */ L"Предоставленный баланс",
    /* DLG_COL_TOPPED_UP    */ L"Пополненный баланс",
    /* DLG_COL_CURRENCY     */ L"Валюта",
    /* DLG_BTN_OK           */ L"ОК(&O)",
    /* DLG_BTN_CANCEL       */ L"Отмена(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Очистить историю(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Автообновление",
    /* STATUS_TESTING       */ L"Тестирование API...",
    /* STATUS_TEST_PASSED   */ L"Тест API пройден",
    /* STATUS_TEST_FAILED   */ L"Тест API не пройден",
    /* BOOL_YES             */ L"Да",
    /* BOOL_NO              */ L"Нет",
    /* MSG_TEST_SUCCESS_TITLE*/ L"Тест API Пройден",
    /* MSG_TEST_SUCCESS_BODY */ L"Тест API пройден!\n\nАккаунт доступен: %s\nОбщий баланс: %s %s\nПредоставленный баланс: %s %s\nПополненный баланс: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"Тест API Не Пройден",
    /* MSG_TEST_FAILED_BODY */ L"Тест API не пройден!\n\nОшибка: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Подтверждение очистки",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Вы уверены, что хотите очистить всю историю?\n\nЭто действие нельзя отменить.",
    /* MSG_INVALID_KEY_TITLE */ L"Неверный формат",
    /* MSG_INVALID_KEY_BODY */ L"Неверный формат ключа API. Проверьте и попробуйте снова.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Ошибка параметра",
    /* MSG_INTERVAL_ERROR_BODY*/ L"Интервал обновления должен быть больше тайм-аута запроса,\nиначе новый запрос начнётся до завершения предыдущего.",
    /* DLG_LABEL_LANGUAGE   */ L"Язык:",
    /* COMBO_LANGUAGE_AUTO  */ L"Авто",
};
