# DeepSeekDeskBand

TrafficMonitor 插件： DeepSeek 余额显示助手。

在 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 的任务栏窗口和主窗口中显示 DeepSeek 余额信息。

## 编译

### 环境要求

- Visual Studio 2026（v145 工具集）
- Windows 10 SDK 或更高版本

### 构建

```batch
build.bat
```

自动编译 x86 和 x64 Release 配置，并将生成的 DLL 复制到 `C:\TrafficMonitor\plugins\`。

也可手动使用 MSBuild：

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### 输出

| 平台 | 输出文件 |
|------|----------|
| x86  | `Release\DeepSeekDeskBand.dll` |
| x64  | `x64\Release\DeepSeekDeskBand64.dll` |

## 安装

1. 将对应版本的 DLL 复制到 TrafficMonitor 程序目录下的 `plugins` 文件夹
2. 重启 TrafficMonitor
3. 在任务栏右键菜单中选择"显示设置"，勾选"DeepSeekDeskBand"即可显示

> x86 版 TrafficMonitor 使用 `DeepSeekDeskBand.dll`，x64 版使用 `DeepSeekDeskBand64.dll`。

## 技术说明

- 插件接口：TrafficMonitor API v7
- 无 MFC 依赖，纯 Win32 C++ 实现
- 采用单例模式管理插件实例

## 许可证

```LICENSE
Copyright (c) 2026 KagurazakaYashi(KagurazakaMiyabi)
TrafficMonitorPlugin-DeepSeekDeskBand is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details.
```
