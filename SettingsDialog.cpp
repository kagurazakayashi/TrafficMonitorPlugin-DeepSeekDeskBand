/************************************************************************//**
 * @file    SettingsDialog.cpp
 * @brief   设置对话框模块实现
 * @details 实现完整的设置对话框 UI，包括：
 *          - 自绘 Win32 窗口（无 MFC 依赖）
 *          - DPI 感知布局
 *          - 多语言即时切换
 *          - API 密钥校验与测试
 *          - DeepSeek 图标异步下载
 *          - 历史记录 ListView（可自动刷新）
 *          - 可调大小的模态窗口
 ****************************************************************************/
#include "SettingsDialog.h"
#include "Logger.h"
#include "framework.h"
#include <CommCtrl.h>
#include <urlmon.h>
#include <string>
#include <stdio.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "urlmon.lib")

// ============================================================
// 全局状态
// ============================================================

/** @brief 当前对话框 DPI 值（96 为 100%） */
static int g_dlgDpi = 96;

/** @brief 防止语言刷新触发的 CBN_SELCHANGE 导致递归调用 */
static bool g_bRefreshingLanguage = false;

/** @brief 设置对话框窗口类名 */
static const wchar_t* SETTINGS_DIALOG_CLASS = L"DeepSeekDeskBandSettingsDlg";

// ============================================================
// 辅助函数
// ============================================================

/** @brief 按 DPI 缩放像素值 */
static inline int DlgScale(int value)
{
    return MulDiv(value, g_dlgDpi, 96);
}

/**
 * @brief 校验 DeepSeek API 密钥格式
 * @param key 待校验的密钥字符串
 * @return 格式正确返回 true
 * @note  有效格式：sk- 开头 + 恰好 32 位小写字母和数字 [a-z0-9]
 */
static bool IsValidApiKey(const wchar_t* key)
{
    size_t prefixLen = wcslen(DSDB_APIKEY_PREFIX);
    if (!key || wcsncmp(key, DSDB_APIKEY_PREFIX, prefixLen) != 0)
        return false;

    const wchar_t* p = key + prefixLen;
    int len = 0;
    while (*p)
    {
        if (!((*p >= L'a' && *p <= L'z') || (*p >= L'0' && *p <= L'9')))
            return false;
        p++;
        len++;
    }

    return (len == DSDB_APIKEY_SUFFIX_LEN);
}

/**
 * @brief 更新 API 相关控件的启用/显示状态
 * @param hDlg       对话框句柄
 * @param hasContent 输入框是否非空
 * @param valid      API 密钥格式是否有效
 * @param tested     API 是否已通过测试
 */
static void UpdateApiDependentControls(HWND hDlg, bool hasContent, bool valid, bool tested)
{
    // 仅当有输入且格式无效时显示格式错误提示
    ShowWindow(GetDlgItem(hDlg, IDC_STATIC_API_HINT), (hasContent && !valid) ? SW_SHOW : SW_HIDE);
    // 测试按钮：仅当密钥非空且格式有效时可用
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_TEST_API), hasContent && valid);
    // 确定按钮：密钥为空或已通过测试的有效密钥时可保存
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_OK), !hasContent || (valid && tested));
}

/**
 * @brief 刷新 ListView 中的历史记录数据
 * @param hList    ListView 控件句柄
 * @param records  历史记录列表（只读）
 */
static void RefreshHistoryList(HWND hList, const std::vector<HistoryRecord>* records)
{
    ListView_DeleteAllItems(hList);

    if (!records || records->empty())
        return;

    // 反向遍历：新记录靠前显示
    for (size_t i = records->size(); i > 0; i--)
    {
        const HistoryRecord& rec = (*records)[i - 1];
        int rowIndex = static_cast<int>(records->size() - i);

        // 列 0：时间戳格式化为可读时间
        wchar_t timeStr[32];
        if (rec.timestamp > 0)
        {
            ULONGLONG ft64 = static_cast<ULONGLONG>(rec.timestamp) * 10000ULL + 116444736000000000ULL;
            FILETIME ft;
            ft.dwLowDateTime = static_cast<DWORD>(ft64 & 0xFFFFFFFF);
            ft.dwHighDateTime = static_cast<DWORD>((ft64 >> 32) & 0xFFFFFFFF);
            SYSTEMTIME st;
            FileTimeToSystemTime(&ft, &st);

            SYSTEMTIME stLocal;
            SystemTimeToTzSpecificLocalTime(nullptr, &st, &stLocal);

            wchar_t dateStr[16];
            GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLocal, nullptr, dateStr, 16);
            wchar_t timePart[16];
            GetTimeFormatW(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &stLocal, nullptr, timePart, 16);
            swprintf_s(timeStr, L"%s %s", dateStr, timePart);
        }
        else
        {
            wcscpy_s(timeStr, L"--");
        }

        // 插入一行
        LVITEMW lvi = {};
        lvi.iItem = rowIndex;
        ListView_InsertItem(hList, &lvi);
        ListView_SetItemText(hList, rowIndex, 0, timeStr);
        ListView_SetItemText(hList, rowIndex, 1,
            const_cast<wchar_t*>(
                rec.is_available ? Strings_Get(StringKey::BOOL_YES) : Strings_Get(StringKey::BOOL_NO)));

        wchar_t numStr[32];
        swprintf_s(numStr, L"%.2f", rec.total_balance);
        ListView_SetItemText(hList, rowIndex, 2, numStr);
        swprintf_s(numStr, L"%.2f", rec.granted_balance);
        ListView_SetItemText(hList, rowIndex, 3, numStr);
        swprintf_s(numStr, L"%.2f", rec.topped_up_balance);
        ListView_SetItemText(hList, rowIndex, 4, numStr);
        ListView_SetItemText(hList, rowIndex, 5, const_cast<wchar_t*>(rec.currency));
    }
}

/**
 * @brief 根据当前语言刷新对话框上所有控件的文本
 * @param hDlg  对话框句柄
 * @param pData 对话框数据
 */
static void RefreshDialogLanguage(HWND hDlg, const SettingsDlgData* pData)
{
    g_bRefreshingLanguage = true;

    // 窗口标题
    SetWindowTextW(hDlg, Strings_Get(StringKey::DLG_TITLE));

    // 标签
    SetDlgItemTextW(hDlg, IDC_STATIC_API_LABEL,           Strings_Get(StringKey::DLG_API_LABEL));
    SetDlgItemTextW(hDlg, IDC_STATIC_API_HINT,             Strings_Get(StringKey::DLG_API_HINT));
    SetDlgItemTextW(hDlg, IDC_STATIC_INTERVAL_LABEL,       Strings_Get(StringKey::DLG_LABEL_INTERVAL));
    SetDlgItemTextW(hDlg, IDC_STATIC_TIMEOUT_LABEL,        Strings_Get(StringKey::DLG_LABEL_TIMEOUT));
    SetDlgItemTextW(hDlg, IDC_STATIC_HISTORY_COUNT_LABEL,  Strings_Get(StringKey::DLG_LABEL_HISTORY_COUNT));
    SetDlgItemTextW(hDlg, IDC_STATIC_HISTORY_LABEL,        Strings_Get(StringKey::DLG_LABEL_HISTORY));
    SetDlgItemTextW(hDlg, IDC_STATIC_LANGUAGE_LABEL,       Strings_Get(StringKey::DLG_LABEL_LANGUAGE));

    // 按钮
    SetDlgItemTextW(hDlg, IDC_BTN_TEST_API,       Strings_Get(StringKey::DLG_BTN_TEST_API));
    SetDlgItemTextW(hDlg, IDC_BTN_OK,             Strings_Get(StringKey::DLG_BTN_OK));
    SetDlgItemTextW(hDlg, IDC_BTN_CANCEL,         Strings_Get(StringKey::DLG_BTN_CANCEL));
    SetDlgItemTextW(hDlg, IDC_BTN_CLEAR_HISTORY,  Strings_Get(StringKey::DLG_BTN_CLEAR_HISTORY));
    SetDlgItemTextW(hDlg, IDC_CHECK_AUTO_REFRESH,  Strings_Get(StringKey::DLG_CHECK_AUTO_REFRESH));

    // 状态文本（仅当处于未测试状态时刷新）
    if (!pData->apiTested && pData->apiKey[0] != L'\0')
        SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, Strings_Get(StringKey::DLG_STATUS_UNTESTED));

    // ListView 列头
    HWND hList = GetDlgItem(hDlg, IDC_LIST_HISTORY);
    if (hList)
    {
        LVCOLUMNW lvc = {};
        lvc.mask = LVCF_TEXT;
        lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_TIME));
        ListView_SetColumn(hList, 0, &lvc);
        lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_AVAILABLE));
        ListView_SetColumn(hList, 1, &lvc);
        lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_TOTAL));
        ListView_SetColumn(hList, 2, &lvc);
        lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_GRANTED));
        ListView_SetColumn(hList, 3, &lvc);
        lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_TOPPED_UP));
        ListView_SetColumn(hList, 4, &lvc);
        lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_CURRENCY));
        ListView_SetColumn(hList, 5, &lvc);

        // 刷新列表数据（"是"/"否" 文本）
        RefreshHistoryList(hList, pData->historyRecords);
    }

    // 语言下拉框 "自动" 项
    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_LANGUAGE);
    if (hCombo)
    {
        int curSel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
        SendMessageW(hCombo, CB_DELETESTRING, 0, 0);
        SendMessageW(hCombo, CB_INSERTSTRING, 0,
            (LPARAM)Strings_Get(StringKey::COMBO_LANGUAGE_AUTO));
        if (curSel >= 0)
            SendMessageW(hCombo, CB_SETCURSEL, (WPARAM)curSel, 0);
    }

    // 刷新 API 提示和按钮状态（联动）
    wchar_t bufKey[DSDB_BUF_APIKEY];
    GetDlgItemTextW(hDlg, IDC_EDIT_API_KEY, bufKey, DSDB_BUF_APIKEY);
    bool hasContent = (bufKey[0] != L'\0');
    bool valid = IsValidApiKey(bufKey);
    UpdateApiDependentControls(hDlg, hasContent, valid, pData->apiTested);

    g_bRefreshingLanguage = false;
}

// ============================================================
// 图标下载线程
// ============================================================

/**
 * @brief 异步下载 DeepSeek 图标线程过程
 * @param lpParam IconDownloadParam 指针（线程负责 delete）
 * @return 始终返回 0
 */
static DWORD WINAPI DownloadIconThreadProc(LPVOID lpParam)
{
    IconDownloadParam* p = static_cast<IconDownloadParam*>(lpParam);

    Logger_Debug(L"DownloadIconThread: 开始下载 DeepSeek 图标");

    HRESULT hr = URLDownloadToFileW(nullptr,
        L"https://www.deepseek.com/favicon.ico",
        p->path, 0, nullptr);

    if (SUCCEEDED(hr))
        Logger_Info(L"DownloadIconThread: 下载成功 \"%s\"", p->path);
    else
        Logger_Warn(L"DownloadIconThread: 下载失败, hr=0x%08X", hr);

    // 通知对话框刷新图标（无论成功与否，让对话框尝试加载）
    PostMessageW(p->hWnd, WM_ICON_DOWNLOADED, SUCCEEDED(hr) ? 1 : 0, 0);

    delete p;
    return 0;
}

// ============================================================
// 对话框窗口过程
// ============================================================

/**
 * @brief 设置对话框窗口过程
 * @details 处理所有窗口消息，管理控件创建、事件响应、DPI 缩放等。
 */
static LRESULT CALLBACK SettingsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SettingsDlgData* pData = (SettingsDlgData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (msg)
    {
    // ============================================================
    // WM_CREATE：创建所有子控件
    // ============================================================
    case WM_CREATE:
    {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

        // 从 CREATESTRUCT 中取出对话框数据
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pData = (SettingsDlgData*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

        Logger_Debug(L"SettingsDlg: WM_CREATE, DPI=%d", g_dlgDpi);

        // ---- 创建系统消息字体（Segoe UI 9pt，现代化外观） ----
        NONCLIENTMETRICSW ncm = { sizeof(NONCLIENTMETRICSW) };
        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        ncm.lfMessageFont.lfHeight = -MulDiv(9, g_dlgDpi, 72);  // 9pt DPI 适配
        ncm.lfMessageFont.lfWeight = FW_NORMAL;
        HFONT hFont = CreateFontIndirectW(&ncm.lfMessageFont);
        SetPropW(hWnd, L"hFont", hFont);       // 存储以便在 WM_DESTROY 释放

        HWND hChild;

        // --- DeepSeek 图标（顶部居中，32x32 DPI 适配） ---
        hChild = CreateWindowW(L"STATIC", nullptr,
            WS_CHILD | WS_VISIBLE | SS_ICON,
            DlgScale(280), DlgScale(8), 0, 0,
            hWnd, (HMENU)IDC_STATIC_ICON, hInst, nullptr);
        HICON hIcon = (HICON)LoadImageW(nullptr, pData->iconPath, IMAGE_ICON,
            DlgScale(32), DlgScale(32), LR_LOADFROMFILE);
        if (hIcon)
        {
            SendMessageW(hChild, STM_SETICON, (WPARAM)hIcon, 0);
            // 设置为窗口图标（注意：WM_SETICON 需要独立句柄，这里用于临时显示）
            SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)CopyIcon(hIcon));
            SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)CopyIcon(hIcon));
            // STM_SETICON 后 static 控件接管句柄所有权，不需要额外 DestroyIcon
        }
        // 记录旧图标句柄以便在 WM_ICON_DOWNLOADED 中释放
        SetPropW(hWnd, L"hIcon", (HANDLE)hIcon);

        // --- "显示语言:" 标签（右上角） ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_LABEL_LANGUAGE),
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            DlgScale(440), DlgScale(10), DlgScale(65), DlgScale(20),
            hWnd, (HMENU)IDC_STATIC_LANGUAGE_LABEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 语言下拉框 ---
        {
            HWND hCombo = CreateWindowW(WC_COMBOBOXW, nullptr,
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_TABSTOP,
                DlgScale(510), DlgScale(8), DlgScale(120), DlgScale(200),
                hWnd, (HMENU)IDC_COMBO_LANGUAGE, hInst, nullptr);
            SendMessageW(hCombo, WM_SETFONT, (WPARAM)hFont, TRUE);

            SendMessageW(hCombo, CB_ADDSTRING, 0,
                (LPARAM)Strings_Get(StringKey::COMBO_LANGUAGE_AUTO));
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"简体中文");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"繁體中文");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"日本語");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"English");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Deutsch");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"עברית");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Magyar");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Italiano");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Polski");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Português (Brasil)");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Русский");
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Türkçe");

            int selIdx = static_cast<int>(pData->language);
            SendMessageW(hCombo, CB_SETCURSEL, (WPARAM)selIdx, 0);
        }

        // --- "DeepSeek API:" 标签 ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_API_LABEL),
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            DlgScale(20), DlgScale(72), DlgScale(125), DlgScale(20),
            hWnd, (HMENU)IDC_STATIC_API_LABEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- API 密钥输入框（密码模式） ---
        hChild = CreateWindowW(L"EDIT", pData->apiKey,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL | WS_TABSTOP,
            DlgScale(158), DlgScale(69), DlgScale(360), DlgScale(22),
            hWnd, (HMENU)IDC_EDIT_API_KEY, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- API 格式校验提示（默认隐藏） ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_API_HINT),
            WS_CHILD | SS_LEFT,
            DlgScale(158), DlgScale(94), DlgScale(370), DlgScale(16),
            hWnd, (HMENU)IDC_STATIC_API_HINT, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "测试API(&T)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", Strings_Get(StringKey::DLG_BTN_TEST_API),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            DlgScale(158), DlgScale(124), DlgScale(100), DlgScale(24),
            hWnd, (HMENU)IDC_BTN_TEST_API, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 测试结果状态文本 ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_STATUS_UNTESTED),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            DlgScale(268), DlgScale(127), DlgScale(290), DlgScale(20),
            hWnd, (HMENU)IDC_STATIC_STATUS, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "更新间隔（秒）:" 标签 ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_LABEL_INTERVAL),
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            DlgScale(20), DlgScale(174), DlgScale(125), DlgScale(20),
            hWnd, (HMENU)IDC_STATIC_INTERVAL_LABEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 更新间隔输入框 ---
        {
            wchar_t bufInterval[DSDB_BUF_NUMBER];
            swprintf_s(bufInterval, L"%d", pData->updateInterval);
            hChild = CreateWindowW(L"EDIT", bufInterval,
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_TABSTOP,
                DlgScale(158), DlgScale(171), DlgScale(100), DlgScale(22),
                hWnd, (HMENU)IDC_EDIT_INTERVAL, hInst, nullptr);
            SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // --- 更新间隔 Spin ---
        hChild = CreateWindowW(UPDOWN_CLASSW, nullptr,
            WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_SPIN_INTERVAL, hInst, nullptr);
        SendMessageW(hChild, UDM_SETRANGE32, DSDB_INTERVAL_MIN, DSDB_INTERVAL_MAX);
        SendMessageW(hChild, UDM_SETPOS32, 0, pData->updateInterval);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "请求超时（秒）:" 标签 ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_LABEL_TIMEOUT),
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            DlgScale(310), DlgScale(174), DlgScale(125), DlgScale(20),
            hWnd, (HMENU)IDC_STATIC_TIMEOUT_LABEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 请求超时输入框 ---
        {
            wchar_t bufTimeout[DSDB_BUF_NUMBER];
            swprintf_s(bufTimeout, L"%d", pData->requestTimeout);
            hChild = CreateWindowW(L"EDIT", bufTimeout,
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_TABSTOP,
                DlgScale(445), DlgScale(171), DlgScale(100), DlgScale(22),
                hWnd, (HMENU)IDC_EDIT_TIMEOUT, hInst, nullptr);
            SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // --- 超时 Spin ---
        {
            HWND hSpin = CreateWindowW(UPDOWN_CLASSW, nullptr,
                WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS,
                0, 0, 0, 0,
                hWnd, (HMENU)IDC_SPIN_TIMEOUT, hInst, nullptr);
            SendMessageW(hSpin, UDM_SETRANGE32, DSDB_TIMEOUT_MIN, DSDB_TIMEOUT_MAX);
            SendMessageW(hSpin, UDM_SETPOS32, 0, pData->requestTimeout);
            SendMessageW(hSpin, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // --- "历史记录数量:" 标签 ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_LABEL_HISTORY_COUNT),
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            DlgScale(20), DlgScale(224), DlgScale(125), DlgScale(20),
            hWnd, (HMENU)IDC_STATIC_HISTORY_COUNT_LABEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 历史记录数量输入框 ---
        {
            wchar_t bufHistoryCount[DSDB_BUF_NUMBER];
            swprintf_s(bufHistoryCount, L"%d", pData->maxHistoryCount);
            hChild = CreateWindowW(L"EDIT", bufHistoryCount,
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_TABSTOP,
                DlgScale(158), DlgScale(221), DlgScale(100), DlgScale(22),
                hWnd, (HMENU)IDC_EDIT_HISTORY_COUNT, hInst, nullptr);
            SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // --- 历史记录数量 Spin ---
        {
            HWND hSpin = CreateWindowW(UPDOWN_CLASSW, nullptr,
                WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS,
                0, 0, 0, 0,
                hWnd, (HMENU)IDC_SPIN_HISTORY_COUNT, hInst, nullptr);
            SendMessageW(hSpin, UDM_SETRANGE32, DSDB_HISTORY_COUNT_MIN, DSDB_HISTORY_COUNT_MAX);
            SendMessageW(hSpin, UDM_SETPOS32, 0, pData->maxHistoryCount);
            SendMessageW(hSpin, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // --- "自动刷新" 复选框 ---
        hChild = CreateWindowW(L"BUTTON", Strings_Get(StringKey::DLG_CHECK_AUTO_REFRESH),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
            DlgScale(288), DlgScale(223), DlgScale(95), DlgScale(22),
            hWnd, (HMENU)IDC_CHECK_AUTO_REFRESH, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(hChild, BM_SETCHECK, BST_CHECKED, 0);
        pData->autoRefresh = true;

        // --- "历史记录:" 标签 ---
        hChild = CreateWindowW(L"STATIC", Strings_Get(StringKey::DLG_LABEL_HISTORY),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            DlgScale(20), DlgScale(256), DlgScale(100), DlgScale(20),
            hWnd, (HMENU)IDC_STATIC_HISTORY_LABEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 历史记录 ListView ---
        {
            HWND hList = CreateWindowW(WC_LISTVIEWW, nullptr,
                WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL |
                LVS_SHOWSELALWAYS | WS_TABSTOP,
                DlgScale(20), DlgScale(278), DlgScale(550), DlgScale(170),
                hWnd, (HMENU)IDC_LIST_HISTORY, hInst, nullptr);
            SendMessageW(hList, WM_SETFONT, (WPARAM)hFont, TRUE);

            ListView_SetExtendedListViewStyle(hList,
                LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

            LVCOLUMNW lvc = {};
            lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
            lvc.fmt = LVCFMT_LEFT;

            lvc.cx = DlgScale(120); lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_TIME));
            ListView_InsertColumn(hList, 0, &lvc);
            lvc.cx = DlgScale(60);  lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_AVAILABLE));
            ListView_InsertColumn(hList, 1, &lvc);
            lvc.cx = DlgScale(100); lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_TOTAL));
            ListView_InsertColumn(hList, 2, &lvc);
            lvc.cx = DlgScale(100); lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_GRANTED));
            ListView_InsertColumn(hList, 3, &lvc);
            lvc.cx = DlgScale(100); lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_TOPPED_UP));
            ListView_InsertColumn(hList, 4, &lvc);
            lvc.cx = DlgScale(65);  lvc.pszText = const_cast<wchar_t*>(Strings_Get(StringKey::DLG_COL_CURRENCY));
            ListView_InsertColumn(hList, 5, &lvc);

            RefreshHistoryList(hList, pData->historyRecords);
        }

        // --- 启动历史刷新定时器（1 秒间隔） ---
        SetTimer(hWnd, 1, 1000, nullptr);

        // --- "确定(&O)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", Strings_Get(StringKey::DLG_BTN_OK),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED | WS_TABSTOP,
            DlgScale(390), DlgScale(460), DlgScale(95), DlgScale(28),
            hWnd, (HMENU)IDC_BTN_OK, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "取消(&C)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", Strings_Get(StringKey::DLG_BTN_CANCEL),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            DlgScale(485), DlgScale(460), DlgScale(95), DlgScale(28),
            hWnd, (HMENU)IDC_BTN_CANCEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "清除历史(&R)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", Strings_Get(StringKey::DLG_BTN_CLEAR_HISTORY),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            DlgScale(20), DlgScale(460), DlgScale(115), DlgScale(28),
            hWnd, (HMENU)IDC_BTN_CLEAR_HISTORY, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // ---- 初始状态：校验已加载的 API 密钥 ----
        pData->apiTested = IsValidApiKey(pData->apiKey);
        {
            bool hasContent = (pData->apiKey[0] != L'\0');
            bool valid = IsValidApiKey(pData->apiKey);
            UpdateApiDependentControls(hWnd, hasContent, valid, pData->apiTested);
        }

        SetFocus(GetDlgItem(hWnd, IDC_EDIT_API_KEY));
        Logger_Debug(L"SettingsDlg: WM_CREATE 完成");
        return 0;
    }

    // ============================================================
    // WM_CTLCOLORSTATIC：自定义绘制静态控件
    // ============================================================
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        HWND hCtrl = (HWND)lParam;

        // "密钥格式错误"提示使用红色文字
        if (hCtrl == GetDlgItem(hWnd, IDC_STATIC_API_HINT))
            SetTextColor(hdcStatic, RGB(0xE0, 0x10, 0x10));

        // 所有 STATIC 控件透明背景
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }

    // ============================================================
    // WM_ICON_DOWNLOADED：异步图标下载完成
    // ============================================================
    case WM_ICON_DOWNLOADED:
    {
        if (wParam != 0 && pData)
        {
            HICON hNewIcon = (HICON)LoadImageW(nullptr, pData->iconPath, IMAGE_ICON,
                DlgScale(32), DlgScale(32), LR_LOADFROMFILE);
            if (hNewIcon)
            {
                // 获取旧图标并销毁（避免 GDI 资源泄漏）
                HWND hIconCtrl = GetDlgItem(hWnd, IDC_STATIC_ICON);
                if (hIconCtrl)
                {
                    HICON hOldIcon = (HICON)SendMessageW(hIconCtrl, STM_GETICON, 0, 0);
                    SendMessageW(hIconCtrl, STM_SETICON, (WPARAM)hNewIcon, 0);
                    if (hOldIcon)
                        DestroyIcon(hOldIcon);
                }

                // 更新窗口图标（释放旧的）
                HICON hOldSmall = (HICON)SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)CopyIcon(hNewIcon));
                HICON hOldBig   = (HICON)SendMessageW(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)CopyIcon(hNewIcon));
                if (hOldSmall) DestroyIcon(hOldSmall);
                if (hOldBig)   DestroyIcon(hOldBig);

                Logger_Debug(L"SettingsDlg: 图标下载完成并已更新");
            }
        }
        return 0;
    }

    // ============================================================
    // WM_TIMER：历史记录自动刷新
    // ============================================================
    case WM_TIMER:
    {
        if (wParam == 1 && pData && pData->autoRefresh)
        {
            HWND hList = GetDlgItem(hWnd, IDC_LIST_HISTORY);
            if (hList)
                RefreshHistoryList(hList, pData->historyRecords);
        }
        return 0;
    }

    // ============================================================
    // WM_COMMAND：控件事件
    // ============================================================
    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        switch (id)
        {
        // --- API 密钥编辑变更 ---
        case IDC_EDIT_API_KEY:
            if (code == EN_CHANGE && pData)
            {
                pData->changed = true;
                pData->apiTested = false;   // API 内容变更后需重新测试

                wchar_t buf[DSDB_BUF_APIKEY];
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, buf, DSDB_BUF_APIKEY);
                bool valid = IsValidApiKey(buf);
                UpdateApiDependentControls(hWnd, buf[0] != L'\0', valid, pData->apiTested);
            }
            break;

        // --- 更新间隔变更 ---
        case IDC_EDIT_INTERVAL:
            if (code == EN_CHANGE && pData)
            {
                pData->changed = true;

                wchar_t buf[DSDB_BUF_APIKEY];
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, buf, DSDB_BUF_APIKEY);
                bool apiValid = IsValidApiKey(buf);
                UpdateApiDependentControls(hWnd, buf[0] != L'\0', apiValid, pData->apiTested);
            }
            break;

        // --- 请求超时变更 ---
        case IDC_EDIT_TIMEOUT:
            if (code == EN_CHANGE && pData)
            {
                pData->changed = true;

                wchar_t buf[DSDB_BUF_APIKEY];
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, buf, DSDB_BUF_APIKEY);
                bool apiValid = IsValidApiKey(buf);
                UpdateApiDependentControls(hWnd, buf[0] != L'\0', apiValid, pData->apiTested);
            }
            break;

        // --- 历史记录数量变更 ---
        case IDC_EDIT_HISTORY_COUNT:
            if (code == EN_CHANGE && pData)
            {
                pData->changed = true;

                wchar_t buf[DSDB_BUF_APIKEY];
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, buf, DSDB_BUF_APIKEY);
                bool apiValid = IsValidApiKey(buf);
                UpdateApiDependentControls(hWnd, buf[0] != L'\0', apiValid, pData->apiTested);
            }
            break;

        // --- 语言下拉框选择变更 ---
        case IDC_COMBO_LANGUAGE:
            if (code == CBN_SELCHANGE && pData && !g_bRefreshingLanguage)
            {
                pData->changed = true;
                HWND hCombo = GetDlgItem(hWnd, IDC_COMBO_LANGUAGE);
                int sel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0)
                    pData->language = static_cast<Language>(sel);

                // 立即应用语言并刷新对话框
                Strings_SetLanguage(pData->language);
                RefreshDialogLanguage(hWnd, pData);
                Logger_Debug(L"SettingsDlg: 语言切换至 index=%d", sel);
            }
            break;

        // --- "自动刷新" 复选框 ---
        case IDC_CHECK_AUTO_REFRESH:
            if (code == BN_CLICKED && pData)
            {
                pData->autoRefresh = (SendMessageW(
                    GetDlgItem(hWnd, IDC_CHECK_AUTO_REFRESH), BM_GETCHECK, 0, 0) == BST_CHECKED);
            }
            break;

        // --- "测试API" 按钮 ---
        case IDC_BTN_TEST_API:
            if (pData)
            {
                wchar_t apiKey[DSDB_BUF_APIKEY];
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, apiKey, DSDB_BUF_APIKEY);

                Logger_Info(L"SettingsDlg: 用户点击测试API, keyLen=%zu", wcslen(apiKey));

                // 禁用按钮，显示"正在测试"状态
                EnableWindow(GetDlgItem(hWnd, IDC_BTN_TEST_API), FALSE);
                SetDlgItemTextW(hWnd, IDC_STATIC_STATUS, Strings_Get(StringKey::STATUS_TESTING));

                // 调用网络模块测试 API（阻塞调用）
                wchar_t bufTimeout[DSDB_BUF_NUMBER];
                GetDlgItemTextW(hWnd, IDC_EDIT_TIMEOUT, bufTimeout, DSDB_BUF_NUMBER);
                int timeoutSec = _wtoi(bufTimeout);
                if (timeoutSec < DSDB_TIMEOUT_MIN) timeoutSec = DSDB_TIMEOUT_MIN;
                if (timeoutSec > DSDB_TIMEOUT_MAX) timeoutSec = DSDB_TIMEOUT_MAX;

                ApiTestResult testResult = TestDeepSeekApi(apiKey, timeoutSec);

                if (testResult.success)
                {
                    // 测试通过：弹出详情提示框
                    wchar_t msg[DSDB_BUF_MESSAGE];
                    swprintf_s(msg,
                        Strings_Get(StringKey::MSG_TEST_SUCCESS_BODY),
                        testResult.is_available
                            ? Strings_Get(StringKey::BOOL_YES)
                            : Strings_Get(StringKey::BOOL_NO),
                        testResult.total_balance.c_str(),
                        testResult.currency.c_str(),
                        testResult.granted_balance.c_str(),
                        testResult.currency.c_str(),
                        testResult.topped_up_balance.c_str(),
                        testResult.currency.c_str());
                    MessageBoxW(hWnd, msg,
                        Strings_Get(StringKey::MSG_TEST_SUCCESS_TITLE),
                        MB_OK | MB_ICONINFORMATION);

                    pData->apiTested = true;
                    SetDlgItemTextW(hWnd, IDC_STATIC_STATUS,
                        Strings_Get(StringKey::STATUS_TEST_PASSED));
                    Logger_Info(L"SettingsDlg: API 测试通过");
                }
                else
                {
                    // 测试失败：弹出错误提示框
                    std::wstring errMsg;
                    if (!testResult.error_message.empty())
                        errMsg = testResult.error_message;
                    else if (testResult.http_status_code != 0)
                        errMsg = L"HTTP " + std::to_wstring(testResult.http_status_code);
                    else
                        errMsg = L"Unknown error";

                    wchar_t msg[DSDB_BUF_MESSAGE];
                    swprintf_s(msg,
                        Strings_Get(StringKey::MSG_TEST_FAILED_BODY),
                        errMsg.c_str());
                    MessageBoxW(hWnd, msg,
                        Strings_Get(StringKey::MSG_TEST_FAILED_TITLE),
                        MB_OK | MB_ICONWARNING);

                    pData->apiTested = false;
                    SetDlgItemTextW(hWnd, IDC_STATIC_STATUS,
                        Strings_Get(StringKey::STATUS_TEST_FAILED));
                    Logger_Warn(L"SettingsDlg: API 测试失败: %s", errMsg.c_str());
                }

                // 恢复按钮可用状态，更新控件联动
                EnableWindow(GetDlgItem(hWnd, IDC_BTN_TEST_API), TRUE);
                bool valid = IsValidApiKey(apiKey);
                UpdateApiDependentControls(hWnd, apiKey[0] != L'\0', valid, pData->apiTested);
            }
            break;

        // --- "清除历史" 按钮 ---
        case IDC_BTN_CLEAR_HISTORY:
            if (pData)
            {
                int result = MessageBoxW(hWnd,
                    Strings_Get(StringKey::MSG_CONFIRM_CLEAR_BODY),
                    Strings_Get(StringKey::MSG_CONFIRM_CLEAR_TITLE),
                    MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
                if (result == IDYES)
                {
                    // 直接从 HistoryManager 获取当前记录数用于日志
                    Logger_Info(L"SettingsDlg: 用户确认清除历史");

                    // 清空 ListView 显示
                    HWND hList = GetDlgItem(hWnd, IDC_LIST_HISTORY);
                    if (hList)
                        ListView_DeleteAllItems(hList);

                    // 标记已清除（由调用方处理 HistoryManager::Clear）
                    pData->historyCleared = true;
                    pData->changed = true;
                }
            }
            break;

        // --- "确定" 按钮 ---
        case IDC_BTN_OK:
            if (pData)
            {
                // 从编辑框读取当前输入
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, pData->apiKey, DSDB_BUF_APIKEY);

                // 保存前二次校验（空密钥允许保存）
                if (pData->apiKey[0] != L'\0' && !IsValidApiKey(pData->apiKey))
                {
                    MessageBoxW(hWnd,
                        Strings_Get(StringKey::MSG_INVALID_KEY_BODY),
                        Strings_Get(StringKey::MSG_INVALID_KEY_TITLE),
                        MB_OK | MB_ICONWARNING);
                    return 0;
                }

                wchar_t bufInterval[DSDB_BUF_NUMBER];
                GetDlgItemTextW(hWnd, IDC_EDIT_INTERVAL, bufInterval, DSDB_BUF_NUMBER);
                pData->updateInterval = _wtoi(bufInterval);
                if (pData->updateInterval < DSDB_INTERVAL_MIN)
                    pData->updateInterval = DSDB_INTERVAL_MIN;

                wchar_t bufTimeout[DSDB_BUF_NUMBER];
                GetDlgItemTextW(hWnd, IDC_EDIT_TIMEOUT, bufTimeout, DSDB_BUF_NUMBER);
                pData->requestTimeout = _wtoi(bufTimeout);
                if (pData->requestTimeout < DSDB_TIMEOUT_MIN)
                    pData->requestTimeout = DSDB_TIMEOUT_MIN;
                if (pData->requestTimeout > DSDB_TIMEOUT_MAX)
                    pData->requestTimeout = DSDB_TIMEOUT_MAX;

                // 更新间隔必须大于请求超时
                if (pData->updateInterval <= pData->requestTimeout)
                {
                    MessageBoxW(hWnd,
                        Strings_Get(StringKey::MSG_INTERVAL_ERROR_BODY),
                        Strings_Get(StringKey::MSG_INTERVAL_ERROR_TITLE),
                        MB_OK | MB_ICONWARNING);
                    return 0;
                }

                wchar_t bufHistoryCount[DSDB_BUF_NUMBER];
                GetDlgItemTextW(hWnd, IDC_EDIT_HISTORY_COUNT, bufHistoryCount, DSDB_BUF_NUMBER);
                pData->maxHistoryCount = _wtoi(bufHistoryCount);
                if (pData->maxHistoryCount < DSDB_HISTORY_COUNT_MIN)
                    pData->maxHistoryCount = DSDB_HISTORY_COUNT_MIN;
                if (pData->maxHistoryCount > DSDB_HISTORY_COUNT_MAX)
                    pData->maxHistoryCount = DSDB_HISTORY_COUNT_MAX;

                Logger_Info(L"SettingsDlg: 用户确认, apiKey[0]='%c' interval=%d timeout=%d history=%d lang=%d",
                    (pData->apiKey[0] ? pData->apiKey[0] : L'-'),
                    pData->updateInterval, pData->requestTimeout,
                    pData->maxHistoryCount, static_cast<int>(pData->language));
            }
            DestroyWindow(hWnd);
            break;

        // --- "取消" 按钮 ---
        case IDC_BTN_CANCEL:
            if (pData)
            {
                // 还原原始值
                wcscpy_s(pData->apiKey, pData->apiKeyOrig);
                pData->updateInterval = pData->updateIntervalOrig;
                pData->requestTimeout = pData->requestTimeoutOrig;
                pData->maxHistoryCount = pData->maxHistoryCountOrig;
                pData->historyCleared = false;
                pData->changed = false;
                pData->language = pData->languageOrig;
                Strings_SetLanguage(pData->languageOrig);  // 恢复原语言

                // 若之前点了清除历史，刷新 ListView 恢复显示
                HWND hList = GetDlgItem(hWnd, IDC_LIST_HISTORY);
                if (hList)
                    RefreshHistoryList(hList, pData->historyRecords);

                Logger_Debug(L"SettingsDlg: 用户取消");
            }
            DestroyWindow(hWnd);
            break;
        }
        break;
    }

    // ============================================================
    // WM_KEYDOWN：键盘快捷键
    // ============================================================
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_RETURN:
            // 按回车触发确定按钮（仅当确定可用时）
            if (pData && IsWindowEnabled(GetDlgItem(hWnd, IDC_BTN_OK)))
            {
                SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_OK, BN_CLICKED),
                    (LPARAM)GetDlgItem(hWnd, IDC_BTN_OK));
            }
            break;

        case VK_ESCAPE:
            // 按 Esc 触发取消按钮
            SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_CANCEL, BN_CLICKED),
                (LPARAM)GetDlgItem(hWnd, IDC_BTN_CANCEL));
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
        break;
    }

    // ============================================================
    // WM_GETMINMAXINFO：限制窗口最小尺寸
    // ============================================================
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pInfo = (MINMAXINFO*)lParam;
        pInfo->ptMinTrackSize.x = DlgScale(640);
        pInfo->ptMinTrackSize.y = DlgScale(460);
        return 0;
    }

    // ============================================================
    // WM_SIZE：响应式布局
    // ============================================================
    case WM_SIZE:
    {
        int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);

        if (cx > 0 && cy > 0)
        {
            // 按钮距底部固定
            int btnY = cy - DlgScale(43);

            // 确定按钮
            HWND hBtn = GetDlgItem(hWnd, IDC_BTN_OK);
            if (hBtn)
                SetWindowPos(hBtn, nullptr, cx - DlgScale(220), btnY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            // 取消按钮
            hBtn = GetDlgItem(hWnd, IDC_BTN_CANCEL);
            if (hBtn)
                SetWindowPos(hBtn, nullptr, cx - DlgScale(115), btnY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            // 清除历史按钮（左对齐）
            hBtn = GetDlgItem(hWnd, IDC_BTN_CLEAR_HISTORY);
            if (hBtn)
                SetWindowPos(hBtn, nullptr, DlgScale(20), btnY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            // 语言下拉框（右侧与取消按钮右侧对齐）
            HWND hCombo = GetDlgItem(hWnd, IDC_COMBO_LANGUAGE);
            if (hCombo)
                SetWindowPos(hCombo, nullptr, cx - DlgScale(140), DlgScale(8), 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER);

            // 语言标签（在下拉框左侧）
            HWND hLangLabel = GetDlgItem(hWnd, IDC_STATIC_LANGUAGE_LABEL);
            if (hLangLabel)
                SetWindowPos(hLangLabel, nullptr, cx - DlgScale(213), DlgScale(10), 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER);

            // ListView：拉伸填充
            HWND hList = GetDlgItem(hWnd, IDC_LIST_HISTORY);
            if (hList)
            {
                int listWidth = cx - DlgScale(40);
                int listHeight = btnY - DlgScale(278) - DlgScale(8);
                if (listWidth < DlgScale(200)) listWidth = DlgScale(200);
                if (listHeight < DlgScale(80)) listHeight = DlgScale(80);
                SetWindowPos(hList, nullptr, 0, 0, listWidth, listHeight, SWP_NOMOVE | SWP_NOZORDER);
            }
        }
        return 0;
    }

    // ============================================================
    // WM_NOTIFY：UpDown 控件增量 + ListView 点击
    // ============================================================
    case WM_NOTIFY:
    {
        NMHDR* pNm = (NMHDR*)lParam;

        // UpDown 控件增量（手动处理，避免千位分隔符问题）
        if (pNm->code == UDN_DELTAPOS)
        {
            NMUPDOWN* pUd = (NMUPDOWN*)lParam;
            int editId = 0;
            int minVal = 0, maxVal = 0;

            switch (pNm->idFrom)
            {
            case IDC_SPIN_INTERVAL:
                editId = IDC_EDIT_INTERVAL; minVal = DSDB_INTERVAL_MIN; maxVal = DSDB_INTERVAL_MAX; break;
            case IDC_SPIN_TIMEOUT:
                editId = IDC_EDIT_TIMEOUT;  minVal = DSDB_TIMEOUT_MIN;  maxVal = DSDB_TIMEOUT_MAX;  break;
            case IDC_SPIN_HISTORY_COUNT:
                editId = IDC_EDIT_HISTORY_COUNT; minVal = DSDB_HISTORY_COUNT_MIN; maxVal = DSDB_HISTORY_COUNT_MAX; break;
            }

            if (editId != 0)
            {
                wchar_t buf[DSDB_BUF_NUMBER];
                GetDlgItemTextW(hWnd, editId, buf, DSDB_BUF_NUMBER);
                int val = _wtoi(buf) + pUd->iDelta;
                if (val < minVal) val = minVal;
                if (val > maxVal) val = maxVal;
                swprintf_s(buf, L"%d", val);
                SetDlgItemTextW(hWnd, editId, buf);
                if (pData)
                    pData->changed = true;
            }
            return 0;
        }

        // 用户点击表格 → 取消自动刷新
        if (pNm->idFrom == IDC_LIST_HISTORY && pNm->code == NM_CLICK && pData && pData->autoRefresh)
        {
            SendMessageW(GetDlgItem(hWnd, IDC_CHECK_AUTO_REFRESH), BM_SETCHECK, BST_UNCHECKED, 0);
            pData->autoRefresh = false;
        }
        return 0;
    }

    // ============================================================
    // WM_CLOSE：标题栏关闭 → 取消操作
    // ============================================================
    case WM_CLOSE:
        if (pData)
        {
            wcscpy_s(pData->apiKey, pData->apiKeyOrig);
            pData->updateInterval = pData->updateIntervalOrig;
            pData->requestTimeout = pData->requestTimeoutOrig;
            pData->maxHistoryCount = pData->maxHistoryCountOrig;
            pData->historyCleared = false;
            pData->changed = false;
            pData->language = pData->languageOrig;
            Strings_SetLanguage(pData->languageOrig);
            Logger_Debug(L"SettingsDlg: WM_CLOSE（标题栏关闭）");
        }
        DestroyWindow(hWnd);
        break;

    // ============================================================
    // WM_DESTROY：清理资源
    // ============================================================
    case WM_DESTROY:
    {
        Logger_Debug(L"SettingsDlg: WM_DESTROY");

        // 停止历史记录刷新定时器
        KillTimer(hWnd, 1);

        // 释放字体资源
        HFONT hFont = (HFONT)RemovePropW(hWnd, L"hFont");
        if (hFont)
            DeleteObject(hFont);

        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ============================================================
// 公开接口：ShowSettingsDialog
// ============================================================

bool ShowSettingsDialog(
    HWND hParent,
    wchar_t* apiKey,
    int& updateInterval,
    int& requestTimeout,
    int& maxHistoryCount,
    Language& language,
    const std::vector<HistoryRecord>& historyRecords,
    HistoryManager* historyManager,
    const std::wstring& configDir)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // 获取系统 DPI 用于缩放
    HDC hdc = GetDC(nullptr);
    g_dlgDpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(nullptr, hdc);

    Logger_Info(L"ShowSettingsDialog: 开始, DPI=%d", g_dlgDpi);

    // 初始化公共控件（ListView 需要 ICC_LISTVIEW_CLASSES）
    static bool s_bCcInit = false;
    if (!s_bCcInit)
    {
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS };
        InitCommonControlsEx(&icex);
        s_bCcInit = true;
    }

    // 注册窗口类（仅首次注册成功，后续同名注册静默忽略）
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = SettingsDlgProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = SETTINGS_DIALOG_CLASS;
    RegisterClassExW(&wc);

    // ---- 准备对话框数据 ----
    SettingsDlgData dlgData;
    wcscpy_s(dlgData.apiKey, apiKey);
    wcscpy_s(dlgData.apiKeyOrig, apiKey);
    dlgData.updateInterval = updateInterval;
    dlgData.updateIntervalOrig = updateInterval;
    dlgData.requestTimeout = requestTimeout;
    dlgData.requestTimeoutOrig = requestTimeout;
    dlgData.maxHistoryCount = maxHistoryCount;
    dlgData.maxHistoryCountOrig = maxHistoryCount;
    dlgData.historyRecords = &historyRecords;
    dlgData.historyCleared = false;
    dlgData.changed = false;
    dlgData.apiTested = false;
    dlgData.language = language;
    dlgData.languageOrig = language;

    // 准备图标路径并检查是否需要下载
    IconDownloadParam* pIconParam = nullptr;
    {
        std::wstring iconFile = configDir + L"\\DeepSeekLogo.ico";
        wcsncpy_s(dlgData.iconPath, iconFile.c_str(), 511);
        dlgData.iconPath[511] = L'\0';

        if (GetFileAttributesW(iconFile.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            Logger_Info(L"ShowSettingsDialog: 图标文件不存在，稍后启动异步下载");
            pIconParam = new (std::nothrow) IconDownloadParam();
            if (pIconParam)
                wcsncpy_s(pIconParam->path, dlgData.iconPath, 511);
            else
                Logger_Error(L"ShowSettingsDialog: 无法分配 IconDownloadParam");
        }
    }

    // 计算居中位置
    int dlgWidth = DlgScale(640);
    int dlgHeight = DlgScale(540);
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    HWND hParentWnd = hParent;
    if (hParentWnd)
    {
        RECT parentRect;
        if (GetWindowRect(hParentWnd, &parentRect))
        {
            x = parentRect.left + ((parentRect.right - parentRect.left) - dlgWidth) / 2;
            y = parentRect.top + ((parentRect.bottom - parentRect.top) - dlgHeight) / 2;
        }
    }

    // 创建对话框窗口（可调大小、可最大化）
    HWND hDlg = CreateWindowExW(
        WS_EX_CLIENTEDGE, SETTINGS_DIALOG_CLASS, Strings_Get(StringKey::DLG_TITLE),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX,
        x, y, dlgWidth, dlgHeight,
        hParentWnd, nullptr, hInst, &dlgData);

    if (!hDlg)
    {
        Logger_Error(L"ShowSettingsDialog: CreateWindowExW 失败, err=%lu", GetLastError());
        delete pIconParam;
        return false;
    }

    // 若图标文件不存在，启动异步下载
    if (pIconParam)
    {
        pIconParam->hWnd = hDlg;
        HANDLE hThread = CreateThread(nullptr, 0, DownloadIconThreadProc,
            pIconParam, 0, nullptr);
        if (hThread)
            CloseHandle(hThread);
        else
        {
            delete pIconParam;
            Logger_Error(L"ShowSettingsDialog: 创建下载线程失败");
        }
    }

    // 禁用父窗口以实现模态效果
    if (hParentWnd)
        EnableWindow(hParentWnd, FALSE);

    ShowWindow(hDlg, SW_SHOW);

    // 模态消息循环
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 恢复父窗口
    if (hParentWnd)
        EnableWindow(hParentWnd, TRUE);

    // ---- 对话框关闭后，根据变更标志决定是否保存 ----
    if (dlgData.changed)
    {
        Logger_Info(L"ShowSettingsDialog: 设置已变更，准备保存");

        wcscpy_s(apiKey, DSDB_BUF_APIKEY, dlgData.apiKey);
        updateInterval = dlgData.updateInterval;
        requestTimeout = dlgData.requestTimeout;
        maxHistoryCount = dlgData.maxHistoryCount;
        language = dlgData.language;

        // 用户手动清除了历史记录
        if (dlgData.historyCleared && historyManager)
        {
            Logger_Info(L"ShowSettingsDialog: 用户手动清除历史");
            historyManager->Clear();
        }

        Logger_Info(L"ShowSettingsDialog: 返回 true（已变更）");
        return true;
    }

    Logger_Debug(L"ShowSettingsDialog: 设置未变更");
    return false;
}
