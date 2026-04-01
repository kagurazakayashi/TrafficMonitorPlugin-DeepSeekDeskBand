/************************************************************************//**
 * @file    Config.h
 * @brief   DeepSeekDeskBand 集中配置宏
 * @details 所有可配置的固定值均在此文件中定义为宏，方便统一修改。
 *          修改此文件后需重新编译整个插件。
 ****************************************************************************/
#pragma once

// ============================================================
// API 端点配置
// ============================================================

/** @brief DeepSeek API 服务器主机名 */
#define DSDB_API_HOST               L"api.deepseek.com"

/** @brief DeepSeek API HTTPS 端口号 */
#define DSDB_API_PORT               443

/** @brief 余额查询 API 路径 */
#define DSDB_API_BALANCE_PATH       L"/user/balance"

/** @brief HTTP Authorization 头前缀 */
#define DSDB_AUTH_PREFIX            L"Authorization: Bearer "

/** @brief HTTP 请求 User-Agent */
#define DSDB_USER_AGENT             L"DeepSeekDeskBand/1.0"

// ============================================================
// API 密钥格式验证规则
// ============================================================

/** @brief API 密钥前缀 */
#define DSDB_APIKEY_PREFIX          L"sk-"

/** @brief API 密钥前缀之后的字符数 */
#define DSDB_APIKEY_SUFFIX_LEN      32

// ============================================================
// 配置文件相关
// ============================================================

/** @brief 配置文件名（位于插件配置目录下） */
#define DSDB_CONFIG_FILENAME        L"DeepSeekDeskBand.dat"

/** @brief 配置文件中的节名（保留用于未来兼容） */
#define DSDB_CONFIG_SECTION         L"Settings"

/** @brief 配置键：API 密钥 */
#define DSDB_CONFIG_KEY_APIKEY      L"DeepSeekAPIKey"

/** @brief 配置键：更新间隔 */
#define DSDB_CONFIG_KEY_INTERVAL    L"UpdateInterval"

/** @brief 配置键：请求超时 */
#define DSDB_CONFIG_KEY_TIMEOUT     L"RequestTimeout"

// ============================================================
// 默认值与范围
// ============================================================

/** @brief 默认更新间隔（秒） */
#define DSDB_DEFAULT_INTERVAL       60

/** @brief 更新间隔最小值（秒） */
#define DSDB_INTERVAL_MIN           1

/** @brief 更新间隔最大值（秒），约等于 1 年 */
#define DSDB_INTERVAL_MAX           31536000

/** @brief 默认请求超时（秒） */
#define DSDB_DEFAULT_TIMEOUT        10

/** @brief 请求超时最小值（秒） */
#define DSDB_TIMEOUT_MIN            3

/** @brief 请求超时最大值（秒） */
#define DSDB_TIMEOUT_MAX            60

// ============================================================
// 缓冲区大小
// ============================================================

/** @brief API 密钥字符串缓冲区大小（宽字符数） */
#define DSDB_BUF_APIKEY             128

/** @brief 数值/间隔/超时输入框字符串缓冲区大小 */
#define DSDB_BUF_NUMBER             16

/** @brief 显示数值文本缓冲区大小 */
#define DSDB_BUF_VALUE              64

/** @brief HTTP 响应原始数据读取缓冲区大小（字节） */
#define DSDB_BUF_HTTP_READ          4096

/** @brief 提示消息对话框格式化缓冲区大小 */
#define DSDB_BUF_MESSAGE            512

/** @brief HTTP 状态码错误字符串缓冲区大小 */
#define DSDB_BUF_STATUS             128

// ============================================================
// 插件信息（版本号等）
// ============================================================

/** @brief 插件版本号字符串 */
#define DSDB_VERSION                L"1.0"

/** @brief 插件名称 */
#define DSDB_NAME                   L"DeepSeekDeskBand"

/** @brief 插件描述 */
#define DSDB_DESCRIPTION            L"DeepSeek 桌面助手插件"

/** @brief 插件作者 */
#define DSDB_AUTHOR                 L"yashi"

// ============================================================
// AES-256 加密密钥（编译时常量，126 字符，不超 CHAR_MAX）
// ============================================================

/** @brief 配置文件 AES-256 加密密钥（明文，编译后嵌入二进制） */
#define DSDB_ENCRYPTION_KEY \
    L"Bx9$kM3#pL7@vR2&qW5!nY8~zC1^bF4*dG6)hJ9(tK2=sN5+uP8:oQ3?wT7!lV1@" \
    L"cX4$eA6#jM8^bR5&yU9~hI2/aO7*mD3#qG1:xK6!pN0$sL4@wV8&tR2~bY7^eA3" \
    L"*cF1%jI5"

/** @brief 配置文件二进制魔数标识 */
#define DSDB_CONFIG_MAGIC           0x44534442  // "DSDB"
