# ATECC608A関数群について

セキュリティーIC「ATECC608A」用の関数群について記載いたします。

## 関数一覧

下表は、ATECC608Aを基板上に実装し、FIDO機能を実現する場合に必要となる関数です。

#### 共通鍵関連

|#|関数名|説明|
|:---:|:---|:---|
|1-1|fido_cryptoauth_sskey_init|ECDH共通鍵生成時に必要となるEC鍵ペアを新規生成します。|
|1-2|fido_cryptoauth_sskey_generate|指定の公開鍵を使用し、ECDH共通鍵を新規生成します。|
|1-3|fido_cryptoauth_sskey_public_key|ECDH共通鍵生成時の秘密鍵に対応する公開鍵を取得します。|
|1-4|fido_cryptoauth_sskey_hash|ECDH共通鍵のSHA-256ハッシュを取得します。|

#### 外部鍵関連

|#|関数名|説明|
|:---:|:---|:---|
|2-1|fido_cryptoauth_install_privkey|FIDO認証器固有の秘密鍵を、14番スロットに格納します。|
|2-1|fido_cryptoauth_extract_pubkey_from_cert|指定された証明書データから、公開鍵を抽出します。|

#### 内部鍵関連

|#|関数名|説明|
|:---:|:---|:---|
|3-1|fido_cryptoauth_keypair_generate|所定のスロットに、EC鍵ペアを新規生成します。|
|3-2|fido_cryptoauth_keypair_public_key|所定のスロットから、EC鍵ペアの公開鍵を取得します。|

#### 署名関連

|#|関数名|説明|
|:---:|:---|:---|
|4-1|fido_cryptoauth_generate_sha256_hash|指定されたデータを使用し、SHA-256ハッシュを生成します。|
|4-2|fido_cryptoauth_ecdsa_sign|所定のスロットに格納された秘密鍵を使用し、<br>指定されたSHA-256ハッシュにECDSA署名を行います。|

#### 暗号化関連

|#|関数名|説明|
|:---:|:---|:---|
|5-1|fido_cryptoauth_aes_cbc_new_password|機密データの暗号化／復号化に使用するAESパスワードを<br>新規生成のうえ、8番スロットに格納します。|
|5-2|fido_cryptoauth_aes_cbc_encrypt|前述のAESパスワードを使用し、機密データを暗号化<br>（AES-128-CBC）します。|
|5-3|fido_cryptoauth_aes_cbc_decrypt|前述のAESパスワードを使用し、暗号化済み機密データを<br>復号化（AES-128-CBC）します。|

#### その他業務関連

|#|関数名|説明|
|:---:|:---|:---|
|6-1|fido_cryptoauth_generate_random_vector|32バイトのランダムなバイトデータを生成します。|
|6-1|fido_cryptoauth_calculate_hmac_sha256|指定されたデータとキーを使用し、<br>HMAC-SHA-256ハッシュを生成します。|

#### 共通関数

|#|関数名|説明|
|:---:|:---|:---|
|7-1|fido_cryptoauth_init|デバイスを初期化し、ATECC608A用関数群を使用可能にします。|
|7-2|fido_cryptoauth_release|デバイスを解放します。|
|7-3|fido_cryptoauth_get_config_bytes|ATECC608A設定内容のバイトデータを取得します。|

## 関数の使用方法

前述の関数について、使用方法を記載します。

### 共通鍵の生成

FIDO認証器とWebAuthnクライアントのセッションにおいて必要となる共通鍵は、ATECC608Aの9番スロットを使用して生成します。<br>
具体的には、下記の流れで行います。

- ECDH共通鍵生成時に必要となるEC鍵ペアを新規生成<br>
EC鍵ペアの秘密鍵は、ATECC608Aの9番スロットに格納されます。

- 指定の公開鍵を使用し、ECDH共通鍵を新規生成<br>
WebAuthnクライアントから転送された公開鍵と、9番スロットに格納された秘密鍵を使用し、ECDH共通鍵を新規生成します。<br>
共通鍵の格納領域は、関数引数として指定します。

- ECDH共通鍵生成時の秘密鍵に対応する公開鍵を取得<br>
9番スロットに格納された秘密鍵から、公開鍵を生成し、WebAuthnクライアントに転送します。<br>
これでセッション開始時の鍵交換が完了します。

- ECDH共通鍵のSHA-256ハッシュを取得して使用<br>
FIDO機能では、ECDH共通鍵を直接使用せず、ECDH共通鍵のSHA-256ハッシュを使用して暗号化する仕様となっております。

```
// WebAuthnクライアントから転送された公開鍵を保持
static uint8_t transferred_public_key[64];

void sskey_generate(void)
{
  // ECDH共通鍵生成時に必要となるEC鍵ペアを新規生成
  fido_cryptoauth_sskey_init(false);

  // 指定の公開鍵を使用し、ECDH共通鍵を新規生成
  fido_cryptoauth_sskey_generate(transferred_public_key);

  // ECDH共通鍵生成時の秘密鍵に対応する公開鍵を取得
  uint8_t *p_public_key = fido_cryptoauth_sskey_public_key();

  // 公開鍵をWebAuthnクライアントへ転送し、鍵交換を完了させる
  ：
  ：
  // FIDO機能で使用する共通鍵は、SHA-256ハッシュ化されたものになります。
  fido_log_debug("fido_cryptoauth_sskey_generate:");
  fido_log_print_hexdump_debug(fido_cryptoauth_sskey_hash(), 32);
}
```

### 外部秘密鍵のインストール

FIDO認証器固有の秘密鍵（外部秘密鍵）は、ATECC608Aの14番スロット内に保管します。<br>
したがって、nRF52840のFlash ROM上には保管しないようにします。

これにより、ひとたびATECC608Aのスロット内に保管された外部秘密鍵は、いかなる方法によっても読み出すことが不可能となります。<br>
他方、外部秘密鍵に対応する公開鍵は、関数`fido_cryptoauth_keypair_public_key`の実行により、随時参照可能となっております。

外部秘密鍵のインストールは以下のような流れで行います。
- nRF52840に、FIDO認証器固有の秘密鍵と証明書を転送<br>
（以下、転送された外部秘密鍵／外部証明書と称します。）

- 転送された外部秘密鍵を、１４番スロットにインストール

- 転送された外部証明書を、Flash ROMにインストール

- １４番スロットの秘密鍵から、公開鍵[A]を生成<br>
（本例では、ATECC608A用のライブラリーを直接利用しています。）

- 転送された外部証明書から、公開鍵[B]を抽出

- 前述 [A] と [B] を比較し、同一内容であることを検証<br>
（異なっている場合は、転送されたデータが不正であるか、ATECC608A内で異常が発生したことが考えられます）

```
// 転送された外部秘密鍵を保持
static uint8_t transferred_private_key[32];

// 転送された外部証明書を保持
static uint8_t transferred_cert_data[1024];
static size_t  transferred_cert_data_length;

void fido_authenticator_privkey_write(void)
{
    // 転送された外部証明書を、Flash ROMにインストール
    ：
    ：
    // 転送された外部秘密鍵を、１４番スロットにインストール
    if (fido_cryptoauth_install_privkey(transferred_private_key) == false) {
        return;
    }

    // １４番スロットの秘密鍵から、公開鍵を生成
    uint8_t *p_public_key = fido_cryptoauth_keypair_public_key(KEY_ID_FOR_INSTALL_PRIVATE_KEY);

    // 転送された外部証明書から、公開鍵を抽出
    uint8_t public_key_ref[64];
    if (fido_cryptoauth_extract_pubkey_from_cert(public_key_ref, transferred_cert_data, transferred_cert_data_length) == false) {
        return;
    }

    // 内容を検証
    if (memcmp(public_key_ref, p_public_key, sizeof(public_key_ref)) != 0) {
        fido_log_error("test_privkey_write failed: Invalid public key");
    }

    // デバイス解放
    fido_cryptoauth_release();
}
```

### 内部秘密鍵の生成

サイト固有の秘密鍵（内部秘密鍵）は、ATECC608Aの0〜7番スロットおよび、10〜13番スロット（計12スロット）内に保管します。<br>
したがって、nRF52840のFlash ROM上には保管しないようにします。

これにより、ひとたびATECC608Aのスロット内に保管された内部秘密鍵は、いかなる方法によっても読み出すことが不可能となります。<br>
他方、内部秘密鍵に対応する公開鍵は、関数`fido_cryptoauth_keypair_public_key`の実行により、随時参照可能となっております。

```
void generate_keypair(uint16_t priv_key_id)
{
    // 所定のスロットに、EC鍵ペアを新規生成
    fido_cryptoauth_keypair_generate(priv_key_id);

    // 所定のスロットから、EC鍵ペアの公開鍵を取得
    uint8_t *p_public_key = fido_cryptoauth_keypair_public_key(priv_key_id);
    ：
    ：
    // デバイス解放
    fido_cryptoauth_release();
}
```

### 署名の生成

FIDO機能においては、既にATECC608A内に格納されている秘密鍵と、署名対象データのSHA-256ハッシュを使用して、ECDSA署名を生成し、公開鍵と一緒にWebAuthnクライアントへ転送します。

```
// 生成されるSHA-256ハッシュの格納領域
static uint8_t hash_digest[32];
static size_t  hash_digest_size = sizeof(hash_digest);

// 生成される署名の格納領域
static uint8_t signature[64];
static size_t  signature_size = sizeof(signature);

void generate_sign(uint16_t priv_key_id)
{
    // データを初期化
    memset(data, 0xbc, sizeof(data));

    // 指定されたデータを使用し、SHA-256ハッシュを生成
    fido_cryptoauth_generate_sha256_hash(data, sizeof(data), hash_digest, &hash_digest_size);

    // 所定のスロットに格納された秘密鍵を使用し、SHA-256ハッシュからECDSA署名を生成
    fido_cryptoauth_ecdsa_sign(priv_key_id, hash_digest, signature, &signature_size);
    fido_log_debug("fido_cryptoauth_ecdsa_sign:");
    fido_log_print_hexdump_debug(signature, signature_size);

    // 所定のスロットから、EC鍵ペアの公開鍵を取得
    uint8_t *p_public_key = fido_cryptoauth_keypair_public_key(priv_key_id);

    // 署名を公開鍵と一緒にWebAuthnクライアントへ転送
    ：
    ：
    // デバイス解放
    fido_cryptoauth_release();
}
```

### 機密データの暗号化／復号化

FIDO機能でのユーザー登録時は、ログイン可能ユーザー情報（機密データ）であるキーハンドル（U2F）／クレデンシャルID（CTAP2）を、認証器側で暗号化して生成し、WebAuthnクライアントへ転送します。<br>
他方、FIDO機能でのユーザー認証（ログイン）時は、WebAuthnクライアントから暗号化済みキーハンドル／クレデンシャルIDを含むリクエストデータが送信されて来ますので、認証器側で復号化して、ユーザーがログイン可能かどうかを判定します。

暗号化／復号化は、ATECC608Aにハードウェア実装されている「AES-128-CBC」を使用して実行します。<br>
また、暗号化／復号化に使用するパスワードは、ATECC608Aの８番スロットに保管します。<br>
したがって、nRF52840のFlash ROM上には保管しないようにします。

まずは、管理ツールによる鍵・証明書インストール時、同時にAESパスワードを新規作成し、AESパスワードをATECC608Aに保管します。
```
// 証明書データFlash ROM書込完了時の処理
void fido_maintenance_command_skey_cert_record_updated(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_INSTALL_SKEY_CERT) {
        // 証明書データ書込完了
        fido_log_debug("Update private key and certificate record completed ");

        // 続いて、AESパスワード生成処理を行う
        if (fido_cryptoauth_aes_cbc_new_password() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 4);
        }
        ：
    }
}
```

U2F、CTAP2におけるユーザー登録時は、キーハンドル（U2F）／クレデンシャルID（CTAP2）を、AESパスワードで暗号化して生成します。
```
// CTAP2におけるユーザー登録時の処理
bool ctap2_pubkey_credential_generate_id(void)
{
    // Public Key Credential Sourceを
    // AES CBCで暗号化し、
    // credentialIdを生成する
    memset(credential_id, 0x00, sizeof(credential_id));
    size_t size = pubkey_cred_source_block_size;
    if (fido_cryptoauth_aes_cbc_encrypt(pubkey_cred_source, credential_id, &size) == false) {
        return false;
    }

    credential_id_size = size;
    return true;
}
```

U2F、CTAP2におけるユーザー認証（ログイン）時は、WebAuthnクライアントから送信されて来た暗号化済みキーハンドル（U2F）／クレデンシャルID（CTAP2）を、AESパスワードで復号化します。
```
// CTAP2におけるユーザー認証（ログイン）時の処理
static bool ctap2_pubkey_credential_restore_source(uint8_t *credential_id, size_t credential_id_size)
{
    // authenticatorGetAssertionリクエストから取得した
    // credentialIdを復号化
    memset(pubkey_cred_source, 0, sizeof(pubkey_cred_source));
    size_t size = credential_id_size;
    if (fido_cryptoauth_aes_cbc_decrypt(credential_id, pubkey_cred_source, &size) == false) {
        return false;
    }
    :
    return true;
}
```

### その他

以下の使用例について記載します。
- 32バイトのランダムなバイトデータを生成
- HMAC-SHA-256ハッシュを生成

```
// 32バイトのランダムベクターの格納領域
static uint8_t vector_buf[32];

// HMAC-SHA-256ハッシュ生成用のキーを保持
static uint8_t hmac_key[16];

// 生成されるHMAC-SHA-256ハッシュの格納領域
static uint8_t hmac_digest[32];

void generate_hmac_sha256_hash(void)
{
    // データを初期化
    uint8_t data[256];
    memset(data, 0xbc, sizeof(data));

    // 32バイトのランダムベクターを作成
    fido_cryptoauth_generate_random_vector(vector_buf, sizeof(vector_buf));
    fido_log_debug("fido_cryptoauth_generate_random_vector (%d bytes):", sizeof(vector_buf));
    fido_log_print_hexdump_debug(vector_buf, sizeof(vector_buf));

    // HMAC SHA-256ハッシュ生成
    fido_cryptoauth_calculate_hmac_sha256(hmac_key, sizeof(hmac_key),
        data, sizeof(data), NULL, 0, hmac_digest);
    fido_log_debug("fido_cryptoauth_calculate_hmac_sha256 (%d bytes):", sizeof(hmac_digest));
    fido_log_print_hexdump_debug(hmac_digest, sizeof(hmac_digest));

    // デバイス解放
    fido_cryptoauth_release();
}
```
