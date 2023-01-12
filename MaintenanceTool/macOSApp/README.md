# macOS版 FIDO認証器管理ツール

最終更新日：2023/1/12

## 概要
PC環境から、FIDO認証器の動作に必要な各種設定／動作テスト等を行うためのツールです。

#### 動作環境
macOS Sierra (Version 10.12.6)〜macOS Catalina (Version 10.15.7)

## 機能

エンドユーザー向けと、[ベンダー向け](../../MaintenanceTool/macOSApp/DEVTOOL.md)の両タイプを用意しております。

#### エンドユーザー向けの機能

* ペアリング実行／解除要求
* ペアリング情報削除
* PINコード設定
* FIDO認証情報消去
* CTAP2ヘルスチェック実行
* U2Fヘルスチェック実行
* PINGテスト機能
* Flash ROM情報取得機能
* バージョン情報取得機能
* PIV機能設定
* OpenPGP機能設定
* OATH機能設定（最終更新日現在、開発中）
* ファームウェア更新機能
* ログファイル格納ディレクトリー参照機能
* 認証器の時刻設定機能

#### ベンダー向けの機能
エンドユーザー向けの機能に加え、下記機能を追加しています。

* FIDO鍵・証明書インストール／削除
* ブートローダーモード遷移機能
* ファームウェアリセット機能

#### 画面イメージ
<img src="../assets/0001.jpg" width="400">

#### 手順書

- <b>[インストール手順](../../MaintenanceTool/macOSApp/INSTALLPRG.md)</b><br>
FIDO認証器管理ツールをmacOS環境にインストールする手順を掲載しています。

- <b>[BLEペアリング手順](../../MaintenanceTool/macOSApp/BLEPAIRING.md)</b><br>
FIDO認証器管理ツールを使用し、PCとFIDO認証器をBLEペアリングする手順について掲載しています。

- <b>[PINコードの設定手順](../../MaintenanceTool/macOSApp/SETPIN.md)</b><br>
FIDO認証器に、PINコード（暗証番号）を設定する手順を掲載しています。

- <b>[FIDO認証情報の消去手順](../../MaintenanceTool/macOSApp/AUTHRESET.md)</b><br>
FIDO認証器から、FIDO認証情報を消去する手順について掲載しています。

- <b>[CTAP2ヘルスチェック実行手順](../../MaintenanceTool/macOSApp/CTAP2HCHECK.md)</b><br>
FIDO認証器のヘルスチェックを実行する手順を掲載しています。

- <b>[PIV機能設定手順](../../MaintenanceTool/macOSApp/PIVSETTING.md)</b><br>
[PIV機能](../../FIDO2Device/MDBT50Q_Dongle/PIVPINLOGIN.md)に必要な各種設定の手順を掲載しています。

- <b>[OpenPGP機能設定手順](../../MaintenanceTool/macOSApp/PGPSETTING.md)</b><br>
[OpenPGP機能](../../CCID/OpenPGP/README.md)に必要な各種設定の手順を掲載しています。

- <b>[ファームウェア更新手順（USB）](../../MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)</b><br>
管理ツールから、MDBT50Q Dongleのファームウェアを更新する手順を掲載しています。

- <b>[管理ツールのログファイル](VIEWLOG.md)</b><br>
FIDO認証器管理ツールから出力されるログファイルについて説明しています。

- <b>[認証器の時刻設定手順](../../MaintenanceTool/macOSApp/RTCC_SETTINGS.md)</b><br>
管理ツールから、MDBT50Q Dongleの現在時刻を設定する手順について掲載しています。
