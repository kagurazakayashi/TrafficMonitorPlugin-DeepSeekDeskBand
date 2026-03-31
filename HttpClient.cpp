/************************************************************************//**
 * @file    HttpClient.cpp
 * @brief   HTTP 客户端工具实现
 * @details 使用 WinHTTP 发送 HTTPS GET 请求，查询 DeepSeek API 余额。
 *          包含简易 JSON 解析器，不依赖任何第三方 JSON 库。
 ****************************************************************************/
#include "HttpClient.h"
#include "Config.h"
#include "framework.h"
#include <winhttp.h>
#include <vector>
#include <cwchar>
#include <cstring>
#pragma comment(lib, "winhttp.lib")

// ============================================================
// 常量定义
// ============================================================

/** @brief API 服务器主机名 */
static const wchar_t* HOST_NAME = L"api.deepseek.com";

/** @brief 余额查询路径 */
static const wchar_t* BALANCE_PATH = L"/user/balance";

/** @brief 请求超时（毫秒），将在函数内部根据参数计算 */
static const int MIN_TIMEOUT_SEC = 3;
static const int MAX_TIMEOUT_SEC = 60;

// ============================================================
// 简易 JSON 解析辅助函数
// ============================================================

/**
 * @brief 在 JSON 字符串中查找指定键，提取其布尔值（true/false）
 * @param json   以 null 结尾的 UTF-16 JSON 字符串
 * @param key    要查找的键（不含引号）
 * @param out    输出布尔值
 * @return 找到并成功解析返回 true
 */
static bool JsonExtractBool(const wchar_t* json, const wchar_t* key, bool& out)
{
    if (!json || !key)
        return false;

    // 构造带引号的键字符串，如 L"\"is_available\""
    std::wstring searchKey = L"\"";
    searchKey += key;
    searchKey += L"\"";

    const wchar_t* pos = wcsstr(json, searchKey.c_str());
    if (!pos)
        return false;

    // 跳过键部分，找到 ":" 分隔符
    pos = pos + searchKey.size();
    while (*pos && (*pos == L':' || *pos == L' ' || *pos == L'\t' || *pos == L'\n' || *pos == L'\r'))
        pos++;

    if (wcsncmp(pos, L"true", 4) == 0)
    {
        out = true;
        return true;
    }
    if (wcsncmp(pos, L"false", 5) == 0)
    {
        out = false;
        return true;
    }
    return false;
}

/**
 * @brief 在 JSON 字符串中查找指定键，提取其字符串值（不含外层引号）
 * @param json   以 null 结尾的 UTF-16 JSON 字符串
 * @param key    要查找的键（不含引号）
 * @param out    输出字符串值
 * @return 找到并成功提取返回 true
 */
static bool JsonExtractString(const wchar_t* json, const wchar_t* key, std::wstring& out)
{
    if (!json || !key)
        return false;

    std::wstring searchKey = L"\"";
    searchKey += key;
    searchKey += L"\"";

    const wchar_t* pos = wcsstr(json, searchKey.c_str());
    if (!pos)
        return false;

    // 跳过键部分，找到 ":" 后的第一个 '"'
    pos = pos + searchKey.size();
    while (*pos && (*pos == L':' || *pos == L' ' || *pos == L'\t' || *pos == L'\n' || *pos == L'\r'))
        pos++;

    if (*pos != L'"')
        return false;

    pos++; // 跳过开引号
    const wchar_t* end = pos;
    while (*end && *end != L'"')
        end++;

    if (*end != L'"')
        return false;

    out.assign(pos, static_cast<size_t>(end - pos));
    return true;
}

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
        return result;
    }

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
        result.error_message = L"无法初始化网络会话 (WinHttpOpen 失败)";
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
        result.error_message = L"无法连接到服务器 (WinHttpConnect 失败)";
        WinHttpCloseHandle(hSession);
        return result;
    }

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
        result.error_message = L"无法创建请求 (WinHttpOpenRequest 失败)";
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
        result.error_message = L"无法设置请求头";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // ---- 发送请求 ----
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
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // ---- 接收响应 ----
    if (!WinHttpReceiveResponse(hRequest, nullptr))
    {
        result.error_message = L"服务器无响应";
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
        return result;
    }

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
        return result;
    }

    // ---- 解析正常余额响应 ----
    // 提取 is_available
    if (!JsonExtractBool(json, L"is_available", result.is_available))
    {
        // 可能响应格式异常
        result.error_message = L"服务器返回数据格式异常，无法解析 is_available";
        return result;
    }

    // 提取余额信息字段（它们位于 balance_infos 数组的第一个元素中）
    JsonExtractString(json, L"currency", result.currency);
    JsonExtractString(json, L"total_balance", result.total_balance);
    JsonExtractString(json, L"granted_balance", result.granted_balance);
    JsonExtractString(json, L"topped_up_balance", result.topped_up_balance);

    result.success = true;
    return result;
}
