/************************************************************************//**
 * @file    DeepSeekDeskBand.cpp
 * @brief   DeepSeekDeskBand 插件实现
 * @details 实现 CDeepSeekDeskBandItem（显示项目）和 CDeepSeekDeskBand（插件主类）
 *          的所有接口函数，以及插件导出函数 TMPluginGetInstance()。
 ****************************************************************************/
#include "DeepSeekDeskBand.h"
#include "framework.h"
#include <string.h>
#include <stdio.h>

// ============================================================
// CDeepSeekDeskBandItem 实现 —— 显示项目
// ============================================================

/**
 * @brief 构造函数，初始化默认显示文本
 */
CDeepSeekDeskBandItem::CDeepSeekDeskBandItem()
{
    // 初始化数值文本为默认值 "就绪"
    wcscpy_s(m_valueText, L"就绪");
}

/**
 * @brief 设置当前显示的数值文本
 * @param text 要显示的文本内容（最大 63 个宽字符）
 */
void CDeepSeekDeskBandItem::SetValueText(const wchar_t* text)
{
    wcscpy_s(m_valueText, text);
}

/**
 * @brief 获取显示项目名称
 * @return 显示在右键菜单"显示项目"子菜单下的名称
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemName() const
{
    return L"DeepSeek 助手";
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
 * @return 标签文本字符串，显示在数值文本的左侧
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemLableText() const
{
    return LABEL_TEXT;
}

/**
 * @brief 获取数值文本
 * @return 数值文本字符串（此函数被频繁调用，不要在内部获取数据）
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemValueText() const
{
    return m_valueText;
}

/**
 * @brief 获取示例文本
 * @return 示例文本字符串，用于计算显示区域宽度和皮肤预览
 */
const wchar_t* CDeepSeekDeskBandItem::GetItemValueSampleText() const
{
    return SAMPLE_TEXT;
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
 *          当前版本以自增计数器作为演示数值。
 */
void CDeepSeekDeskBand::DataRequired()
{
    // 静态计数器，每次调用自增，用于演示数值变化
    static int counter = 0;
    counter++;

    wchar_t valueText[64];
    swprintf_s(valueText, L"%d", counter);
    m_item.SetValueText(valueText);
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
// 设置对话框 —— 空白占位对话框
// ============================================================

/** @brief 设置对话框窗口类名 */
static const wchar_t* SETTINGS_DIALOG_CLASS = L"DeepSeekDeskBandSettingsDlg";

/** @brief 控件 ID */
enum { IDC_BTN_OK = 1001 };

/**
 * @brief 设置对话框窗口过程
 */
static LRESULT CALLBACK SettingsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
        // 创建静态文本标签
        CreateWindowW(L"STATIC", L"你好",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            30, 25, 270, 24,
            hWnd, nullptr, hInst, nullptr);
        // 创建"确定"按钮
        CreateWindowW(L"BUTTON", L"确定",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            130, 90, 80, 26,
            hWnd, (HMENU)IDC_BTN_OK, hInst, nullptr);
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BTN_OK)
        {
            DestroyWindow(hWnd);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

/**
 * @brief 打开插件的设置对话框
 * @param hParent 父窗口句柄
 * @return OR_OPTION_CHANGED
 */
ITMPlugin::OptionReturn CDeepSeekDeskBand::ShowOptionsDialog(void* hParent)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // 注册窗口类（仅首次注册）
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = SettingsDlgProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = SETTINGS_DIALOG_CLASS;
    RegisterClassExW(&wc);

    // 计算居中位置
    int dlgWidth = 350;
    int dlgHeight = 170;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    HWND hParentWnd = (HWND)hParent;
    if (hParentWnd)
    {
        RECT parentRect;
        GetWindowRect(hParentWnd, &parentRect);
        x = parentRect.left + ((parentRect.right - parentRect.left) - dlgWidth) / 2;
        y = parentRect.top + ((parentRect.bottom - parentRect.top) - dlgHeight) / 2;
    }

    // 创建对话框窗口
    HWND hDlg = CreateWindowExW(
        0, SETTINGS_DIALOG_CLASS, L"DeepSeek 设置",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, dlgWidth, dlgHeight,
        hParentWnd, nullptr, hInst, nullptr);

    if (!hDlg)
        return OR_OPTION_NOT_PROVIDED;

    // 禁用父窗口以实现模态效果
    if (hParentWnd)
        EnableWindow(hParentWnd, FALSE);

    ShowWindow(hDlg, SW_SHOW);

    // 模态消息循环
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 恢复父窗口
    if (hParentWnd)
        EnableWindow(hParentWnd, TRUE);

    return OR_OPTION_CHANGED;
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
