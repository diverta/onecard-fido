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
後報

## Credential Protection
FIDO2の「ユーザー所在確認 (up)」機能（ユーザーにボタンを押させて所在確認する仕組み）は、それが必須か省略可かのいずれかを選択できます。<br>
一方の「ユーザー生体確認 (uv)」機能（ユーザーの指紋スキャン等で本人確認する仕組み）については、そのような選択はできませんでした。

そこでCTAP 2.1では「ユーザー所在確認 (up)」機能と同様、FIDO2の「ユーザー生体確認 (uv)」機能についても、必須or省略可を選択できるようにする仕組みを提供するもののようです。<br>
クライアント（Webブラウザー）側、または認証器側の両方で、この選択（指定）ができる仕組みとなっています。
