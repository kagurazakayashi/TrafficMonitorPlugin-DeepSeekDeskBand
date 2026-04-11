# DeepSeek 餘額助手 (DeepSeekDeskBand)

![screenshot](screenshot.png)

[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 桌面小工具外掛程式。在工作列視窗中即時顯示 DeepSeek 官網餘額資訊，支援餘額變動歷史記錄。

[English](README.md) | [简体中文](README.zh-Hans.md) | 繁體中文 | [日本語](README.ja.md) | [Deutsch](README.de.md) | [עברית](README.he.md) | [Magyar](README.hu.md) | [Italiano](README.it.md) | [Polski](README.pl.md) | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## 功能特色

- 定時查詢 DeepSeek 官網 API 餘額
- 在 TrafficMonitor 的工作列和主視窗中顯示餘額數值與幣別
- 餘額變動歷史記錄（自動增量儲存，上限可調）
- 支援多語言（自動偵測系統語言或手動選擇）
- 設定檔（含 API 金鑰）與歷史記錄使用 Windows DPAPI 加密，僅限目前 Windows 使用者解密

## 安裝

1. 從 [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases) 下載對應版本的 DLL：
   - x86 版 TrafficMonitor → `DeepSeekDeskBand.dll`
   - x64 版 TrafficMonitor → `DeepSeekDeskBand64.dll`
2. 將 DLL 複製到 TrafficMonitor 程式目錄下的 `plugins` 資料夾
3. 重新啟動 TrafficMonitor
4. 在工作列空白區域按右鍵，選擇「顯示設定」，勾選「DeepSeek 餘額助手」即可顯示

> **注意**：DLL 必須放在 `plugins` 子資料夾內，放在其他目錄無法載入。

## 顯示內容說明

外掛程式顯示的內容由兩部分組成：

- **左側標籤文字**：可在 TrafficMonitor 的「選項」→「工作列視窗設定」→「顯示文字設定」中修改。
- **右側數值文字**：顯示 `餘額數值 幣別`，例如 `100.00 CNY`。餘額資料定時自動更新。

在 TrafficMonitor 右鍵選單的「顯示項目」子選單中，外掛程式的顯示名稱隨語言設定變化（如繁體中文環境下為「DeepSeek 餘額助手」）。

## 開啟外掛程式設定視窗

在 TrafficMonitor 的工作列視窗或主視窗上按右鍵，於右鍵選單中選擇「外掛程式管理」，再於本外掛程式上按右鍵並選擇「選項」，即可開啟設定對話方塊。在設定對話方塊中可設定 API 金鑰等選項，亦可檢視歷史記錄。

## 各項設定說明

### DeepSeek API 金鑰

- 在輸入方塊中填入你的 [DeepSeek API Key](https://platform.deepseek.com/api_keys)
- 金鑰以密碼遮罩模式顯示（不顯示明碼）
- 輸入方塊下方會即時校驗格式：必須以 `sk-` 開頭，後接 32 位字母和數字
- 格式正確後點選「測試 API」按鈕驗證金鑰有效性
- 測試通過後會彈出餘額詳情提示方塊，確定按鈕變為可用

> **必須通過 API 測試**，點選確定按鈕方可儲存。若金鑰為空（不需要顯示），可直接儲存。

### 更新間隔（秒）

- 預設值：**60** 秒
- 範圍：1 ~ 31536000
- 每隔設定秒數自動查詢一次 DeepSeek API 餘額
- 更新間隔必須大於請求逾時時間，否則無法儲存

### 請求逾時（秒）

- 預設值：**10** 秒
- 範圍：3 ~ 60 秒
- API 請求的最大等待時間，逾時後視為請求失敗
- 建議根據網路狀況調整，網路較差時可適當增加

### 歷史記錄數量

- 預設值：**1000** 條
- 範圍：0 ~ 10000（設為 0 表示不記錄歷史）
- 僅當餘額發生實際變動（差值 > 0.001）時才新增記錄，避免冗餘
- 超出上限的舊記錄會自動清除

### 自動重新整理

- 預設勾選
- 勾選後歷史記錄列表每秒自動重新整理
- 點選列表任意位置可取消自動重新整理

### 顯示語言

- 預設值為「自動」（根據系統語言自動偵測）
- 支援 12 種語言：简体中文、繁體中文、日本語、English、Deutsch、עברית、Magyar、Italiano、Polski、Português (Brasil)、Русский、Türkçe
- 切換語言後對話方塊即時生效，但需點選「確定」按鈕才會持續儲存

### 清除歷史

- 點選「清除歷史」按鈕並確認後，清空所有歷史記錄
- 清除操作無法復原

## 編譯

### 環境需求

- Visual Studio 2026（v145 工具組）
- Windows 10 SDK 或更高版本
- 必須啟用 `/utf-8` 編譯選項（中文原始碼所必需）

### 快速編譯（建議）

```batch
build.bat
```

自動根據目前系統架構編譯 Release 組態，並將產生的 DLL 複製到 `C:\TrafficMonitor\plugins\`。

> **注意**：請根據 TrafficMonitor 的實際安裝路徑修改 `build.bat` 中的 `PLUGINDIR` 變數。

### 手動編譯

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### 輸出檔案

| 平台 | 組態   | 輸出檔案                             |
|------|--------|--------------------------------------|
| x86  | Debug  | `Debug\DeepSeekDeskBand.dll`         |
| x86  | Release| `Release\DeepSeekDeskBand.dll`       |
| x64  | Debug  | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64  | Release| `x64\Release\DeepSeekDeskBand64.dll` |

## 隱私與資料安全

本外掛程式高度重視使用者隱私和資料安全：

- **API 金鑰**：使用 Windows DPAPI 加密儲存於本機設定檔 `DeepSeekDeskBand.dat`，僅限目前 Windows 使用者存取和解密。金鑰絕不會上傳至任何第三方。
- **餘額資料**：僅透過 HTTPS 從 DeepSeek 官方 API（`api.deepseek.com`）取得，不經過任何中間伺服器。
- **歷史記錄**：餘額變動記錄使用 DPAPI 加密儲存於本機檔案 `DeepSeekDeskBand_History.dat`，僅限目前 Windows 使用者存取。
- **網路請求**：僅向 `api.deepseek.com` 傳送 HTTPS 請求（Authorization: Bearer Token），不請求其他網域。
- **無資料收集**：本外掛程式不收集、不上傳任何使用者資料或使用統計資訊。
- **圖示快取**：僅在首次開啟設定對話方塊時從 `www.deepseek.com` 下載一次圖示（favicon.ico），快取於外掛程式設定目錄。

## 技術說明

- 外掛程式介面：TrafficMonitor API v7
- 無 MFC 依賴，純 Win32 C++ 實作
- 單例模式管理外掛程式執行個體
- WinHTTP HTTPS 用戶端
- 餘額變動偵測閾值 > 0.001，減少冗餘記錄

## 授權條款

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

本專案基於 Mulan PSL v2 授權條款開放原始碼。你可以在遵守 Mulan PSL v2 條款的前提下使用本軟體。授權條款副本見：http://license.coscl.org.cn/MulanPSL2

本軟體按「現狀」提供，不提供任何形式的明示或默示擔保，包括但不限於適銷性、特定用途適用性及不侵權的保證。
