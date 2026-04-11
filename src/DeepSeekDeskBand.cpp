/************************************************************************//**
 * @file    DeepSeekDeskBand.cpp
 * @brief   DeepSeekDeskBand 插件核心实现
 * @details 实现 CDeepSeekDeskBandItem（显示项目）和 CDeepSeekDeskBand（插件主类）
 *          的所有接口函数，以及插件导出函数 TMPluginGetInstance()。
 *
 *          功能委托：
 *            - 历史记录管理  → HistoryManager
 *            - 设置对话框 UI → SettingsDialog
 *            - 配置加解密   → ConfigEncrypt
 *            - 日志输出     → Logger
 ****************************************************************************/
#include "DeepSeekDeskBand.h"
#include "ConfigEncrypt.h"
#include "Logger.h"
#include "SettingsDialog.h"
#include "framework.h"
#include <cstdint>
#include <fstream>
#include <vector>
#include <string>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "crypt32.lib")

// ============================================================
// CDeepSeekDeskBandItem 实现 —— 显示项目
// ============================================================

/**
 * @brief 构造函数，初始化默认显示文本为 "--"
 */
CDeepSeekDeskBandItem::CDeepSeekDeskBandItem()
{
    wcscpy_s(m_valueText, L"--");
}

/**
 * @brief 设置当前显示的数值文本
 * @param text 要显示的文本内容（最大 DSDB_BUF_VALUE - 1 个宽字符）
 */
void CDeepSeekDeskBandItem::SetValueText(const wchar_t* text)
{
    wcscpy_s(m_valueText, text);
}

/**
 * @brief 获取显示项目名称
 * @return 显示在右键菜单"显示项目"子菜单下的本地化名称
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemName() const
{
    return Strings_Get(StringKey::ITEM_NAME);
}

/**
 * @brief 获取显示项目唯一 ID
 * @return 仅包含字母和数字的唯一标识符，用于 TrafficMonitor 区分不同显示项目
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemId() const
{
    return L"DeepSeekDeskBandItem";
}

/**
 * @brief 获取标签文本
 * @return 始终返回 "DeepSeek"，显示在数值文本的左侧
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemLableText() const
{
    return LABEL_TEXT;
}

/**
 * @brief 获取数值文本
 * @return 当前缓存的数值文本（此函数被 TrafficMonitor 频繁调用，不要在此获取数据）
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemValueText() const
{
    return m_valueText;
}

/**
 * @brief 获取示例文本
 * @return 示例文本 "9999.99 CNY"，用于计算显示区域宽度和皮肤预览
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
    InitializeCriticalSection(&m_csRequest);
    Logger_Debug(L"CDeepSeekDeskBand: 构造完成");
}

/** @brief 私有析构函数 —— 释放临界区资源 */
CDeepSeekDeskBand::~CDeepSeekDeskBand()
{
    Logger_Debug(L"CDeepSeekDeskBand: 析构开始");

    // 关闭待处理的请求线程
    if (m_hRequestThread)
    {
        CloseHandle(m_hRequestThread);
        m_hRequestThread = nullptr;
    }

    DeleteCriticalSection(&m_csRequest);
    Logger_Debug(L"CDeepSeekDeskBand: 析构完成");
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
 * @brief 校验语言枚举值是否在有效范围内
 * @param lang 待校验的语言枚举值
 * @return 有效范围（Auto ~ Turkish）内的值，越界则返回 Language::Auto
 */
Language CDeepSeekDeskBand::ValidateLanguage(Language lang)
{
    if (lang >= Language::Auto && lang <= Language::Turkish)
        return lang;
    Logger_Warn(L"CDeepSeekDeskBand::ValidateLanguage: 无效语言值 %d，回退到 Auto",
        static_cast<int>(lang));
    return Language::Auto;
}

/**
 * @brief 根据索引获取显示项目对象
 * @param index 显示项目索引，从 0 开始
 * @return 对应索引的 IPluginItem 指针，越界时返回 nullptr
 */
IPluginItem* CDeepSeekDeskBand::GetItem(int index)
{
    if (index == 0)
        return &m_item;     // 当前仅有一个显示项目
    return nullptr;         // 索引越界，返回空指针
}

/**
 * @brief 异步 API 请求线程过程
 * @details 在独立线程中执行阻塞的网络调用 TestDeepSeekApi。
 *          完成后通过临界区传回结果，由主线程在 DataRequired() 中处理。
 *          关键：在获取锁之前复制 API 密钥和超时参数，避免线程间竞争。
 */
DWORD WINAPI CDeepSeekDeskBand::ApiRequestThreadProc(LPVOID /*lpParam*/)
{
    CDeepSeekDeskBand& plugin = CDeepSeekDeskBand::Instance();

    // 复制请求参数（在临界区内安全读取）
    wchar_t apiKey[DSDB_BUF_APIKEY];
    int timeout;
    EnterCriticalSection(&plugin.m_csRequest);
    wcscpy_s(apiKey, plugin.m_apiKey);
    timeout = plugin.m_requestTimeout;
    LeaveCriticalSection(&plugin.m_csRequest);

    Logger_Info(L"APiRequestThread: 开始请求, timeout=%ds", timeout);

    // 执行阻塞的网络调用（不持有锁，避免长时间阻塞其他线程）
    ApiTestResult result = TestDeepSeekApi(apiKey, timeout);

    if (result.success)
        Logger_Info(L"ApiRequestThread: 请求成功, total=%s %s",
            result.total_balance.c_str(), result.currency.c_str());
    else
        Logger_Warn(L"ApiRequestThread: 请求失败, err=\"%s\" http=%d",
            result.error_message.c_str(), result.http_status_code);

    // 传回结果并标记完成
    EnterCriticalSection(&plugin.m_csRequest);
    plugin.m_pendingResult = result;
    plugin.m_pendingResultReady = true;
    LeaveCriticalSection(&plugin.m_csRequest);

    return 0;
}

/**
 * @brief 定时数据获取
 * @details 由 TrafficMonitor 主程序每隔一定时间调用（通常每秒一次）。
 *          此函数执行以下步骤：
 *            1. 检查是否有异步请求已完成，如果有则处理结果
 *            2. 更新显示项目上的文本（余额数值或 "--"）
 *            3. 若已达到更新间隔且无请求进行中，启动新的异步请求
 */
void CDeepSeekDeskBand::DataRequired()
{
    // ---- 步骤 1：处理已完成异步请求的结果 ----
    if (m_requestInProgress)
    {
        EnterCriticalSection(&m_csRequest);
        if (m_pendingResultReady)
        {
            ApiTestResult result = m_pendingResult;
            m_pendingResultReady = false;
            m_requestInProgress = false;

            // 关闭并释放线程句柄
            if (m_hRequestThread)
            {
                CloseHandle(m_hRequestThread);
                m_hRequestThread = nullptr;
            }
            LeaveCriticalSection(&m_csRequest);

            if (result.success)
            {
                m_lastBalanceResult = result;
                m_hasBalance = true;
                m_lastFetchTime = GetTickCount64();
                m_historyManager.Append(result);
                Logger_Debug(L"DataRequired: 余额更新, total=%s %s",
                    result.total_balance.c_str(), result.currency.c_str());
            }
            else
            {
                Logger_Warn(L"DataRequired: 请求失败, 余额显示不变");
                // 失败时不改变 m_hasBalance，保留上次的有效数据显示
            }
        }
        else
        {
            LeaveCriticalSection(&m_csRequest);
        }
    }

    // ---- 步骤 2：更新显示文本 ----
    if (m_apiKey[0] == L'\0')
    {
        // 密钥为空：清除余额，显示 "--"
        if (m_hasBalance)
        {
            Logger_Info(L"DataRequired: API 密钥为空，清除余额显示");
            m_hasBalance = false;
        }
        m_item.SetValueText(L"--");
    }
    else if (m_hasBalance)
    {
        // 有余额数据：显示 "数值 币种"
        wchar_t valueText[DSDB_BUF_VALUE];
        swprintf_s(valueText, L"%s %s",
            m_lastBalanceResult.total_balance.c_str(),
            m_lastBalanceResult.currency.c_str());
        m_item.SetValueText(valueText);
    }
    else
    {
        // 有密钥但无数据：显示 "--"（等待首次请求完成）
        m_item.SetValueText(L"--");
    }

    // ---- 步骤 3：按间隔启动新的异步请求 ----
    if (!m_requestInProgress && m_apiKey[0] != L'\0')
    {
        ULONGLONG now = GetTickCount64();
        ULONGLONG intervalMs = static_cast<ULONGLONG>(m_updateInterval) * 1000;

        if (m_lastFetchTime == 0 || (now - m_lastFetchTime) >= intervalMs)
        {
            Logger_Info(L"DataRequired: 启动异步 API 请求, interval=%ds", m_updateInterval);

            m_requestInProgress = true;
            HANDLE hThread = CreateThread(nullptr, 0, ApiRequestThreadProc, nullptr, 0, nullptr);
            if (hThread)
                m_hRequestThread = hThread;
            else
            {
                m_requestInProgress = false;
                Logger_Error(L"DataRequired: CreateThread 失败, err=%lu", GetLastError());
            }
        }
    }
}

/**
 * @brief 获取插件信息
 * @param index 信息索引（参见 PluginInfoIndex 枚举）
 * @return 对应的信息字符串，未知索引返回空字符串
 */
const wchar_t* CDeepSeekDeskBand::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:          // 插件名称（使用显示项目名称，保持多语言一致）
        return Strings_Get(StringKey::ITEM_NAME);
    case TMI_DESCRIPTION:   // 插件功能描述（本地化）
        return Strings_Get(StringKey::PLUGIN_DESCRIPTION);
    case TMI_AUTHOR:        // 作者（本地化：中文/日文=神楽坂雅詩，其他=KagurazakaYashi(Miyabi)）
        return Strings_Get(StringKey::PLUGIN_AUTHOR);
    case TMI_COPYRIGHT:     // 版权信息（固定英文）
        return L"Copyright (c) 2026 KagurazakaYashi(KagurazakaMiyabi), licensed under Mulan PSL v2.";
    case TMI_VERSION:       // 版本号
        return DSDB_VERSION;
    case TMI_URL:           // 项目主页
        return L"https://github.com/kagurazakayashi/TrafficMonitorPlugin-DeepSeekDeskBand";
    default:
        break;
    }
    return L"";
}

// ============================================================
// 配置管理 —— 加密文件加载 / 保存
// ============================================================

/**
 * @brief 获取配置文件完整路径
 * @return 配置文件路径字符串（位于 TrafficMonitor 插件配置目录下）
 * @note   若 m_pApp 为空（尚未初始化），返回空字符串
 */
std::wstring CDeepSeekDeskBand::GetConfigFilePath() const
{
    if (m_pApp)
    {
        std::wstring path = std::wstring(m_pApp->GetPluginConfigDir()) + L"\\" DSDB_CONFIG_FILENAME;
        Logger_Debug(L"GetConfigFilePath: \"%s\"", path.c_str());
        return path;
    }
    Logger_Warn(L"GetConfigFilePath: m_pApp 为空，无法获取路径");
    return L"";
}

/**
 * @brief 从加密的二进制配置文件加载设置
 * @details 读取 DPAPI 加密的配置文件，解密后应用各项设置。
 *          解密失败或文件不存在时，自动使用默认值，不中断插件运行。
 *          加载成功后同步初始化历史记录管理器和语言设置。
 */
void CDeepSeekDeskBand::LoadConfig()
{
    std::wstring configPath = GetConfigFilePath();
    if (configPath.empty())
    {
        Logger_Info(L"LoadConfig: 路径为空，使用默认值");
        return;
    }

    // 检查文件是否存在
    DWORD attr = GetFileAttributesW(configPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        Logger_Info(L"LoadConfig: 文件不存在 (attr=INVALID), 使用默认值");
        return;
    }
    Logger_Debug(L"LoadConfig: 文件存在, attr=0x%08X", attr);

    // 读取加密文件为原始字节
    std::ifstream file(configPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        Logger_Warn(L"LoadConfig: 无法打开文件，使用默认值");
        return;
    }

    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0)
    {
        Logger_Warn(L"LoadConfig: 文件大小为 0，使用默认值");
        return;
    }
    Logger_Debug(L"LoadConfig: 文件大小=%lld 字节", static_cast<long long>(fileSize));

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> encrypted(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(encrypted.data()), fileSize))
    {
        Logger_Error(L"LoadConfig: 读取文件失败，使用默认值");
        return;
    }

    // 解密并反序列化
    ConfigBlob blob;
    if (!ConfigDecrypt(encrypted, blob))
    {
        Logger_Warn(L"LoadConfig: ConfigDecrypt 失败，使用默认值（可能是不同用户或数据损坏）");
        return;
    }

    Logger_Info(L"LoadConfig: 解密成功, interval=%d timeout=%d maxHistory=%d lang=%d apiKeyLen=%zu",
        blob.updateInterval, blob.requestTimeout, blob.maxHistoryCount,
        blob.language, blob.apiKey.size());

    // 将解密结果应用到成员变量（带范围校验）
    wcsncpy_s(m_apiKey, blob.apiKey.c_str(), DSDB_BUF_APIKEY - 1);
    m_apiKey[DSDB_BUF_APIKEY - 1] = L'\0';

    m_updateInterval = blob.updateInterval;
    if (m_updateInterval < DSDB_INTERVAL_MIN)
        m_updateInterval = DSDB_INTERVAL_MIN;
    if (m_updateInterval > DSDB_INTERVAL_MAX)
        m_updateInterval = DSDB_INTERVAL_MAX;

    m_requestTimeout = blob.requestTimeout;
    if (m_requestTimeout < DSDB_TIMEOUT_MIN)
        m_requestTimeout = DSDB_TIMEOUT_MIN;
    if (m_requestTimeout > DSDB_TIMEOUT_MAX)
        m_requestTimeout = DSDB_TIMEOUT_MAX;

    int maxHistory = blob.maxHistoryCount;
    if (maxHistory < DSDB_HISTORY_COUNT_MIN)
        maxHistory = DSDB_HISTORY_COUNT_MIN;
    if (maxHistory > DSDB_HISTORY_COUNT_MAX)
        maxHistory = DSDB_HISTORY_COUNT_MAX;
    m_historyManager.SetMaxHistoryCount(maxHistory);

    // 语言偏好：校验并应用（修复：原来只校验到 English，现在校验到 Turkish）
    Language lang = ValidateLanguage(static_cast<Language>(blob.language));
    m_language = lang;
    Strings_SetLanguage(m_language);

    Logger_Info(L"LoadConfig: 应用完成, apiKey[0]='%c' interval=%d timeout=%d history=%d lang=%d",
        (m_apiKey[0] ? m_apiKey[0] : L'-'), m_updateInterval, m_requestTimeout,
        maxHistory, static_cast<int>(m_language));

    // 加载历史记录
    m_historyManager.Load();
}

/**
 * @brief 保存设置到加密的二进制配置文件
 * @details 组装当前配置值，通过 DPAPI 加密后写入二进制文件。
 *          写入失败时记录错误日志但不中断程序运行。
 */
void CDeepSeekDeskBand::SaveConfig()
{
    std::wstring configPath = GetConfigFilePath();
    if (configPath.empty())
    {
        Logger_Error(L"SaveConfig: 路径为空，放弃保存");
        return;
    }

    // 组装明文配置
    ConfigBlob blob;
    blob.updateInterval = m_updateInterval;
    blob.requestTimeout = m_requestTimeout;
    blob.maxHistoryCount = m_historyManager.GetMaxHistoryCount();
    blob.language = static_cast<int32_t>(m_language);
    blob.apiKey = m_apiKey;

    Logger_Info(L"SaveConfig: interval=%d timeout=%d maxHistory=%d lang=%d apiKeyLen=%zu",
        blob.updateInterval, blob.requestTimeout, blob.maxHistoryCount,
        blob.language, blob.apiKey.size());

    // 加密
    std::vector<uint8_t> encrypted;
    if (!ConfigEncrypt(blob, encrypted))
    {
        Logger_Error(L"SaveConfig: ConfigEncrypt 失败！配置未保存");
        return;
    }
    Logger_Debug(L"SaveConfig: 加密后 %zu 字节", encrypted.size());

    // 写入二进制文件
    std::ofstream file(configPath, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        Logger_Error(L"SaveConfig: 无法创建文件 \"%s\", err=%lu", configPath.c_str(), GetLastError());
        return;
    }

    file.write(reinterpret_cast<const char*>(encrypted.data()),
        static_cast<std::streamsize>(encrypted.size()));
    file.close();

    if (file.good())
        Logger_Info(L"SaveConfig: 写入成功 \"%s\"", configPath.c_str());
    else
        Logger_Error(L"SaveConfig: 写入失败 \"%s\"", configPath.c_str());
}

// ============================================================
// 设置对话框
// ============================================================

/**
 * @brief 打开插件的设置对话框（模态）
 * @param hParent 父窗口句柄
 * @return 设置变更时返回 OR_OPTION_CHANGED，否则返回 OR_OPTION_UNCHANGED
 * @details 委托给 SettingsDialog 模块实现。
 *          对话框返回后，若有变更则保存配置并重置获取状态。
 */
ITMPlugin::OptionReturn CDeepSeekDeskBand::ShowOptionsDialog(void* hParent)
{
    Logger_Info(L"ShowOptionsDialog: 开始");

    // 备份当前值（用于变更比较和取消还原）
    int oldMaxHistoryCount = m_historyManager.GetMaxHistoryCount();
    Language oldLanguage = m_language;

    wchar_t apiKeyCopy[DSDB_BUF_APIKEY];
    wcscpy_s(apiKeyCopy, m_apiKey);
    int updateIntervalCopy = m_updateInterval;
    int requestTimeoutCopy = m_requestTimeout;
    int maxHistoryCountCopy = m_historyManager.GetMaxHistoryCount();
    Language languageCopy = m_language;

    // 调用设置对话框
    std::wstring configDir = m_pApp ? m_pApp->GetPluginConfigDir() : L"";
    bool changed = ShowSettingsDialog(
        (HWND)hParent,
        apiKeyCopy,
        updateIntervalCopy,
        requestTimeoutCopy,
        maxHistoryCountCopy,
        languageCopy,
        m_historyManager.GetRecords(),
        &m_historyManager,
        configDir);

    if (changed)
    {
        Logger_Info(L"ShowOptionsDialog: 设置已变更，准备保存");

        // 应用新值
        wcscpy_s(m_apiKey, apiKeyCopy);
        m_updateInterval = updateIntervalCopy;
        m_requestTimeout = requestTimeoutCopy;
        m_historyManager.SetMaxHistoryCount(maxHistoryCountCopy);
        m_language = languageCopy;
        Strings_SetLanguage(m_language);  // 立即应用语言设置

        // 重置余额获取状态，下次 DataRequired 会立即发起请求
        m_hasBalance = false;
        m_lastFetchTime = 0;

        // 历史记录数量变更时，裁剪并重写
        if (maxHistoryCountCopy != oldMaxHistoryCount)
        {
            if (maxHistoryCountCopy == 0)
            {
                Logger_Info(L"ShowOptionsDialog: maxHistoryCount=0，清除所有历史记录");
                m_historyManager.Clear();
            }
            else
            {
                Logger_Info(L"ShowOptionsDialog: 历史记录上限变更 %d -> %d，裁剪并重写",
                    oldMaxHistoryCount, maxHistoryCountCopy);
                m_historyManager.TrimToMaxCount();
            }
        }
        else
        {
            // 上限未变但可能记录数超限（如手动清除导致）
            m_historyManager.TrimToMaxCount();
        }

        SaveConfig();
        Logger_Info(L"ShowOptionsDialog: 保存完毕，返回 OR_OPTION_CHANGED");
        return OR_OPTION_CHANGED;
    }

    Logger_Debug(L"ShowOptionsDialog: 设置未变更，返回 OR_OPTION_UNCHANGED");
    return OR_OPTION_UNCHANGED;
}

// ============================================================
// 右键菜单命令
// ============================================================

/**
 * @brief 获取插件命令数量（右键菜单项数）
 * @return 始终返回 1（仅一个"DeepSeek余额助手"菜单项）
 */
int CDeepSeekDeskBand::GetCommandCount()
{
    return 1;
}

/**
 * @brief 获取插件命令名称
 * @param command_index 命令索引
 * @return 菜单项显示文本（本地化），越界返回 nullptr
 */
const wchar_t* CDeepSeekDeskBand::GetCommandName(int command_index)
{
    if (command_index == 0)
        return Strings_Get(StringKey::COMMAND_NAME);
    return nullptr;
}

/**
 * @brief 执行插件命令
 * @param command_index 命令索引
 * @param hWnd 主窗口句柄，用作设置对话框的父窗口
 * @details 当前仅响应索引 0，打开设置对话框
 */
void CDeepSeekDeskBand::OnPluginCommand(int command_index, void* hWnd, void* /*para*/)
{
    if (command_index == 0)
        ShowOptionsDialog(hWnd);
}

// ============================================================
// 插件初始化
// ============================================================

/**
 * @brief 插件初始化回调
 * @param pApp 主程序 ITrafficMonitor 接口指针
 * @details 此函数在插件被 TrafficMonitor 加载时调用。
 *          保存主程序指针，设置历史记录管理器目录，加载加密配置。
 */
void CDeepSeekDeskBand::OnInitialize(ITrafficMonitor* pApp)
{
    m_pApp = pApp;

    Logger_Info(L"======== 插件初始化 ========");
    Logger_Info(L"OnInitialize: pApp=%p, configDir=\"%s\"",
        pApp, pApp ? pApp->GetPluginConfigDir() : L"(null)");

    // 设置历史记录管理器目录
    if (pApp)
        m_historyManager.SetConfigDir(pApp->GetPluginConfigDir());

    // 加载加密配置文件（内部会调用 LoadHistory）
    LoadConfig();

    Logger_Info(L"OnInitialize: 加载完成, apiKey[0]='%c' interval=%d timeout=%d historyMax=%d historyCount=%zu lang=%d",
        (m_apiKey[0] ? m_apiKey[0] : L'-'), m_updateInterval, m_requestTimeout,
        m_historyManager.GetMaxHistoryCount(), m_historyManager.GetRecordCount(),
        static_cast<int>(m_language));
}

// ============================================================
// 导出函数 —— 插件入口
// ============================================================

/**
 * @brief 插件导出入口函数
 * @return ITMPlugin 接口指针，指向插件的全局唯一实例
 * @note   主程序通过此函数获取插件接口。
 *         返回的对象在程序运行期间始终有效，不会被主程序释放。
 */
ITMPlugin* TMPluginGetInstance()
{
    return &CDeepSeekDeskBand::Instance();
}
