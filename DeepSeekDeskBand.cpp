/************************************************************************//**
 * @file    DeepSeekDeskBand.cpp
 * @brief   DeepSeekDeskBand 插件实现
 * @details 实现 CDeepSeekDeskBandItem（显示项目）和 CDeepSeekDeskBand（插件主类）
 *          的所有接口函数，以及插件导出函数 TMPluginGetInstance()。
 ****************************************************************************/
#include "DeepSeekDeskBand.h"
#include "ConfigEncrypt.h"
#include "HttpClient.h"
#include "framework.h"
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

// ============================================================
// CDeepSeekDeskBandItem 实现 —— 显示项目
// ============================================================

/**
 * @brief 构造函数，初始化默认显示文本
 */
CDeepSeekDeskBandItem::CDeepSeekDeskBandItem()
{
    // 初始化数值文本为默认值 "就绪"
    wcscpy_s(m_valueText, L"就绪");
}

/**
 * @brief 设置当前显示的数值文本
 * @param text 要显示的文本内容（最大 63 个宽字符）
 */
void CDeepSeekDeskBandItem::SetValueText(const wchar_t* text)
{
    wcscpy_s(m_valueText, text);
}

/**
 * @brief 获取显示项目名称
 * @return 显示在右键菜单"显示项目"子菜单下的名称
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemName() const
{
    return L"DeepSeek 助手";
}

/**
 * @brief 获取显示项目唯一 ID
 * @return 仅包含字母和数字的唯一标识符
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemId() const
{
    return L"DeepSeekDeskBandItem";
}

/**
 * @brief 获取标签文本
 * @return 标签文本字符串，显示在数值文本的左侧
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemLableText() const
{
    return LABEL_TEXT;
}

/**
 * @brief 获取数值文本
 * @return 数值文本字符串（此函数被频繁调用，不要在内部获取数据）
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemValueText() const
{
    return m_valueText;
}

/**
 * @brief 获取示例文本
 * @return 示例文本字符串，用于计算显示区域宽度和皮肤预览
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemValueSampleText() const
{
    return SAMPLE_TEXT;
}

// ============================================================
// CDeepSeekDeskBand 实现 —— 插件主类
// ============================================================

/** @brief 静态全局单例实例的定义 */
CDeepSeekDeskBand CDeepSeekDeskBand::m_instance;

/** @brief 私有构造函数 */
CDeepSeekDeskBand::CDeepSeekDeskBand()
    : m_pApp(nullptr)
    , m_updateInterval(DSDB_DEFAULT_INTERVAL)
    , m_requestTimeout(DSDB_DEFAULT_TIMEOUT)
{
    m_apiKey[0] = L'\0';
}

/**
 * @brief 获取全局唯一实例
 * @return 插件的单例引用
 */
CDeepSeekDeskBand& CDeepSeekDeskBand::Instance()
{
    return m_instance;
}

/**
 * @brief 根据索引获取显示项目对象
 * @param index 显示项目索引，从 0 开始
 * @return 对应索引的 IPluginItem 指针，越界时返回 nullptr
 * @note   必须对 index 越界进行判断，返回 nullptr
 */
IPluginItem* CDeepSeekDeskBand::GetItem(int index)
{
    if (index == 0)
        return &m_item;     // 当前仅有一个显示项目
    return nullptr;         // 索引越界，返回空指针
}

/**
 * @brief 定时数据获取
 * @details 由主程序每隔一定时间调用。
 *          按设定的更新间隔调用 DeepSeek API 查询余额，结果缓存显示。
 */
void CDeepSeekDeskBand::DataRequired()
{
    // API 密钥为空 → 显示 "X"
    if (m_apiKey[0] == L'\0')
    {
        m_hasBalance = false;
        m_item.SetValueText(L"X");
        return;
    }

    ULONGLONG now = GetTickCount64();
    ULONGLONG intervalMs = static_cast<ULONGLONG>(m_updateInterval) * 1000;

    // 到达更新周期时发起 API 请求
    if (m_lastFetchTime == 0 || (now - m_lastFetchTime) >= intervalMs)
    {
        ApiTestResult result = TestDeepSeekApi(m_apiKey, m_requestTimeout);

        if (result.success)
        {
            m_lastBalanceResult = result;
            m_hasBalance = true;
            m_lastFetchTime = now;
        }
        else
        {
            m_hasBalance = false;
        }
    }

    // 更新显示文本
    if (m_hasBalance)
    {
        wchar_t valueText[DSDB_BUF_VALUE];
        swprintf_s(valueText, L"%s %s",
            m_lastBalanceResult.total_balance.c_str(),
            m_lastBalanceResult.currency.c_str());
        m_item.SetValueText(valueText);
    }
    else
    {
        m_item.SetValueText(L"X");
    }
}

/**
 * @brief 获取插件信息
 * @param index 信息索引（名称/描述/作者/版权/版本/URL）
 * @return 对应的信息字符串，未知索引返回空字符串
 */
const wchar_t* CDeepSeekDeskBand::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:          // 插件名称
        return DSDB_NAME;
    case TMI_DESCRIPTION:   // 插件功能描述
        return DSDB_DESCRIPTION;
    case TMI_AUTHOR:        // 作者
        return DSDB_AUTHOR;
    case TMI_COPYRIGHT:     // 版权信息
        return L"";
    case TMI_VERSION:       // 版本号
        return DSDB_VERSION;
    case TMI_URL:           // 项目主页
        return L"";
    default:
        break;
    }
    return L"";
}

// ============================================================
// 配置管理 —— 从 INI 文件加载 / 保存
// ============================================================

/**
 * @brief 获取配置文件完整路径
 * @return 配置文件路径字符串，始终使用 DLL 所在目录
 */
std::wstring CDeepSeekDeskBand::GetConfigFilePath()
{
    // 获取当前 DLL 的完整路径，以此确定配置目录
    wchar_t dllPath[MAX_PATH];
    HMODULE hMod = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&Instance),
        &hMod);
    DWORD len = GetModuleFileNameW(hMod, dllPath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return L"";

    // 去掉文件名，得到 DLL 所在目录
    std::wstring dir(dllPath, len);
    size_t lastSep = dir.find_last_of(L"\\/");
    if (lastSep == std::wstring::npos)
        return L"";
    dir = dir.substr(0, lastSep);

    return dir + L"\\" DSDB_CONFIG_FILENAME;
}

/**
 * @brief 从加密的二进制配置文件加载设置
 * @note  解密失败时自动恢复默认设置。
 */
void CDeepSeekDeskBand::LoadConfig()
{
    std::wstring configPath = GetConfigFilePath();
    if (configPath.empty())
        return;

    // 诊断：将使用的配置路径写入 dsdb_path.txt（与配置文件同目录）
    {
        std::wstring diagPath = configPath;
        size_t lastSep = diagPath.find_last_of(L"\\/");
        if (lastSep != std::wstring::npos)
            diagPath = diagPath.substr(0, lastSep + 1);
        diagPath += L"dsdb_path.txt";
        std::ofstream diag(diagPath, std::ios::trunc);
        if (diag.is_open())
        {
            diag << "LoadConfig path: ";
            // 宽字符转 UTF-8 写入
            int len = WideCharToMultiByte(CP_UTF8, 0, configPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (len > 0)
            {
                std::vector<char> buf(len);
                WideCharToMultiByte(CP_UTF8, 0, configPath.c_str(), -1, buf.data(), len, nullptr, nullptr);
                diag.write(buf.data(), len - 1);
            }
            diag << "\n";
        }
    }

    // 读取加密文件为原始字节
    std::ifstream file(configPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return;     // 文件不存在，使用默认值

    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0)
        return;

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> encrypted(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(encrypted.data()), fileSize))
        return;

    // 解密并反序列化
    ConfigBlob blob;
    if (!ConfigDecrypt(encrypted, blob))
        return;     // 解密失败，使用默认值

    // 将解密结果应用到成员变量
    wcsncpy_s(m_apiKey, blob.apiKey.c_str(), DSDB_BUF_APIKEY - 1);
    m_apiKey[DSDB_BUF_APIKEY - 1] = L'\0';

    m_updateInterval = blob.updateInterval;
    if (m_updateInterval < DSDB_INTERVAL_MIN)
        m_updateInterval = DSDB_INTERVAL_MIN;

    m_requestTimeout = blob.requestTimeout;
    if (m_requestTimeout < DSDB_TIMEOUT_MIN)
        m_requestTimeout = DSDB_TIMEOUT_MIN;
    if (m_requestTimeout > DSDB_TIMEOUT_MAX)
        m_requestTimeout = DSDB_TIMEOUT_MAX;
}

/**
 * @brief 保存设置到加密的二进制配置文件
 */
void CDeepSeekDeskBand::SaveConfig()
{
    std::wstring configPath = GetConfigFilePath();
    if (configPath.empty())
        return;

    // 组装明文配置
    ConfigBlob blob;
    blob.updateInterval = m_updateInterval;
    blob.requestTimeout = m_requestTimeout;
    blob.apiKey = m_apiKey;

    // 加密
    std::vector<uint8_t> encrypted;
    if (!ConfigEncrypt(blob, encrypted))
        return;

    // 确保目标目录存在
    std::wstring dirPath = configPath;
    size_t lastSep = dirPath.find_last_of(L"\\/");
    if (lastSep != std::wstring::npos)
    {
        dirPath = dirPath.substr(0, lastSep);
        CreateDirectoryW(dirPath.c_str(), nullptr);
    }

    // 写入二进制文件
    std::ofstream file(configPath, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
        return;

    file.write(reinterpret_cast<const char*>(encrypted.data()),
        static_cast<std::streamsize>(encrypted.size()));
    file.close();   // 显式关闭确保写入磁盘
}

/**
 * @brief 插件初始化回调
 * @param pApp 主程序 ITrafficMonitor 接口指针
 * @details 保存主程序指针，加载配置文件
 */
void CDeepSeekDeskBand::OnInitialize(ITrafficMonitor* pApp)
{
    m_pApp = pApp;
    LoadConfig();
}

// ============================================================
// 设置对话框
// ============================================================

/** @brief 设置对话框窗口类名 */
static const wchar_t* SETTINGS_DIALOG_CLASS = L"DeepSeekDeskBandSettingsDlg";

/** @brief 对话框控件 ID */
enum
{
    IDC_EDIT_API_KEY    = 1001,   // DeepSeek API 输入框
    IDC_STATIC_API_HINT = 1002,   // API 格式提示
    IDC_BTN_TEST_API    = 1003,   // 测试API 按钮
    IDC_STATIC_STATUS   = 1004,   // 测试结果状态文本
    IDC_EDIT_INTERVAL   = 1005,   // 更新间隔输入框
    IDC_SPIN_INTERVAL   = 1006,   // 更新间隔 UpDown 控件
    IDC_BTN_OK          = 1007,   // 确定按钮
    IDC_BTN_CANCEL      = 1008,   // 取消按钮
    IDC_EDIT_TIMEOUT    = 1009,   // 请求超时输入框
    IDC_SPIN_TIMEOUT    = 1010,   // 请求超时 UpDown 控件
};

/**
 * @brief 设置对话框数据
 * @details 在创建对话框前分配，通过 CreateWindowExW 的 lpParam 传递给
 *          WM_CREATE 消息，窗口过程通过 GWLP_USERDATA 存取。
 */
struct SettingsDlgData
{
    wchar_t apiKey[DSDB_BUF_APIKEY];              // 当前输入的 API 密钥
    wchar_t apiKeyOrig[DSDB_BUF_APIKEY];          // 原始 API 密钥（用于取消时还原）
    int     updateInterval;           // 当前输入的更新间隔
    int     updateIntervalOrig;       // 原始更新间隔
    int     requestTimeout;           // 当前输入的请求超时时间
    int     requestTimeoutOrig;       // 原始请求超时时间
    bool    changed;                  // 是否有未保存的变更
    bool    apiTested;                // API 是否已通过测试
};

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
 * @brief 更新控件启用状态（由 API 密钥有效性和测试状态决定）
 * @param hDlg       对话框句柄
 * @param hasContent 输入框是否非空（空时不显示格式错误提示）
 * @param valid      API 密钥格式是否有效
 * @param tested     API 是否已通过测试
 */
static void UpdateApiDependentControls(HWND hDlg, bool hasContent, bool valid, bool tested)
{
    // 仅当有输入且格式无效时显示提示；空输入或有效输入不显示
    ShowWindow(GetDlgItem(hDlg, IDC_STATIC_API_HINT), (hasContent && !valid) ? SW_SHOW : SW_HIDE);
    // 测试按钮：仅当密钥非空且格式有效时可用
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_TEST_API), hasContent && valid);
    // 确定按钮：密钥为空或已通过测试的有效密钥时可保存
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_OK), !hasContent || (valid && tested));
}

/**
 * @brief 设置对话框窗口过程
 */
static LRESULT CALLBACK SettingsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SettingsDlgData* pData = (SettingsDlgData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CREATE:
    {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

        // 从 CREATESTRUCT 中取出对话框数据
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pData = (SettingsDlgData*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

        // ---- 创建系统消息字体（Segoe UI 9pt，现代外观） ----
        NONCLIENTMETRICSW ncm = { sizeof(NONCLIENTMETRICSW) };
        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        ncm.lfMessageFont.lfHeight = -12;      // 9pt 高度
        ncm.lfMessageFont.lfWeight = FW_NORMAL;
        HFONT hFont = CreateFontIndirectW(&ncm.lfMessageFont);
        SetPropW(hWnd, L"hFont", hFont);       // 存储以便在 WM_DESTROY 释放

        HWND hChild;

        // --- "DeepSeek API:" 标签 ---
        hChild = CreateWindowW(L"STATIC", L"DeepSeek API:",
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            20, 22, 110, 20,
            hWnd, nullptr, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- API 密钥输入框（密码掩码，宽度足够容纳 35 字符） ---
        hChild = CreateWindowW(L"EDIT", pData->apiKey,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL | WS_TABSTOP,
            135, 19, 375, 22,
            hWnd, (HMENU)IDC_EDIT_API_KEY, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- API 格式校验提示（默认隐藏） ---
        hChild = CreateWindowW(L"STATIC", L"密钥格式错误：必须以 sk- 开头，后跟32位小写字母和数字",
            WS_CHILD | SS_LEFT,
            135, 44, 375, 16,
            hWnd, (HMENU)IDC_STATIC_API_HINT, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "测试API(&T)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", L"测试API(&T)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            135, 74, 90, 24,
            hWnd, (HMENU)IDC_BTN_TEST_API, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 测试结果状态文本 ---
        hChild = CreateWindowW(L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            235, 77, 280, 20,
            hWnd, (HMENU)IDC_STATIC_STATUS, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "更新间隔（秒）:" 标签 ---
        hChild = CreateWindowW(L"STATIC", L"更新间隔（秒）:",
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            20, 124, 110, 20,
            hWnd, nullptr, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 更新间隔输入框（Spin 控件的伙伴编辑框） ---
        wchar_t bufInterval[DSDB_BUF_NUMBER];
        swprintf_s(bufInterval, L"%d", pData->updateInterval);
        hChild = CreateWindowW(L"EDIT", bufInterval,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_TABSTOP,
            135, 121, 100, 22,
            hWnd, (HMENU)IDC_EDIT_INTERVAL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- UpDown Spin 控件（自动与伙伴编辑框对齐，范围 1~31,536,000） ---
        hChild = CreateWindowW(UPDOWN_CLASSW, nullptr,
            WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_SETBUDDYINT,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_SPIN_INTERVAL, hInst, nullptr);
            SendMessageW(hChild, UDM_SETRANGE32, DSDB_INTERVAL_MIN, DSDB_INTERVAL_MAX);   // 范围
        SendMessageW(hChild, UDM_SETPOS32, 0, pData->updateInterval);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "请求超时（秒）:" 标签 ---
        hChild = CreateWindowW(L"STATIC", L"请求超时（秒）:",
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            20, 174, 110, 20,
            hWnd, nullptr, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 请求超时输入框（Spin 控件的伙伴编辑框） ---
        wchar_t bufTimeout[DSDB_BUF_NUMBER];
        swprintf_s(bufTimeout, L"%d", pData->requestTimeout);
        hChild = CreateWindowW(L"EDIT", bufTimeout,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_TABSTOP,
            135, 171, 100, 22,
            hWnd, (HMENU)IDC_EDIT_TIMEOUT, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 超时 UpDown Spin 控件（范围 3~60 秒） ---
        {
            HWND hSpin = CreateWindowW(UPDOWN_CLASSW, nullptr,
                WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hWnd, (HMENU)IDC_SPIN_TIMEOUT, hInst, nullptr);
            SendMessageW(hSpin, UDM_SETRANGE32, DSDB_TIMEOUT_MIN, DSDB_TIMEOUT_MAX);
            SendMessageW(hSpin, UDM_SETPOS32, 0, pData->requestTimeout);
            SendMessageW(hSpin, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // --- "确定(&O)" 按钮（默认不可用） ---
        hChild = CreateWindowW(L"BUTTON", L"确定(&O)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED | WS_TABSTOP,
            340, 375, 85, 28,
            hWnd, (HMENU)IDC_BTN_OK, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "取消(&C)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", L"取消(&C)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            435, 375, 85, 28,
            hWnd, (HMENU)IDC_BTN_CANCEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // ---- 初始状态：校验已加载的 API 密钥 ----
        // 若已保存有效密钥（必然通过过测试），视为已测试
        pData->apiTested = IsValidApiKey(pData->apiKey);
        {
            bool hasContent = (pData->apiKey[0] != L'\0');
            bool valid = IsValidApiKey(pData->apiKey);
            UpdateApiDependentControls(hWnd, hasContent, valid, pData->apiTested);
        }

        SetFocus(GetDlgItem(hWnd, IDC_EDIT_API_KEY));
        return 0;
    }

    case WM_CTLCOLORSTATIC:
    {
        // 仅对"密钥格式错误"提示设置红色文字，不干预背景色
        HWND hCtrl = (HWND)lParam;
        if (hCtrl == GetDlgItem(hWnd, IDC_STATIC_API_HINT))
        {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(0xE0, 0x10, 0x10));
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        break;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        switch (id)
        {
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

        case IDC_BTN_TEST_API:
            if (pData)
            {
                // 读取当前输入的 API 密钥
                wchar_t apiKey[DSDB_BUF_APIKEY];
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, apiKey, DSDB_BUF_APIKEY);

                // 禁用按钮，显示"正在测试"状态
                EnableWindow(GetDlgItem(hWnd, IDC_BTN_TEST_API), FALSE);
                SetDlgItemTextW(hWnd, IDC_STATIC_STATUS, L"正在测试 API...");

                // 调用网络模块测试 API（阻塞调用，使用当前设置的超时时间）
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
                        L"API 测试通过！\n\n"
                        L"账户可用：%s\n"
                        L"总余额：%s %s\n"
                        L"赠送余额：%s %s\n"
                        L"充值余额：%s %s",
                        testResult.is_available ? L"是" : L"否",
                        testResult.total_balance.c_str(),
                        testResult.currency.c_str(),
                        testResult.granted_balance.c_str(),
                        testResult.currency.c_str(),
                        testResult.topped_up_balance.c_str(),
                        testResult.currency.c_str());
                    MessageBoxW(hWnd, msg, L"API 测试成功", MB_OK | MB_ICONINFORMATION);

                    pData->apiTested = true;
                    SetDlgItemTextW(hWnd, IDC_STATIC_STATUS, L"API 测试通过");
                }
                else
                {
                    // 测试失败：弹出错误提示框
                    std::wstring errMsg;
                    if (!testResult.error_message.empty())
                        errMsg = testResult.error_message;
                    else if (testResult.http_status_code != 0)
                    {
                        wchar_t statusMsg[DSDB_BUF_STATUS];
                        swprintf_s(statusMsg, L"HTTP 状态码 %d", testResult.http_status_code);
                        errMsg = statusMsg;
                    }
                    else
                        errMsg = L"未知错误";

                    wchar_t msg[DSDB_BUF_MESSAGE];
                    swprintf_s(msg, L"API 测试失败！\n\n错误信息：%s", errMsg.c_str());
                    MessageBoxW(hWnd, msg, L"API 测试失败", MB_OK | MB_ICONWARNING);

                    pData->apiTested = false;
                    SetDlgItemTextW(hWnd, IDC_STATIC_STATUS, L"API 测试失败");
                }

                // 恢复按钮可用状态，更新控件联动
                EnableWindow(GetDlgItem(hWnd, IDC_BTN_TEST_API), TRUE);
                bool valid = IsValidApiKey(apiKey);
                UpdateApiDependentControls(hWnd, apiKey[0] != L'\0', valid, pData->apiTested);
            }
            break;

        case IDC_BTN_OK:
            if (pData)
            {
                // 从编辑框读取当前输入
                GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, pData->apiKey, DSDB_BUF_APIKEY);

                // 保存前二次校验（防御性编程，空密钥允许保存）
                if (pData->apiKey[0] != L'\0' && !IsValidApiKey(pData->apiKey))
                {
                    MessageBoxW(hWnd, L"API 密钥格式无效，请检查后重试。",
                        L"格式错误", MB_OK | MB_ICONWARNING);
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
            }
            DestroyWindow(hWnd);
            break;

        case IDC_BTN_CANCEL:
            if (pData)
            {
                // 还原原始值
                wcscpy_s(pData->apiKey, pData->apiKeyOrig);
                pData->updateInterval = pData->updateIntervalOrig;
                pData->requestTimeout = pData->requestTimeoutOrig;
                pData->changed = false;
            }
            DestroyWindow(hWnd);
            break;
        }
        break;
    }

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

    case WM_CLOSE:
        if (pData)
        {
            // 点击标题栏关闭 → 取消操作
            wcscpy_s(pData->apiKey, pData->apiKeyOrig);
            pData->updateInterval = pData->updateIntervalOrig;
            pData->requestTimeout = pData->requestTimeoutOrig;
            pData->changed = false;
        }
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
    {
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

/**
 * @brief 打开插件的设置对话框
 * @param hParent 父窗口句柄
 * @return 设置变更时返回 OR_OPTION_CHANGED，否则返回 OR_OPTION_UNCHANGED
 */
ITMPlugin::OptionReturn CDeepSeekDeskBand::ShowOptionsDialog(void* hParent)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);

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
    wcscpy_s(dlgData.apiKey, m_apiKey);
    wcscpy_s(dlgData.apiKeyOrig, m_apiKey);
    dlgData.updateInterval = m_updateInterval;
    dlgData.updateIntervalOrig = m_updateInterval;
    dlgData.requestTimeout = m_requestTimeout;
    dlgData.requestTimeoutOrig = m_requestTimeout;
    dlgData.changed = false;

    // 计算居中位置
    int dlgWidth = 560;
    int dlgHeight = 480;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    HWND hParentWnd = (HWND)hParent;
    if (hParentWnd)
    {
        RECT parentRect;
        GetWindowRect(hParentWnd, &parentRect);
        x = parentRect.left + ((parentRect.right - parentRect.left) - dlgWidth) / 2;
        y = parentRect.top + ((parentRect.bottom - parentRect.top) - dlgHeight) / 2;
    }

    // 创建对话框窗口（WS_EX_CLIENTEDGE 提供现代凹陷边框）
    HWND hDlg = CreateWindowExW(
        WS_EX_CLIENTEDGE, SETTINGS_DIALOG_CLASS, L"DeepSeek 设置",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, dlgWidth, dlgHeight,
        hParentWnd, nullptr, hInst, &dlgData);

    if (!hDlg)
        return OR_OPTION_NOT_PROVIDED;

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
        wcscpy_s(m_apiKey, dlgData.apiKey);
        m_updateInterval = dlgData.updateInterval;
        m_requestTimeout = dlgData.requestTimeout;
        m_hasBalance = false;       // 设置变更后立即重新获取余额
        m_lastFetchTime = 0;
        SaveConfig();
        return OR_OPTION_CHANGED;
    }
    return OR_OPTION_UNCHANGED;
}

// ============================================================
// 导出函数 —— 插件入口
// ============================================================

/**
 * @brief 插件导出入口函数
 * @return ITMPlugin 接口指针，指向插件的全局唯一实例
 * @note   主程序通过此函数获取插件接口。
 *         返回的对象应在程序运行期间始终有效，不会被主程序释放。
 */
ITMPlugin* TMPluginGetInstance()
{
    return &CDeepSeekDeskBand::Instance();
}
