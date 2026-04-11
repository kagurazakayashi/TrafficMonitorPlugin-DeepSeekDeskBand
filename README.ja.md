# DeepSeek 残高アシスタント (DeepSeekDeskBand)

![screenshot](screenshot.png)

[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) デスクトップアシスタントプラグインです。タスクバーウィンドウに DeepSeek 公式サイトの残高情報をリアルタイム表示し、残高変動履歴にも対応します。

[English](README.md) | [简体中文](README.zh-Hans.md) | [繁體中文](README.zh-Hant.md) | 日本語 | [Deutsch](README.de.md) | [עברית](README.he.md) | [Magyar](README.hu.md) | [Italiano](README.it.md) | [Polski](README.pl.md) | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## 機能

- DeepSeek 公式 API 残高の定期照会
- TrafficMonitor のタスクバーおよびメインウィンドウに残高数値と通貨を表示
- 残高変動履歴（自動増分保存、上限調整可能）
- 多言語対応（システム言語の自動検出または手動選択）
- 設定ファイル（API キーを含む）と履歴は Windows DPAPI で暗号化され、現在の Windows ユーザーのみが復号可能

## インストール

1. [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases) から対応バージョンの DLL をダウンロードします：
   - x86 版 TrafficMonitor → `DeepSeekDeskBand.dll`
   - x64 版 TrafficMonitor → `DeepSeekDeskBand64.dll`
2. DLL を TrafficMonitor プログラムディレクトリ内の `plugins` フォルダにコピーします
3. TrafficMonitor を再起動します
4. タスクバー空き領域を右クリックし、「表示設定」を選択、「DeepSeek 残高アシスタント」にチェックを入れると表示されます

> **注意**：DLL は必ず `plugins` サブフォルダ内に配置してください。他のディレクトリでは読み込まれません。

## 表示内容について

プラグインの表示内容は 2 つの部分で構成されます：

- **左側ラベルテキスト**：TrafficMonitor の「オプション」→「タスクバーウィンドウ設定」→「表示テキスト設定」で変更できます。
- **右側数値テキスト**：`残高数値 通貨` の形式（例：`100.00 CNY`）で表示されます。残高データは定期的に自動更新されます。

TrafficMonitor 右クリックメニューの「表示項目」サブメニューでは、プラグインの表示名が言語設定に応じて変化します（日本語環境では「DeepSeek 残高アシスタント」）。

## プラグイン設定ウィンドウを開く

TrafficMonitor のタスクバーウィンドウまたはメインウィンドウを右クリックし、右クリックメニューから「プラグイン管理」を選択、本プラグインを右クリックして「オプション」を選択すると、設定ダイアログが開きます。設定ダイアログでは API キーなどのオプションを設定したり、履歴を確認したりできます。

## 各設定の説明

### DeepSeek API キー

- 入力欄にあなたの [DeepSeek API Key](https://platform.deepseek.com/api_keys) を入力します
- キーはパスワードマスクモードで表示されます（平文表示なし）
- 入力欄の下にリアルタイムで形式検証が表示されます：`sk-` で始まり、その後に 32 桁の英数字が続く必要があります
- 形式が正しければ「API テスト」ボタンをクリックしてキーの有効性を検証します
- テストに成功すると残高詳細のダイアログが表示され、確定ボタンが有効になります

> **必ず API テストに合格**してから確定ボタンをクリックしてください。キーが空（表示不要）の場合は、そのまま保存できます。

### 更新間隔（秒）

- デフォルト値：**60** 秒
- 範囲：1 ～ 31536000
- 設定した秒数ごとに DeepSeek API 残高を自動照会します
- 更新間隔はリクエストタイムアウトより大きくする必要があります。そうでないと保存できません

### リクエストタイムアウト（秒）

- デフォルト値：**10** 秒
- 範囲：3 ～ 60 秒
- API リクエストの最大待機時間です。タイムアウトした場合はリクエスト失敗とみなします
- ネットワーク状況に応じて調整することを推奨します。回線が悪い場合は適宜増やしてください

### 履歴件数

- デフォルト値：**1000** 件
- 範囲：0 ～ 10000（0 に設定すると履歴を記録しません）
- 残高に実際の変動（差分 > 0.001）があった場合のみ新規レコードを追加し、冗長な記録を回避します
- 上限を超えた古いレコードは自動的に削除されます

### 自動更新

- デフォルトでチェック有効
- チェック有効時、履歴リストが毎秒自動更新されます
- リストの任意の場所をクリックすると自動更新が解除されます

### 表示言語

- デフォルト値は「自動」（システム言語に応じて自動検出）
- 12 言語対応：简体中文、繁體中文、日本語、English、Deutsch、עברית、Magyar、Italiano、Polski、Português (Brasil)、Русский、Türkçe
- 言語切り替えはダイアログ上で即時反映されますが、永続保存するには「確定」ボタンをクリックする必要があります

### 履歴消去

- 「履歴消去」ボタンをクリックして確認すると、すべての履歴レコードが消去されます
- 消去操作は取り消せません

## ビルド

### 環境要件

- Visual Studio 2026（v145 ツールセット）
- Windows 10 SDK 以降
- `/utf-8` コンパイルオプションの有効化が必須（中国語ソースコードに必要）

### クイックビルド（推奨）

```batch
build.bat
```

現在のシステムアーキテクチャに応じて Release 構成を自動ビルドし、生成された DLL を `C:\TrafficMonitor\plugins\` にコピーします。

> **注意**：TrafficMonitor の実際のインストールパスに合わせて `build.bat` 内の `PLUGINDIR` 変数を変更してください。

### 手動ビルド

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### 出力ファイル

| プラットフォーム | 構成    | 出力ファイル                          |
|------------------|---------|---------------------------------------|
| x86              | Debug   | `Debug\DeepSeekDeskBand.dll`         |
| x86              | Release | `Release\DeepSeekDeskBand.dll`       |
| x64              | Debug   | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64              | Release | `x64\Release\DeepSeekDeskBand64.dll` |

## プライバシーとデータセキュリティ

本プラグインはユーザーのプライバシーとデータセキュリティを重視しています：

- **API キー**：Windows DPAPI を使用してローカル設定ファイル `DeepSeekDeskBand.dat` に暗号化保存され、現在の Windows ユーザーのみがアクセス・復号できます。キーが第三者にアップロードされることは決してありません。
- **残高データ**：DeepSeek 公式 API（`api.deepseek.com`）からのみ HTTPS 経由で取得し、中間サーバーを一切経由しません。
- **履歴**：残高変動記録は DPAPI で暗号化され、ローカルファイル `DeepSeekDeskBand_History.dat` に保存されます。現在の Windows ユーザーのみがアクセスできます。
- **ネットワークリクエスト**：`api.deepseek.com` に対してのみ HTTPS リクエスト（Authorization: Bearer Token）を送信し、他のドメインにはリクエストしません。
- **データ収集なし**：本プラグインはユーザーデータや利用統計情報を一切収集・アップロードしません。
- **アイコンキャッシュ**：設定ダイアログを初めて開いたときにのみ、`www.deepseek.com` からアイコン（favicon.ico）を一度だけダウンロードし、プラグイン設定ディレクトリにキャッシュします。

## 技術情報

- プラグインインターフェース：TrafficMonitor API v7
- MFC 非依存、純粋な Win32 C++ 実装
- シングルトンパターンによるプラグインインスタンス管理
- WinHTTP HTTPS クライアント
- 残高変動検出閾値 > 0.001、冗長な記録を削減

## ライセンス

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

本プロジェクトは Mulan PSL v2 ライセンスの下でオープンソース公開されています。Mulan PSL v2 の条項に従う限り、本ソフトウェアを使用することができます。ライセンスの副本はこちらをご覧ください：http://license.coscl.org.cn/MulanPSL2

本ソフトウェアは「現状有姿」で提供され、明示または黙示を問わず、商品性、特定目的への適合性、および権利非侵害の保証を含む、いかなる種類の保証も行いません。
