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

#### PINの設定

<b>2020/09/23現在、この機能は未実装です。</b>

現状、PIV機能で使用するデフォルトのPIN番号は、`123456` となっております。

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

以下のコマンドを実行します。

```
./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" --pin=123456 -a set-mgm-key
```

以下は実行例になります。

```
bash-3.2$ ./yubico-piv-tool -v --reader="Diverta Inc. Secure Dongle" --pin=123456 -a set-mgm-key
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'set-mgm-key' needs that.
Successful application authentication.
Now processing for action 'set-mgm-key'.
Enter new management key: [注1]
Verifying - Enter new management key: [注1]
Successfully set new management key.
Disconnect card #0.
bash-3.2$
```
[注1]デフォルトの管理用パスワードは`010203040506070801020304050607080102030405060708`となっております。<br>
[注2]`Enter new management key: `や`Verifying - Enter new management key: `のプロンプトに続いて、前述のパスワード（HEX文字列）を入力します。入力文字列はエコーバックされません。

## 秘密鍵・証明書の導入

[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)のPIV機能を利用するためには、秘密鍵・証明書の導入が必要です。

以下に、EC秘密鍵・証明書（ECCP256）を利用する前提で、手順を掲載いたします。<br>
この例では、使用する証明書は「自己署名証明書」とします。

#### 秘密鍵の作成

まずは以下のコマンドを実行し、秘密鍵ファイルを生成します。<br>
本例では自己署名証明書をツールで作成するため、公開鍵ファイルも同時に生成しておきます。

```
openssl ecparam -name prime256v1 -genkey > ecc_prv_9a.pem
openssl ec -pubout < ecc_prv_9a.pem > ecc_pub_9a.pem

openssl ecparam -name prime256v1 -genkey > ecc_prv_9c.pem
openssl ec -pubout < ecc_prv_9c.pem > ecc_pub_9c.pem

openssl ecparam -name prime256v1 -genkey > ecc_prv_9d.pem
openssl ec -pubout < ecc_prv_9d.pem > ecc_pub_9d.pem
```

以下は実行例になります。

```
bash-3.2$ openssl ecparam -name prime256v1 -genkey > ecc_prv_9a.pem
bash-3.2$ openssl ec -pubout < ecc_prv_9a.pem > ecc_pub_9a.pem
read EC key
writing EC key
bash-3.2$
bash-3.2$ openssl ecparam -name prime256v1 -genkey > ecc_prv_9c.pem
bash-3.2$ openssl ec -pubout < ecc_prv_9c.pem > ecc_pub_9c.pem
read EC key
writing EC key
bash-3.2$
bash-3.2$ openssl ecparam -name prime256v1 -genkey > ecc_prv_9d.pem
bash-3.2$ openssl ec -pubout < ecc_prv_9d.pem > ecc_pub_9d.pem
read EC key
writing EC key
bash-3.2$
```

#### 秘密鍵の導入

生成した秘密鍵を、MDBT50Q Dongle内部のPIVデバイススロット[注1]に導入します。<br>
以下のコマンドを実行します。

```
YPT_COMMAND=./yubico-piv-tool
READER_NAME=Diverta\ Inc.\ Secure\ Dongle

${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-key -s 9a -i ecc_prv_9a.pem
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-key -s 9c -i ecc_prv_9c.pem
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-key -s 9d -i ecc_prv_9d.pem
```

以下は実行例になります。

```
bash-3.2$ YPT_COMMAND=./yubico-piv-tool
bash-3.2$ READER_NAME=Diverta\ Inc.\ Secure\ Dongle
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-key -s 9a -i ecc_prv_9a.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'import-key' needs that.
Successful application authentication.
Now processing for action 'import-key'.
Successfully imported a new private key.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-key -s 9c -i ecc_prv_9c.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'import-key' needs that.
Successful application authentication.
Now processing for action 'import-key'.
Successfully imported a new private key.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-key -s 9d -i ecc_prv_9d.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'import-key' needs that.
Successful application authentication.
Now processing for action 'import-key'.
Successfully imported a new private key.
Disconnect card #0.
bash-3.2$
```

[注1]9a（PIV Authentication）、9c（Digital Signature）、9d（Key Management）の３点があります。

#### 自己署名証明書の作成

Yubico PIV Tool (command line) を使用し、自己署名証明書を作成します。<br>
以下のコマンドを実行します。

```
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a selfsign-certificate -s 9a -S "/CN=pivauth.divert.co.jp/" -i ecc_pub_9a.pem -o ecc_crt_9a.pem
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a selfsign-certificate -s 9c -S "/CN=digsign.divert.co.jp/" -i ecc_pub_9c.pem -o ecc_crt_9c.pem
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a selfsign-certificate -s 9d -S "/CN=keymgmt.divert.co.jp/" -i ecc_pub_9d.pem -o ecc_crt_9d.pem
```

以下は実行例になります。

```
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a selfsign-certificate -s 9a -S "/CN=pivauth.divert.co.jp/" -i ecc_pub_9a.pem -o ecc_crt_9a.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'selfsign-certificate' does not need authentication.
Now processing for action 'selfsign-certificate'.
Successfully generated a new self signed certificate.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a selfsign-certificate -s 9c -S "/CN=digsign.divert.co.jp/" -i ecc_pub_9c.pem -o ecc_crt_9c.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'selfsign-certificate' does not need authentication.
Now processing for action 'selfsign-certificate'.
Successfully generated a new self signed certificate.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a selfsign-certificate -s 9d -S "/CN=keymgmt.divert.co.jp/" -i ecc_pub_9d.pem -o ecc_crt_9d.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Action 'selfsign-certificate' does not need authentication.
Now processing for action 'selfsign-certificate'.
Successfully generated a new self signed certificate.
Disconnect card #0.
bash-3.2$
bash-3.2$ ls -al
total 312
drwxr-xr-x@ 13 makmorit  staff     416  9 23 10:45 .
drwxr-xr-x@  8 makmorit  staff     256  7  7 12:09 ..
-rw-r--r--   1 makmorit  staff     591  9 23 10:43 ecc_crt_9a.pem
-rw-r--r--   1 makmorit  staff     591  9 23 10:44 ecc_crt_9c.pem
-rw-r--r--   1 makmorit  staff     591  9 23 10:44 ecc_crt_9d.pem
-rw-r--r--   1 makmorit  staff     302  9 23 10:34 ecc_prv_9a.pem
-rw-r--r--   1 makmorit  staff     302  9 23 10:34 ecc_prv_9c.pem
-rw-r--r--   1 makmorit  staff     302  9 23 10:34 ecc_prv_9d.pem
-rw-r--r--   1 makmorit  staff     178  9 23 10:34 ecc_pub_9a.pem
-rw-r--r--   1 makmorit  staff     178  9 23 10:34 ecc_pub_9c.pem
-rw-r--r--   1 makmorit  staff     178  9 23 10:34 ecc_pub_9d.pem
drwxr-xr-x@  8 makmorit  staff     256  9 16 16:08 test_ecc_key_cert
-rwxr-xr-x@  1 makmorit  staff  119632  1 30  2020 yubico-piv-tool
bash-3.2$
```

#### 証明書の導入

生成した自己署名証明書を、MDBT50Q Dongle内部のPIVデバイススロット[注1]に導入します。<br>
以下のコマンドを実行します。

```
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-certificate -s 9a -i ecc_crt_9a.pem
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-certificate -s 9c -i ecc_crt_9c.pem
${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-certificate -s 9d -i ecc_crt_9d.pem
```

以下は実行例になります。

```
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-certificate -s 9a -i ecc_crt_9a.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'import-certificate' needs that.
Successful application authentication.
Now processing for action 'import-certificate'.
Successfully imported a new certificate.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-certificate -s 9c -i ecc_crt_9c.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'import-certificate' needs that.
Successful application authentication.
Now processing for action 'import-certificate'.
Successfully imported a new certificate.
Disconnect card #0.
bash-3.2$
bash-3.2$ ${YPT_COMMAND} -v -r "${READER_NAME}" --pin=123456 -a import-certificate -s 9d -i ecc_crt_9d.pem
Connect reader 'Diverta Inc. Secure Dongle' matching 'Diverta Inc. Secure Dongle'.
Authenticating since action 'import-certificate' needs that.
Successful application authentication.
Now processing for action 'import-certificate'.
Successfully imported a new certificate.
Disconnect card #0.
bash-3.2$
```

[注1]9a（PIV Authentication）、9c（Digital Signature）、9d（Key Management）の３点があります。

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
CHUID:	3019d4e739da739ced39ce739d836858210842108421c84210c3eb3410b5c80bc05da625be7f24c83b7bead81b350832303330303130313e00fe00
CCC:	f015a000000116ff02a645024f3eb1d5d16f954cbd6224f10121f20121f300f40100f50110f600f700fa00fb00fc00fd00fe00
Slot 9a:
	Algorithm:	ECCP256
	Subject DN:	CN=pivauth.divert.co.jp
	Issuer DN:	CN=pivauth.divert.co.jp
	Fingerprint:	5d4e657eab5b47d7a93ba1f9bb93acb13e9c578584f24e258e8fdd4e4841a55a
	Not Before:	Sep 23 01:43:23 2020 GMT
	Not After:	Sep 23 01:43:23 2021 GMT
Slot 9c:
	Algorithm:	ECCP256
	Subject DN:	CN=digsign.divert.co.jp
	Issuer DN:	CN=digsign.divert.co.jp
	Fingerprint:	ac06f574ae9231a1533d9c8f0f860484d9340e54e17a9e16cc2006367bea88af
	Not Before:	Sep 23 01:44:29 2020 GMT
	Not After:	Sep 23 01:44:29 2021 GMT
Slot 9d:
	Algorithm:	ECCP256
	Subject DN:	CN=keymgmt.divert.co.jp
	Issuer DN:	CN=keymgmt.divert.co.jp
	Fingerprint:	2054a93c322fc71914dda4a174f05031c4ad25c78acc1da6cd5ad5d572e9cea2
	Not Before:	Sep 23 01:44:38 2020 GMT
	Not After:	Sep 23 01:44:38 2021 GMT
PIN tries left:	3
Disconnect card #0.
bash-3.2$
```

以上で、PIV機能を利用するための初期データ導入は終了となります。
