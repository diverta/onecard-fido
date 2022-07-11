# Windows版 FIDO認証器管理ツール

最終更新日：2022/6/28

## 概要
PC環境から、FIDO認証器の動作に必要な各種設定／動作テスト等を行う、エンドユーザー向けのツールです。

#### 動作環境
Windows 10（64bit、November 2019 Update以降のバージョン）

#### 実行にあたってのご注意

Windows 10のバージョン「Windows 10 November 2019 Update」以降においては、管理者として実行されていないプログラムの場合、FIDOデバイスとの直接的なUSB通信ができない仕様となったようです。<br>
このため、FIDO認証器管理ツールでは、管理者として実行させる前提としております。

FIDO認証器管理ツールを「管理者として実行」するためには、プログラム「FIDO Authenticator Development Tool」のアイコンを右クリックし、メニューから「管理者として実行」を選択します。

<img src="assets04/0007.jpg" width="600">

FIDO認証器管理ツールを「管理者として実行」しなかった場合、下記のようなエラーメッセージがポップアップ表示されます。

<img src="assets04/0008.jpg" width="400">

## 機能
* ペアリング実行／解除
* PINコード設定
* FIDO認証情報消去
* CTAP2ヘルスチェック実行
* U2Fヘルスチェック実行
* コマンドテスト機能
* Flash ROM情報取得機能
* バージョン情報取得機能
* OpenPGP機能設定
* ファームウェア更新機能
* ログファイル格納ディレクトリー参照機能

#### 画面イメージ
<img src="../assets/0002.jpg" width="500">

#### 手順書

- <b>[インストール手順](INSTALLPRG.md)</b><br>
FIDO認証器管理ツールをWindows環境にインストールする手順を掲載しています。

- <b>[BLEペアリング手順](BLEPAIRING.md)</b><br>
FIDO認証器管理ツールを使用し、PCとFIDO認証器をBLEペアリングする手順について掲載しています。

- <b>[PINコードの設定手順](SETPIN.md)</b><br>
FIDO認証器に、PINコード（暗証番号）を設定する手順を掲載しています。

- <b>[FIDO認証情報の消去手順](../../MaintenanceTool/WindowsExe/AUTHRESET.md)</b><br>
FIDO認証器から、FIDO認証情報を消去する手順について掲載しています。

- <b>[CTAP2ヘルスチェック実行手順](CTAP2HCHECK.md)</b><br>
FIDO認証器のヘルスチェックを実行する手順を掲載しています。

- <b>[OpenPGP機能設定手順](PGPSETTING.md)</b><br>
[OpenPGP機能](../../CCID/OpenPGP/README.md)に必要な各種設定の手順を掲載しています。

- <b>[ファームウェア更新手順（BLE）](UPDATEFW_BLE.md)</b><br>
管理ツールから、Zephyrプラットフォームを使用したファームウェアを更新する手順を掲載しています。

- <b>[管理ツールのログファイル](VIEWLOG.md)</b><br>
FIDO認証器管理ツールから出力されるログファイルについて説明しています。
