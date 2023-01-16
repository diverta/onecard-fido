# Yubico PIV Toolによる初期データ導入手順

Yubico PIV Tool (command line) を使用して、鍵・証明書などを[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)に導入する手順を掲載します。

## 事前準備

Yubico PIV Tool (command line) を、PC環境上で使用できるようにするための手順は下記手順書をご参照願います。
- <b>[Yubico PIV Tool (command line) macOS版 導入手順](../CCID/PIVTOOLMACINST.md)</b>
- <b>[Yubico PIV Tool (command line) Windows版 導入手順](../CCID/PIVTOOLWININST.md)</b>

## 基本設定

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)をPCのUSBポートに装着します。

ターミナル（コマンドプロンプト）から、Yubico PIV Tool (command line) の実行可能ファイル「`${HOME}/opt/yubico-piv-tool-2.0.0-mac/bin/yubico-piv-tool`」を使用し、以下のコマンドを実行します。

#### 接続確認

コマンド`list-readers`を実行し、PIVデバイスの一覧を画面表示します。<br>
以下のコマンドを実行します。

```
cd ${HOME}/opt/yubico-piv-tool-2.0.0/bin/
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a list-readers
```

以下は実行例になります。<br>
MDBT50Q Dongleと接続できたことが、メッセージにより確認できます。

```
bash-3.2$ cd ${HOME}/opt/yubico-piv-tool-2.0.0/bin/
bash-3.2$ pwd
/Users/makmorit/opt/yubico-piv-tool-2.0.0/bin
bash-3.2$
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a list-readers
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'list-readers' does not need authentication.
Now processing for action 'list-readers'.
Diverta Inc. Secure Dongle
Disconnect card #0.
bash-3.2$
```

#### PINの変更（オプション）

現状、PIV機能で使用するデフォルトのPIN番号は、`123456` となっております。<br>
こちらを適宜、別のPIN番号に変更できます。

コマンド`change-pin`を実行し、PIV機能で使用するPIN番号を変更します。<br>
以下のコマンドを実行します。

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a change-pin -P 123456
```

以下は実行例になります。<br>

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a change-pin -P 123456
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'change-pin' does not need authentication.
Now processing for action 'change-pin'.
Enter new pin: [注1]
Verifying - Enter new pin: [注1]
Successfully changed the pin code.
Disconnect card #0.
bash-3.2$
```

[注1]`Enter new pin: `や`Verifying - Enter new pin: `のプロンプトに続いて、新しいPIN番号を入力します。入力文字列はエコーバックされません。

#### デバイスIDを設定

CHUIDの生成を行い、PIVデバイスに設定します。<br>
以下のコマンドを実行します。

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a set-chuid
```

以下は実行例になります。

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a set-chuid
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'set-chuid' needs that.
Successful application authentication.
Now processing for action 'set-chuid'.
Set the CHUID ID to: b5 c8 0b c0 5d a6 25 be 7f 24 c8 3b 7b ea d8 1b
Successfully set new CHUID.
Disconnect card #0.
bash-3.2$
```

続いてCCCの生成を行い、PIVデバイスに設定します。<br>
以下のコマンドを実行します。

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a set-ccc
```

以下は実行例になります。

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a set-ccc
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'set-ccc' needs that.
Successful application authentication.
Now processing for action 'set-ccc'.
Set the CCC ID to: a6 45 02 4f 3e b1 d5 d1 6f 95 4c bd 62 24
Successfully set new CCC.
Disconnect card #0.
bash-3.2$
```

#### 管理用パスワードの変更設定

適宜、管理用パスワードを変更設定します。[注1]

パスワード（HEX文字列）は「3DES暗号」（８バイトの暗号が３通りある暗号）になります。<br>
半角24文字で指定してください。<br>
例えば、`010203040506070801020304050607080102030405060708`というHEX文字列が、パスワードとして設定できます。

以下のコマンドを実行します。[注2]

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a verify-pin -P 123456 -a set-mgm-key
```

以下は実行例になります。

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a verify-pin -P 123456 -a set-mgm-key
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Authenticating since action 'set-mgm-key' needs that.
Successful application authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'set-mgm-key'.
Enter new management key: [注3]
Verifying - Enter new management key: [注3]
Successfully set new management key.
Disconnect card #0.
bash-3.2$
```
[注1]デフォルトの管理用パスワードは`010203040506070801020304050607080102030405060708`となっております。<br>
[注2]`-P 123456`は、デフォルトPIN番号を指定した例です。前述手順でPIN番号をデフォルトから変更した場合は、そのPIN番号を代わりに指定してください。<br>
[注3]`Enter new management key: `や`Verifying - Enter new management key: `のプロンプトに続いて、前述のパスワード（HEX文字列）を入力します。入力文字列はエコーバックされません。

## 秘密鍵・証明書の導入

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のPIV機能を利用するためには、秘密鍵・証明書の導入が必要です。

以下に手順を掲載いたします。<br>
この例では、使用する証明書は「自己署名証明書」とします。

#### 秘密鍵／証明書の導入（PIV認証用）

PIV認証に使用する秘密鍵／証明書は、2048ビットRSAアルゴリズム（RSA2048）を使用するようにします。

<b>（１）秘密鍵ファイルの生成</b><br>
まずは以下のコマンドを実行し、秘密鍵ファイルを生成します。<br>
本例では自己署名証明書をツールで作成するため、公開鍵ファイルも同時に生成しておきます。

```
openssl genrsa 2048 > rsa_prv_9a.pem
openssl rsa -pubout < rsa_prv_9a.pem > rsa_pub_9a.pem
```

以下は実行例になります。

```
bash-3.2$ openssl genrsa 2048 > rsa_prv_9a.pem
Generating RSA private key, 2048 bit long modulus
..............................................+++
..................+++
e is 65537 (0x10001)
bash-3.2$ openssl rsa -pubout < rsa_prv_9a.pem > rsa_pub_9a.pem
writing RSA key
bash-3.2$
bash-3.2$ ls -alrt
total 352
：
-rw-r--r--   1 makmorit  staff    1675 11 17 15:28 rsa_prv_9a.pem
-rw-r--r--   1 makmorit  staff     451 11 17 15:28 rsa_pub_9a.pem
bash-3.2$
```

<b>（２）秘密鍵の導入</b><br>
次に、生成した秘密鍵を、MDBT50Q Dongle内部のPIVデバイススロット[注1]に導入します。<br>
以下のコマンドを実行します。[注2]

```
YPT_COMMAND=./yubico-piv-tool
READER_NAME=Diverta\ Inc.\ Secure\ Dongle

${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-key -s 9a -i rsa_prv_9a.pem
```

以下は実行例になります。

```
bash-3.2$ YPT_COMMAND=./yubico-piv-tool
bash-3.2$ READER_NAME=Diverta\ Inc.\ Secure\ Dongle
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-key -s 9a -i rsa_prv_9a.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Authenticating since action 'import-key' needs that.
Successful application authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'import-key'.
Successfully imported a new private key.
Disconnect card #0.
bash-3.2$
```

<b>（３）自己署名証明書の作成</b><br>
続いて、Yubico PIV Tool (command line) を使用し、自己署名証明書を作成します。<br>
以下のコマンドを実行します。[注2]

```
${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a selfsign-certificate -s 9a -S "/CN=pivauth.diverta.co.jp/" -i rsa_pub_9a.pem -o rsa_crt_9a.pem
```

以下は実行例になります。

```
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a selfsign-certificate -s 9a -S "/CN=pivauth.diverta.co.jp/" -i rsa_pub_9a.pem -o rsa_crt_9a.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Action 'selfsign-certificate' does not need authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'selfsign-certificate'.
Successfully generated a new self signed certificate.
Disconnect card #0.
bash-3.2$
bash-3.2$
bash-3.2$ ls -alrt
total 352
：
-rw-r--r--   1 makmorit  staff    1675 11 17 15:28 rsa_prv_9a.pem
-rw-r--r--   1 makmorit  staff     451 11 17 15:28 rsa_pub_9a.pem
-rw-r--r--   1 makmorit  staff    1131 11 17 15:33 rsa_crt_9a.pem
bash-3.2$
```

<b>（４）証明書の導入</b><br>
最後に、生成した自己署名証明書を、MDBT50Q Dongle内部のPIVデバイススロット[注1]に導入します。<br>
以下のコマンドを実行します。[注2]

```
${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-certificate -s 9a -i rsa_crt_9a.pem
```

以下は実行例になります。

```
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-certificate -s 9a -i rsa_crt_9a.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Authenticating since action 'import-certificate' needs that.
Successful application authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'import-certificate'.
Successfully imported a new certificate.
Disconnect card #0.
bash-3.2$
```

[注1]9a（PIV Authentication）を使用します。<br>
[注2]`-P 123456`は、デフォルトPIN番号を指定した例です。前述手順でPIN番号をデフォルトから変更した場合は、そのPIN番号を代わりに指定してください。

以上で、秘密鍵／証明書の導入（PIV認証用）は完了です。

#### 秘密鍵／証明書の導入（PIV認証以外用）

PIV認証以外の用途に使用する秘密鍵／証明書は、256ビットECCアルゴリズム（ECCP256）を使用するようにします。

<b>（１）秘密鍵ファイルの生成</b><br>
まずは以下のコマンドを実行し、秘密鍵ファイルを生成します。<br>
本例では自己署名証明書をツールで作成するため、公開鍵ファイルも同時に生成しておきます。

```
openssl ecparam -name prime256v1 -genkey > ecc_prv_9c.pem
openssl ec -pubout < ecc_prv_9c.pem > ecc_pub_9c.pem

openssl ecparam -name prime256v1 -genkey > ecc_prv_9d.pem
openssl ec -pubout < ecc_prv_9d.pem > ecc_pub_9d.pem
```

以下は実行例になります。

```
bash-3.2$ openssl ecparam -name prime256v1 -genkey > ecc_prv_9c.pem
bash-3.2$ openssl ec -pubout < ecc_prv_9c.pem > ecc_pub_9c.pem
read EC key
writing EC key
bash-3.2$ openssl ecparam -name prime256v1 -genkey > ecc_prv_9d.pem
bash-3.2$ openssl ec -pubout < ecc_prv_9d.pem > ecc_pub_9d.pem
read EC key
writing EC key
bash-3.2$
bash-3.2$ ls -alrt
total 352
：
-rw-r--r--   1 makmorit  staff     302 11 17 16:44 ecc_prv_9c.pem
-rw-r--r--   1 makmorit  staff     178 11 17 16:44 ecc_pub_9c.pem
-rw-r--r--   1 makmorit  staff     302 11 17 16:45 ecc_prv_9d.pem
-rw-r--r--   1 makmorit  staff     178 11 17 16:45 ecc_pub_9d.pem
bash-3.2$

```

<b>（２）秘密鍵の導入</b><br>
次に、生成した秘密鍵を、MDBT50Q Dongle内部のPIVデバイススロット[注1]に導入します。<br>
以下のコマンドを実行します。[注2]

```
YPT_COMMAND=./yubico-piv-tool
READER_NAME=Diverta\ Inc.\ Secure\ Dongle

${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-key -s 9c -i ecc_prv_9c.pem
${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-key -s 9d -i ecc_prv_9d.pem
```

以下は実行例になります。

```
bash-3.2$ YPT_COMMAND=./yubico-piv-tool
bash-3.2$ READER_NAME=Diverta\ Inc.\ Secure\ Dongle
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-key -s 9c -i ecc_prv_9c.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Authenticating since action 'import-key' needs that.
Successful application authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'import-key'.
Successfully imported a new private key.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-key -s 9d -i ecc_prv_9d.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Authenticating since action 'import-key' needs that.
Successful application authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'import-key'.
Successfully imported a new private key.
Disconnect card #0.
bash-3.2$
```

<b>（３）自己署名証明書の作成</b><br>
続いて、Yubico PIV Tool (command line) を使用し、自己署名証明書を作成します。<br>
以下のコマンドを実行します。[注2]

```
${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a selfsign-certificate -s 9c -S "/CN=digsign.diverta.co.jp/" -i ecc_pub_9c.pem -o ecc_crt_9c.pem
${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a selfsign-certificate -s 9d -S "/CN=keymgmt.diverta.co.jp/" -i ecc_pub_9d.pem -o ecc_crt_9d.pem
```

以下は実行例になります。

```
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a selfsign-certificate -s 9c -S "/CN=digsign.diverta.co.jp/" -i ecc_pub_9c.pem -o ecc_crt_9c.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Action 'selfsign-certificate' does not need authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'selfsign-certificate'.
Successfully generated a new self signed certificate.
Disconnect card #0.
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a selfsign-certificate -s 9d -S "/CN=keymgmt.diverta.co.jp/" -i ecc_pub_9d.pem -o ecc_crt_9d.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Action 'selfsign-certificate' does not need authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'selfsign-certificate'.
Successfully generated a new self signed certificate.
Disconnect card #0.
bash-3.2$ ls -alrt
total 352
：
-rw-r--r--   1 makmorit  staff     302 11 17 16:44 ecc_prv_9c.pem
-rw-r--r--   1 makmorit  staff     178 11 17 16:44 ecc_pub_9c.pem
-rw-r--r--   1 makmorit  staff     302 11 17 16:45 ecc_prv_9d.pem
-rw-r--r--   1 makmorit  staff     178 11 17 16:45 ecc_pub_9d.pem
-rw-r--r--   1 makmorit  staff     595 11 17 17:13 ecc_crt_9c.pem
-rw-r--r--   1 makmorit  staff     595 11 17 17:13 ecc_crt_9d.pem
bash-3.2$
```

<b>（４）証明書の導入</b><br>
最後に、生成した自己署名証明書を、MDBT50Q Dongle内部のPIVデバイススロット[注1]に導入します。<br>
以下のコマンドを実行します。[注2]

```
${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-certificate -s 9c -i ecc_crt_9c.pem
${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-certificate -s 9d -i ecc_crt_9d.pem
```

以下は実行例になります。

```
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-certificate -s 9c -i ecc_crt_9c.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Authenticating since action 'import-certificate' needs that.
Successful application authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'import-certificate'.
Successfully imported a new certificate.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" -a verify-pin -P 123456 -a import-certificate -s 9d -i ecc_crt_9d.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'verify-pin' does not need authentication.
Authenticating since action 'import-certificate' needs that.
Successful application authentication.
Now processing for action 'verify-pin'.
Successfully verified PIN.
Now processing for action 'import-certificate'.
Successfully imported a new certificate.
Disconnect card #0.
bash-3.2$
```

[注1]9c（Digital Signature）、9d（Key Management）の２点を使用します。<br>
[注2]`-P 123456`は、デフォルトPIN番号を指定した例です。前述手順でPIN番号をデフォルトから変更した場合は、そのPIN番号を代わりに指定してください。

以上で、秘密鍵／証明書の導入（PIV認証以外用）は完了です。


## 導入結果の確認

以下のコマンドを実行します。

```
cd ${HOME}/opt/yubico-piv-tool-2.0.0/bin/
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a status
```

以下は実行例になります。

```
bash-3.2$ cd ${HOME}/opt/yubico-piv-tool-2.0.0/bin/
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" -a status
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'status' does not need authentication.
Now processing for action 'status'.
Version:	5.0.0
Serial Number:	0
CHUID:	3019d4e739da739ced39ce739d836858210842108421c84210c3eb3410eec0904e09f000fb8f476a2bb9e1546a350832303330303130313e00fe00
CCC:	f015a000000116ff026f37a41d28ce84f867c002a0d2f5f10121f20121f300f40100f50110f600f700fa00fb00fc00fd00fe00
Slot 9a:
	Algorithm:	RSA2048
	Subject DN:	CN=pivauth.diverta.co.jp
	Issuer DN:	CN=pivauth.diverta.co.jp
	Fingerprint:	8890a73a6179a36a11aefcae98ab83e49ee471898d70b2aad4e1f289f9b266ec
	Not Before:	Nov 17 06:33:55 2020 GMT
	Not After:	Nov 17 06:33:55 2021 GMT
Slot 9c:
	Algorithm:	ECCP256
	Subject DN:	CN=digsign.diverta.co.jp
	Issuer DN:	CN=digsign.diverta.co.jp
	Fingerprint:	37c0faeae89b2e3195cd09a0116694731eb8079a75979782ea35b54885c14244
	Not Before:	Nov 17 08:13:26 2020 GMT
	Not After:	Nov 17 08:13:26 2021 GMT
Slot 9d:
	Algorithm:	ECCP256
	Subject DN:	CN=keymgmt.diverta.co.jp
	Issuer DN:	CN=keymgmt.diverta.co.jp
	Fingerprint:	5cea8fd55f1f94832196c7c92b8df57c350fb37cbf371a2092636c56730d4419
	Not Before:	Nov 17 08:13:47 2020 GMT
	Not After:	Nov 17 08:13:47 2021 GMT
PIN tries left:	3
Disconnect card #0.
bash-3.2$
```

以上で、PIV機能を利用するための初期データ導入は終了となります。
