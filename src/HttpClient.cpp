/************************************************************************//**
 * @file    HttpClient.cpp
 * @brief   HTTP 客户端工具实现
 * @details 使用 WinHTTP 发送 HTTPS GET 请求，查询 DeepSeek API 余额。
 *          使用 JsonParser 模块进行响应体 JSON 解析。
 ****************************************************************************/
#include "HttpClient.h"
#include "Config.h"
#include "JsonParser.h"
#include "Logger.h"
#include "framework.h"
#include <winhttp.h>
#include <vector>
#include <cwchar>
#include <cstring>
#pragma comment(lib, "winhttp.lib")

// ============================================================
// WinHTTP 请求与 JSON 解析
// ============================================================

/**
 * @brief 测试 DeepSeek API 密钥有效性并查询余额
 * @param apiKey DeepSeek API 密钥
 * @return ApiTestResult 结构体
 */
ApiTestResult TestDeepSeekApi(const wchar_t* apiKey, int timeoutSeconds)
{
    ApiTestResult result = {};
    result.success = false;
    result.is_available = false;
    result.http_status_code = 0;

    if (!apiKey || apiKey[0] == L'\0')
    {
        result.error_message = L"API 密钥为空";
        Logger_Warn(L"TestDeepSeekApi: API 密钥为空");
        return result;
    }

    Logger_Debug(L"TestDeepSeekApi: 开始, keyLen=%zu timeout=%ds", wcslen(apiKey), timeoutSeconds);

    // 钳制超时范围
    if (timeoutSeconds < DSDB_TIMEOUT_MIN)
        timeoutSeconds = DSDB_TIMEOUT_MIN;
    if (timeoutSeconds > DSDB_TIMEOUT_MAX)
        timeoutSeconds = DSDB_TIMEOUT_MAX;

    // ---- 构造 Authorization 头 ----
    std::wstring authHeader = DSDB_AUTH_PREFIX;
    authHeader += apiKey;
    authHeader += L"\r\n";

    // ---- 打开 WinHTTP 会话 ----
    HINTERNET hSession = WinHttpOpen(
        DSDB_USER_AGENT,                    // 用户代理
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,  // 使用系统默认代理
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);                                 // 同步模式

    if (!hSession)
    {
        DWORD err = GetLastError();
        result.error_message = L"无法初始化网络会话 (WinHttpOpen 失败)";
        Logger_Error(L"TestDeepSeekApi: WinHttpOpen 失败, err=%lu", err);
        return result;
    }

    // 设置超时（毫秒）
    int timeoutMs = timeoutSeconds * 1000;
    WinHttpSetTimeouts(hSession,
        timeoutMs,      // 解析超时
        timeoutMs,      // 连接超时
        timeoutMs,      // 发送超时
        timeoutMs);     // 接收超时

    // ---- 连接到服务器 ----
    HINTERNET hConnect = WinHttpConnect(hSession, DSDB_API_HOST, DSDB_API_PORT, 0);
    if (!hConnect)
    {
        DWORD err = GetLastError();
        result.error_message = L"无法连接到服务器 (WinHttpConnect 失败)";
        Logger_Error(L"TestDeepSeekApi: WinHttpConnect 失败, host=%s port=%d err=%lu",
            DSDB_API_HOST, DSDB_API_PORT, err);
        WinHttpCloseHandle(hSession);
        return result;
    }

    Logger_Debug(L"TestDeepSeekApi: WinHttpConnect 成功, host=%s", DSDB_API_HOST);

    // ---- 打开 HTTPS 请求 ----
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        DSDB_API_BALANCE_PATH,
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);   // HTTPS

    if (!hRequest)
    {
        DWORD err = GetLastError();
        result.error_message = L"无法创建请求 (WinHttpOpenRequest 失败)";
        Logger_Error(L"TestDeepSeekApi: WinHttpOpenRequest 失败, err=%lu", err);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // ---- 设置 Authorization 头 ----
    if (!WinHttpAddRequestHeaders(
            hRequest,
            authHeader.c_str(),
            static_cast<DWORD>(authHeader.size()),
            WINHTTP_ADDREQ_FLAG_ADD))
    {
        DWORD err = GetLastError();
        result.error_message = L"无法设置请求头";
        Logger_Error(L"TestDeepSeekApi: WinHttpAddRequestHeaders 失败, err=%lu", err);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    Logger_Debug(L"TestDeepSeekApi: Authorization 头已设置");

    // ---- 发送请求 ----
    Logger_Debug(L"TestDeepSeekApi: 正在发送请求...");
    if (!WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0,
            0, 0))
    {
        DWORD err = GetLastError();
        result.error_message = L"请求发送失败";
        if (err == ERROR_WINHTTP_TIMEOUT)
            result.error_message = L"请求超时，请检查网络连接";
        else if (err == ERROR_WINHTTP_CANNOT_CONNECT)
            result.error_message = L"无法连接到 api.deepseek.com，请检查网络";
        else if (err == ERROR_WINHTTP_NAME_NOT_RESOLVED)
            result.error_message = L"无法解析域名 api.deepseek.com，请检查 DNS";
        Logger_Error(L"TestDeepSeekApi: WinHttpSendRequest 失败, err=%lu msg=\"%s\"",
            err, result.error_message.c_str());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // ---- 接收响应 ----
    if (!WinHttpReceiveResponse(hRequest, nullptr))
    {
        DWORD err = GetLastError();
        result.error_message = L"服务器无响应";
        Logger_Error(L"TestDeepSeekApi: WinHttpReceiveResponse 失败, err=%lu", err);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // ---- 获取 HTTP 状态码 ----
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusCodeSize,
        WINHTTP_NO_HEADER_INDEX);

    result.http_status_code = static_cast<int>(statusCode);
    Logger_Debug(L"TestDeepSeekApi: HTTP 状态码=%d", result.http_status_code);

    // ---- 读取响应体 ----
    std::string utf8Body;
    DWORD bytesAvailable = 0;
    char buffer[DSDB_BUF_HTTP_READ];      // UTF-8 原始数据缓冲区
    DWORD bytesRead = 0;

    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0)
    {
        DWORD toRead = (bytesAvailable < sizeof(buffer)) ? bytesAvailable : sizeof(buffer);
        if (WinHttpReadData(hRequest, buffer, toRead, &bytesRead))
        {
            if (bytesRead > 0)
                utf8Body.append(buffer, bytesRead);
        }
        else
        {
            break;
        }
    }

    // ---- 释放 WinHTTP 句柄 ----
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // ---- 如果响应体为空，返回 HTTP 状态码错误 ----
    if (utf8Body.empty())
    {
        wchar_t statusMsg[DSDB_BUF_STATUS];
        swprintf_s(statusMsg, L"HTTP 错误 %d", statusCode);
        result.error_message = statusMsg;
        Logger_Warn(L"TestDeepSeekApi: 响应体为空, HTTP=%d", statusCode);
        return result;
    }

    Logger_Debug(L"TestDeepSeekApi: 响应体=%zu 字节 (UTF-8)", utf8Body.size());

    // ---- UTF-8 转 UTF-16 ----
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Body.c_str(), -1, nullptr, 0);
    if (wideLen <= 0)
    {
        result.error_message = L"响应数据编码转换失败";
        return result;
    }

    std::vector<wchar_t> wideBody(wideLen + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8Body.c_str(), -1, wideBody.data(), wideLen);
    wideBody[wideLen] = L'\0';

    const wchar_t* json = wideBody.data();

    // ---- 检查是否为错误响应（{ "error": { "message": "..." } }） ----
    std::wstring errorMsg;
    if (JsonExtractString(json, L"message", errorMsg) && !errorMsg.empty())
    {
        result.error_message = errorMsg;
        Logger_Warn(L"TestDeepSeekApi: API 返回错误消息: \"%s\"", errorMsg.c_str());
        return result;
    }

    // ---- 解析正常余额响应 ----
    // 提取 is_available
    if (!JsonExtractBool(json, L"is_available", result.is_available))
    {
        result.error_message = L"服务器返回数据格式异常，无法解析 is_available";
        Logger_Error(L"TestDeepSeekApi: 无法解析 is_available");
        return result;
    }

    // 提取余额信息字段（它们位于 balance_infos 数组的第一个元素中）
    JsonExtractString(json, L"currency", result.currency);
    JsonExtractString(json, L"total_balance", result.total_balance);
    JsonExtractString(json, L"granted_balance", result.granted_balance);
    JsonExtractString(json, L"topped_up_balance", result.topped_up_balance);

    result.success = true;
    Logger_Info(L"TestDeepSeekApi: 解析成功, available=%d total=%s granted=%s topped=%s currency=%s",
        result.is_available, result.total_balance.c_str(), result.granted_balance.c_str(),
        result.topped_up_balance.c_str(), result.currency.c_str());
    return result;
}
