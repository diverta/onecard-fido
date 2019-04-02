# 運用情報

## FIDO2関連の各種手順

現在開発を継続していますが、2019/4/1現在で実装完了している部分について、各種手順を掲載しております。

* [Edgeブラウザーを使用したWebAuthnテスト手順](../Research/FIDO_2_0/EDGETEST.md)<br>
[FIDO2認証器](../nRF5_SDK_v15.2.0)と、最新バージョンのEdgeブラウザーを使用し、WebAuthnユーザー登録／ログインをPIN番号により行う手順を掲載しています。

* [macOS版 FIDO認証器管理ツール](../MaintenanceTool/macOSApp/README.md)<br>
[FIDO2認証器](../nRF5_SDK_v15.2.0)を管理するために必要な[FIDO認証器管理ツール](../MaintenanceTool/README.md)について、プログラムのインストール手順や、鍵・証明書の導入手順をまとめています。

* [Windows版 FIDO認証器管理ツール](../MaintenanceTool/WindowsExe/README.md)<br>
こちらはWindows環境版になります。

## U2F関連の各種手順

FIDO2への移行に伴い、現在開発を終了していますが、履歴として残します。

* [U2F管理ツールインストール手順](TOOL_INSTALL.md) <br>
[One Card BLE U2Fサービス](../nRF5_SDK_v13.0.0)の管理ツールをインストールする手順について掲載しています。

* [鍵・証明書インストール手順（macOS版）](INSTALL.md) <br>
macOS環境での、鍵・証明書作成／インストール手順を掲載しています。

* [鍵・証明書インストール手順（Windows版）](INSTALL_WINDOWS.md) <br>
Windows環境での、鍵・証明書作成／インストール手順を掲載しています。

* [One Cardペアリング手順](PAIRING.md) <br>
ペアリングモード変更／ペアリング実行の手順を掲載しています。

* [U2F Helperインストール手順](HELPER_INSTALL.md) <br>
U2F Helperのインストール手順を掲載しています。

* [Googleアカウントを使ったテスト手順](GOOGLEACCTEST.md)<br>
[Googleアカウントの２段階認証ページ](https://myaccount.google.com/signinoptions/two-step-verification/enroll-welcome)を使用して、One CardのU2F機能（Register/Authenticate）をテストする手順を掲載しています。

* [U2Fデモサイトを使ったテスト手順](DEMOSITETEST.md)<br>
[U2Fデモサイト](https://crxjs-dot-u2fdemo.appspot.com/)を使用して、One CardのU2F機能（Register/Authenticate）をテストする手順を掲載しています。

* [FIDO U2F認証テスト手順](CERTTEST.md) <br>
FIDOアライアンスから提供されている、FIDO U2F認証取得のための事前テストツール（BLECertificationTool.exe）について、実行手順を掲載しています。

## 補足説明

（未作成）

後日、随時追加していく予定です。

## TODO

（未作成）

後日、随時追加していく予定です。
