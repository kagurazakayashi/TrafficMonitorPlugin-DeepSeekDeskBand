/************************************************************************//**
 * @file    HistoryManager.h
 * @brief   余额刷新历史记录管理模块声明
 * @details 提供历史记录的序列化、DPAPI 加密/解密、增量文件 I/O 功能。
 *          每条记录 49 字节（packed），使用 Windows DPAPI 逐条加密存储。
 *          文件格式：
 *            [Header, 12B, 明文] magic(4B) + version(4B) + record_count(4B)
 *            [Record 0] enc_size(4B) + DPAPI加密数据(enc_size B)
 *            [Record 1] enc_size(4B) + DPAPI加密数据(enc_size B)
 *            ...
 ****************************************************************************/
#pragma once
#include "Config.h"
#include "HttpClient.h"
#include <windows.h>
#include <vector>
#include <cstdint>
#include <string>

/************************************************************************//**
 * @struct HistoryRecord
 * @brief  余额刷新历史记录（紧凑打包，单条 49 字节）
 * @details 每次刷新余额时记录时间戳、可用状态、各余额值和币种。
 *          所有数值以 double 存储以保证精度。
 *          #pragma pack(1) 确保跨平台/跨版本兼容。
 ****************************************************************************/
#pragma pack(push, 1)
struct HistoryRecord
{
    int64_t timestamp;              ///< Unix 时间戳（毫秒）
    uint8_t is_available;           ///< 账户是否可用（0=不可用, 1=可用）
    double  total_balance;          ///< 总余额数值
    double  granted_balance;        ///< 赠送余额数值
    double  topped_up_balance;      ///< 充值余额数值
    wchar_t currency[8];            ///< 币种字符串（最多 7 字符 + null 终止符，如 "CNY"）
};
#pragma pack(pop)

// ============================================================
// 历史记录管理器
// ============================================================

/************************************************************************//**
 * @class HistoryManager
 * @brief 历史记录管理器
 * @details 管理余额查询历史记录的完整生命周期：
 *          - 内存缓存（std::vector<HistoryRecord>）
 *          - 序列化 / 反序列化
 *          - DPAPI 逐条加密 / 解密
 *          - 增量文件追加（性能优先）
 *          - 整体文件重写（配置变更时）
 *          - 自动截断超出上限的旧记录
 ****************************************************************************/
class HistoryManager
{
public:
    /** @brief 构造函数 */
    HistoryManager();

    /** @brief 析构函数 —— 确保文件已刷新 */
    ~HistoryManager();

    // ---- 路径设置 ----

    /** @brief 设置历史记录文件所在目录路径
     *  @param configDir 插件配置目录（由 TrafficMonitor 提供）
     *  @note  必须在任何 I/O 操作之前调用 */
    void SetConfigDir(const std::wstring& configDir);

    // ---- 记录数上限 ----

    /** @brief 设置最大保留记录数
     *  @param count 0=不记录, 1~10000=保留最近 N 条
     *  @note  若新上限小于现有记录数，需调用 TrimToMaxCount() 后重写文件 */
    void SetMaxHistoryCount(int count);
    int  GetMaxHistoryCount() const { return m_maxHistoryCount; }

    // ---- 内存访问 ----

    /** @brief 获取内存中历史记录的只读引用 */
    const std::vector<HistoryRecord>& GetRecords() const { return m_records; }

    /** @brief 获取记录数量 */
    size_t GetRecordCount() const { return m_records.size(); }

    // ---- 数据操作 ----

    /** @brief 从文件加载所有历史记录到内存
     *  @details 自动按 m_maxHistoryCount 截断旧记录 */
    void Load();

    /** @brief 追加一条历史记录
     *  @param result DeepSeek API 查询结果
     *  @details 只有当余额发生实际变动（与上条差值 > 0.001）时才记录。
     *           自动截断超出上限的旧记录，并使用增量模式写入文件。 */
    void Append(const ApiTestResult& result);

    /** @brief 保存所有记录到文件
     *  @param fullRewrite true=整体加密重写; false=仅追加最后一条（增量模式） */
    void Save(bool fullRewrite);

    /** @brief 清空所有历史记录（内存 + 文件） */
    void Clear();

    /** @brief 裁剪记录至最大数量上限（从旧记录开始移除） */
    void TrimToMaxCount();

private:
    /** @brief 历史记录文件完整路径 */
    std::wstring GetFilePath() const;

    /** @brief 将单条记录序列化为 49 字节紧凑格式 */
    static std::vector<uint8_t> SerializeRecord(const HistoryRecord& rec);

    /** @brief 从紧凑格式反序列化为单条记录 */
    static HistoryRecord DeserializeRecord(const uint8_t* data, size_t size);

    /** @brief 使用 DPAPI 加密单条记录 */
    static bool EncryptRecord(const HistoryRecord& rec, std::vector<uint8_t>& outEncrypted);

    /** @brief 使用 DPAPI 解密单条记录 */
    static bool DecryptRecord(const uint8_t* encryptedData, size_t encryptedSize,
                              HistoryRecord& outRecord);

    /** @brief 将 ApiTestResult 转换为 HistoryRecord */
    static HistoryRecord ResultToRecord(const ApiTestResult& result);

    /** @brief 判断两条记录的余额是否有变动（差值 > 0.001） */
    static bool HasBalanceChanged(const HistoryRecord& a, const HistoryRecord& b);

private:
    std::wstring                m_configDir;        ///< 配置文件目录
    int                         m_maxHistoryCount;  ///< 最大保留记录数（0=不记录）
    std::vector<HistoryRecord>  m_records;          ///< 内存缓存（按时间升序排列）
};
