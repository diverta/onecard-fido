# CTAP 2.1関連調査

FIDO認証器の新しい仕様である[CTAP 2.1](https://fidoalliance.org/specs/fido2/fido-client-to-authenticator-protocol-v2.1-rd-20191217.html)がプレビュー公開されております。<br>
そのCTAP 2.1について、機能や実装などの調査を行います。

## 追加された仕様

[現行の仕様（CTAP 2.0）](https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html)から、幾点か仕様追加されているようです。

#### 追加された主機能（API）
- authenticatorBioEnrollment
- authenticatorCredentialManagement
- authenticatorSelection
- authenticatorConfig

#### 追加された拡張機能
- Credential Protection (credProtect)

## 本件プロジェクトへの影響

#### 対応が必要となるもの
- authenticatorSelection
- authenticatorConfig

#### 対応が不要であるもの
- authenticatorBioEnrollment
- authenticatorCredentialManagement
- Credential Protection (credProtect)

## 追加仕様について

以下、項目ごとに分けて詳述したいと思います。

### authenticatorBioEnrollment
認証器が「ユーザー生体確認（uv）」機能を持っている場合、生体データ（指紋）を初期登録／列挙／削除するための仕組みです。<br>
以下のコマンドが用意されています。

- `getFingerprintSensorInfo` <br>
生体確認のタイプ（タッチ／スワイプ）および最大サンプル回数を取得

- `enrollBegin`／`enrollCaptureNextSample` <br>
生体データ（指紋）を指定されたサンプル回数分取得して登録

- `cancelCurrentEnrollment` <br>
生体データのサンプル取得を途中でキャンセル

- `enumerateEnrollments` <br>
認証器に登録されている生体データを列挙

- `setFriendlyName` <br>
認証器に登録した生体データに任意の名称をつける

- `removeEnrollment` <br>
認証器に登録されている生体データを削除

<b>本プロジェクトでは、ユーザー生体確認（uv）機能は実装しておりませんので、本件仕様追加の影響はありません。</b>

### authenticatorCredentialManagement
認証器に永続化されている認証情報（サイト属性、ログイン用機密データ）が存在する場合、それらを管理するための仕組みです。<br>
以下のコマンドが用意されています。
- `getCredsMetadata` <br>
認証器に格納可能な認証情報の最大数、および実際に格納されている認証情報の数を取得

- `enumerateRPsBegin`／`enumerateRPsGetNextRP` <br>
認証器に格納されている認証情報のサイト属性を列挙

- `enumerateRPsBegin`／`enumerateRPsGetNextRP` <br>
認証器に格納されている認証情報の機密データを列挙（サイト属性が検索キーとなります）

- `deleteCredential` <br>
認証器に格納されている認証情報を削除


<b>本プロジェクトでは、認証情報は認証器に永続化しない実装としておりますので、本件仕様追加の影響はありません。</b>

### authenticatorSelection
PCに接続されているFIDO認証器が複数ある場合、クライアント（Webブラウザー）側から、どの認証器を使って認証するかを選択できるようにする仕組みです。

クライアント側から、選択したい認証器に対し`authenticatorSelection`コマンドをリクエストすると、ユーザー登録／ログイン時同様、認証器がユーザー所在確認を行ったうえで、選択が完了する仕様となっています。

`authenticatorSelection`コマンドを受領した場合、認証器は以下のいずれかをクライアントにレスポンスする必要があります。
- `CTAP2_OK` - ユーザーによるボタン押下等で、ユーザー所在確認が完了 [注1]
- `CTAP2_ERR_OPERATION_DENIED` - ユーザーが所在確認を拒否 [注2]
- `CTAP2_ERR_USER_ACTION_TIMEOUT` - ユーザーによるボタン押下が指定秒数以上行われなかった等で、ユーザー所在確認がタイムアウト [注3]

<b>こちらの仕様追加につきましては、本プロジェクトでも追加実装が必要と考えます。</b>

[注1] [BLE近接認証機能](../../FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)が有効になっている場合、指定秒数以内でBLEデバイスがスキャンされた時は、ユーザーによるボタン押下を不要としています。<br>
[注2] 本プロジェクトでは、ユーザーが所在確認を拒否する仕組み（キャンセルボタン等）を実装していません。<br>
[注3] [BLE近接認証機能](../../FIDO2Device/MDBT50Q_Dongle/BLEDAUTH.md)が有効になっている場合、BLEデバイススキャンがタイムアウトした時は、このケースに該当することになります。

### authenticatorConfig
WebAuthnに関するクライアント（Webブラウザー）側の構成情報を、認証器側に永続化（保存）するためのコマンドです。

このコマンドによりクライアント側から転送される構成情報は、認証器側で解析可能である部分（`authenticator config map`）と、解析不可能である部分（`platform config map`）の２パートに別れています。

とは言え、仕様書を見る限りでは、この構成情報の内容を認証器側で読込み、その内容に基づいて何らかの処理を行うことを禁止しています。<br>
単に、認証器を構成情報の保存場所として使用するだけの要件のようです。

なお、`authenticatorConfig`をサポートするかしないかは、認証器側で明示的に指定することが可能です。

<b>こちらの仕様追加につきましては、本プロジェクトでも追加実装が必要と考えます</b>（ただし必須ではないと思われます）。

### Credential Protection
FIDO2の「ユーザー所在確認 (up)」機能（ユーザーにボタンを押させて所在確認する仕組み）は、それが必須か省略可かのいずれかを選択できます。<br>
一方の「ユーザー生体確認 (uv)」機能（ユーザーの指紋スキャン等で本人確認する仕組み）については、そのような選択はできませんでした。

そこでCTAP 2.1では「ユーザー所在確認 (up)」機能と同様、FIDO2の「ユーザー生体確認 (uv)」機能についても、必須or省略可を選択できるようにする仕組みを提供するもののようです。<br>
クライアント（Webブラウザー）側、または認証器側の両方で、この選択（指定）ができる仕組みとなっています。

<b>本プロジェクトでは、ユーザー生体確認（uv）機能は実装しておりませんので、本件仕様追加の影響はありません。</b>
