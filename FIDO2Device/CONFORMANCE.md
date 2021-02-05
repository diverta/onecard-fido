# FIDO仕様適合テスト手順書

FIDOアライアンスが無償提供しているテストツール「FIDO Conformance Tools」を使用し、FIDO仕様適合テストを実行する手順を掲載しています。

## テスト前の準備

[FIDO Conformance Tools（v1.5.2）](https://builds.fidoalliance.org/Desktop%20UAF%20FIDO2%20U2F/v1.5.2/Standalone%20-%20FIDO%20Alliance%20-%20Certification%20Conformance%20Testing%20Tools%201.5.2.exe)を起動します。<br>
プログラムアイコンを右クリック後「管理者として実行」を選択します。

<img src="assets01/0001.jpg" width="500">

FIDO Conformance Toolsが起動されたら、ホーム画面の「FIDO2.0 Tests」の「RUN」をクリックします。

<img src="assets01/0002.jpg" width="540">

#### METADATA STATEMENTの設定

画面右下の「DROP YOUR METADATA STATEMENT HERE」をクリックします。

<img src="assets01/0003.jpg" width="540">

ファイル選択ダイアログで、ファイル[「Virtual&#032;Secp256R1&#032;FIDO2&#032;Conformance&#032;Testing&#032;CTAP2&#032;Authenticator&#040;Diverta&#041;.json」](../Research/provisionalCA/Virtual&#032;Secp256R1&#032;FIDO2&#032;Conformance&#032;Testing&#032;CTAP2&#032;Authenticator&#040;Diverta&#041;.json)を選択します。

<img src="assets01/0004.jpg" width="540">

点線枠内に「Diverta FIDO Authenticator」という名称が表示されたら、METADATA STATEMENTの設定は完了です。

<img src="assets01/0005.jpg" width="540">

#### 使用するFIDO認証器の設定

FIDO認証器をWindows PCのUSBポートに装着します。<br>
今回はFIDO認証器として、MDBT50Q Dongle（rev2）を使用します。

緑色のLEDが点滅している事を確認します。

<img src="assets01/0000.jpg" width="500">

下図のように、「[HID] Secure Dongle READY」と表示されますので、右横のラジオボタンを選択します。

<img src="assets01/0006.jpg" width="540">

これでテスト前の準備は完了です。

## CTAP2テストの実行

テストを一括実行することは可能ですが、途中エラーが発生してしまうと、最初から大量のテスト項目をやり直す必要があります。<br>
これでは不便ですので、個別実行により、項目を小分けしてテストを実行したほうが良いです。

本手順では、以下のグループに分けて個別実行しました。

- テスト１
  - Transports
  - Generic
- テスト２
  - MakeCredential Request
  - MakeCredential Response
- テスト３
  - GetAssertion Request
  - GetAssertion Response
- テスト４
  - Reset
  - Options
  - Extensions
- テスト５
  - ClientPin1
- テスト６
  - Metadata tests

以下、テスト１〜テスト６の順で実行していきます。

#### テスト１

テスト項目をチェックボックスで選択し、下部の「RUN」ボタンをクリックしてテストを開始させます。

<img src="assets01/0007.jpg" width="540">

以下のようなメッセージボックスが表示されたら、リセットが行われますので、基板上の赤いLEDが点滅したらすぐ基板上のボタンを押し、テストを進めます。

<img src="assets01/0008.jpg" width="540">

結果は以下の通り、エラー０件で終了しています。

<img src="assets01/0009.jpg" width="480">

#### テスト２

テスト項目をチェックボックスで選択し、下部の「RUN」ボタンをクリックしてテストを開始させます。

<img src="assets01/0010.jpg" width="540">

結果は以下の通り、エラー０件で終了しています。

<img src="assets01/0011.jpg" width="480">

#### テスト３

テスト項目をチェックボックスで選択し、下部の「RUN」ボタンをクリックしてテストを開始させます。

<img src="assets01/0012.jpg" width="540">

結果は以下の通り、エラー０件で終了しています。

<img src="assets01/0013.jpg" width="480">

#### テスト４

テスト項目をチェックボックスで選択し、下部の「RUN」ボタンをクリックしてテストを開始させます。

<img src="assets01/0014.jpg" width="540">

結果は以下の通り、エラー０件で終了しています。

<img src="assets01/0015.jpg" width="480">

#### テスト５

テスト項目をチェックボックスで選択し、下部の「RUN」ボタンをクリックしてテストを開始させます。

<img src="assets01/0016.jpg" width="540">

結果は以下の通り、エラー０件で終了しています。

<img src="assets01/0018.jpg" width="480">

#### テスト６

テスト項目をチェックボックスで選択し、下部の「RUN」ボタンをクリックしてテストを開始させます。

<img src="assets01/0019.jpg" width="540">

結果は以下の通り、エラー０件で終了しています。

<img src="assets01/0020.jpg" width="480">
