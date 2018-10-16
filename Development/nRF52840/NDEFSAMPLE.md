# NFCサンプルアプリ動作確認手順

サンプルアプリ[`Writable NDEF Message Example`](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fnfc_writable_ndef_msg.html&cp=4_0_1_4_7_6)を使用して、nRF52840のNFC機能を確認する手順を掲載しています。

## 機材の準備

NFC対応スマートフォン（Android）と、NFCタグとなるnRF52840を準備します。

### スマートフォンの準備

NFCに対応しているスマートフォンを準備します。

今回の検証では、ASUS製のスマートフォン<b>[ZenFone 2](https://www.asus.com/jp/Phone/ZenFone_2_ZE551ML/Features/)</b>を使用しております。
他のスマートフォンを利用する場合、NFCに対応しているかどうかは、スマートフォンのメーカーサイトのスペックページ等で確認できるかと存じます。

次に、スマートフォンに、[NFC Tools for Android](
https://play.google.com/store/apps/details?id=com.wakdev.wdnfc)というアプリを導入しておきます。

<img src="assets/0032.png" width="400">

## nRF52840の準備

今回の検証では、[nRF52840-DK](https://www.mouser.jp/new/nordicsemiconductor/nordic-nrf52840-dev-kit/)という開発ボードを使用します。

<img src="assets/0033.png" width="500">

## サンプルアプリケーションの準備

サンプルアプリ[`Writable NDEF Message Example`](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fnfc_writable_ndef_msg.html&cp=4_0_1_4_7_6)を、[NetBeans](NETBEANS.md)によりビルドし、nRF52840-DKに書込みます。

### サンプルアプリソースのコピー／配置

別途手順「[NetBeans開発環境構築手順](NETBEANS.md)」で、NetBeansとnRF5 SDK（v15.2）を導入したら、以下の場所にあるサンプルアプリを、フォルダーごと任意の場所にコピーします。

サンプルアプリの場所：<br>
`~/opt/nRF5_SDK_15.2.0/examples/nfc/writable_ndef_msg`

コピー先は以下の場所としておきます。<br>
`~/GitHub/onecard-fido/nRF5_SDK_v15.2.0/examples/nfc/writable_ndef_msg`

### Makefileの修正

ここで、サンプルアプリの場所（`~/opt/nRF5_SDK_15.2.0/`）と、コピー先の場所（`~/GitHub/onecard-fido/nRF5_SDK_v15.2.0/`）が異なるため、Makefileを事前に修正する必要があります。<br>
（修正を忘れると、プロジェクト新規作成後の自動ビルドに失敗してしまうための事前措置になります）

`~/GitHub/onecard-fido/nRF5_SDK_v15.2.0/examples/nfc/writable_ndef_msg/pca10056/blank/armgcc`配下のMakefileというファイルをエディターで開き、<br>
`SDK_ROOT := ../../../../../..`<br>とあるのを<br>`SDK_ROOT := $(HOME)/opt/nRF5_SDK_15.2.0`<br>と修正してください。

<img src="assets/0034.png" width="600">

### NetBeansプロジェクトの作成

NetBeansを起動して「ファイル(F)--->新規プロジェクト(W)」を実行します。<br>
下図のような、新規プロジェクト画面が表示されますので、一覧から「既存のソースを使用するC/C++プロジェクト」を選択し「次 >」をクリックします。

<img src="assets/0035.png" width="500">

下図のような画面に遷移しますので、以下のように設定します。

- 既存のソースを含むフォルダを指定 - サンプルアプリが格納されているフォルダー「`examples/nfc/writable_ndef_msg`」を指定します。<br>
下図の例では「`/Users/makmorit/GitHub/onecard-fido/nRF5_SDK_v15.2.0/examples/nfc/writable_ndef_msg`」という文字列が設定されています。

- ツール・コレクション - 「GNU_ARM」を選択します。

- 構成モードを選択 - 「カスタム(C)」をチェックします。

設定が完了したら「次 >」をクリックします。

<img src="assets/0036.png" width="500">

下図のような画面に遷移しますので、以下のように設定します。

- 「事前ビルド・ステップが必要」にチェック

- フォルダで実行(U) - サンプルアプリのサブフォルダー「`pca10056/blank/armgcc`」を指定します。<br>
下図の例では「`/Users/makmorit/GitHub/onecard-fido/nRF5_SDK_v15.2.0/examples/nfc/writable_ndef_msg/pca10056/blank/armgcc`」という文字列が設定されています。

- 「カスタム・コマンド」にチェック

- コマンド(O) - 「make」と入力します。

設定が完了したら「次 >」をクリックします。

<img src="assets/0037.png" width="500">

「4. ビルド・アクション」に遷移しますが、以降は「7. プロジェクトの名前と場所」に遷移するまではデフォルト設定のまま「次 >」をクリックします。<br>
「7. プロジェクトの名前と場所」に遷移したら、プロジェクト名(P)を「writable_ndef_msg」から「writable_ndef_msg_proj」に変更しておきます。<br>
（オリジナルのプロジェクト「writable_ndef_msg」を上書きしたくないための措置です）

設定が完了したら「終了(F)」をクリックします。

<img src="assets/0038.png" width="500">

自動的にビルドがスタートしますので、しばらくそのまま待ちます。<br>
しばらくするとビルドが完了し「ビルド SUCCESSFUL」と表示されれば、ビルドは成功です。

<img src="assets/0039.png" width="600">

これでNetBeansにプロジェクト「writable_ndef_msg_proj」が作成されました。

## サンプルアプリケーションの導入

事前に、nRF52840-DKを、USBケーブルでPCと接続します。<br>
その後、nRF52840-DKの電源スイッチ（ボードの左端部）をOnにし、ボードに電源を投入します。

### プロジェクト設定の変更

NetBeansを起動し、プロジェクト「writable_ndef_msg」を右クリックして「プロパティ」を実行します。

<img src="assets/0040.png" width="600">

プロジェクト・プロパティ画面が表示されます。<br>
左ペインの「実行」をクリックしで、以下のように設定を変更します。

- コマンドの実行 - 「make flash」に変更します。

- 実行ディレクトリ - 実行するMakefileが配置されているディレクトリーに変更します。<br>
下図の例では `../writable_ndef_msg/pca10056/blank/armgcc` という文字列が設定されています。

変更が完了したら「OK」をクリックして、いったんプロジェクト・プロパティ画面を閉じます。

<img src="assets/0041.png" width="500">

### サンプルアプリケーションの書込み

NetBeansの実行ボタンをクリックして「プロジェクト(writable_ndef_msg)を実行」を実行します。

<img src="assets/0042.png" width="500">

プログラムが自動的に書き込まれ、nRF52840 DK上でアプリケーションが実行されます。<br>
NetBeansの右下部のコンソールには「実行 FINISHED; 終了値0」と表示されます。

<img src="assets/0043.png" width="500">

適宜、ボードからのデバッグ出力（UARTプリント）を確認します。<br>
nRF52840-DK上で、プログラム実行が開始され、下図のようなデバッグ出力が表示されれば、サンプルアプリケーションの導入は完了です。

<img src="assets/0044.png" width="500">

## 動作確認

下記URLの情報を参考に、動作確認を実施します。<br>
[<b>Nordic Semiconductor Infocenter - Writable NDEF Message Example</b>](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.0.0/nfc_writable_ndef_msg.html?cp=4_0_1_4_7_6)
