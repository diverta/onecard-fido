# ATECC608A関数群について

セキュリティーIC「ATECC608A」用の関数群について記載いたします。

## 関数一覧

下表は、ATECC608Aを基板上に実装し、FIDO機能を実現する場合に必要となる関数です。

#### メンテナンス関連

| # |関数名|説明|
|:---:|:---|:---|
|1|fido_cryptoauth_sskey_init|ECDH共通鍵生成時に必要となるEC鍵ペアを新規生成します。|
|2|fido_cryptoauth_sskey_generate|指定の公開鍵を使用し、ECDH共通鍵を新規生成します。|
|3|fido_cryptoauth_sskey_public_key|ECDH共通鍵生成時の秘密鍵に対応する公開鍵を取得します。|
|4|fido_cryptoauth_sskey_hash|ECDH共通鍵のSHA-256ハッシュを取得します。|
|5|fido_cryptoauth_install_privkey|FIDO認証器固有の秘密鍵を、14番スロットに格納します。|
|6|fido_cryptoauth_extract_pubkey_from_cert|指定された証明書データから、公開鍵を抽出します。|

#### 業務処理関連

| # |関数名|説明|
|:---:|:---|:---|
|1|fido_cryptoauth_init|デバイスを初期化し、ATECC608A用関数群を使用可能にします。|
|2|fido_cryptoauth_release|デバイスを解放します。|
|3|fido_cryptoauth_keypair_generate|所定のスロットに、EC鍵ペアを新規生成します。|
|4|fido_cryptoauth_keypair_public_key|所定のスロットから、EC鍵ペアの公開鍵を取得します。|
|5|fido_cryptoauth_generate_sha256_hash|指定されたデータを使用し、SHA-256ハッシュを生成します。|
|6|fido_cryptoauth_generate_random_vector|32バイトのランダムなバイトデータを生成します。|
|7|fido_cryptoauth_ecdsa_sign|所定のスロットに格納された秘密鍵を使用し、指定されたSHA-256ハッシュにECDSA署名を行います。|
|8|fido_cryptoauth_calculate_hmac_sha256|指定されたデータとキーを使用し、HMAC-SHA-256ハッシュを生成します。|
