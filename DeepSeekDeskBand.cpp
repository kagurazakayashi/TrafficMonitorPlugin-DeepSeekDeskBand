/************************************************************************//**
 * @file    DeepSeekDeskBand.cpp
 * @brief   DeepSeekDeskBand 插件实现
 * @details 实现 CDeepSeekDeskBandItem（显示项目）和 CDeepSeekDeskBand（插件主类）
 *          的所有接口函数，以及插件导出函数 TMPluginGetInstance()。
 ****************************************************************************/
#include "DeepSeekDeskBand.h"

// ============================================================
// CDeepSeekDeskBandItem 实现 —— 显示项目
// ============================================================

/**
 * @brief 获取显示项目名称
 * @return 显示在右键菜单"显示项目"子菜单下的名称
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemName() const
{
    return L"DeepSeekDeskBand";
}

/**
 * @brief 获取显示项目唯一 ID
 * @return 仅包含字母和数字的唯一标识符
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemId() const
{
    return L"DeepSeekDeskBandItem";
}

/**
 * @brief 获取标签文本
 * @return 标签文本字符串（当前返回空字符串，不显示标签）
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemLableText() const
{
    return L"";
}

/**
 * @brief 获取数值文本
 * @return 数值文本字符串（此函数被频繁调用，不要在内部获取数据）
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemValueText() const
{
    return L"DeepSeekDeskBand";
}

/**
 * @brief 获取示例文本
 * @return 示例文本字符串，用于计算显示区域宽度和皮肤预览
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemValueSampleText() const
{
    return L"DeepSeekDeskBand";
}

// ============================================================
// CDeepSeekDeskBand 实现 —— 插件主类
// ============================================================

/** @brief 静态全局单例实例的定义 */
CDeepSeekDeskBand CDeepSeekDeskBand::m_instance;

/** @brief 私有构造函数 */
CDeepSeekDeskBand::CDeepSeekDeskBand()
{
}

/**
 * @brief 获取全局唯一实例
 * @return 插件的单例引用
 */
CDeepSeekDeskBand& CDeepSeekDeskBand::Instance()
{
    return m_instance;
}

/**
 * @brief 根据索引获取显示项目对象
 * @param index 显示项目索引，从 0 开始
 * @return 对应索引的 IPluginItem 指针，越界时返回 nullptr
 * @note   必须对 index 越界进行判断，返回 nullptr
 */
IPluginItem* CDeepSeekDeskBand::GetItem(int index)
{
    if (index == 0)
        return &m_item;     // 当前仅有一个显示项目
    return nullptr;         // 索引越界，返回空指针
}

/**
 * @brief 定时数据获取
 * @details 由主程序每隔一定时间调用，在此获取所有显示项目需要的监控数据。
 *          当前为空白插件，无需获取任何数据。
 */
void CDeepSeekDeskBand::DataRequired()
{
    // 空白插件：此版本暂不获取任何监控数据
}

/**
 * @brief 获取插件信息
 * @param index 信息索引（名称/描述/作者/版权/版本/URL）
 * @return 对应的信息字符串，未知索引返回空字符串
 */
const wchar_t* CDeepSeekDeskBand::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:          // 插件名称
        return L"DeepSeekDeskBand";
    case TMI_DESCRIPTION:   // 插件功能描述
        return L"DeepSeek 桌面助手插件";
    case TMI_AUTHOR:        // 作者
        return L"yashi";
    case TMI_COPYRIGHT:     // 版权信息
        return L"";
    case TMI_VERSION:       // 版本号
        return L"1.0";
    case TMI_URL:           // 项目主页
        return L"";
    default:
        break;
    }
    return L"";
}

// ============================================================
// 导出函数 —— 插件入口
// ============================================================

/**
 * @brief 插件导出入口函数
 * @return ITMPlugin 接口指针，指向插件的全局唯一实例
 * @note   主程序通过此函数获取插件接口。
 *         返回的对象应在程序运行期间始终有效，不会被主程序释放。
 */
ITMPlugin* TMPluginGetInstance()
{
    return &CDeepSeekDeskBand::Instance();
}
