/************************************************************************//**
 * @file    DeepSeekDeskBand.h
 * @brief   DeepSeekDeskBand 插件声明
 * @details 包含插件主类 CDeepSeekDeskBand 和显示项目类 CDeepSeekDeskBandItem 的声明。
 *          插件采用单例模式，管理 API 余额查询、配置文件加解密、历史记录等生命周期。
 *
 *          模块拆分：
 *            - HistoryManager    : 历史记录序列化/加密/文件 I/O
 *            - SettingsDialog    : 设置对话框 UI
 *            - JsonParser        : 简易 JSON 解析
 *            - Logger            : 分级日志
 *            - ConfigEncrypt     : 配置文件 DPAPI 加解密
 *            - HttpClient        : WinHTTP API 请求
 ****************************************************************************/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Config.h"
#include "HttpClient.h"
#include "HistoryManager.h"
#include "PluginInterface.h"
#include "Strings.h"
#include <string>

/************************************************************************//**
 * @class CDeepSeekDeskBandItem
 * @brief DeepSeekDeskBand 显示项目类
 * @details 实现 IPluginItem 接口，在 TrafficMonitor 任务栏或主窗口中显示一个项目。
 *          左侧显示标签 "DeepSeek"，右侧显示数值文本（余额 + 币种）。
 *          GetItemValueText() 被频繁调用，内部不做数据获取，仅返回缓存值。
 ****************************************************************************/
class CDeepSeekDeskBandItem : public IPluginItem
{
public:
    /** @brief 构造函数，初始化默认显示文本为 "--" */
    CDeepSeekDeskBandItem();

    /** @brief 获取显示项目的名称，显示在右键菜单的"显示项目"子菜单下 */
    virtual const wchar_t* GetItemName() const override;

    /** @brief 获取显示项目的唯一标识符，用于区分每个显示项目 */
    virtual const wchar_t* GetItemId() const override;

    /** @brief 获取项目标签文本，显示在数值文本的左侧 */
    virtual const wchar_t* GetItemLableText() const override;

    /** @brief 获取项目数值文本，显示在标签文本的右侧（此函数被频繁调用） */
    virtual const wchar_t* GetItemValueText() const override;

    /** @brief 获取项目数值的示例文本，用于计算显示区域宽度和皮肤预览 */
    virtual const wchar_t* GetItemValueSampleText() const override;

    /** @brief 由插件主类调用，设置要显示的数值文本内容
     *  @param text 要显示的文本内容（最大 DSDB_BUF_VALUE - 1 个宽字符） */
    void SetValueText(const wchar_t* text);

private:
    /** @brief 显示标签文本（不可本地化，始终为 "DeepSeek"） */
    static constexpr const wchar_t* LABEL_TEXT = L"DeepSeek";

    /** @brief 示例文本，用于宽度计算（足够宽以显示余额格式） */
    static constexpr const wchar_t* SAMPLE_TEXT = L"9999.99 CNY";

    /** @brief 当前显示的数值文本缓冲区 */
    wchar_t m_valueText[DSDB_BUF_VALUE];
};

/************************************************************************//**
 * @class CDeepSeekDeskBand
 * @brief DeepSeekDeskBand 插件主类
 * @details 实现 ITMPlugin 接口，管理插件的生命周期和所有显示项目。
 *          采用单例模式，通过 Instance() 获取全局唯一实例。
 *
 *          核心功能：
 *            - 定时从 DeepSeek API 查询余额（异步非阻塞）
 *            - 配置文件 DPAPI 加解密存储
 *            - 余额历史记录管理（增量追加 + DPAPI 加密）
 *            - 多语言 UI（12 种语言，自动检测系统语言）
 *            - Win32 自绘设置对话框（DPI 感知）
 ****************************************************************************/
class CDeepSeekDeskBand : public ITMPlugin
{
public:
    /** @brief 获取插件的全局唯一实例（单例模式）
     *  @return 插件单例引用，进程生命周期内始终有效 */
    static CDeepSeekDeskBand& Instance();

    /** @brief 根据索引返回对应的显示项目对象
     *  @param index 显示项目索引，从 0 开始
     *  @return 对应索引的 IPluginItem 指针，越界时返回 nullptr */
    virtual IPluginItem* GetItem(int index) override;

    /** @brief 定时数据获取函数，由主程序每隔一定时间调用
     *  @details 检查异步请求结果，按更新间隔启动新请求，更新显示文本 */
    virtual void DataRequired() override;

    /** @brief 获取插件信息（名称、描述、作者、版本等）
     *  @param index 信息索引，参见 PluginInfoIndex 枚举
     *  @return 对应的信息字符串，未知索引返回空字符串 */
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;

    /** @brief 打开插件的设置对话框（模态）
     *  @param hParent 父窗口句柄
     *  @return 设置变更时返回 OR_OPTION_CHANGED，否则返回 OR_OPTION_UNCHANGED */
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;

    /** @brief 获取插件命令数量（右键菜单项数）
     *  @return 始终返回 1（仅一个菜单项） */
    virtual int GetCommandCount() override;

    /** @brief 获取插件命令名称
     *  @param command_index 命令索引（0-based）
     *  @return 菜单项显示文本 */
    virtual const wchar_t* GetCommandName(int command_index) override;

    /** @brief 执行插件命令
     *  @param command_index 命令索引
     *  @param hWnd 主窗口句柄
     *  @param para 额外参数 */
    virtual void OnPluginCommand(int command_index, void* hWnd, void* para) override;

    /** @brief 插件初始化回调
     *  @param pApp 主程序 ITrafficMonitor 接口指针
     *  @details 保存主程序指针，加载配置文件，初始化历史记录管理器和语言设置 */
    virtual void OnInitialize(ITrafficMonitor* pApp) override;

    /** @brief 异步 API 请求线程过程（静态，通过单例访问成员）
     *  @param lpParam 未使用
     *  @return 始终返回 0 */
    static DWORD WINAPI ApiRequestThreadProc(LPVOID lpParam);

private:
    /** @brief 私有构造函数，防止外部直接创建实例 */
    CDeepSeekDeskBand();

    /** @brief 私有析构函数，释放 CriticalSection */
    ~CDeepSeekDeskBand();

    /** @brief 全局唯一实例 */
    static CDeepSeekDeskBand m_instance;

    /** @brief 插件提供的显示项目对象（当前仅一个） */
    CDeepSeekDeskBandItem m_item;

    /** @brief 主程序接口指针，用于获取配置目录等信息 */
    ITrafficMonitor* m_pApp = nullptr;

    /** @brief DeepSeek API 密钥缓冲区 */
    wchar_t m_apiKey[DSDB_BUF_APIKEY] = {};

    /** @brief 更新间隔，单位秒 */
    int m_updateInterval = DSDB_DEFAULT_INTERVAL;

    /** @brief 请求超时时间，单位秒 */
    int m_requestTimeout = DSDB_DEFAULT_TIMEOUT;

    /** @brief 上次成功获取余额的时间戳（GetTickCount64），0 表示从未获取 */
    ULONGLONG m_lastFetchTime = 0;

    /** @brief 最近一次余额查询结果 */
    ApiTestResult m_lastBalanceResult;

    /** @brief 是否有有效的余额数据可供显示 */
    bool m_hasBalance = false;

    /** @brief API 请求是否正在进行中（防止重复发起） */
    bool m_requestInProgress = false;

    /** @brief 请求线程句柄 */
    HANDLE m_hRequestThread = nullptr;

    /** @brief 异步请求的待处理结果 */
    ApiTestResult m_pendingResult;

    /** @brief 待处理结果是否就绪 */
    bool m_pendingResultReady = false;

    /** @brief 线程安全临界区（保护 m_pendingResult / m_pendingResultReady / m_apiKey 等） */
    CRITICAL_SECTION m_csRequest;

    /** @brief 余额刷新历史记录管理器 */
    HistoryManager m_historyManager;

    /** @brief 显示语言偏好（Auto=自动检测系统语言） */
    Language m_language = Language::Auto;

    /** @brief 获取配置文件完整路径
     *  @return 配置文件路径字符串，使用 TrafficMonitor 提供的插件配置目录 */
    std::wstring GetConfigFilePath() const;

    /** @brief 从加密的二进制配置文件加载设置
     *  @details 解密失败时自动恢复默认设置，不中断插件运行 */
    void LoadConfig();

    /** @brief 保存设置到加密的二进制配置文件 */
    void SaveConfig();

    /** @brief 校验加载的语言枚举值是否在有效范围内
     *  @param lang 待校验的语言枚举值
     *  @return 有效范围 (Auto ~ Turkish) 内的值，否则返回 Auto */
    static Language ValidateLanguage(Language lang);
};

#ifdef __cplusplus
extern "C" {
#endif
    /** @brief 插件导出函数，返回 ITMPlugin 接口指针
     *  @return 插件单例的 ITMPlugin 接口指针
     *  @note   主程序通过此函数获取插件接口，返回的对象在程序运行期间始终有效 */
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif
