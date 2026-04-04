/************************************************************************//**
 * @file    DeepSeekDeskBand.cpp
 * @brief   DeepSeekDeskBand 插件实现
 * @details 实现 CDeepSeekDeskBandItem（显示项目）和 CDeepSeekDeskBand（插件主类）
 *          的所有接口函数，以及插件导出函数 TMPluginGetInstance()。
 ****************************************************************************/
#include "DeepSeekDeskBand.h"
#include "ConfigEncrypt.h"
#include "HttpClient.h"
#include "Logger.h"
#include "framework.h"
#include <string.h>
#include <stdio.h>
#include <cstdint>
#include <fstream>
#include <vector>
#include <string>
#include <CommCtrl.h>
#include <dpapi.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "crypt32.lib")

// ============================================================
// CDeepSeekDeskBandItem 实现 —— 显示项目
// ============================================================

/**
 * @brief 构造函数，初始化默认显示文本
 */
CDeepSeekDeskBandItem::CDeepSeekDeskBandItem()
{
    // 初始化数值文本为默认值 "就绪"
    wcscpy_s(m_valueText, L"--");
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
    // API 密钥为空 → 显示 "--"
    if (m_apiKey[0] == L'\0')
    {
        if (m_hasBalance)
        {
            Logger_Log(L"DataRequired: API 密钥为空，清除余额显示");
            m_hasBalance = false;
        }
        m_item.SetValueText(L"--");
        return;
    }

    ULONGLONG now = GetTickCount64();
    ULONGLONG intervalMs = static_cast<ULONGLONG>(m_updateInterval) * 1000;

    // 到达更新周期时发起 API 请求
    if (m_lastFetchTime == 0 || (now - m_lastFetchTime) >= intervalMs)
    {
        Logger_Log(L"DataRequired: 开始获取余额 (interval=%ds timeout=%ds)",
            m_updateInterval, m_requestTimeout);
        ApiTestResult result = TestDeepSeekApi(m_apiKey, m_requestTimeout);

        if (result.success)
        {
            Logger_Log(L"DataRequired: 获取成功 total=%s %s",
                result.total_balance.c_str(), result.currency.c_str());
            m_lastBalanceResult = result;
            m_hasBalance = true;
            m_lastFetchTime = now;
            AppendHistoryRecord(result);
        }
        else
        {
            Logger_Log(L"DataRequired: 获取失败 err=\"%s\" http=%d",
                result.error_message.c_str(), result.http_status_code);
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
        m_item.SetValueText(L"--");
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
// 历史记录辅助函数 —— 序列化 / 加密 / 解密单条记录
// ============================================================

/**
 * @brief 将单条历史记录序列化为紧凑二进制格式（49 字节）
 */
static std::vector<uint8_t> SerializeHistoryRecord(const HistoryRecord& rec)
{
    std::vector<uint8_t> data(49);
    size_t off = 0;
    memcpy(&data[off], &rec.timestamp, 8);       off += 8;
    data[off] = rec.is_available;                 off += 1;
    memcpy(&data[off], &rec.total_balance, 8);    off += 8;
    memcpy(&data[off], &rec.granted_balance, 8);  off += 8;
    memcpy(&data[off], &rec.topped_up_balance, 8); off += 8;
    memcpy(&data[off], rec.currency, 16);         off += 16;  // wchar_t[8] = 16 bytes
    return data;
}

/**
 * @brief 从紧凑二进制格式反序列化为单条历史记录
 */
static HistoryRecord DeserializeHistoryRecord(const uint8_t* data, size_t size)
{
    HistoryRecord rec = {};
    if (size >= 49)
    {
        size_t off = 0;
        memcpy(&rec.timestamp, &data[off], 8);    off += 8;
        rec.is_available = data[off];              off += 1;
        memcpy(&rec.total_balance, &data[off], 8); off += 8;
        memcpy(&rec.granted_balance, &data[off], 8); off += 8;
        memcpy(&rec.topped_up_balance, &data[off], 8); off += 8;
        memcpy(rec.currency, &data[off], 16);      off += 16;
        rec.currency[7] = L'\0';  // 确保 null 终止
    }
    return rec;
}

/**
 * @brief 使用 DPAPI 加密单条历史记录
 */
static bool EncryptHistoryRecord(const HistoryRecord& rec, std::vector<uint8_t>& outEncrypted)
{
    std::vector<uint8_t> plain = SerializeHistoryRecord(rec);

    DATA_BLOB dataIn;
    dataIn.pbData = plain.data();
    dataIn.cbData = static_cast<DWORD>(plain.size());

    DATA_BLOB dataOut = {};
    if (!CryptProtectData(&dataIn, nullptr, nullptr, nullptr, nullptr,
            CRYPTPROTECT_LOCAL_MACHINE, &dataOut))
    {
        Logger_Log(L"EncryptHistoryRecord: CryptProtectData 失败 err=%lu", GetLastError());
        return false;
    }

    outEncrypted.assign(dataOut.pbData, dataOut.pbData + dataOut.cbData);
    LocalFree(dataOut.pbData);
    return true;
}

/**
 * @brief 使用 DPAPI 解密单条历史记录
 */
static bool DecryptHistoryRecord(const uint8_t* encryptedData, size_t encryptedSize,
                                  HistoryRecord& outRecord)
{
    DATA_BLOB dataIn;
    dataIn.pbData = const_cast<BYTE*>(encryptedData);
    dataIn.cbData = static_cast<DWORD>(encryptedSize);

    DATA_BLOB dataOut = {};
    if (!CryptUnprotectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut))
    {
        Logger_Log(L"DecryptHistoryRecord: CryptUnprotectData 失败 err=%lu", GetLastError());
        return false;
    }

    if (dataOut.cbData < 49)
    {
        Logger_Log(L"DecryptHistoryRecord: 解密后数据不足 49 字节 (实为 %lu)", dataOut.cbData);
        LocalFree(dataOut.pbData);
        return false;
    }

    outRecord = DeserializeHistoryRecord(dataOut.pbData, dataOut.cbData);
    LocalFree(dataOut.pbData);
    return true;
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
    if (m_pApp)
    {
        std::wstring path = std::wstring(m_pApp->GetPluginConfigDir()) + L"\\" DSDB_CONFIG_FILENAME;
        Logger_Log(L"GetConfigFilePath: \"%s\"", path.c_str());
        return path;
    }
    Logger_Log(L"GetConfigFilePath: m_pApp 为空，无法获取路径");
    return L"";
}

/**
 * @brief 获取历史记录文件完整路径
 * @return 历史记录文件路径字符串
 */
std::wstring CDeepSeekDeskBand::GetHistoryFilePath()
{
    if (m_pApp)
    {
        std::wstring path = std::wstring(m_pApp->GetPluginConfigDir()) + L"\\" DSDB_HISTORY_FILENAME;
        Logger_Log(L"GetHistoryFilePath: \"%s\"", path.c_str());
        return path;
    }
    Logger_Log(L"GetHistoryFilePath: m_pApp 为空，无法获取路径");
    return L"";
}

/**
 * @brief 从加密的二进制配置文件加载设置
 * @note  解密失败时自动恢复默认设置。
 */
void CDeepSeekDeskBand::LoadConfig()
{
    std::wstring configPath = GetConfigFilePath();
    if (configPath.empty())
    {
        Logger_Log(L"LoadConfig: 路径为空，使用默认值");
        return;
    }

    // 检查文件是否存在
    DWORD attr = GetFileAttributesW(configPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        Logger_Log(L"LoadConfig: 文件不存在 (attr=INVALID), 使用默认值");
        return;
    }
    Logger_Log(L"LoadConfig: 文件存在, attr=0x%08X", attr);

    // 读取加密文件为原始字节
    std::ifstream file(configPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        Logger_Log(L"LoadConfig: 无法打开文件，使用默认值");
        return;
    }

    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0)
    {
        Logger_Log(L"LoadConfig: 文件大小为 0，使用默认值");
        return;
    }
    Logger_Log(L"LoadConfig: 文件大小=%lld 字节", static_cast<long long>(fileSize));

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> encrypted(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(encrypted.data()), fileSize))
    {
        Logger_Log(L"LoadConfig: 读取文件失败，使用默认值");
        return;
    }

    // 解密并反序列化
    ConfigBlob blob;
    if (!ConfigDecrypt(encrypted, blob))
    {
        Logger_Log(L"LoadConfig: ConfigDecrypt 失败，使用默认值");
        return;
    }

    Logger_Log(L"LoadConfig: 解密成功 updateInterval=%d requestTimeout=%d maxHistory=%d apiKey=\"%s\"",
        blob.updateInterval, blob.requestTimeout, blob.maxHistoryCount, blob.apiKey.c_str());

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

    m_maxHistoryCount = blob.maxHistoryCount;
    if (m_maxHistoryCount < DSDB_HISTORY_COUNT_MIN)
        m_maxHistoryCount = DSDB_HISTORY_COUNT_MIN;
    if (m_maxHistoryCount > DSDB_HISTORY_COUNT_MAX)
        m_maxHistoryCount = DSDB_HISTORY_COUNT_MAX;

    Logger_Log(L"LoadConfig: 应用后 updateInterval=%d requestTimeout=%d maxHistory=%d apiKey[0]='%c'",
        m_updateInterval, m_requestTimeout, m_maxHistoryCount,
        (m_apiKey[0] ? m_apiKey[0] : L'-'));

    // 加载历史记录
    LoadHistory();
}

/**
 * @brief 保存设置到加密的二进制配置文件
 */
void CDeepSeekDeskBand::SaveConfig()
{
    std::wstring configPath = GetConfigFilePath();
    if (configPath.empty())
    {
        Logger_Log(L"SaveConfig: 路径为空，放弃保存");
        return;
    }

    // 组装明文配置
    ConfigBlob blob;
    blob.updateInterval = m_updateInterval;
    blob.requestTimeout = m_requestTimeout;
    blob.maxHistoryCount = m_maxHistoryCount;
    blob.apiKey = m_apiKey;

    Logger_Log(L"SaveConfig: interval=%d timeout=%d maxHistory=%d apiKey=\"%s\"",
        blob.updateInterval, blob.requestTimeout, blob.maxHistoryCount, blob.apiKey.c_str());

    // 加密
    std::vector<uint8_t> encrypted;
    if (!ConfigEncrypt(blob, encrypted))
    {
        Logger_Log(L"SaveConfig: ConfigEncrypt 失败！");
        return;
    }
    Logger_Log(L"SaveConfig: 加密后大小=%zu 字节", encrypted.size());

    // 写入二进制文件
    std::ofstream file(configPath, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        Logger_Log(L"SaveConfig: 无法创建文件 \"%s\"", configPath.c_str());
        return;
    }

    file.write(reinterpret_cast<const char*>(encrypted.data()),
        static_cast<std::streamsize>(encrypted.size()));
    file.close();

    if (file.good())
        Logger_Log(L"SaveConfig: 写入成功 \"%s\"", configPath.c_str());
    else
        Logger_Log(L"SaveConfig: 写入失败！\"%s\"", configPath.c_str());
}

// ============================================================
// 历史记录管理 —— 增量追加加密，性能优先
// ============================================================

/**
 * @brief 从加密的历史记录文件加载所有记录
 * @details 文件格式：
 *   [Header, 12B, 未加密] magic(4B) + version(4B) + record_count(4B)
 *   [Record 0] enc_size(4B) + DPAPI加密数据(enc_size B)
 *   [Record 1] enc_size(4B) + DPAPI加密数据(enc_size B)
 *   ...
 * @note   逐条解密，只保留最近 maxHistoryCount 条到内存中
 */
void CDeepSeekDeskBand::LoadHistory()
{
    std::wstring historyPath = GetHistoryFilePath();
    if (historyPath.empty())
    {
        Logger_Log(L"LoadHistory: 路径为空");
        return;
    }

    m_historyRecords.clear();

    // 检查文件是否存在
    DWORD attr = GetFileAttributesW(historyPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        Logger_Log(L"LoadHistory: 文件不存在，初始化为空");
        return;
    }

    std::ifstream file(historyPath, std::ios::binary);
    if (!file.is_open())
    {
        Logger_Log(L"LoadHistory: 无法打开文件");
        return;
    }

    // 读取文件头（12 字节）
    uint8_t header[12];
    if (!file.read(reinterpret_cast<char*>(header), 12))
    {
        Logger_Log(L"LoadHistory: 文件头读取失败");
        return;
    }

    uint32_t magic    = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
    uint32_t version  = header[4] | (header[5] << 8) | (header[6] << 16) | (header[7] << 24);
    uint32_t count    = header[8] | (header[9] << 8) | (header[10] << 16) | (header[11] << 24);

    if (magic != DSDB_HISTORY_MAGIC || version != 1)
    {
        Logger_Log(L"LoadHistory: 魔数/版本不匹配 magic=0x%08X version=%u", magic, version);
        return;
    }

    Logger_Log(L"LoadHistory: 文件中有 %u 条记录", count);

    std::vector<HistoryRecord> allRecords;
    allRecords.reserve(count);

    for (uint32_t i = 0; i < count; i++)
    {
        // 读取加密数据长度（4 字节）
        uint8_t sizeBuf[4];
        if (!file.read(reinterpret_cast<char*>(sizeBuf), 4))
        {
            Logger_Log(L"LoadHistory: 读取第 %u 条记录长度失败", i);
            break;
        }
        uint32_t encSize = sizeBuf[0] | (sizeBuf[1] << 8) | (sizeBuf[2] << 16) | (sizeBuf[3] << 24);

        if (encSize == 0 || encSize > 4096)
        {
            Logger_Log(L"LoadHistory: 第 %u 条记录长度异常 encSize=%u，跳过", i, encSize);
            break;
        }

        // 读取加密数据
        std::vector<uint8_t> encData(encSize);
        if (!file.read(reinterpret_cast<char*>(encData.data()), encSize))
        {
            Logger_Log(L"LoadHistory: 读取第 %u 条记录数据失败", i);
            break;
        }

        // 逐条解密
        HistoryRecord rec;
        if (DecryptHistoryRecord(encData.data(), encSize, rec))
            allRecords.push_back(rec);
        else
            Logger_Log(L"LoadHistory: 第 %u 条记录解密失败，跳过", i);
    }

    Logger_Log(L"LoadHistory: 成功解密 %zu 条记录", allRecords.size());

    // 只保留最近 maxHistoryCount 条
    if (m_maxHistoryCount > 0 && allRecords.size() > static_cast<size_t>(m_maxHistoryCount))
    {
        size_t trim = allRecords.size() - m_maxHistoryCount;
        m_historyRecords.assign(allRecords.begin() + trim, allRecords.end());
        Logger_Log(L"LoadHistory: 截断 %zu 条旧记录，保留 %zu 条", trim, m_historyRecords.size());
    }
    else if (m_maxHistoryCount == 0)
    {
        Logger_Log(L"LoadHistory: maxHistoryCount=0，不保留历史记录");
    }
    else
    {
        m_historyRecords = std::move(allRecords);
    }
}

/**
 * @brief 保存历史记录到文件
 * @param fullRewrite true=整体加密重写所有记录；false=仅追加最后一条（增量模式）
 * @details 增量模式性能最优：只加密并写入最后一条记录，仅更新头部计数。
 *          整体重写用于：设置变更导致数量限制改变时、记录数远超上限时。
 */
void CDeepSeekDeskBand::SaveHistory(bool fullRewrite)
{
    std::wstring historyPath = GetHistoryFilePath();
    if (historyPath.empty())
    {
        Logger_Log(L"SaveHistory: 路径为空");
        return;
    }

    if (fullRewrite)
    {
        // ---- 整体加密重写 ----
        Logger_Log(L"SaveHistory: 整体重写 %zu 条记录", m_historyRecords.size());

        std::ofstream file(historyPath, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            Logger_Log(L"SaveHistory: 无法创建文件 \"%s\"", historyPath.c_str());
            return;
        }

        // 写入文件头（12 字节，明文）
        uint8_t header[12];
        header[0] = static_cast<uint8_t>(DSDB_HISTORY_MAGIC & 0xFF);
        header[1] = static_cast<uint8_t>((DSDB_HISTORY_MAGIC >> 8) & 0xFF);
        header[2] = static_cast<uint8_t>((DSDB_HISTORY_MAGIC >> 16) & 0xFF);
        header[3] = static_cast<uint8_t>((DSDB_HISTORY_MAGIC >> 24) & 0xFF);
        header[4] = 1; header[5] = 0; header[6] = 0; header[7] = 0;  // version=1
        uint32_t count = static_cast<uint32_t>(m_historyRecords.size());
        header[8]  = static_cast<uint8_t>(count & 0xFF);
        header[9]  = static_cast<uint8_t>((count >> 8) & 0xFF);
        header[10] = static_cast<uint8_t>((count >> 16) & 0xFF);
        header[11] = static_cast<uint8_t>((count >> 24) & 0xFF);
        file.write(reinterpret_cast<const char*>(header), 12);

        // 逐条加密写入
        for (const auto& rec : m_historyRecords)
        {
            std::vector<uint8_t> enc;
            if (!EncryptHistoryRecord(rec, enc))
            {
                Logger_Log(L"SaveHistory: 加密某条记录失败，中止写入");
                file.close();
                return;
            }

            uint32_t encSize = static_cast<uint32_t>(enc.size());
            uint8_t sizeBuf[4];
            sizeBuf[0] = static_cast<uint8_t>(encSize & 0xFF);
            sizeBuf[1] = static_cast<uint8_t>((encSize >> 8) & 0xFF);
            sizeBuf[2] = static_cast<uint8_t>((encSize >> 16) & 0xFF);
            sizeBuf[3] = static_cast<uint8_t>((encSize >> 24) & 0xFF);
            file.write(reinterpret_cast<const char*>(sizeBuf), 4);
            file.write(reinterpret_cast<const char*>(enc.data()), encSize);
        }

        file.close();
        Logger_Log(L"SaveHistory: 整体重写完成");
    }
    else
    {
        // ---- 增量追加：仅追加最后一条记录 ----
        if (m_historyRecords.empty())
        {
            Logger_Log(L"SaveHistory: 增量模式，记录为空，跳过");
            return;
        }

        const HistoryRecord& lastRec = m_historyRecords.back();
        Logger_Log(L"SaveHistory: 增量追加一条记录");

        // 加密最后一条
        std::vector<uint8_t> enc;
        if (!EncryptHistoryRecord(lastRec, enc))
        {
            Logger_Log(L"SaveHistory: 加密失败");
            return;
        }

        // 打开文件（读写模式，不移除内容）
        // 注意：文件可能较大，使用 Windows API 而非 ofstream::app 以便精确控制
        HANDLE hFile = CreateFileW(historyPath.c_str(),
            GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD err = GetLastError();
            // 若文件不存在则创建
            if (err == ERROR_FILE_NOT_FOUND)
            {
                Logger_Log(L"SaveHistory: 文件不存在，回退到整体重写");
                SaveHistory(true);
                return;
            }
            Logger_Log(L"SaveHistory: 无法打开文件 err=%lu", err);
            return;
        }

        // 读取现有文件头以获取当前记录数
        DWORD bytesRead = 0;
        uint8_t header[12] = {};
        ReadFile(hFile, header, 12, &bytesRead, nullptr);

        bool isNewFile = (bytesRead == 0);
        uint32_t currentCount = 0;

        if (!isNewFile && bytesRead == 12)
        {
            uint32_t magic = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
            if (magic == DSDB_HISTORY_MAGIC)
            {
                currentCount = header[8] | (header[9] << 8) | (header[10] << 16) | (header[11] << 24);
            }
            else
            {
                Logger_Log(L"SaveHistory: 现有文件魔数不匹配，回退到整体重写");
                CloseHandle(hFile);
                SaveHistory(true);
                return;
            }
        }

        // 移动到文件末尾
        SetFilePointer(hFile, 0, nullptr, FILE_END);

        // 追加加密数据（4B 长度 + 加密块）
        uint32_t encSize = static_cast<uint32_t>(enc.size());
        uint8_t sizeBuf[4];
        sizeBuf[0] = static_cast<uint8_t>(encSize & 0xFF);
        sizeBuf[1] = static_cast<uint8_t>((encSize >> 8) & 0xFF);
        sizeBuf[2] = static_cast<uint8_t>((encSize >> 16) & 0xFF);
        sizeBuf[3] = static_cast<uint8_t>((encSize >> 24) & 0xFF);

        DWORD bytesWritten = 0;
        WriteFile(hFile, sizeBuf, 4, &bytesWritten, nullptr);
        if (bytesWritten != 4)
        {
            Logger_Log(L"SaveHistory: 写入长度失败");
            CloseHandle(hFile);
            return;
        }
        WriteFile(hFile, enc.data(), encSize, &bytesWritten, nullptr);
        if (bytesWritten != encSize)
        {
            Logger_Log(L"SaveHistory: 写入加密数据失败");
            CloseHandle(hFile);
            return;
        }

        // 更新文件头中的记录数（覆盖第 8~11 字节）
        uint32_t newCount = currentCount + 1;
        SetFilePointer(hFile, 8, nullptr, FILE_BEGIN);
        header[8]  = static_cast<uint8_t>(newCount & 0xFF);
        header[9]  = static_cast<uint8_t>((newCount >> 8) & 0xFF);
        header[10] = static_cast<uint8_t>((newCount >> 16) & 0xFF);
        header[11] = static_cast<uint8_t>((newCount >> 24) & 0xFF);
        WriteFile(hFile, header + 8, 4, &bytesWritten, nullptr);

        FlushFileBuffers(hFile);
        CloseHandle(hFile);

        Logger_Log(L"SaveHistory: 增量追加完成 newCount=%u", newCount);
    }
}

/**
 * @brief 追加一条历史记录并保存到文件
 * @param result DeepSeek API 查询结果
 * @details 在内存中追加记录，若超出最大数量则移除最旧的记录。
 *          使用增量模式写入文件（仅追加一条加密记录）。
 *          当记录数为 0 时不记录。
 */
void CDeepSeekDeskBand::AppendHistoryRecord(const ApiTestResult& result)
{
    if (m_maxHistoryCount == 0)
    {
        Logger_Log(L"AppendHistoryRecord: maxHistoryCount=0，不记录");
        return;
    }

    // 构造历史记录
    HistoryRecord rec = {};
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    // FILETIME -> Unix 毫秒时间戳
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    // FILETIME 从 1601-01-01 起，100ns 单位；Unix 从 1970-01-01 起
    rec.timestamp = static_cast<int64_t>((uli.QuadPart - 116444736000000000ULL) / 10000ULL);

    rec.is_available = result.is_available ? 1 : 0;
    rec.total_balance = _wtof(result.total_balance.c_str());
    rec.granted_balance = _wtof(result.granted_balance.c_str());
    rec.topped_up_balance = _wtof(result.topped_up_balance.c_str());
    wcsncpy_s(rec.currency, result.currency.c_str(), 7);
    rec.currency[7] = L'\0';

    // 存入内存
    m_historyRecords.push_back(rec);

    // 超出上限则移除最旧的
    while (m_historyRecords.size() > static_cast<size_t>(m_maxHistoryCount))
        m_historyRecords.erase(m_historyRecords.begin());

    Logger_Log(L"AppendHistoryRecord: 记录 ts=%lld avail=%d total=%.2f %s (当前 %zu 条)",
        rec.timestamp, rec.is_available, rec.total_balance,
        rec.currency, m_historyRecords.size());

    // 增量写入文件
    SaveHistory(false);
}

/**
 * @brief 插件初始化回调
 * @param pApp 主程序 ITrafficMonitor 接口指针
 * @details 保存主程序指针，加载配置文件
 */
void CDeepSeekDeskBand::OnInitialize(ITrafficMonitor* pApp)
{
    m_pApp = pApp;
    Logger_Log(L"======== 插件初始化 ========");
    Logger_Log(L"OnInitialize: pApp=%p, GetPluginConfigDir()=\"%s\"",
        pApp, pApp ? pApp->GetPluginConfigDir() : L"(null)");
    LoadConfig();
    Logger_Log(L"加载配置完成: apiKey[0]='%c' updateInterval=%d requestTimeout=%d maxHistory=%d historyCount=%zu",
        (m_apiKey[0] ? m_apiKey[0] : L'-'), m_updateInterval, m_requestTimeout,
        m_maxHistoryCount, m_historyRecords.size());
}

// ============================================================
// 设置对话框
// ============================================================

/** @brief 设置对话框窗口类名 */
static const wchar_t* SETTINGS_DIALOG_CLASS = L"DeepSeekDeskBandSettingsDlg";

/** @brief 对话框控件 ID */
enum
{
    IDC_EDIT_API_KEY        = 1001,   // DeepSeek API 输入框
    IDC_STATIC_API_HINT     = 1002,   // API 格式提示
    IDC_BTN_TEST_API        = 1003,   // 测试API 按钮
    IDC_STATIC_STATUS       = 1004,   // 测试结果状态文本
    IDC_EDIT_INTERVAL       = 1005,   // 更新间隔输入框
    IDC_SPIN_INTERVAL       = 1006,   // 更新间隔 UpDown 控件
    IDC_BTN_OK              = 1007,   // 确定按钮
    IDC_BTN_CANCEL          = 1008,   // 取消按钮
    IDC_EDIT_TIMEOUT        = 1009,   // 请求超时输入框
    IDC_SPIN_TIMEOUT        = 1010,   // 请求超时 UpDown 控件
    IDC_EDIT_HISTORY_COUNT  = 1011,   // 历史记录数量输入框
    IDC_SPIN_HISTORY_COUNT  = 1012,   // 历史记录数量 UpDown 控件
    IDC_LIST_HISTORY        = 1013,   // 历史记录 ListView 控件
    IDC_BTN_CLEAR_HISTORY   = 1014,   // 清除历史按钮
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
    int     maxHistoryCount;          // 当前设置的历史记录最大数量
    int     maxHistoryCountOrig;      // 原始历史记录最大数量
    const std::vector<HistoryRecord>* historyRecords; // 指向插件实例中的历史记录（只读）
    bool    historyCleared;            // 历史记录是否已被清除
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
 * @brief 刷新 ListView 中的历史记录数据
 * @param hList       ListView 控件句柄
 * @param records     历史记录列表（只读）
 */
static void RefreshHistoryList(HWND hList, const std::vector<HistoryRecord>* records)
{
    ListView_DeleteAllItems(hList);

    if (!records || records->empty())
        return;

    for (size_t i = 0; i < records->size(); i++)
    {
        const HistoryRecord& rec = (*records)[i];

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

            // 转换为本地时区
            SYSTEMTIME stLocal;
            SystemTimeToTzSpecificLocalTime(nullptr, &st, &stLocal);

            GetDateFormatW(LOCALE_USER_DEFAULT, 0, &stLocal, L"MM-dd", timeStr, 6);
            timeStr[5] = L' ';
            GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &stLocal, L"HH:mm:ss", timeStr + 6, 9);
        }
        else
        {
            wcscpy_s(timeStr, L"--");
        }

        // 插入一行
        LVITEMW lvi = {};
        lvi.mask = 0;
        lvi.iItem = static_cast<int>(i);
        ListView_InsertItem(hList, &lvi);
        ListView_SetItemText(hList, static_cast<int>(i), 0, timeStr);
        ListView_SetItemText(hList, static_cast<int>(i), 1,
            const_cast<wchar_t*>(rec.is_available ? L"是" : L"否"));

        wchar_t numStr[32];
        swprintf_s(numStr, L"%.2f", rec.total_balance);
        ListView_SetItemText(hList, static_cast<int>(i), 2, numStr);
        swprintf_s(numStr, L"%.2f", rec.granted_balance);
        ListView_SetItemText(hList, static_cast<int>(i), 3, numStr);
        swprintf_s(numStr, L"%.2f", rec.topped_up_balance);
        ListView_SetItemText(hList, static_cast<int>(i), 4, numStr);
        ListView_SetItemText(hList, static_cast<int>(i), 5, const_cast<wchar_t*>(rec.currency));
    }
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

        // --- "历史记录数量:" 标签 ---
        hChild = CreateWindowW(L"STATIC", L"历史记录数量:",
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            20, 224, 110, 20,
            hWnd, nullptr, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 历史记录数量输入框（Spin 控件的伙伴编辑框） ---
        wchar_t bufHistoryCount[DSDB_BUF_NUMBER];
        swprintf_s(bufHistoryCount, L"%d", pData->maxHistoryCount);
        hChild = CreateWindowW(L"EDIT", bufHistoryCount,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_TABSTOP,
            135, 221, 100, 22,
            hWnd, (HMENU)IDC_EDIT_HISTORY_COUNT, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 历史记录数量 UpDown Spin 控件（范围 0~10000） ---
        {
            HWND hSpin = CreateWindowW(UPDOWN_CLASSW, nullptr,
                WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hWnd, (HMENU)IDC_SPIN_HISTORY_COUNT, hInst, nullptr);
            SendMessageW(hSpin, UDM_SETRANGE32, DSDB_HISTORY_COUNT_MIN, DSDB_HISTORY_COUNT_MAX);
            SendMessageW(hSpin, UDM_SETPOS32, 0, pData->maxHistoryCount);
            SendMessageW(hSpin, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // --- "历史记录:" 标签 ---
        hChild = CreateWindowW(L"STATIC", L"历史记录:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 256, 80, 20,
            hWnd, nullptr, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- 历史记录 ListView 控件（报告模式，多列显示） ---
        {
            HWND hList = CreateWindowW(WC_LISTVIEWW, nullptr,
                WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL |
                LVS_SHOWSELALWAYS | WS_TABSTOP,
                20, 278, 500, 180,
                hWnd, (HMENU)IDC_LIST_HISTORY, hInst, nullptr);
            SendMessageW(hList, WM_SETFONT, (WPARAM)hFont, TRUE);

            // 启用整行选择和网格线
            ListView_SetExtendedListViewStyle(hList,
                LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

            // 添加列标题
            LVCOLUMNW lvc = {};
            lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
            lvc.fmt = LVCFMT_LEFT;

            lvc.cx = 140; lvc.pszText = const_cast<wchar_t*>(L"时间");
            ListView_InsertColumn(hList, 0, &lvc);

            lvc.cx = 50;  lvc.pszText = const_cast<wchar_t*>(L"可用");
            ListView_InsertColumn(hList, 1, &lvc);

            lvc.cx = 85;  lvc.pszText = const_cast<wchar_t*>(L"总余额");
            ListView_InsertColumn(hList, 2, &lvc);

            lvc.cx = 85;  lvc.pszText = const_cast<wchar_t*>(L"赠送余额");
            ListView_InsertColumn(hList, 3, &lvc);

            lvc.cx = 85;  lvc.pszText = const_cast<wchar_t*>(L"充值余额");
            ListView_InsertColumn(hList, 4, &lvc);

            lvc.cx = 55;  lvc.pszText = const_cast<wchar_t*>(L"币种");
            ListView_InsertColumn(hList, 5, &lvc);

            // 填充初始数据
            RefreshHistoryList(hList, pData->historyRecords);
        }

        // 启动定时器，每秒刷新一次 ListView（自动反映新增记录）
        SetTimer(hWnd, 1, 1000, nullptr);

        // --- "确定(&O)" 按钮（默认不可用） ---
        hChild = CreateWindowW(L"BUTTON", L"确定(&O)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED | WS_TABSTOP,
            340, 475, 85, 28,
            hWnd, (HMENU)IDC_BTN_OK, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "取消(&C)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", L"取消(&C)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            435, 475, 85, 28,
            hWnd, (HMENU)IDC_BTN_CANCEL, hInst, nullptr);
        SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

        // --- "清除历史(&R)" 按钮 ---
        hChild = CreateWindowW(L"BUTTON", L"清除历史(&R)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            20, 475, 100, 28,
            hWnd, (HMENU)IDC_BTN_CLEAR_HISTORY, hInst, nullptr);
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

    case WM_TIMER:
    {
        // 定时刷新 ListView，自动反映新增的历史记录
        if (wParam == 1 && pData)
        {
            HWND hList = GetDlgItem(hWnd, IDC_LIST_HISTORY);
            if (hList)
                RefreshHistoryList(hList, pData->historyRecords);
        }
        return 0;
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

        case IDC_BTN_CLEAR_HISTORY:
            if (pData)
            {
                int result = MessageBoxW(hWnd,
                    L"确定要清除所有历史记录吗？\n\n此操作不可撤销。",
                    L"确认清除", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
                if (result == IDYES)
                {
                    // 立即清空内存中的记录
                    if (pData->historyRecords)
                        const_cast<std::vector<HistoryRecord>*>(pData->historyRecords)->clear();
                    // 立即清空文件
                    CDeepSeekDeskBand::Instance().SaveHistory(true);
                    // 刷新 ListView
                    HWND hList = GetDlgItem(hWnd, IDC_LIST_HISTORY);
                    if (hList)
                        ListView_DeleteAllItems(hList);
                }
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

                wchar_t bufHistoryCount[DSDB_BUF_NUMBER];
                GetDlgItemTextW(hWnd, IDC_EDIT_HISTORY_COUNT, bufHistoryCount, DSDB_BUF_NUMBER);
                pData->maxHistoryCount = _wtoi(bufHistoryCount);
                if (pData->maxHistoryCount < DSDB_HISTORY_COUNT_MIN)
                    pData->maxHistoryCount = DSDB_HISTORY_COUNT_MIN;
                if (pData->maxHistoryCount > DSDB_HISTORY_COUNT_MAX)
                    pData->maxHistoryCount = DSDB_HISTORY_COUNT_MAX;
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
                pData->maxHistoryCount = pData->maxHistoryCountOrig;
                pData->historyCleared = false;
                pData->changed = false;

                // 若之前点了清除历史，刷新 ListView 恢复显示
                HWND hList = GetDlgItem(hWnd, IDC_LIST_HISTORY);
                if (hList)
                    RefreshHistoryList(hList, pData->historyRecords);
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
            pData->maxHistoryCount = pData->maxHistoryCountOrig;
            pData->historyCleared = false;
            pData->changed = false;
        }
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
    {
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

/**
 * @brief 打开插件的设置对话框
 * @param hParent 父窗口句柄
 * @return 设置变更时返回 OR_OPTION_CHANGED，否则返回 OR_OPTION_UNCHANGED
 */
ITMPlugin::OptionReturn CDeepSeekDeskBand::ShowOptionsDialog(void* hParent)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);

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
    wcscpy_s(dlgData.apiKey, m_apiKey);
    wcscpy_s(dlgData.apiKeyOrig, m_apiKey);
    dlgData.updateInterval = m_updateInterval;
    dlgData.updateIntervalOrig = m_updateInterval;
    dlgData.requestTimeout = m_requestTimeout;
    dlgData.requestTimeoutOrig = m_requestTimeout;
    dlgData.maxHistoryCount = m_maxHistoryCount;
    dlgData.maxHistoryCountOrig = m_maxHistoryCount;
    dlgData.historyRecords = &m_historyRecords;
    dlgData.historyCleared = false;
    dlgData.changed = false;

    // 计算居中位置（窗口扩大以容纳历史记录 ListView）
    int dlgWidth = 560;
    int dlgHeight = 560;
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
        Logger_Log(L"ShowOptionsDialog: 设置已变更，准备保存");
        int oldMaxHistoryCount = m_maxHistoryCount;
        wcscpy_s(m_apiKey, dlgData.apiKey);
        m_updateInterval = dlgData.updateInterval;
        m_requestTimeout = dlgData.requestTimeout;
        m_maxHistoryCount = dlgData.maxHistoryCount;
        m_hasBalance = false;       // 设置变更后立即重新获取余额
        m_lastFetchTime = 0;
        SaveConfig();

        // 历史记录数量变更时，截断内存中的记录并整体重写历史文件
        if (m_maxHistoryCount != oldMaxHistoryCount)
        {
            if (m_maxHistoryCount == 0)
            {
                m_historyRecords.clear();
                Logger_Log(L"ShowOptionsDialog: maxHistoryCount=0，清除所有历史记录");
                SaveHistory(true);
            }
            else if (m_maxHistoryCount < oldMaxHistoryCount || oldMaxHistoryCount == 0)
            {
                while (m_historyRecords.size() > static_cast<size_t>(m_maxHistoryCount))
                    m_historyRecords.erase(m_historyRecords.begin());
                Logger_Log(L"ShowOptionsDialog: 裁剪历史记录至 %zu 条", m_historyRecords.size());
                SaveHistory(true);
            }
        }

        // 无论上限是否改变，只要实际记录数超过上限就立即清理（从旧记录开始）
        if (m_maxHistoryCount > 0)
        {
            size_t before = m_historyRecords.size();
            while (m_historyRecords.size() > static_cast<size_t>(m_maxHistoryCount))
                m_historyRecords.erase(m_historyRecords.begin());
            if (m_historyRecords.size() < before)
            {
                Logger_Log(L"ShowOptionsDialog: 强制清理超限记录 %zu → %zu", before,
                    m_historyRecords.size());
                SaveHistory(true);
            }
        }

        // 用户手动清除了历史记录，重写为空文件
        if (dlgData.historyCleared)
        {
            Logger_Log(L"ShowOptionsDialog: 用户手动清除历史，重写文件");
            SaveHistory(true);
        }

        Logger_Log(L"ShowOptionsDialog: 保存完毕，返回 OR_OPTION_CHANGED");
        return OR_OPTION_CHANGED;
    }
    Logger_Log(L"ShowOptionsDialog: 设置未变更，返回 OR_OPTION_UNCHANGED");
    return OR_OPTION_UNCHANGED;
}

/**
 * @brief 获取插件命令数量（右键菜单项数）
 * @return 1（仅一个"DeepSeek余额助手"菜单项）
 */
int CDeepSeekDeskBand::GetCommandCount()
{
    return 1;
}

/**
 * @brief 获取插件命令名称
 * @param command_index 命令索引
 * @return 菜单项显示文本
 */
const wchar_t* CDeepSeekDeskBand::GetCommandName(int command_index)
{
    if (command_index == 0)
        return L"DeepSeek余额助手";
    return nullptr;
}

/**
 * @brief 执行插件命令
 * @param command_index 命令索引
 * @param hWnd 主窗口句柄
 * @details 点击右键菜单项时打开设置对话框
 */
void CDeepSeekDeskBand::OnPluginCommand(int command_index, void* hWnd, void* para)
{
    if (command_index == 0)
        ShowOptionsDialog(hWnd);
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
