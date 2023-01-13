# macOS版 FIDO認証器管理ツール（ベンダー向け）

最終更新日：2023/1/12

## 概要
PC環境から、FIDO認証器の動作に必要な各種設定／動作テスト等を行うためのツールです。

#### 動作環境
macOS Sierra (Version 10.12.6)〜macOS Catalina (Version 10.15.7)

## 機能
[エンドユーザー向けの機能](README.md)に加え、下記機能を追加しています。

* FIDO鍵・証明書インストール／削除
* ブートローダーモード遷移機能
* ファームウェアリセット機能

#### 画面イメージ
<img src="assets08/0001.jpg" width="400">

#### 手順書

- <b>[インストール手順](../../MaintenanceTool/macOSApp/DEVTOOLINST.md)</b><br>
ベンダー向けのFIDO認証器管理ツールをmacOS環境にインストールする手順について掲載しています。

- <b>[鍵・証明書の導入手順](../../MaintenanceTool/macOSApp/ATTESTATION.md)</b><br>
FIDO認証器に、鍵・証明書をインストールする手順について掲載しています。

- <b>[ブートローダーモード遷移手順](../../MaintenanceTool/macOSApp/BOOTLOADERMODE.md)</b><br>
FIDO認証器のファームウェアを手動インストールする際、ブートローダーモードに遷移させる手順について掲載しています。

- <b>[認証器のリセット手順](../../MaintenanceTool/macOSApp/ATTESTATION.md)</b><br>
FIDO認証器のファームウェアを再始動する手順について掲載しています。

- <b>[管理ツールのログファイル](../../MaintenanceTool/macOSApp/DEVTOOLLOG.md)</b><br>
ベンダー向けのFIDO認証器管理ツールから出力されるログファイルについて説明しています。
