/************************************************************************//**
 * @file    SettingsDialog.h
 * @brief   设置对话框模块声明
 * @details 提供插件的设置对话框 UI，支持：
 *          - API 密钥输入与格式校验
 *          - API 测试功能
 *          - 更新间隔、请求超时、历史记录数量设置
 *          - 语言即时切换
 *          - 余额历史记录 ListView 显示
 *          - 自动刷新 / 手动刷新历史列表
 *          - DeepSeek 图标异步下载
 *          - DPI 感知的自绘 Win32 窗口
 ****************************************************************************/
#pragma once
#include "Config.h"
#include "HttpClient.h"
#include "HistoryManager.h"
#include "Strings.h"
#include <windows.h>
#include <vector>

// ============================================================
// 对话框控件 ID
// ============================================================

enum SettingsDlgControlId
{
    IDC_EDIT_API_KEY            = 1001,  ///< DeepSeek API 输入框
    IDC_STATIC_API_HINT         = 1002,  ///< API 格式提示（红色警告文字）
    IDC_BTN_TEST_API            = 1003,  ///< "测试API" 按钮
    IDC_STATIC_STATUS           = 1004,  ///< 测试结果状态文本
    IDC_EDIT_INTERVAL           = 1005,  ///< 更新间隔输入框
    IDC_SPIN_INTERVAL           = 1006,  ///< 更新间隔 UpDown 控件
    IDC_BTN_OK                  = 1007,  ///< "确定" 按钮
    IDC_BTN_CANCEL              = 1008,  ///< "取消" 按钮
    IDC_EDIT_TIMEOUT            = 1009,  ///< 请求超时输入框
    IDC_SPIN_TIMEOUT            = 1010,  ///< 请求超时 UpDown 控件
    IDC_EDIT_HISTORY_COUNT      = 1011,  ///< 历史记录数量输入框
    IDC_SPIN_HISTORY_COUNT      = 1012,  ///< 历史记录数量 UpDown 控件
    IDC_LIST_HISTORY            = 1013,  ///< 历史记录 ListView 控件
    IDC_BTN_CLEAR_HISTORY       = 1014,  ///< "清除历史" 按钮
    IDC_CHECK_AUTO_REFRESH      = 1015,  ///< "自动刷新" 复选框
    IDC_STATIC_ICON             = 1016,  ///< DeepSeek 图标静态控件
    IDC_COMBO_LANGUAGE          = 1017,  ///< 显示语言下拉框
    IDC_STATIC_API_LABEL        = 1018,  ///< "DeepSeek API:" 标签
    IDC_STATIC_INTERVAL_LABEL   = 1019,  ///< "更新间隔（秒）:" 标签
    IDC_STATIC_TIMEOUT_LABEL    = 1020,  ///< "请求超时（秒）:" 标签
    IDC_STATIC_HISTORY_COUNT_LABEL = 1021, ///< "历史记录数量:" 标签
    IDC_STATIC_HISTORY_LABEL    = 1022,  ///< "历史记录:" 标签
    IDC_STATIC_LANGUAGE_LABEL   = 1023,  ///< "显示语言:" 标签
};

/** @brief 图标下载完成的自定义消息（WM_APP + 1） */
#define WM_ICON_DOWNLOADED  (WM_APP + 1)

// ============================================================
// 对话框数据结构
// ============================================================

/************************************************************************//**
 * @struct SettingsDlgData
 * @brief  设置对话框数据
 * @details 在创建对话框前分配，通过 CreateWindowExW 的 lpParam 传递给
 *          WM_CREATE 消息，窗口过程通过 GWLP_USERDATA 存取。
 *          包含所有控件的当前值和原始值，用于变更追踪和取消还原。
 ****************************************************************************/
struct SettingsDlgData
{
    wchar_t apiKey[DSDB_BUF_APIKEY];              ///< 当前输入的 API 密钥
    wchar_t apiKeyOrig[DSDB_BUF_APIKEY];          ///< 原始 API 密钥（取消时还原）
    int     updateInterval;           ///< 当前输入的更新间隔
    int     updateIntervalOrig;       ///< 原始更新间隔
    int     requestTimeout;           ///< 当前输入的请求超时时间
    int     requestTimeoutOrig;       ///< 原始请求超时时间
    int     maxHistoryCount;          ///< 当前设置的历史记录最大数量
    int     maxHistoryCountOrig;      ///< 原始历史记录最大数量
    const std::vector<HistoryRecord>* historyRecords; ///< 指向历史记录列表（只读）
    bool    historyCleared;            ///< 历史记录是否已被清除
    bool    autoRefresh;               ///< 列表是否自动刷新（默认 true）
    bool    changed;                  ///< 是否有未保存的变更
    bool    apiTested;                ///< API 是否已通过测试
    wchar_t iconPath[512];            ///< DeepSeek 图标文件本地路径
    Language language;                 ///< 显示语言偏好
    Language languageOrig;             ///< 原始语言偏好
};

/************************************************************************//**
 * @struct IconDownloadParam
 * @brief  图标异步下载线程参数
 ****************************************************************************/
struct IconDownloadParam
{
    HWND    hWnd;           ///< 对话框窗口句柄（用于 PostMessage 通知）
    wchar_t path[512];      ///< 图标保存路径
};

// ============================================================
// 导出函数
// ============================================================

/************************************************************************//**
 * @brief  显示插件的设置对话框（模态）
 * @param  hParent          父窗口句柄
 * @param  apiKey           输入/输出：API 密钥
 * @param  updateInterval   输入/输出：更新间隔
 * @param  requestTimeout   输入/输出：请求超时
 * @param  maxHistoryCount  输入/输出：历史记录上限
 * @param  language         输入/输出：语言偏好
 * @param  historyRecords   历史记录列表只读引用
 * @param  historyManager   历史记录管理器（用于清除/裁剪操作）
 * @param  configDir        配置文件目录（用于图标路径）
 * @return 用户点击"确定"且设置有变更时返回 true，否则返回 false
 ****************************************************************************/
bool ShowSettingsDialog(
    HWND hParent,
    wchar_t* apiKey,
    int& updateInterval,
    int& requestTimeout,
    int& maxHistoryCount,
    Language& language,
    const std::vector<HistoryRecord>& historyRecords,
    HistoryManager* historyManager,
    const std::wstring& configDir);
