# CTAP 2.1関連調査

FIDO認証器の新しい仕様である[CTAP 2.1](https://fidoalliance.org/specs/fido2/fido-client-to-authenticator-protocol-v2.1-rd-20191217.html)がプレビュー公開されております。<br>
そのCTAP 2.1について、機能や実装などの調査を行います。

## 追加された仕様

[現行の仕様（CTAP 2.0）](https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html)から、幾点か仕様追加されているようです。

### 追加された主機能（API）
- authenticatorBioEnrollment
- authenticatorCredentialManagement
- authenticatorSelection
- authenticatorConfig

### 追加された拡張機能
- Credential Protection (credProtect)

以下、項目ごとに分けて詳述したいと思います。

## authenticatorBioEnrollment
後報

## authenticatorCredentialManagement
後報

## authenticatorSelection
後報

## authenticatorConfig
WebAuthnに関するクライアント（Webブラウザー）側の構成情報を、認証器側に永続化（保存）するためのコマンドです。

このコマンドによりクライアント側から転送される構成情報は、認証器側で解析可能である部分（`authenticator config map`）と、解析不可能である部分（`platform config map`）の２パートに別れています。

とは言え、仕様書を見る限りでは、この構成情報の内容を認証器側で読込み、その内容に基づいて何らかの処理を行うことを禁止しています。<br>
単に、認証器を構成情報の保存場所として使用するだけの要件のようです。

## Credential Protection
FIDO2の「ユーザー所在確認 (up)」機能（ユーザーにボタンを押させて所在確認する仕組み）は、それが必須か省略可かのいずれかを選択できます。<br>
一方の「ユーザー生体確認 (uv)」機能（ユーザーの指紋スキャン等で本人確認する仕組み）については、そのような選択はできませんでした。

そこでCTAP 2.1では「ユーザー所在確認 (up)」機能と同様、FIDO2の「ユーザー生体確認 (uv)」機能についても、必須or省略可を選択できるようにする仕組みを提供するもののようです。<br>
クライアント（Webブラウザー）側、または認証器側の両方で、この選択（指定）ができる仕組みとなっています。
