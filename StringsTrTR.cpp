/************************************************************************//**
 * @file    StringsTrTR.cpp
 * @brief   土耳其语翻译表
 * @details 所有 UI 文本的土耳其语翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsTrTR[] = {
    /* ITEM_NAME            */ L"DeepSeek Asistanı",
    /* COMMAND_NAME         */ L"DeepSeek Bakiye Asistanı",
    /* PLUGIN_DESCRIPTION   */ L"DeepSeek DeskBand Eklentisi",
    /* DLG_TITLE            */ L"DeepSeek Ayarları",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Geçersiz anahtar formatı: sk- ile başlamalı ve 32 küçük harf ve rakam içermelidir",
    /* DLG_BTN_TEST_API     */ L"API'yi Test Et(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Kaydetmeden önce test edin",
    /* DLG_LABEL_INTERVAL   */ L"Güncelleme aralığı (sn):",
    /* DLG_LABEL_TIMEOUT    */ L"İstek zaman aşımı (sn):",
    /* DLG_LABEL_HISTORY_COUNT */ L"Geçmiş sayısı:",
    /* DLG_LABEL_HISTORY    */ L"Bakiye geçmişi:",
    /* DLG_COL_TIME         */ L"Zaman",
    /* DLG_COL_AVAILABLE    */ L"Kullanılabilir",
    /* DLG_COL_TOTAL        */ L"Toplam Bakiye",
    /* DLG_COL_GRANTED      */ L"Verilen Bakiye",
    /* DLG_COL_TOPPED_UP    */ L"Yüklenen Bakiye",
    /* DLG_COL_CURRENCY     */ L"Para Birimi",
    /* DLG_BTN_OK           */ L"Tamam(&O)",
    /* DLG_BTN_CANCEL       */ L"İptal(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Geçmişi Temizle(&T)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Otomatik Yenileme",
    /* STATUS_TESTING       */ L"API test ediliyor...",
    /* STATUS_TEST_PASSED   */ L"API testi başarılı",
    /* STATUS_TEST_FAILED   */ L"API testi başarısız",
    /* BOOL_YES             */ L"Evet",
    /* BOOL_NO              */ L"Hayır",
    /* MSG_TEST_SUCCESS_TITLE*/ L"API Testi Başarılı",
    /* MSG_TEST_SUCCESS_BODY */ L"API testi başarılı!\n\nHesap kullanılabilir: %s\nToplam bakiye: %s %s\nVerilen bakiye: %s %s\nYüklenen bakiye: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"API Testi Başarısız",
    /* MSG_TEST_FAILED_BODY */ L"API testi başarısız!\n\nHata: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Temizlemeyi Onayla",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Tüm geçmişi temizlemek istediğinizden emin misiniz?\n\nBu işlem geri alınamaz.",
    /* MSG_INVALID_KEY_TITLE */ L"Geçersiz Format",
    /* MSG_INVALID_KEY_BODY */ L"Geçersiz API anahtarı formatı. Lütfen kontrol edip tekrar deneyin.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Parametre Hatası",
    /* MSG_INTERVAL_ERROR_BODY*/ L"Güncelleme aralığı istek zaman aşımından büyük olmalıdır,\naksi takdirde önceki istek tamamlanmadan yeni bir istek başlatılır.",
    /* DLG_LABEL_LANGUAGE   */ L"Dil:",
    /* COMBO_LANGUAGE_AUTO  */ L"Otomatik",
};
