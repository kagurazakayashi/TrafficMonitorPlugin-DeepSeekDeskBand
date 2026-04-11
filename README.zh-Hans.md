# DeepSeek 余额助手 (DeepSeekDeskBand)

![screenshot](screenshot.png)

[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 桌面助手插件。在任务栏窗口中实时显示 DeepSeek 官网余额信息，支持余额变动历史记录。

[English](README.md) | 简体中文 | [繁體中文](README.zh-Hant.md) | [日本語](README.ja.md) | [Deutsch](README.de.md) | [עברית](README.he.md) | [Magyar](README.hu.md) | [Italiano](README.it.md) | [Polski](README.pl.md) | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## 功能特性

- 定时查询 DeepSeek 官网 API 余额
- 在 TrafficMonitor 的任务栏和主窗口中显示余额数值与币种
- 余额变动历史记录（自动增量存储，上限可调）
- 支持多语言（自动检测系统语言或手动选择）
- 配置文件（含 API Key）与历史记录使用 Windows DPAPI 加密，仅限当前 Windows 用户解密

## 安装

1. 从 [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases) 下载对应版本的 DLL：
   - x86 版 TrafficMonitor → `DeepSeekDeskBand.dll`
   - x64 版 TrafficMonitor → `DeepSeekDeskBand64.dll`
2. 将 DLL 复制到 TrafficMonitor 程序目录下的 `plugins` 文件夹
3. 重启 TrafficMonitor
4. 在任务栏空白区域右键，选择「显示设置」，勾选「DeepSeek 余额助手」即可显示

> **注意**：DLL 必须放在 `plugins` 子文件夹内，放在其他目录无法加载。

## 显示内容说明

插件显示的内容由两部分组成：

- **左侧标签文本**：可在 TrafficMonitor 的「选项」→「任务栏窗口设置」→「显示文本设置」中修改。
- **右侧数值文本**：显示 `余额数值 币种`，例如 `100.00 CNY`。余额数据定时自动更新。

在 TrafficMonitor 右键菜单的「显示项目」子菜单中，插件的显示名称随语言设置变化（如简体中文环境下为「DeepSeek 余额助手」）。

## 打开插件配置窗口

右键点击 TrafficMonitor 的任务栏窗口或主窗口，在右键菜单中选择「插件管理」，右键点击本插件并选择「选项」，即可打开配置对话框。在配置对话框中可设置 API Key 等选项，亦可查看历史记录。

## 各项配置说明

### DeepSeek API 密钥

- 在输入框中填入你的 [DeepSeek API Key](https://platform.deepseek.com/api_keys)
- 密钥以密码遮罩模式显示（不显示明文）
- 输入框下方会实时校验格式：必须以 `sk-` 开头，后跟 32 位字母和数字
- 格式正确后点击「测试 API」按钮验证密钥有效性
- 测试通过后会弹出余额详情提示框，确定按钮变为可用

> **必须通过 API 测试**，点击确定按钮方可保存。若密钥为空（不需要显示），可直接保存。

### 更新间隔（秒）

- 默认值：**60** 秒
- 范围：1 ~ 31536000
- 每隔设定秒数自动查询一次 DeepSeek API 余额
- 更新间隔必须大于请求超时时间，否则无法保存

### 请求超时（秒）

- 默认值：**10** 秒
- 范围：3 ~ 60 秒
- API 请求的最大等待时间，超时后视为请求失败
- 建议根据网络状况调整，网络较差时可适当增加

### 历史记录数量

- 默认值：**1000** 条
- 范围：0 ~ 10000（设为 0 表示不记录历史）
- 仅当余额发生实际变动（差值 > 0.001）时才新增记录，避免冗余
- 超出上限的旧记录会自动清除

### 自动刷新

- 默认勾选
- 勾选后历史记录列表每秒自动刷新
- 点击列表任意位置可取消自动刷新

### 显示语言

- 默认值为「自动」（根据系统语言自动检测）
- 支持 12 种语言：简体中文、繁體中文、日本語、English、Deutsch、עברית、Magyar、Italiano、Polski、Português (Brasil)、Русский、Türkçe
- 切换语言后对话框即时生效，但需点击「确定」按钮才会持久化保存

### 清除历史

- 点击「清除历史」按钮并确认后，清空所有历史记录
- 清空操作不可撤销

## 编译

### 环境要求

- Visual Studio 2026（v145 工具集）
- Windows 10 SDK 或更高版本
- 必须启用 `/utf-8` 编译选项（中文源码所必需）

### 快速编译（推荐）

```batch
build.bat
```

自动根据当前系统架构编译 Release 配置，并将生成的 DLL 复制到 `C:\TrafficMonitor\plugins\`。

> **注意**：请根据 TrafficMonitor 的实际安装路径修改 `build.bat` 中的 `PLUGINDIR` 变量。

### 手动编译

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### 输出文件

| 平台 | 配置   | 输出文件                             |
|------|--------|--------------------------------------|
| x86  | Debug  | `Debug\DeepSeekDeskBand.dll`         |
| x86  | Release| `Release\DeepSeekDeskBand.dll`       |
| x64  | Debug  | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64  | Release| `x64\Release\DeepSeekDeskBand64.dll` |

## 隐私与数据安全

本插件高度重视用户隐私和数据安全：

- **API 密钥**：使用 Windows DPAPI 加密存储于本地配置文件 `DeepSeekDeskBand.dat`，仅限当前 Windows 用户访问和解密。密钥绝不会上传至任何第三方。
- **余额数据**：仅通过 HTTPS 从 DeepSeek 官方 API（`api.deepseek.com`）获取，不经过任何中间服务器。
- **历史记录**：余额变动记录使用 DPAPI 加密存储于本地文件 `DeepSeekDeskBand_History.dat`，仅限当前 Windows 用户访问。
- **网络请求**：仅向 `api.deepseek.com` 发送 HTTPS 请求（Authorization: Bearer Token），不请求其他域名。
- **无数据收集**：本插件不收集、不上传任何用户数据或使用统计信息。
- **图标缓存**：仅在首次打开设置对话框时从 `www.deepseek.com` 下载一次图标（favicon.ico），缓存于插件配置目录。

## 技术说明

- 插件接口：TrafficMonitor API v7
- 无 MFC 依赖，纯 Win32 C++ 实现
- 单例模式管理插件实例
- WinHTTP HTTPS 客户端
- 余额变动检测阈值 > 0.001，减少冗余记录

## 许可证

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

本项目基于 Mulan PSL v2 许可证开源。你可以在遵守 Mulan PSL v2 条款的前提下使用本软件。许可证副本见：http://license.coscl.org.cn/MulanPSL2

本软件按「现状」提供，不提供任何形式的明示或默示担保，包括但不限于适销性、特定用途适用性及不侵权的保证。
