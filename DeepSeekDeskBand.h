/************************************************************************//**
 * @file    DeepSeekDeskBand.h
 * @brief   DeepSeekDeskBand 插件声明
 * @details 包含插件主类 CDeepSeekDeskBand 和显示项目类 CDeepSeekDeskBandItem 的声明，
 *          以及插件导出函数 TMPluginGetInstance 的声明。
 ****************************************************************************/
#pragma once
#include "PluginInterface.h"

/************************************************************************//**
 * @class CDeepSeekDeskBandItem
 * @brief DeepSeekDeskBand 显示项目类
 * @details 实现 IPluginItem 接口，在 TrafficMonitor 任务栏或主窗口中显示一个项目。
 *          当前为空白插件，仅显示 "DeepSeekDeskBand" 文本。
 ****************************************************************************/
class CDeepSeekDeskBandItem : public IPluginItem
{
public:
    /** @brief 获取显示项目的名称，显示在右键菜单的"显示项目"子菜单下 */
    virtual const wchar_t* GetItemName() const override;

    /** @brief 获取显示项目的唯一标识符，用于区分每个显示项目 */
    virtual const wchar_t* GetItemId() const override;

    /** @brief 获取项目标签文本，显示在数值文本的左侧 */
    virtual const wchar_t* GetItemLableText() const override;

    /** @brief 获取项目数值文本，显示在标签文本的右侧（频繁调用） */
    virtual const wchar_t* GetItemValueText() const override;

    /** @brief 获取项目数值的示例文本，用于计算显示区域宽度 */
    virtual const wchar_t* GetItemValueSampleText() const override;
};

/************************************************************************//**
 * @class CDeepSeekDeskBand
 * @brief DeepSeekDeskBand 插件主类
 * @details 实现 ITMPlugin 接口，管理插件的生命周期和所有显示项目。
 *          采用单例模式，通过 Instance() 获取全局唯一实例。
 ****************************************************************************/
class CDeepSeekDeskBand : public ITMPlugin
{
public:
    /** @brief 获取插件的全局唯一实例（单例模式） */
    static CDeepSeekDeskBand& Instance();

    /** @brief 根据索引返回对应的显示项目对象 */
    virtual IPluginItem* GetItem(int index) override;

    /** @brief 定时数据获取函数，由主程序每隔一定时间调用 */
    virtual void DataRequired() override;

    /** @brief 获取插件信息（名称、描述、作者、版本等） */
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;

private:
    /** @brief 私有构造函数，防止外部直接创建实例 */
    CDeepSeekDeskBand();

    /** @brief 全局唯一实例 */
    static CDeepSeekDeskBand m_instance;

    /** @brief 插件提供的显示项目对象（当前仅一个） */
    CDeepSeekDeskBandItem m_item;
};

#ifdef __cplusplus
extern "C" {
#endif
    /** @brief 插件导出函数，返回 ITMPlugin 接口指针 */
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif
