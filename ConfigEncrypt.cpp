/************************************************************************//**
 * @file    ConfigEncrypt.cpp
 * @brief   配置文件加密模块实现
 * @details 使用 Windows DPAPI (CryptProtectData / CryptUnprotectData)
 *          数据仅限当前 Windows 用户解密，无需管理密钥。
 ****************************************************************************/
#include "ConfigEncrypt.h"
#include "Logger.h"
#include "framework.h"
#include <windows.h>
#include <dpapi.h>
#include <vector>
#include <string>
#pragma comment(lib, "crypt32.lib")

// ============================================================
// 配置序列化 / 反序列化（明文二进制格式）
// ============================================================

/**
 * @brief 将 ConfigBlob 序列化为字节数组
 */
static std::vector<uint8_t> SerializeConfig(const ConfigBlob& config)
{
    std::vector<uint8_t> data;
    auto push32 = [&data](int32_t val) {
        data.push_back(static_cast<uint8_t>(val & 0xFF));
        data.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
        data.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
    };

    push32(DSDB_CONFIG_MAGIC);
    push32(3);  // version（版本 3：新增 language 字段）
    push32(config.updateInterval);
    push32(config.requestTimeout);
    push32(config.maxHistoryCount);
    push32(config.language);

    int32_t apiKeyByteLen = static_cast<int32_t>((config.apiKey.size() + 1) * sizeof(wchar_t));
    push32(apiKeyByteLen);

    const uint8_t* keyBytes = reinterpret_cast<const uint8_t*>(config.apiKey.c_str());
    data.insert(data.end(), keyBytes, keyBytes + apiKeyByteLen);

    return data;
}

/**
 * @brief 从字节数组反序列化为 ConfigBlob
 */
static bool DeserializeConfig(const std::vector<uint8_t>& data, ConfigBlob& config)
{
    // v2 最小 24 字节, v3 最小 28 字节
    if (data.size() < 24)
        return false;

    auto read32 = [&data](size_t offset) -> int32_t {
        return static_cast<int32_t>(
            data[offset] |
            (data[offset + 1] << 8) |
            (data[offset + 2] << 16) |
            (data[offset + 3] << 24));
    };

    if (read32(0) != DSDB_CONFIG_MAGIC)
        return false;

    int32_t version = read32(4);
    if (version < 2 || version > 3)
        return false;

    config.updateInterval = read32(8);
    config.requestTimeout = read32(12);
    config.maxHistoryCount = read32(16);
    config.language = (version >= 3) ? read32(20) : 0;   // v2 兼容：默认自动
    int32_t apiKeyByteLen = (version >= 3) ? read32(24) : read32(20);

    if (apiKeyByteLen < 2 || apiKeyByteLen % 2 != 0)
        return false;
    if (data.size() < static_cast<size_t>((version >= 3 ? 28 : 24)) + apiKeyByteLen)
        return false;

    int32_t wcharCount = apiKeyByteLen / static_cast<int32_t>(sizeof(wchar_t));
    const wchar_t* keyPtr = reinterpret_cast<const wchar_t*>(
        data.data() + (version >= 3 ? 28 : 24));
    config.apiKey.assign(keyPtr, wcharCount);

    if (!config.apiKey.empty() && config.apiKey.back() != L'\0')
        config.apiKey.push_back(L'\0');

    return true;
}

// ============================================================
// DPAPI 加密 / 解密
// ============================================================

bool ConfigEncrypt(const ConfigBlob& config, std::vector<uint8_t>& outEncrypted)
{
    std::vector<uint8_t> plaintext = SerializeConfig(config);
    if (plaintext.empty())
    {
        Logger_Log(L"ConfigEncrypt: SerializeConfig 返回空");
        return false;
    }
    Logger_Log(L"ConfigEncrypt: 明文大小=%zu 字节", plaintext.size());

    DATA_BLOB dataIn;
    dataIn.pbData = plaintext.data();
    dataIn.cbData = static_cast<DWORD>(plaintext.size());

    DATA_BLOB dataOut = {};
    if (!CryptProtectData(&dataIn, nullptr, nullptr, nullptr, nullptr,
            CRYPTPROTECT_LOCAL_MACHINE, &dataOut))
    {
        Logger_Log(L"ConfigEncrypt: CryptProtectData 失败 err=%lu", GetLastError());
        return false;
    }

    outEncrypted.assign(dataOut.pbData, dataOut.pbData + dataOut.cbData);
    LocalFree(dataOut.pbData);

    Logger_Log(L"ConfigEncrypt: 加密后=%zu 字节", outEncrypted.size());
    return true;
}

bool ConfigDecrypt(const std::vector<uint8_t>& encrypted, ConfigBlob& outConfig)
{
    Logger_Log(L"ConfigDecrypt: 输入大小=%zu 字节", encrypted.size());

    if (encrypted.empty())
    {
        Logger_Log(L"ConfigDecrypt: 输入为空");
        return false;
    }

    DATA_BLOB dataIn;
    dataIn.pbData = const_cast<BYTE*>(encrypted.data());
    dataIn.cbData = static_cast<DWORD>(encrypted.size());

    DATA_BLOB dataOut = {};
    if (!CryptUnprotectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut))
    {
        Logger_Log(L"ConfigDecrypt: CryptUnprotectData 失败 err=%lu", GetLastError());
        return false;
    }

    std::vector<uint8_t> plaintext(dataOut.pbData, dataOut.pbData + dataOut.cbData);
    LocalFree(dataOut.pbData);

    Logger_Log(L"ConfigDecrypt: 解密后=%zu 字节", plaintext.size());

    if (!DeserializeConfig(plaintext, outConfig))
    {
        Logger_Log(L"ConfigDecrypt: DeserializeConfig 失败");
        return false;
    }

    Logger_Log(L"ConfigDecrypt: 反序列化成功 interval=%d timeout=%d maxHistory=%d language=%d apiKey=\"%s\"",
        outConfig.updateInterval, outConfig.requestTimeout, outConfig.maxHistoryCount,
        outConfig.language, outConfig.apiKey.c_str());
    return true;
}
