/************************************************************************//**
 * @file    StringsPtBR.cpp
 * @brief   葡萄牙语（巴西）翻译表
 * @details 所有 UI 文本的葡萄牙语（巴西）翻译。
 ****************************************************************************/
#include "Strings.h"

const wchar_t* kStringsPtBR[] = {
    /* ITEM_NAME            */ L"Assistente DeepSeek",
    /* COMMAND_NAME         */ L"Assistente de Saldo DeepSeek",
    /* PLUGIN_DESCRIPTION   */ L"Plugin DeepSeek DeskBand",
    /* DLG_TITLE            */ L"Configurações do DeepSeek",
    /* DLG_API_LABEL        */ L"DeepSeek API:",
    /* DLG_API_HINT         */ L"Formato de chave inválido: deve começar com sk- seguido por 32 letras minúsculas e dígitos",
    /* DLG_BTN_TEST_API     */ L"Testar API(&T)",
    /* DLG_STATUS_UNTESTED  */ L"< Teste antes de salvar",
    /* DLG_LABEL_INTERVAL   */ L"Intervalo de atualização (s):",
    /* DLG_LABEL_TIMEOUT    */ L"Tempo limite (s):",
    /* DLG_LABEL_HISTORY_COUNT */ L"Qtd. de histórico:",
    /* DLG_LABEL_HISTORY    */ L"Histórico de saldo:",
    /* DLG_COL_TIME         */ L"Horário",
    /* DLG_COL_AVAILABLE    */ L"Disponível",
    /* DLG_COL_TOTAL        */ L"Saldo Total",
    /* DLG_COL_GRANTED      */ L"Saldo Concedido",
    /* DLG_COL_TOPPED_UP    */ L"Saldo Recarregado",
    /* DLG_COL_CURRENCY     */ L"Moeda",
    /* DLG_BTN_OK           */ L"OK(&O)",
    /* DLG_BTN_CANCEL       */ L"Cancelar(&C)",
    /* DLG_BTN_CLEAR_HISTORY*/ L"Limpar Histórico(&R)",
    /* DLG_CHECK_AUTO_REFRESH*/ L"Atualização automática",
    /* STATUS_TESTING       */ L"Testando API...",
    /* STATUS_TEST_PASSED   */ L"Teste de API aprovado",
    /* STATUS_TEST_FAILED   */ L"Teste de API falhou",
    /* BOOL_YES             */ L"Sim",
    /* BOOL_NO              */ L"Não",
    /* MSG_TEST_SUCCESS_TITLE*/ L"Teste de API Aprovado",
    /* MSG_TEST_SUCCESS_BODY */ L"Teste de API aprovado!\n\nConta disponível: %s\nSaldo total: %s %s\nSaldo concedido: %s %s\nSaldo recarregado: %s %s",
    /* MSG_TEST_FAILED_TITLE */ L"Teste de API Falhou",
    /* MSG_TEST_FAILED_BODY */ L"Teste de API falhou!\n\nErro: %s",
    /* MSG_CONFIRM_CLEAR_TITLE*/ L"Confirmar Limpeza",
    /* MSG_CONFIRM_CLEAR_BODY*/ L"Tem certeza de que deseja limpar todo o histórico?\n\nEsta ação não pode ser desfeita.",
    /* MSG_INVALID_KEY_TITLE */ L"Formato Inválido",
    /* MSG_INVALID_KEY_BODY */ L"Formato de chave API inválido. Verifique e tente novamente.",
    /* MSG_INTERVAL_ERROR_TITLE*/ L"Erro de Parâmetro",
    /* MSG_INTERVAL_ERROR_BODY*/ L"O intervalo de atualização deve ser maior que o tempo limite,\ncaso contrário, uma nova solicitação começará antes que a anterior seja concluída.",
    /* DLG_LABEL_LANGUAGE   */ L"Idioma:",
    /* COMBO_LANGUAGE_AUTO  */ L"Automático",
};
