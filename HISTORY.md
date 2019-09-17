# 更新履歴

## プログラム更新履歴

#### 2019/09/12（Version 0.1.18）

FIDO認証器管理ツールを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)

修正点は以下になります。
- ヘルスチェック機能を強化（[#202](https://github.com/diverta/onecard-fido/issues/202) ご参照）<br>
「BLE経由でのCTAP2ヘルスチェック」「USB経由でのU2Fヘルスチェック」を追加しています。
- Windows版管理ツールで、複数デバイスとペアリング済み時に接続エラーが発生してしまう不具合を解消（[#239](https://github.com/diverta/onecard-fido/issues/239) ご参照）

また、管理ツール関連のドキュメントを最新情報に更新しております。

#### 2019/08/29（Version 0.1.17）

FIDO認証器管理ツールを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)

修正点は以下になります。
- Windows版管理ツールでBLEペアリングエラーが発生してしまう不具合を解消（[#233](https://github.com/diverta/onecard-fido/issues/233) ご参照）

#### 2019/08/27（Version 0.1.16）

FIDO認証器管理ツール、およびファームウェアを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)
- [nRF52840版 FIDO2認証器](nRF5_SDK_v15.3.0)

修正点は以下になります。
- 鍵・証明書がない状態でCTAP2ヘルスチェックを実行すると、ハングしてしまう不具合を解消（[#230](https://github.com/diverta/onecard-fido/issues/230) ご参照）

#### 2019/08/19（Version 0.1.15）

FIDO認証器管理ツール、およびファームウェアを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)
- [nRF52840版 FIDO2認証器](nRF5_SDK_v15.3.0)

主な修正点は以下になります。
- PINGテスト機能を追加（[#224](https://github.com/diverta/onecard-fido/pull/224)、[#225](https://github.com/diverta/onecard-fido/pull/225) ご参照）
- ボタン長押し判定方法の見直し（[#222](https://github.com/diverta/onecard-fido/pull/222) ご参照）
- 一部ソースコードのリファクタリング（[#215](https://github.com/diverta/onecard-fido/pull/215) ご参照）

#### 2019/07/29（Version 0.1.14）

FIDO認証器管理ツールを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)

主な修正点は以下になります。
- Flash ROM情報取得機能を追加（#213、#214 ご参照 ）
- ヘルスチェック時に、管理ツールがハングするを防止する（#211 ご参照）

#### 2019/06/13

[nRF52840版 FIDO2認証器](nRF5_SDK_v15.3.0)を、最新のSDKバージョン「v15.3.0」に移行いたしました。

なお、[移行前のプログラム](nRF5_SDK_v15.2.0)は、そのまま残してあります。<br>
（今後はメンテナンスする予定はございません。ご容赦ください）

また、管理ツールの修正はございません。

#### 2019/05/27（Version 0.1.13）

以下のプログラムを修正しました。<br>

- [nRF52840版 FIDO2認証器](nRF5_SDK_v15.2.0)
- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)

主な修正点は以下になります。
- CTAP2ヘルスチェック機能を追加（`hmac-secret`検証機能付き）[注1]
- PIN認証で使用されるPINコードをメンテナンスする機能を追加
- CTAP2のBLEトランスポート対応[注2]
- U2F／CTAP2で使用する鍵・証明書管理を、USB HID経由で実行できるよう修正
- USB HIDサービスとBLEペリフェラルサービスが同居できないよう修正[注3]

[注1] `hmac-secret`検証機能＝ログイン実行ごとに、ブラウザーと認証器でやり取りされる暗号（Salt）の整合性をチェックすることにより、ユーザーや認証器の成り替わり・成りすましを抑止する機能<br>
[注2] Android Chromeは標準でWebAuthn対応していますが、現時点ではU2F認証が実行されます。<br>
[注3] USB HIDサービスと、BLE<b>セントラル</b>サービスは同居できるように実装しています。

#### 2018/09/27（Version 0.1.6）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- U2F Helper制作にともない、不要となった機能「Chrome設定」(Chrome Native Messaging) を、U2F管理ツール画面から削除
- C++で制作したWIndows版U2F管理コマンド（U2FMaintenanceToolCMD.exe）を、C#の画面アプリ内に移植しました（Issue #72）

U2F Helper（ヘルパーアプリ）は、別途製作した[U2F USB HIDデバイス（ヘルパーデバイス）](U2FHIDDevice/readme.md)と連携し、One Cardを使ったU2F認証を、Chromeブラウザーの標準機能で実行するために必要なプログラムです。

- [macOS版U2F Helper](U2FMaintenanceTool/macOSApp/U2FHelper.pkg)
- [Windows版U2F Helper](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

具体的な使用方法は、別途手順書[「Googleアカウントを使ったテスト手順」](Usage/GOOGLEACCTEST.md)をご参照ください。

#### 2018/09/03

[ヘルパーデバイス（U2F USB HIDデバイス）](U2FHIDDevice/readme.md)を制作しました。<br>
ヘルパーアプリ（U2F Helper）との組み合わせで、One Cardを使用したChromeブラウザーでのU2F認証が可能となりました。

具体的な使用方法は、別途手順書[「Googleアカウントを使ったテスト手順」](Usage/GOOGLEACCTEST.md)をご参照ください。

#### 2018/08/21（Version 0.1.5）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- U2F Helperの新規制作（[Issue #72](https://github.com/diverta/onecard-fido/issues/72)）に伴う機能追加／修正

U2F Helper（ヘルパーアプリ）は、現在製作中のU2F HIDデバイス（ヘルパーデバイス）と連携し、One Cardを使ったU2F認証を、Chromeブラウザーの標準機能で実行するために必要なプログラムです。

- [macOS版U2F Helper](U2FMaintenanceTool/macOSApp/U2FHelper.pkg)
- [Windows版U2F Helper](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

#### 2018/06/15（Version 0.1.4）

以下のプログラムを修正しました。<br>

- [BLE U2Fサービス](nRF5_SDK_v13.0.0)
- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- U2F管理ツールのヘルスチェックにより、以前の認証に成功したサイトのトークンカウンターが上書きされてしまう不具合を解消（[Issue #63](https://github.com/diverta/onecard-fido/issues/63)）
- VS2015でビルド時の警告を解消（[Issue #57](https://github.com/diverta/onecard-fido/issues/57)）
- macOS版U2F管理ツールのペアリング時、U2F_PINGを使用しないよう修正（[Issue #59](https://github.com/diverta/onecard-fido/issues/59)）

#### 2018/06/05

Chrome 67以降、パッケージ済みエクステンションが、Chromeウェブストアで取得したもの以外インストールできなくなってしまったようです。<br>
そのため、（パッケージ済みエクステンション導入を前提とした）[U2Fローカルテストサーバーの手順](U2FDemoServer/README.md)等を更新しております。

具体的には従来通り、パッケージ化されていないエクステンションを使用するように、実装および手順を修正しております。<br>
何卒ご容赦ください。

#### 2018/05/30

U2Fローカルテストサーバーを約半年ぶりにアップデートしました。<br>

- [U2FDemoServer](U2FDemoServer)
- [手順書などのドキュメント](U2FDemoServer/README.md)

下記の通り、大幅にアップデートしています。

- 従来の[Javaベースのライブラリーサーバー](Research/u2f-test-server)から、[Pythonベースのライブラリーサーバー](U2FDemoServer/python-u2flib-server)に変更<br>
[Yubico社提供のPythonライブラリーサーバー](https://developers.yubico.com/python-u2flib-server/)を使用し、[サンプル](https://github.com/Yubico/python-u2flib-server/blob/master/examples/u2f_server.py)を若干拡張して[サーバープログラム](U2FDemoServer/python-u2flib-server/u2f_server.py)を作成しました。<br>

- [Chrome U2Fエクステンション](U2FDemoServer/u2f-chrome-extension.crx)をパッケージ化<br>
この結果、エクステンションIDが以前のものから変更になっております。

- 上記変更後のエクステンションIDを反映した、[専用のU2F管理ツール](U2FDemoServer/U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)を用意<br>
ただし機能上の変更がない為、バージョンは、0.1.3のままとしております。

- 関連トピックス<br>[Issue #22](https://github.com/diverta/onecard-fido/issues/22), [Pull request #62](https://github.com/diverta/onecard-fido/pull/62) ご参照

#### 2018/05/09（Version 0.1.3）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- Windows版U2F管理コマンドのコードを整理し、Visual Studio 2015ソリューションを再構築しました。（[Pull request #51](https://github.com/diverta/onecard-fido/pull/51)）
- Windows版U2F管理ツールの処理中、画面がフリーズしないようにしました。（[Issue #52](https://github.com/diverta/onecard-fido/issues/52)）
- ヘルスチェック時、U2F Authenticateの所在確認待ちである旨をガイダンスさせるようにしました。（[Issue #55](https://github.com/diverta/onecard-fido/issues/55)）
- U2F管理ツールから、One Card側のペアリング情報を消去できないようにしました。（[Issue #56](https://github.com/diverta/onecard-fido/issues/56)）

#### 2018/05/02（Version 0.1.2）

以下のプログラムを修正しました。<br>

- [BLE U2Fサービス](nRF5_SDK_v13.0.0)
- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- U2F管理ツールからペアリングができるようにする（[Issue #23](https://github.com/diverta/onecard-fido/issues/23)）

#### 2018/04/25（Version 0.1.1）

以下のプログラムを修正しました。<br>

- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)

修正点は以下になります。
- ディスカバー・タイムアウトを検知できるようにする（[Issue #29](https://github.com/diverta/onecard-fido/issues/29)）
- 完了通知前にBLE接続を切断させるようにする（[Issue #36](https://github.com/diverta/onecard-fido/issues/36)）
- アプリ自身による切断時、接続リトライが行われないようにする（[Issue #37](https://github.com/diverta/onecard-fido/issues/37)）
- 予期しない切断発生時のタイムアウト誤検知を抑止する（[Issue #46](https://github.com/diverta/onecard-fido/issues/46)）
- スキャンタイムアウト時にスキャンを停止させるようにする（[Issue #47](https://github.com/diverta/onecard-fido/issues/47)）
- 接続リトライ上限到達時にメッセージを表示させるようにする（[Issue #48](https://github.com/diverta/onecard-fido/issues/48)）

#### 2018/04/18（Version 0.1.0）

以下のプログラムを修正しました。<br>

- [BLE U2Fサービス](nRF5_SDK_v13.0.0)
- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

修正点は以下になります。
- nRF52において、NRF_ERROR_INVALID_STATEにおけるレスポンス実行を回避（[Issue #40](https://github.com/diverta/onecard-fido/issues/40)）
- U2F管理ツールのバージョンが確認できるようにする（[Issue #43](https://github.com/diverta/onecard-fido/issues/43)）

本リリースから便宜的にバージョン番号を付与させていただきたく存じます。

#### 2018/04/10

nRF52側のFIDO機能でエラー発生時、発生箇所／原因がU2F管理ツールのログを参照して特定できるよう、以下のプログラムを修正しました。

- [BLE U2Fサービス](nRF5_SDK_v13.0.0)
- [macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)
- [Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)

あわせて、[FIDO機能のステータスワード一覧](nRF5_SDK_v13.0.0/FIDOSWLIST.md)を別途まとめました。

#### 2018/04/03

[macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)を更新しました。<br>
以下の不具合を修正しております。
- U2F Authenticate実行後、Chromeブラウザーがハング (Issue #25)
- 再起動したChromeブラウザーで、U2F Authenticateが再度失敗 (Issue #26)
- BLEリクエストが送信されないことがある (Issue #27)
- BLEリクエストは送信されるが、２件目以降のフレームが受信されないことがある (Issue #28)
- macOSで「Bluetooth: Off」時にChromeブラウザーがハング (Issue #30)
- U2F Registerでエラーレスポンスを受信時、Chromeブラウザーがハング (Issue #32)

これに伴い、[U2Fデモサイト用のChromeエクステンション](U2FMaintenanceTool/u2f-chrome-extension.zip)も同時に更新しました。

#### 2018/03/15

[Windows版U2F管理ツール](U2FMaintenanceTool/WindowsExe/U2FMaintenanceToolWin.zip)を更新しました。<br>
秘密鍵・自己署名証明書の作成から、One Cardへのインストールまでの作業を、Windows版U2F管理ツールを使って出来るようになりました。<br>
手順書は以下になります。
- [One Cardペアリング手順](Usage/PAIRING.md)<br>
One Cardに、BLEを使用して秘密鍵・証明書をインストールするためには、まず最初にペアリングが必要になります。

- [鍵・証明書インストール手順](Usage/INSTALL_WINDOWS.md)<br>
秘密鍵・自己署名証明書の作成およびインストール手順についての説明になります。

- [U2Fデモサイトを使ったテスト手順](Usage/DEMOSITETEST.md)<br>
[U2Fデモサイト](https://crxjs-dot-u2fdemo.appspot.com/)を使用して、One CardのU2F機能をテストする手順です。

#### 2018/03/08

[OpenSSL未導入のmacOSで、U2F管理ツールが起動時にクラッシュしてしまう障害](https://github.com/diverta/onecard-fido/issues/20)を解消いたしました。

#### 2018/03/07

[macOS版U2F管理ツール](U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg)を更新しました。<br>
秘密鍵・自己署名証明書の作成から、One Cardへのインストールまでの作業を、macOS版U2F管理ツールを使って出来るようになりました。<br>
手順書は以下になります。
- [One Cardペアリング手順](Usage/PAIRING.md)<br>
One Cardに、BLEを使用して秘密鍵・証明書をインストールするためには、まず最初にペアリングが必要になります。

- [鍵・証明書インストール手順](Usage/INSTALL.md)<br>
秘密鍵・自己署名証明書の作成およびインストール手順についての説明になります。<br>
（以前の手順書にあった、nrfutilやOpenSSL等の追加インストールは、不要になりました）
