/************************************************************************//**
 * @file    HistoryManager.cpp
 * @brief   余额刷新历史记录管理模块实现
 * @details 实现 HistoryManager 类的全部功能：
 *          序列化/反序列化、DPAPI 加密/解密、增量文件 I/O、整体重写。
 ****************************************************************************/
#include "HistoryManager.h"
#include "Logger.h"
#include "framework.h"
#include <windows.h>
#include <dpapi.h>
#include <fstream>
#include <cstring>
#include <math.h>
#pragma comment(lib, "crypt32.lib")

// ============================================================
// HistoryManager 构造/析构
// ============================================================

HistoryManager::HistoryManager()
    : m_maxHistoryCount(DSDB_DEFAULT_HISTORY_COUNT)
{
    Logger_Debug(L"HistoryManager: 构造完成, maxHistoryCount=%d", m_maxHistoryCount);
}

HistoryManager::~HistoryManager()
{
    Logger_Debug(L"HistoryManager: 析构, 记录数=%zu", m_records.size());
}

// ============================================================
// 路径设置
// ============================================================

void HistoryManager::SetConfigDir(const std::wstring& configDir)
{
    m_configDir = configDir;
    Logger_Info(L"HistoryManager::SetConfigDir: \"%s\"", configDir.c_str());
}

std::wstring HistoryManager::GetFilePath() const
{
    if (m_configDir.empty())
    {
        Logger_Warn(L"HistoryManager::GetFilePath: configDir 为空");
        return L"";
    }
    std::wstring path = m_configDir + L"\\" DSDB_HISTORY_FILENAME;
    Logger_Debug(L"HistoryManager::GetFilePath: \"%s\"", path.c_str());
    return path;
}

void HistoryManager::SetMaxHistoryCount(int count)
{
    if (count < DSDB_HISTORY_COUNT_MIN)
        count = DSDB_HISTORY_COUNT_MIN;
    if (count > DSDB_HISTORY_COUNT_MAX)
        count = DSDB_HISTORY_COUNT_MAX;
    int oldCount = m_maxHistoryCount;
    m_maxHistoryCount = count;
    Logger_Info(L"HistoryManager::SetMaxHistoryCount: %d -> %d", oldCount, count);
}

// ============================================================
// 序列化 / 反序列化
// ============================================================

std::vector<uint8_t> HistoryManager::SerializeRecord(const HistoryRecord& rec)
{
    std::vector<uint8_t> data(49);
    size_t off = 0;
    memcpy(&data[off], &rec.timestamp, 8);          off += 8;
    data[off] = rec.is_available;                     off += 1;
    memcpy(&data[off], &rec.total_balance, 8);       off += 8;
    memcpy(&data[off], &rec.granted_balance, 8);     off += 8;
    memcpy(&data[off], &rec.topped_up_balance, 8);   off += 8;
    memcpy(&data[off], rec.currency, 16);             off += 16;  // wchar_t[8] = 16 bytes
    return data;
}

HistoryRecord HistoryManager::DeserializeRecord(const uint8_t* data, size_t size)
{
    HistoryRecord rec = {};
    if (size >= 49)
    {
        size_t off = 0;
        memcpy(&rec.timestamp, &data[off], 8);       off += 8;
        rec.is_available = data[off];                 off += 1;
        memcpy(&rec.total_balance, &data[off], 8);   off += 8;
        memcpy(&rec.granted_balance, &data[off], 8); off += 8;
        memcpy(&rec.topped_up_balance, &data[off], 8);off += 8;
        memcpy(rec.currency, &data[off], 16);         off += 16;
        rec.currency[7] = L'\0';  // 确保 null 终止
    }
    return rec;
}

// ============================================================
// DPAPI 加密 / 解密
// ============================================================

bool HistoryManager::EncryptRecord(const HistoryRecord& rec, std::vector<uint8_t>& outEncrypted)
{
    std::vector<uint8_t> plain = SerializeRecord(rec);

    DATA_BLOB dataIn;
    dataIn.pbData = plain.data();
    dataIn.cbData = static_cast<DWORD>(plain.size());

    DATA_BLOB dataOut = {};
    if (!CryptProtectData(&dataIn, nullptr, nullptr, nullptr, nullptr,
            CRYPTPROTECT_LOCAL_MACHINE, &dataOut))
    {
        DWORD err = GetLastError();
        Logger_Error(L"HistoryManager::EncryptRecord: CryptProtectData 失败, err=%lu", err);
        return false;
    }

    outEncrypted.assign(dataOut.pbData, dataOut.pbData + dataOut.cbData);
    LocalFree(dataOut.pbData);

    Logger_Debug(L"HistoryManager::EncryptRecord: 加密后 %zu 字节", outEncrypted.size());
    return true;
}

bool HistoryManager::DecryptRecord(const uint8_t* encryptedData, size_t encryptedSize,
                                    HistoryRecord& outRecord)
{
    DATA_BLOB dataIn;
    dataIn.pbData = const_cast<BYTE*>(encryptedData);
    dataIn.cbData = static_cast<DWORD>(encryptedSize);

    DATA_BLOB dataOut = {};
    if (!CryptUnprotectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut))
    {
        DWORD err = GetLastError();
        Logger_Error(L"HistoryManager::DecryptRecord: CryptUnprotectData 失败, err=%lu", err);
        return false;
    }

    if (dataOut.cbData < 49)
    {
        Logger_Error(L"HistoryManager::DecryptRecord: 解密后数据不足 49 字节 (实为 %lu)", dataOut.cbData);
        LocalFree(dataOut.pbData);
        return false;
    }

    outRecord = DeserializeRecord(dataOut.pbData, dataOut.cbData);
    LocalFree(dataOut.pbData);
    return true;
}

// ============================================================
// ApiTestResult 转 HistoryRecord
// ============================================================

HistoryRecord HistoryManager::ResultToRecord(const ApiTestResult& result)
{
    HistoryRecord rec = {};

    // 构造高精度时间戳（毫秒级 Unix 时间）
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    // FILETIME 起始于 1601-01-01，Unix 起始于 1970-01-01，差值 116444736000000000 * 100ns
    rec.timestamp = static_cast<int64_t>((uli.QuadPart - 116444736000000000ULL) / 10000ULL);

    rec.is_available = result.is_available ? 1 : 0;
    rec.total_balance = _wtof(result.total_balance.c_str());
    rec.granted_balance = _wtof(result.granted_balance.c_str());
    rec.topped_up_balance = _wtof(result.topped_up_balance.c_str());
    wcsncpy_s(rec.currency, result.currency.c_str(), 7);
    rec.currency[7] = L'\0';

    return rec;
}

bool HistoryManager::HasBalanceChanged(const HistoryRecord& a, const HistoryRecord& b)
{
    return (fabs(a.total_balance   - b.total_balance)   >= 0.001 ||
            fabs(a.granted_balance - b.granted_balance) >= 0.001 ||
            fabs(a.topped_up_balance - b.topped_up_balance) >= 0.001);
}

// ============================================================
// 文件 I/O：加载
// ============================================================

void HistoryManager::Load()
{
    std::wstring path = GetFilePath();
    if (path.empty())
        return;

    m_records.clear();

    // 检查文件是否存在
    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        Logger_Info(L"HistoryManager::Load: 文件不存在，初始化为空 \"%s\"", path.c_str());
        return;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        Logger_Error(L"HistoryManager::Load: 无法打开文件 \"%s\"", path.c_str());
        return;
    }

    // 读取文件头（12 字节）
    uint8_t header[12];
    if (!file.read(reinterpret_cast<char*>(header), 12))
    {
        Logger_Error(L"HistoryManager::Load: 文件头读取失败");
        return;
    }

    uint32_t magic   = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
    uint32_t version = header[4] | (header[5] << 8) | (header[6] << 16) | (header[7] << 24);
    uint32_t count   = header[8] | (header[9] << 8) | (header[10] << 16) | (header[11] << 24);

    if (magic != DSDB_HISTORY_MAGIC || version != 1)
    {
        Logger_Warn(L"HistoryManager::Load: 魔数/版本不匹配 magic=0x%08X version=%u", magic, version);
        return;
    }

    Logger_Info(L"HistoryManager::Load: 文件中有 %u 条记录", count);

    std::vector<HistoryRecord> allRecords;
    allRecords.reserve(count);

    for (uint32_t i = 0; i < count; i++)
    {
        // 读取加密数据长度（4 字节）
        uint8_t sizeBuf[4];
        if (!file.read(reinterpret_cast<char*>(sizeBuf), 4))
        {
            Logger_Warn(L"HistoryManager::Load: 读取第 %u 条记录长度失败", i);
            break;
        }
        uint32_t encSize = sizeBuf[0] | (sizeBuf[1] << 8) | (sizeBuf[2] << 16) | (sizeBuf[3] << 24);

        if (encSize == 0 || encSize > 4096)
        {
            Logger_Warn(L"HistoryManager::Load: 第 %u 条记录长度异常 encSize=%u，跳过", i, encSize);
            break;
        }

        // 读取加密数据
        std::vector<uint8_t> encData(encSize);
        if (!file.read(reinterpret_cast<char*>(encData.data()), encSize))
        {
            Logger_Error(L"HistoryManager::Load: 读取第 %u 条记录数据失败", i);
            break;
        }

        // 逐条解密
        HistoryRecord rec;
        if (DecryptRecord(encData.data(), encSize, rec))
            allRecords.push_back(rec);
        else
            Logger_Warn(L"HistoryManager::Load: 第 %u 条记录解密失败，跳过", i);
    }

    file.close();
    Logger_Info(L"HistoryManager::Load: 成功解密 %zu 条记录", allRecords.size());

    // 只保留最近 maxHistoryCount 条
    if (m_maxHistoryCount > 0 && allRecords.size() > static_cast<size_t>(m_maxHistoryCount))
    {
        size_t trim = allRecords.size() - m_maxHistoryCount;
        m_records.assign(allRecords.begin() + trim, allRecords.end());
        Logger_Info(L"HistoryManager::Load: 截断 %zu 条旧记录，保留 %zu 条", trim, m_records.size());
    }
    else if (m_maxHistoryCount == 0)
    {
        Logger_Info(L"HistoryManager::Load: maxHistoryCount=0，不保留历史记录");
    }
    else
    {
        m_records = std::move(allRecords);
    }
}

// ============================================================
// 文件 I/O：保存（全量 / 增量）
// ============================================================

void HistoryManager::Save(bool fullRewrite)
{
    std::wstring path = GetFilePath();
    if (path.empty())
        return;

    if (fullRewrite)
    {
        // ---- 整体加密重写 ----
        Logger_Info(L"HistoryManager::Save: 整体重写 %zu 条记录", m_records.size());

        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            Logger_Error(L"HistoryManager::Save: 无法创建文件 \"%s\"", path.c_str());
            return;
        }

        // 写入文件头（12 字节，明文）
        uint8_t header[12];
        header[0] = static_cast<uint8_t>(DSDB_HISTORY_MAGIC & 0xFF);
        header[1] = static_cast<uint8_t>((DSDB_HISTORY_MAGIC >> 8) & 0xFF);
        header[2] = static_cast<uint8_t>((DSDB_HISTORY_MAGIC >> 16) & 0xFF);
        header[3] = static_cast<uint8_t>((DSDB_HISTORY_MAGIC >> 24) & 0xFF);
        header[4] = 1; header[5] = 0; header[6] = 0; header[7] = 0;  // version=1
        uint32_t count = static_cast<uint32_t>(m_records.size());
        header[8]  = static_cast<uint8_t>(count & 0xFF);
        header[9]  = static_cast<uint8_t>((count >> 8) & 0xFF);
        header[10] = static_cast<uint8_t>((count >> 16) & 0xFF);
        header[11] = static_cast<uint8_t>((count >> 24) & 0xFF);
        file.write(reinterpret_cast<const char*>(header), 12);

        // 逐条加密写入
        for (const auto& rec : m_records)
        {
            std::vector<uint8_t> enc;
            if (!EncryptRecord(rec, enc))
            {
                Logger_Error(L"HistoryManager::Save: 加密某条记录失败，中止写入");
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
        Logger_Info(L"HistoryManager::Save: 整体重写完成");
    }
    else
    {
        // ---- 增量追加：仅追加最后一条记录 ----
        if (m_records.empty())
        {
            Logger_Debug(L"HistoryManager::Save: 增量模式，记录为空，跳过");
            return;
        }

        const HistoryRecord& lastRec = m_records.back();
        Logger_Debug(L"HistoryManager::Save: 增量追加一条记录");

        // 加密最后一条
        std::vector<uint8_t> enc;
        if (!EncryptRecord(lastRec, enc))
        {
            Logger_Error(L"HistoryManager::Save: 加密失败");
            return;
        }

        // 打开文件（读写模式，使用 Windows API 精确控制）
        HANDLE hFile = CreateFileW(path.c_str(),
            GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD err = GetLastError();
            if (err == ERROR_FILE_NOT_FOUND)
            {
                Logger_Info(L"HistoryManager::Save: 文件不存在，回退到整体重写");
                Save(true);
                return;
            }
            Logger_Error(L"HistoryManager::Save: 无法打开文件, err=%lu", err);
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
                Logger_Warn(L"HistoryManager::Save: 现有文件魔数不匹配，回退到整体重写");
                CloseHandle(hFile);
                Save(true);
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
            Logger_Error(L"HistoryManager::Save: 写入长度失败");
            CloseHandle(hFile);
            return;
        }
        WriteFile(hFile, enc.data(), encSize, &bytesWritten, nullptr);
        if (bytesWritten != encSize)
        {
            Logger_Error(L"HistoryManager::Save: 写入加密数据失败");
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

        Logger_Info(L"HistoryManager::Save: 增量追加完成, newCount=%u", newCount);
    }
}

// ============================================================
// 追加记录
// ============================================================

void HistoryManager::Append(const ApiTestResult& result)
{
    if (m_maxHistoryCount == 0)
    {
        Logger_Debug(L"HistoryManager::Append: maxHistoryCount=0，不记录");
        return;
    }

    // 构造新记录
    HistoryRecord newRec = ResultToRecord(result);

    // 只有余额发生实际变动时才记录（差值 > 0.001）
    if (!m_records.empty())
    {
        if (!HasBalanceChanged(newRec, m_records.back()))
        {
            Logger_Debug(L"HistoryManager::Append: 余额未变动 (total=%.2f)，跳过记录",
                newRec.total_balance);
            return;
        }
    }

    // 存入内存
    m_records.push_back(newRec);

    // 超出上限则移除最旧的
    size_t before = m_records.size();
    while (m_records.size() > static_cast<size_t>(m_maxHistoryCount))
        m_records.erase(m_records.begin());

    Logger_Info(L"HistoryManager::Append: 记录 ts=%lld avail=%d total=%.2f %s (当前 %zu 条%s)",
        newRec.timestamp, newRec.is_available, newRec.total_balance,
        newRec.currency, m_records.size(),
        (before > m_records.size()) ? L", 已截断" : L"");

    // 增量写入文件
    Save(false);
}

// ============================================================
// 清空 / 裁剪
// ============================================================

void HistoryManager::Clear()
{
    size_t oldCount = m_records.size();
    m_records.clear();
    Logger_Info(L"HistoryManager::Clear: 已清除 %zu 条记录", oldCount);
    Save(true);  // 重写为空文件
}

void HistoryManager::TrimToMaxCount()
{
    size_t before = m_records.size();
    while (m_records.size() > static_cast<size_t>(m_maxHistoryCount))
        m_records.erase(m_records.begin());
    if (m_records.size() < before)
    {
        Logger_Info(L"HistoryManager::TrimToMaxCount: %zu -> %zu 条", before, m_records.size());
        Save(true);
    }
}
