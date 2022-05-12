# Windows版 FIDO認証器開発ツール

最終更新日：2022/5/12

## 概要
PC環境から、FIDO認証器の動作に必要な各種設定／動作テスト等を行う、ベンダー向けのツールです。

#### 動作環境
Windows 10（64bit、November 2019 Update以降のバージョン）

#### 実行にあたってのご注意

Windows 10のバージョン「Windows 10 November 2019 Update」以降においては、管理者として実行されていないプログラムの場合、FIDOデバイスとの直接的なUSB通信ができない仕様となったようです。<br>
このため、FIDO認証器開発ツールでは、管理者として実行させる前提としております。

FIDO認証器開発ツールを「管理者として実行」するためには、プログラム「FIDO Authenticator Development Tool」のアイコンを右クリックし、メニューから「管理者として実行」を選択します。

<img src="assets08/0002.jpg" width="500">

FIDO認証器開発ツールを「管理者として実行」しなかった場合、下記のようなエラーメッセージがポップアップ表示されます。

<img src="assets08/0003.jpg" width="200">

## 機能
* FIDO鍵・証明書設定
* ログファイル格納ディレクトリー参照機能

#### 画面イメージ
<img src="assets08/0001.jpg" width="400">

#### 手順書

- <b>[インストール手順](../../MaintenanceTool/WindowsExe/DEVTOOLINST.md)</b><br>
FIDO認証器開発ツールをWindows環境にインストールする手順を掲載しています。

- <b>[鍵・証明書の導入手順](../../MaintenanceTool/WindowsExe/ATTESTATION.md)</b><br>
FIDO認証器に、鍵・証明書をインストールする手順を掲載しています。

- <b>[開発ツールのログファイル](../../MaintenanceTool/WindowsExe/DEVTOOLLOG.md)</b><br>
FIDO認証器開発ツールから出力されるログファイルについて説明しています。
