/************************************************************************//**
 * @file    StringsHeIL.cpp
 * @brief   希伯来语翻译表
 * @details 所有 UI 文本的希伯来语翻译（右向左文字）。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsHeIL[] = {
    /* ITEM_NAME            */ L"DeepSeek עוזר יתרה",
    /* COMMAND_NAME         */ L"DeepSeek עוזר יתרה",
    /* PLUGIN_DESCRIPTION   */ L"מציג מידע על יתרת DeepSeek בחלון שורת המשימות של TrafficMonitor, עם מעקב אחר היסטוריית שינויים ביתרה.",
    /* PLUGIN_AUTHOR        */ L"KagurazakaYashi(Miyabi)",
    /* DLG_TITLE            */ L"הגדרות DeepSeek",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"פורמט מפתח לא חוקי: חייב להתחיל ב-sk- ואחריו 32 אותיות קטנות וספרות",
    /* DLG_BTN_TEST_API     */ L"בדוק API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< נא לבדוק לפני השמירה",
    /* DLG_LABEL_INTERVAL   */ L"מרווח עדכון (שניות):",
    /* DLG_LABEL_TIMEOUT    */ L"זמן תפוגה (שניות):",
    /* DLG_LABEL_HISTORY_COUNT */ L"מספר רשומות:",
    /* DLG_LABEL_HISTORY    */ L"היסטוריית יתרה:",
    /* DLG_COL_TIME         */ L"זמן",
    /* DLG_COL_AVAILABLE    */ L"זמין",
    /* DLG_COL_TOTAL        */ L"יתרה כוללת",
    /* DLG_COL_GRANTED      */ L"יתרה מוענקת",
    /* DLG_COL_TOPPED_UP    */ L"יתרה מוטענת",
    /* DLG_COL_CURRENCY     */ L"מטבע",
    /* DLG_BTN_OK           */ L"אישור(&O)",
    /* DLG_BTN_CANCEL       */ L"ביטול(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"נקה היסטוריה(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"רענון אוטומטי",
    /* STATUS_TESTING       */ L"בודק API...",
    /* STATUS_TEST_PASSED   */ L"בדיקת API הצליחה",
    /* STATUS_TEST_FAILED   */ L"בדיקת API נכשלה",
    /* BOOL_YES             */ L"כן",
    /* BOOL_NO              */ L"לא",
    /* MSG_TEST_SUCCESS_TITLE*/ L"בדיקת API הצליחה",
    /* MSG_TEST_SUCCESS_BODY */ L"בדיקת API הצליחה!\n\nחשבון זמין: %s\nיתרה כוללת: %s %s\nיתרה מוענקת: %s %s\nיתרה מוטענת: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"בדיקת API נכשלה",
    /* MSG_TEST_FAILED_BODY */ L"בדיקת API נכשלה!\n\nשגיאה: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"אישור ניקוי",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"האם אתה בטוח שברצונך לנקות את כל ההיסטוריה?\n\nלא ניתן לבטל פעולה זו.",
    /* MSG_INVALID_KEY_TITLE */ L"פורמט לא חוקי",
    /* MSG_INVALID_KEY_BODY */ L"פורמט מפתח API לא חוקי. נא לבדוק ולנסות שוב.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"שגיאת פרמטר",
    /* MSG_INTERVAL_ERROR_BODY*/ L"מרווח העדכון חייב להיות גדול מזמן התפוגה,\nאחרת תתחיל בקשה חדשה לפני שהקודמת מסתיימת.",
    /* DLG_LABEL_LANGUAGE   */ L"שפה:",
    /* COMBO_LANGUAGE_AUTO  */ L"אוטומטי",
};
