/************************************************************************//**
 * @file    HttpClient.h
 * @brief   HTTP 客户端工具声明
 * @details 基于 WinHTTP 的网络请求模块，封装 DeepSeek API 余额查询功能。
 *          本模块不依赖任何第三方库，可在插件各处复用。
 ****************************************************************************/
#pragma once
#include <string>

/************************************************************************//**
 * @struct ApiTestResult
 * @brief  API 测试结果结构体
 * @details 包含余额查询的所有字段，以及错误信息。
 *          success 为 true 时，余额字段有效；
 *          success 为 false 时，error_message 和/或 http_status_code 包含错误详情。
 ****************************************************************************/
struct ApiTestResult
{
    bool        success;                ///< API 密钥是否有效
    bool        is_available;           ///< 账户是否可用
    std::wstring currency;              ///< 币种（如 "CNY"）
    std::wstring total_balance;         ///< 总余额
    std::wstring granted_balance;       ///< 赠送余额
    std::wstring topped_up_balance;     ///< 充值余额
    std::wstring error_message;         ///< 错误消息（来自 JSON 或异常）
    int         http_status_code;       ///< HTTP 状态码（0 表示未收到 HTTP 响应）
};

/************************************************************************//**
 * @brief  测试 DeepSeek API 密钥有效性并查询余额
 * @param  apiKey         DeepSeek API 密钥（以 "sk-" 开头）
 * @param  timeoutSeconds 请求超时时间（秒），范围 3~60，默认 10
 * @return ApiTestResult 结构体，包含测试结果和余额信息
 ****************************************************************************/
ApiTestResult TestDeepSeekApi(const wchar_t* apiKey, int timeoutSeconds = 10);
