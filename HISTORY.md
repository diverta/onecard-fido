# 更新履歴

## プログラム更新履歴

#### 2020/09/22

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.32）](https://github.com/diverta/onecard-fido/tree/bug-FIDO2MT-macOS-20200923/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.32）](https://github.com/diverta/onecard-fido/blob/bug-FIDO2MT-Windows-20200923/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。（[#366](https://github.com/diverta/onecard-fido/issues/366) ご参照）
- 管理ツールの「Flash ROM情報取得」機能を実行時、Flash ROM空き容量として表示される％（百分率）が、実態の空き容量と異なる不具合を解消
- 画面上のテキストが隠れて表示されてしまう不具合を解消（macOS版管理ツールのみ）

#### 2020/09/01

MDBT50Q Dongle、およびファームウェアを修正しました。<br>

- <b>[MDBT50Q Dongle（Rev2.1.2）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2DEV-MDBT50Q-Dongle-rev2_1_2/FIDO2Device/MDBT50Q_Dongle/README.md)</b>

- <b>[nRF52840ファームウェア（Version 0.2.11）](https://github.com/diverta/onecard-fido/tree/improve-FIDO2DEV-MDBT50Q-Dongle-rev2_1_2/nRF5_SDK_v15.3.0/firmwares)</b>

MDBT50Q Dongleの主な修正点は以下になります。
- nRF52840アプリケーションに、セキュアIC（[ATECC608A](https://www.mouser.jp/new/microchip/microchip-atecc608a-crypto-devices/)）を組込み、秘密鍵／AESパスワードを読出し不可とする（[#347](https://github.com/diverta/onecard-fido/issues/347) ご参照）
- CCIDインターフェースを装備し、スマートカードエミュレーションを可能とする（[#339](https://github.com/diverta/onecard-fido/issues/339) ご参照。現時点で、業務アプリケーションは未実装）
- [署名検証機能付きUSBブートローダー](https://github.com/diverta/onecard-fido/blob/improve-FIDO2DEV-MDBT50Q-Dongle-rev2_1_2/nRF5_SDK_v15.3.0/firmwares/secure_bootloader/README.md)を採用し、不正ファームウェアの書込みを抑止
- 外形サイズを 5cm x 2cm に縮小

#### 2020/09/07

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.31）](https://github.com/diverta/onecard-fido/blob/bug-FIDO2MT-macOS-20200901/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.31）](https://github.com/diverta/onecard-fido/blob/bug-FIDO2MT-Windows-20200903/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。（[#361](https://github.com/diverta/onecard-fido/issues/361) ご参照）
- 「ファームウェア更新」機能実行時、認証器に導入されているファームウェアのバージョンが、管理ツールに同梱されているバージョンより新しい場合、更新処理が行われないよう修正
- 管理ツール同梱のファームウェア更新イメージファイルを入替え（Version 0.2.11 にアップグレード）
- ファームウェア更新時、MDBT50Q Dongleの基板名（`rev2=PCA10059` or `rev2.1.2=PCA10059_02`）に応じ、更新対象のイメージファイルが自動的に選択されるよう修正

#### 2020/07/20

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.30）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-20200715/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.30）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-20200716/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。（[#302](https://github.com/diverta/onecard-fido/issues/302) ご参照）
- [BLE自動認証機能](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-20200715/FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)有効時のヘルスチェック動作を改善

#### 2020/07/13

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.29a）](https://github.com/diverta/onecard-fido/blob/bug-FIDO2MT-macOS-20200713/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

修正点は以下になります。（[#340](https://github.com/diverta/onecard-fido/issues/340) ご参照）
- macOS版管理ツールのファイル参照ダイアログが表示されない不具合を解消

#### 2020/06/24

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.29）](https://github.com/diverta/onecard-fido/blob/bug-FIDO2MT-macOS-20200623/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.29）](https://github.com/diverta/onecard-fido/blob/bug-FIDO2MT-Windows-20200622/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。（[#334](https://github.com/diverta/onecard-fido/issues/334) ご参照）
- ファームウェア更新中に表示されるポップアップが閉じられない不具合を解消
- インストーラーで更新インストールができない不具合を解消（Windows版のみ）

#### 2020/06/18

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.28）](https://github.com/diverta/onecard-fido/tree/improve-FIDO2MT-macOS-verify-cert/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.28）](https://github.com/diverta/onecard-fido/tree/improve-FIDO2MT-Windows-verify-cert/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。（[#295](https://github.com/diverta/onecard-fido/issues/295) ご参照）
- 鍵・証明書をインストール時、両者の整合性検証を行うようにする機能を追加

#### 2020/06/10

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[nRF52840ファームウェア（Version 0.2.9）](https://github.com/diverta/onecard-fido/blob/impl-nRF52840-CCID-Interface-01/nRF5_SDK_v15.3.0/firmwares/app_dfu_package.0.2.9.zip)</b>

修正点は以下になります。[注1]
- USB CCIDインタフェースを追加実装 [注2]<br>
（[#323](https://github.com/diverta/onecard-fido/issues/323)、[#327](https://github.com/diverta/onecard-fido/pull/327) ご参照）<br>
実装内容につきましては、別ドキュメント<b>「[USB CCIDインターフェース](https://github.com/diverta/onecard-fido/blob/doc-20200610/CCID/ccid_lib/README.md)」</b>をご参照願います。

[注1] FIDO機能には修正はありませんので、管理ツールには同梱していません。[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)に導入の際は<b>「[[開発運用] アプリケーション書込み手順](https://github.com/diverta/onecard-fido/blob/impl-nRF52840-CCID-Interface-01/nRF5_SDK_v15.3.0/APPINSTALL.md)」</b>をご参照願います。<br>
[注2] macOSとのCCID接続機能のみの実装であり、業務アプリケーション（PIV、OpenPGP、OATH等が候補）は未実装となります。今後の作業で実装予定です。


#### 2020/04/15

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェア導入・更新が、[FIDO認証器管理ツール(Windows版)](MaintenanceTool/WindowsExe)により実行できるようになりました（[#300](https://github.com/diverta/onecard-fido/pull/300) ご参照）。<br>
下記のバージョンをご使用願います。

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.27）](https://github.com/diverta/onecard-fido/blob/research-FIDO2MT-Windows-update-firmware-01/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b><br>
MDBT50Q_Dongleの最新ファームウェア（Version 0.2.8）は、Windows版 FIDO認証器管理ツールに同梱されております。

#### 2020/03/31

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェア導入・更新が、[FIDO認証器管理ツール(macOS版)](MaintenanceTool/macOSApp)により実行できるようになりました（[#319](https://github.com/diverta/onecard-fido/pull/319) ご参照）。<br>
下記のバージョンをご使用願います。

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.27）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-cmd-BLmode/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b><br>
MDBT50Q_Dongleの最新ファームウェア（Version 0.2.8）は、macOS版 FIDO認証器管理ツールに同梱されております。

#### ご注意

管理ツールを使用して、MDBT50Q Dongleにファームウェア導入・更新を実行するためには、MDBT50Q Dongleに、新規制作した[「署名機能付きUSBブートローダー」](https://github.com/diverta/onecard-fido/tree/research-FIDO2MT-Windows-update-firmware-01/nRF5_SDK_v15.3.0/firmwares/secure_bootloader)を導入する必要がございます。<br>
手順につきましては<b>[「署名機能付きUSBブートローダー移行手順書」](https://github.com/diverta/onecard-fido/blob/research-FIDO2MT-Windows-update-firmware-01/nRF5_SDK_v15.3.0/firmwares/secure_bootloader/MIGRATION.md)</b>をご参照願います。


#### 2020/03/30

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[nRF52840ファームウェア（Version 0.2.8）](https://github.com/diverta/onecard-fido/blob/improve-nRF52840-jumping-to-BLmode/nRF5_SDK_v15.3.0/firmwares)</b>

修正点は以下になります。
- ブートローダーモード遷移コマンドを追加実装<br>（[#318](https://github.com/diverta/onecard-fido/pull/318) ご参照）<br>
MDBT50Q Dongle基板上の物理的な操作無しで、MDBT50Q Dongleをブートローダーモードに遷移させるためのUSB HIDコマンド「ブートローダーモード遷移コマンド」を新設しています。<br>
「ブートローダーモード遷移コマンド」は、別途新規制作した「[署名機能付きUSBブートローダー](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-jumping-to-BLmode/nRF5_SDK_v15.3.0/firmwares/secure_bootloader)」を導入したMDBT50Q Dongleで、実行可能です。<br>
その他の業務処理につきましては、前回（[Version 0.2.7](https://github.com/diverta/onecard-fido/tree/bug-nRF52840-BLE-auth-scanparam/nRF5_SDK_v15.3.0/firmwares)）から変更は一切ありません。
#### 2020/03/16

[FIDO認証器管理ツール(Windows版)](MaintenanceTool/WindowsExe)を修正しました。<br>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.23.1）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-starting-message/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。
- 2020/03/10におけるファームウェア修正により、認証データのサイズが拡張されたため、Windows版管理ツールのプログラムを修正<br>
（[#312](https://github.com/diverta/onecard-fido/pull/312) ご参照）
- Windows版管理ツールの起動時、管理者として実行されているかどうかのチェック処理を追加<br>
（[#311](https://github.com/diverta/onecard-fido/issues/311)、[#315](https://github.com/diverta/onecard-fido/pull/315) ご参照）

#### 2020/03/11

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>
お手数ですが[FIDO認証器管理ツール(macOS版)](MaintenanceTool/macOSApp)を使用し、<b>[ファームウェア更新手順書](https://github.com/diverta/onecard-fido/blob/bug-nRF52840-BLE-auth-scanparam/MaintenanceTool/macOSApp/UPDATEFIRMWARE.md)</b>をご参照のうえ、[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを更新いただくようお願いします。

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.26）](https://github.com/diverta/onecard-fido/blob/bug-nRF52840-BLE-auth-scanparam/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b><br>
MDBT50Q_Dongleの最新ファームウェア（Version 0.2.7）が、macOS版 FIDO認証器管理ツールに同梱されております。

修正点は以下になります。（[#313](https://github.com/diverta/onecard-fido/issues/313) ご参照）
- [BLEデバイスによる自動認証機能](https://github.com/diverta/onecard-fido/blob/bug-nRF52840-BLE-auth-scanparam/FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)を２度連続して実行時、ユーザー登録時に使用したBLEデバイスのスキャンが失敗する不具合を解消（ファームウェアを修正）
- BLEデバイスによる自動認証機能を無効化した後にヘルスチェックを再実行時、BLEデバイスがスキャンされてしまう不具合を解消（ファームウェアを修正）
- 管理ツール自体に、修正はございません。

#### 2020/03/10

[FIDO認証器管理ツール(macOS版)](MaintenanceTool/macOSApp)、および[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.25）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-authdata-extension/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b><br>
（MDBT50Q_Dongleの最新ファームウェアは、macOS版 FIDO認証器管理ツールに同梱されております）

修正点は以下になります。（[#307](https://github.com/diverta/onecard-fido/issues/307) ご参照）
- [BLEデバイスによる自動認証機能](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-authdata-extension/FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)を有効化した場合は、ユーザー登録時に、必ずユーザー所在確認のためのBLEデバイススキャンが行われるよう、ファームウェアを修正
- 前項修正により、認証データのサイズが拡張されたため、管理ツールのプログラムを修正

#### 2020/2/27

macOS版 FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.24）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-dfufunc-01/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

修正点は以下になります。（[#292](https://github.com/diverta/onecard-fido/issues/292) ご参照）
- FIDO認証器のファームウェア更新を、FIDO認証器管理ツールから実行できるようにする機能を追加

#### 2020/1/15

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[nRF52840ファームウェア（Version 0.2.5）](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-disable-usb-cdc/nRF5_SDK_v15.3.0/firmwares)</b>

修正点は以下になります。（[#299](https://github.com/diverta/onecard-fido/pull/299) ご参照）
- MDBT50Q Dongleの仮想COMポートを閉塞<br>
macOS上で、仮想COMポートドライバー関連の障害が確認されているため、仮想COMポートを使用している機能「[デバイスのRSSI値ログ出力](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-disable-usb-cdc/FIDO2Device/MDBT50Q_Dongle/DEMOFUNC_1.md)」を、為念で閉塞させていただきたく存じます。<br>
（[#260](https://github.com/diverta/onecard-fido/issues/260) ご参照。FIDO機能ではなく、デモ機能として追加したものになります。）

#### 2019/12/25

Windows版 FIDO認証器管理ツールのインストーラーを作成しました。<br>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.23）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-make-installer/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

インストール方法につきましては、<b>[こちらの手順書](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-make-installer/MaintenanceTool/WindowsExe/INSTALLPRG.md)</b>をご参照願います。<br>
なお、ツール本体のプログラム修正はありません。

#### 2019/12/24

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.23）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-view-log-file/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.23）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-view-log-file/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。（[#278](https://github.com/diverta/onecard-fido/issues/278) ご参照）
- 管理ツールに、ログファイルが格納されているディレクトリーを開いて参照できるようにする機能を追加

<b>【ご注意】<br>
このバージョンのFIDO認証器管理ツールを使用される場合は、お手数ですが、合わせて[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを下記バージョンに更新いただきたくお願いします。</b><br>
（鍵・証明書インストール時、管理ツール側で転送内容を暗号化するように修正したため、ファームウェア側が旧バージョンのままだと、転送内容が復号化されず、鍵・証明書インストールが正しく実行できません）

- <b>[nRF52840ファームウェア（Version 0.2.4）](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-decrypt-pkcert/nRF5_SDK_v15.3.0/firmwares)</b>

#### 2019/12/11

FIDO認証器管理ツール、および[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.22）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-encrypt-pkcert/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.22）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-encrypt-pkcert/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

- <b>[nRF52840ファームウェア（Version 0.2.4）](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-decrypt-pkcert/nRF5_SDK_v15.3.0/firmwares)</b>

修正点は以下になります。（[#281](https://github.com/diverta/onecard-fido/issues/281) ご参照）
- 管理ツールによる秘密鍵転送時に、ECDH共通鍵により暗号化（AES256-CBC）を行うよう修正
- 管理ツールで鍵・証明書インストール時、確認ダイアログを表示させるよう修正
- 管理ツールで鍵・証明書削除／インストール時、nRF52840側から発行されたチャネルIDを使用し、USB HID通信を行うよう修正

#### 2019/11/19

FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.21）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-macOS-replace-nslog/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.21）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-refactor-log/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

修正点は以下になります。（[#272](https://github.com/diverta/onecard-fido/issues/272) ご参照）
- macOS管理ツールのログを、コンソールではなく、ログファイルに出力させるよう修正
- macOS版管理ツールと、Windows版管理ツールのログ出力内容を整合させるよう修正
- ログファイルが、ユーザーディレクトリー配下の所定の位置に出力されるよう修正

ログファイルの位置については、下記ドキュメントをご参照願います。
- [macOS版管理ツールのログファイル](MaintenanceTool/macOSApp/VIEWLOG.md)
- [Windows版管理ツールのログファイル](MaintenanceTool/WindowsExe/VIEWLOG.md)

#### 2019/11/11

FIDO認証器管理ツール、および[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.20）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-demofunc-param/MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

- <b>[Windows版 FIDO認証器管理ツール（Version 0.1.20）](https://github.com/diverta/onecard-fido/blob/improve-FIDO2MT-Windows-demofunc-param/MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)</b>

- <b>[nRF52840ファームウェア（Version 0.2.3）](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-demofunc-param/nRF5_SDK_v15.3.0/firmwares)</b>

修正点は以下になります。
- BLEデバイスによる自動認証機能に必要なパラメーター登録機能を追加（[#266](https://github.com/diverta/onecard-fido/issues/266) ご参照）<br>
自動認証機能については、[こちらのドキュメント](https://github.com/diverta/onecard-fido/blob/improve-nRF52840-demofunc-param/FIDO2Device/MDBT50Q_Dongle/DEMOFUNC_2.md)に記載しております。

- BLE PING機能失敗時、エラーメッセージが２回表示される不具合を解消（[#271](https://github.com/diverta/onecard-fido/issues/271) ご参照）

#### 2019/11/05

FIDO認証器管理ツールのログ参照手順書を作成しました。

- <b>[Windows版 FIDO認証器管理ツール ログ参照手順](MaintenanceTool/WindowsExe/VIEWLOG.md)</b>

- <b>[macOS版 FIDO認証器管理ツール ログ参照手順](MaintenanceTool/macOSApp/VIEWLOG.md)</b><br>
（こちらは、ログをmacOSのコンソールではなく、所定のログファイルに出力されるよう、後日改修予定です。）

#### 2019/10/24

[MDBT50Q Dongle](FIDO2Device/MDBT50Q_Dongle)のファームウェアを修正しました。<br>

- <b>[nRF52840ファームウェア（Version 0.2.2）](nRF5_SDK_v15.3.0/firmwares)</b>

修正点は以下になります。
- BLEデバイスによる自動認証機能（デモ機能）を追加（[#263](https://github.com/diverta/onecard-fido/issues/263) ご参照）<br>
操作方法は、[こちらの手順書](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-disable-usb-cdc/FIDO2Device/MDBT50Q_Dongle/DEMOFUNC_2.md)に記載しております。

- BLEデバイスのRSSI値のログ出力機能（デモ機能）を追加（[#262](https://github.com/diverta/onecard-fido/issues/262) ご参照）<br>
操作方法は、[こちらの手順書](https://github.com/diverta/onecard-fido/tree/improve-nRF52840-disable-usb-cdc/FIDO2Device/MDBT50Q_Dongle/DEMOFUNC_1.md)に記載しております。

#### 2019/10/21

macOS版 FIDO認証器管理ツールを修正しました。<br>

- <b>[macOS版 FIDO認証器管理ツール（Version 0.1.19a）](MaintenanceTool/macOSApp/MaintenanceTool.pkg)</b>

修正点は以下になります。
- HID PINGテスト機能でタイムアウト発生時、ポップアップメッセージが表示されない不具合を解消（[#264](https://github.com/diverta/onecard-fido/issues/264)、[#265](https://github.com/diverta/onecard-fido/pull/265) ご参照）

Windows版 FIDO認証器管理ツール（Version 0.1.19）の修正はありません。

#### 2019/09/30

MDBT50Q Dongle、およびファームウェアを修正しました。<br>

- <b>[MDBT50Q Dongle（rev2）](FIDO2Device/MDBT50Q_Dongle)</b>
- <b>[FIDO2アプリケーション（Version 0.2.1）](nRF5_SDK_v15.3.0)</b>

修正点は以下になります。
- MDBT50Q Dongleの基板（回路）を修正（[#237](https://github.com/diverta/onecard-fido/issues/237) ご参照）
- MDBT50Q Dongleに簡易USBブートローダーを導入（[#256](https://github.com/diverta/onecard-fido/issues/256) ご参照）
- MDBT50Q DongleのLED点灯方法を変更（[#255](https://github.com/diverta/onecard-fido/issues/255) ご参照）

#### 2019/09/17（Version 0.1.19）

FIDO認証器管理ツール、およびファームウェアを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)
- [FIDO2アプリケーション](nRF5_SDK_v15.3.0)

修正点は以下になります。
- MDBT50Q Dongleのバージョン情報を管理ツールから参照できるよう修正（[#248](https://github.com/diverta/onecard-fido/issues/248) ご参照）
- PINコード解除時にタイムアウト（30秒）が効かない不具合を解消（[#247](https://github.com/diverta/onecard-fido/issues/247) ご参照）

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
- [FIDO2アプリケーション](nRF5_SDK_v15.3.0)

修正点は以下になります。
- 鍵・証明書がない状態でCTAP2ヘルスチェックを実行すると、ハングしてしまう不具合を解消（[#230](https://github.com/diverta/onecard-fido/issues/230) ご参照）

#### 2019/08/19（Version 0.1.15）

FIDO認証器管理ツール、およびファームウェアを修正しました。<br>

- [macOS版 FIDO認証器管理ツール](MaintenanceTool/macOSApp/MaintenanceTool.pkg)
- [Windows版 FIDO認証器管理ツール](MaintenanceTool/WindowsExe/MaintenanceToolWin.zip)
- [FIDO2アプリケーション](nRF5_SDK_v15.3.0)

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

[FIDO2アプリケーション](nRF5_SDK_v15.3.0)を、最新のSDKバージョン「v15.3.0」に移行いたしました。

なお、[移行前のプログラム](nRF5_SDK_v15.2.0)は、そのまま残してあります。<br>
（今後はメンテナンスする予定はございません。ご容赦ください）

また、管理ツールの修正はございません。

#### 2019/05/27（Version 0.1.13）

以下のプログラムを修正しました。<br>

- [FIDO2アプリケーション](nRF5_SDK_v15.2.0)
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
