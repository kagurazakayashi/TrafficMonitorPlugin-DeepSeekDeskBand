/************************************************************************//**
 * @file    ConfigEncrypt.h
 * @brief   配置文件加密模块声明
 * @details 使用 Windows DPAPI (CryptProtectData / CryptUnprotectData)，
 *          数据仅限当前 Windows 用户解密，无需管理密钥。
 ****************************************************************************/
#pragma once
#include "Config.h"
#include <vector>
#include <string>
#include <cstdint>

// ============================================================
// 配置数据结构（明文）
// ============================================================

/************************************************************************//**
 * @struct ConfigBlob
 * @brief  配置文件明文数据
 ****************************************************************************/
struct ConfigBlob
{
    int32_t     updateInterval;          ///< 更新间隔（秒）
    int32_t     requestTimeout;          ///< 请求超时（秒）
    int32_t     maxHistoryCount;         ///< 历史记录最大保留数量
    std::wstring apiKey;                 ///< DeepSeek API 密钥
};

// ============================================================
// 加密 / 解密接口
// ============================================================

/************************************************************************//**
 * @brief  使用 DPAPI 加密配置数据
 * @param  config      要加密的明文配置
 * @param  outEncrypted 输出：加密后的二进制数据
 * @return 成功返回 true
 ****************************************************************************/
bool ConfigEncrypt(const ConfigBlob& config, std::vector<uint8_t>& outEncrypted);

/************************************************************************//**
 * @brief  使用 DPAPI 解密配置数据
 * @param  encrypted   加密数据
 * @param  outConfig   输出：解密后的配置明文
 * @return 成功返回 true；失败（不同用户、数据损坏等）返回 false
 ****************************************************************************/
bool ConfigDecrypt(const std::vector<uint8_t>& encrypted, ConfigBlob& outConfig);
