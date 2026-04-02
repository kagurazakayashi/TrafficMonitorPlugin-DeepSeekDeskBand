/************************************************************************//**
 * @file    ConfigEncrypt.cpp
 * @brief   配置文件 AES-256-CBC 加密模块实现
 * @details 使用 Windows BCrypt (CNG) API：
 *          1. 将 DSDB_ENCRYPTION_KEY 经 SHA-256 派生为 32 字节 AES 密钥
 *          2. 生成随机 16 字节 IV
 *          3. 序列化配置 → PKCS#7 填充 → AES-256-CBC 加密
 *          4. 输出格式：[16B IV][密文]
 ****************************************************************************/
#include "ConfigEncrypt.h"
#include "framework.h"
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>
#pragma comment(lib, "bcrypt.lib")

// ============================================================
// NTSTATUS 成功检测宏（bcrypt.h 中 BCRYPT_SUCCESS 等价）
// ============================================================
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

// ============================================================
// 内部常量
// ============================================================

/** @brief AES-256 密钥长度（字节） */
static const ULONG AES256_KEY_SIZE = 32;

/** @brief AES 块大小 / IV 长度（字节） */
static const ULONG AES_BLOCK_SIZE = 16;

/** @brief SHA-256 输出长度（字节） */
static const ULONG SHA256_HASH_SIZE = 32;

// ============================================================
// SHA-256 密钥派生
// ============================================================

/**
 * @brief 对宽字符串进行 SHA-256 哈希，导出 AES 密钥
 * @param keyStr    输入密钥字符串（以 null 结尾）
 * @param keyOut    输出 32 字节哈希值
 * @return 成功返回 true
 */
static bool DeriveAesKey(const wchar_t* keyStr, uint8_t keyOut[AES256_KEY_SIZE])
{
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    bool success = false;
    size_t keyLen = wcslen(keyStr);
    int utf8Len = 0;
    std::vector<char> utf8Key;

    if (!NT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM,
            nullptr, 0)))
        return false;

    if (!NT_SUCCESS(BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0)))
        goto cleanup;

    // 将宽字符串转换为 UTF-8 字节再哈希（确保跨平台一致性）
    utf8Len = WideCharToMultiByte(CP_UTF8, 0, keyStr, static_cast<int>(keyLen),
        nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0)
        goto cleanup;

    utf8Key.resize(utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, keyStr, static_cast<int>(keyLen),
        utf8Key.data(), utf8Len, nullptr, nullptr);

    if (!NT_SUCCESS(BCryptHashData(hHash, reinterpret_cast<PUCHAR>(utf8Key.data()),
            static_cast<ULONG>(utf8Len), 0)))
        goto cleanup;

    if (!NT_SUCCESS(BCryptFinishHash(hHash, keyOut, SHA256_HASH_SIZE, 0)))
        goto cleanup;

    success = true;

cleanup:
    if (hHash)
        BCryptDestroyHash(hHash);
    if (hAlg)
        BCryptCloseAlgorithmProvider(hAlg, 0);
    return success;
}

// ============================================================
// 配置序列化 / 反序列化（明文二进制格式）
// ============================================================

/**
 * @brief 将 ConfigBlob 序列化为字节数组
 * @details 格式：
 *          [4B magic: DSDB_CONFIG_MAGIC]
 *          [4B version: 1]
 *          [4B updateInterval]
 *          [4B requestTimeout]
 *          [4B apiKeyByteLen]（包含 null 终止符的字节数）
 *          [apiKeyByteLen 字节]（UTF-16LE）
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
    push32(1);  // version
    push32(config.updateInterval);
    push32(config.requestTimeout);

    // API 密钥：包含 null 终止符的字节数
    int32_t apiKeyByteLen = static_cast<int32_t>((config.apiKey.size() + 1) * sizeof(wchar_t));
    push32(apiKeyByteLen);

    const uint8_t* keyBytes = reinterpret_cast<const uint8_t*>(config.apiKey.c_str());
    data.insert(data.end(), keyBytes, keyBytes + apiKeyByteLen);

    return data;
}

/**
 * @brief 从字节数组反序列化为 ConfigBlob
 * @param data   序列化数据
 * @param config 输出配置
 * @return 成功返回 true
 */
static bool DeserializeConfig(const std::vector<uint8_t>& data, ConfigBlob& config)
{
    if (data.size() < 20)   // 最小：magic(4) + version(4) + interval(4) + timeout(4) + keyLen(4)
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
    if (read32(4) != 1)     // 仅支持 version 1
        return false;

    config.updateInterval = read32(8);
    config.requestTimeout = read32(12);
    int32_t apiKeyByteLen = read32(16);

    if (apiKeyByteLen < 2 || apiKeyByteLen % 2 != 0)
        return false;
    if (data.size() < static_cast<size_t>(20) + apiKeyByteLen)
        return false;

    int32_t wcharCount = apiKeyByteLen / static_cast<int32_t>(sizeof(wchar_t));
    const wchar_t* keyPtr = reinterpret_cast<const wchar_t*>(data.data() + 20);
    config.apiKey.assign(keyPtr, wcharCount);

    // 确保以 null 结尾
    if (!config.apiKey.empty() && config.apiKey.back() != L'\0')
        config.apiKey.push_back(L'\0');

    return true;
}

// ============================================================
// BCrypt RAII 辅助类
// ============================================================

/** @brief BCrypt 算法句柄 RAII 包装 */
class BcryptAlgGuard
{
public:
    BcryptAlgGuard() : m_handle(nullptr) {}
    ~BcryptAlgGuard() { if (m_handle) BCryptCloseAlgorithmProvider(m_handle, 0); }
    BCRYPT_ALG_HANDLE* operator&() { return &m_handle; }
    BCRYPT_ALG_HANDLE get() const { return m_handle; }
private:
    BCRYPT_ALG_HANDLE m_handle;
};

/** @brief BCrypt 密钥句柄 RAII 包装 */
class BcryptKeyGuard
{
public:
    BcryptKeyGuard() : m_handle(nullptr) {}
    ~BcryptKeyGuard() { if (m_handle) BCryptDestroyKey(m_handle); }
    BCRYPT_KEY_HANDLE* operator&() { return &m_handle; }
    BCRYPT_KEY_HANDLE get() const { return m_handle; }
private:
    BCRYPT_KEY_HANDLE m_handle;
};

// ============================================================
// AES-256-CBC 加密 / 解密
// ============================================================

/**
 * @brief 加密明文数据
 * @param key        32 字节 AES 密钥
 * @param iv         16 字节初始化向量（输出，调用者确保可写）
 * @param plaintext  明文数据
 * @param ciphertext 密文数据（输出）
 * @return 成功返回 true
 */
static bool AesEncrypt(const uint8_t key[AES256_KEY_SIZE],
    uint8_t iv[AES_BLOCK_SIZE],
    const std::vector<uint8_t>& plaintext,
    std::vector<uint8_t>& ciphertext)
{
    BcryptAlgGuard hAlg;
    BcryptKeyGuard hKey;

    // 1. 打开 AES 算法句柄并设为 CBC 模式
    if (!NT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0)))
        return false;

    if (!NT_SUCCESS(BCryptSetProperty(hAlg.get(), BCRYPT_CHAINING_MODE,
            reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_CBC)),
            static_cast<ULONG>(sizeof(BCRYPT_CHAIN_MODE_CBC)), 0)))
        return false;

    // 2. 生成对称密钥
    if (!NT_SUCCESS(BCryptGenerateSymmetricKey(hAlg.get(), &hKey, nullptr, 0,
            const_cast<PUCHAR>(key), AES256_KEY_SIZE, 0)))
        return false;

    // 3. 生成随机 IV
    if (!NT_SUCCESS(BCryptGenRandom(nullptr, iv, AES_BLOCK_SIZE, BCRYPT_USE_SYSTEM_PREFERRED_RNG)))
        return false;

    // 4. 第一次调用获取密文缓冲区大小
    ULONG ciphertextLen = 0;
    NTSTATUS status = BCryptEncrypt(hKey.get(),
        const_cast<PUCHAR>(plaintext.data()),
        static_cast<ULONG>(plaintext.size()),
        nullptr, iv, AES_BLOCK_SIZE,
        nullptr, 0, &ciphertextLen,
        BCRYPT_BLOCK_PADDING);  // 自动 PKCS#7 填充

    if (!NT_SUCCESS(status))
        return false;

    // 5. 第二次调用执行加密
    ciphertext.resize(ciphertextLen);
    ULONG outLen = 0;
    status = BCryptEncrypt(hKey.get(),
        const_cast<PUCHAR>(plaintext.data()),
        static_cast<ULONG>(plaintext.size()),
        nullptr, iv, AES_BLOCK_SIZE,
        ciphertext.data(), ciphertextLen, &outLen,
        BCRYPT_BLOCK_PADDING);

    if (!NT_SUCCESS(status))
        return false;

    ciphertext.resize(outLen);
    return true;
}

/**
 * @brief 解密密文数据
 * @param key        32 字节 AES 密钥
 * @param iv         16 字节初始化向量（调用者确保可写）
 * @param ciphertext 密文数据
 * @param plaintext  明文数据（输出）
 * @return 成功返回 true
 */
static bool AesDecrypt(const uint8_t key[AES256_KEY_SIZE],
    uint8_t iv[AES_BLOCK_SIZE],
    const std::vector<uint8_t>& ciphertext,
    std::vector<uint8_t>& plaintext)
{
    if (ciphertext.empty())
        return false;

    BcryptAlgGuard hAlg;
    BcryptKeyGuard hKey;

    // 1. 打开 AES 算法句柄并设为 CBC 模式
    if (!NT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0)))
        return false;

    if (!NT_SUCCESS(BCryptSetProperty(hAlg.get(), BCRYPT_CHAINING_MODE,
            reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_CBC)),
            static_cast<ULONG>(sizeof(BCRYPT_CHAIN_MODE_CBC)), 0)))
        return false;

    // 2. 生成对称密钥
    if (!NT_SUCCESS(BCryptGenerateSymmetricKey(hAlg.get(), &hKey, nullptr, 0,
            const_cast<PUCHAR>(key), AES256_KEY_SIZE, 0)))
        return false;

    // 3. 拷贝 IV（BCryptDecrypt 在 CBC 模式下会修改 IV，两次调用不可共用）
    uint8_t ivCopy[AES_BLOCK_SIZE];
    memcpy(ivCopy, iv, AES_BLOCK_SIZE);

    ULONG plaintextLen = 0;
    NTSTATUS status = BCryptDecrypt(hKey.get(),
        const_cast<PUCHAR>(ciphertext.data()),
        static_cast<ULONG>(ciphertext.size()),
        nullptr, ivCopy, AES_BLOCK_SIZE,
        nullptr, 0, &plaintextLen,
        BCRYPT_BLOCK_PADDING);

    if (!NT_SUCCESS(status))
        return false;

    // 4. 第二次调用执行解密（使用原始 IV）
    plaintext.resize(plaintextLen);
    ULONG outLen = 0;
    status = BCryptDecrypt(hKey.get(),
        const_cast<PUCHAR>(ciphertext.data()),
        static_cast<ULONG>(ciphertext.size()),
        nullptr, iv, AES_BLOCK_SIZE,
        plaintext.data(), plaintextLen, &outLen,
        BCRYPT_BLOCK_PADDING);

    if (!NT_SUCCESS(status))
        return false;

    plaintext.resize(outLen);
    return true;
}

// ============================================================
// 对外接口实现
// ============================================================

bool ConfigEncrypt(const ConfigBlob& config, std::vector<uint8_t>& outEncrypted)
{
    // 1. 派生 AES 密钥
    uint8_t aesKey[AES256_KEY_SIZE];
    if (!DeriveAesKey(DSDB_ENCRYPTION_KEY, aesKey))
        return false;

    // 2. 序列化明文
    std::vector<uint8_t> plaintext = SerializeConfig(config);
    if (plaintext.empty())
        return false;

    // 3. AES 加密
    uint8_t iv[AES_BLOCK_SIZE];
    std::vector<uint8_t> ciphertext;
    if (!AesEncrypt(aesKey, iv, plaintext, ciphertext))
        return false;

    // 4. 组装输出：[IV][密文]
    outEncrypted.clear();
    outEncrypted.insert(outEncrypted.end(), iv, iv + AES_BLOCK_SIZE);
    outEncrypted.insert(outEncrypted.end(), ciphertext.begin(), ciphertext.end());

    // 5. 从栈中清除密钥
    SecureZeroMemory(aesKey, sizeof(aesKey));
    SecureZeroMemory(iv, sizeof(iv));

    return true;
}

bool ConfigDecrypt(const std::vector<uint8_t>& encrypted, ConfigBlob& outConfig)
{
    // 最少需要 IV (16B) + 1 个密文块 (16B)
    if (encrypted.size() < AES_BLOCK_SIZE * 2)
        return false;

    // 1. 派生 AES 密钥
    uint8_t aesKey[AES256_KEY_SIZE];
    if (!DeriveAesKey(DSDB_ENCRYPTION_KEY, aesKey))
        return false;

    // 2. 分离 IV 和密文
    uint8_t iv[AES_BLOCK_SIZE];
    memcpy(iv, encrypted.data(), AES_BLOCK_SIZE);

    std::vector<uint8_t> ciphertext(
        encrypted.begin() + AES_BLOCK_SIZE,
        encrypted.end());

    // 3. AES 解密
    std::vector<uint8_t> plaintext;
    if (!AesDecrypt(aesKey, iv, ciphertext, plaintext))
    {
        SecureZeroMemory(aesKey, sizeof(aesKey));
        return false;
    }

    SecureZeroMemory(aesKey, sizeof(aesKey));

    // 4. 反序列化
    if (!DeserializeConfig(plaintext, outConfig))
        return false;

    return true;
}
