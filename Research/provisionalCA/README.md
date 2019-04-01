# 仮設認証局

## 概要
[nRF52840版FIDO認証器](https://github.com/diverta/onecard-fido/tree/master/nRF5_SDK_v15.2.0)について、[FIDO2仕様適合テスト](https://github.com/diverta/onecard-fido/issues/119) 実行時、自己署名証明書を使用すると、テスト結果がNGとなってしまいます。

対策として、仮設認証局（仮の証明機関）を作成し、仮設認証局が署名した開発用証明書を使用して、FIDO2仕様適合テストを実行するようにしたいと考えます。

### 背景・理由

具体的には、FIDO2仕様適合テスト「CTAP2 Tests - MakeCredential」のテスト項目「Authr-MakeCred-Resp-3 - P-2(c)」実行時に、下記のようなエラーが発生し、結果NG判定されてしまいます。

```
P-2 If "x5c" presented:
(a) Check that "ecdaaKeyId" is NOT presented
(b) Check that "x5c" is of type SEQUENCE
(c) Check that metadata statement contains "attestationRootCertificates" field, and it’s not empty.
(d) Check that metadata statement "attestationTypes" SEQUENCE contains ATTESTATION_BASIC_FULL
(0x3E07) 	
(e) Decode certificate chain
(f) If certificate chain does not contain attestationRootCertificates, append them to the chain
(g) Verify certificate chain
(h) Pick a leaf certificate of the chain and check that:
  (1) Version is of type INTEGER and is set to 3
  (2) Subject-C - is of type UTF8String, and is set to ISO 3166 code specifying the country where the Authenticator vendor is incorporated (UTF8String)
  (3) Subject-O - is of type UTF8String, and is set to the legal name of the Authenticator vendor
  (4) Subject-OU - is of type UTF8String, and is set to literal string “Authenticator Attestation”
  (5) Subject-CN - is of type UTF8String, and is not empty
  (6) Basic Constraints extension MUST have the CA component set to false.
  (7) [TBD] If the related attestation root certificate is used for multiple authenticator models, the Extension OID 1.3.6.1.4.1.45724.1.1.4 (id-fido-gen-ce-aaguid) MUST be present, containing the AAGUID as a 16-byte OCTET STRING. The extension MUST NOT be marked as critical.
  (8) Check that certificate is not expired, is current(notBefore is set to the past date), and is valid for at least 5 years [TBD]
(i) Concatenate authenticatorData and signData to clientDataHash. Using key extracted from leaf certificate, signData verify signature in "sig" field. ‣
AssertionError: MetadataStatement.attestationRootCertificates MUST NOT be empty!: expected [] not to be empty
：
```

場当たり的に「Metadata Statement」に項目`attestationRootCertificates`を追加する対応ではNGで、後続のテスト項目 (d)〜(g) でも、自己署名証明書を使用することにより証明書チェーンが検証できず、NG判定されると判断しています。

これを回避し、テストを正常判定させるためには、ルート証明機関（またはそれに準ずるもの）により署名された証明書を使用する必要があります。

ただし正規の証明書（ベリサインやグローバルサインなどの証明機関が署名した証明書）は入手に時間がかかる上に高価なのと、あくまでも開発目的であるという理由から、仮設認証局を設置し、それに証明書を署名させるといった仮運用で、十分かと考えております。

### 参考事例
いくつかの事例が、インターネット上に公開されております。<br>
調査および手順／結果の検証時は、これらの情報を参考にいたしました。

- macOSでオレオレ認証局を立てて証明書を発行する<br>
http://rikuga.me/2017/12/24/oreore-ca-and-ssl-cert/
- オレだよオレオレ認証局で証明書つくる - Qiita<br>
https://qiita.com/ll_kuma_ll/items/13c962a6a74874af39c6
- オレオレ認証局とオレオレ証明書<br>
https://qiita.com/softark/items/15a5280bd38c5dd97b48

## 仮設認証局の設置

おおまかには以下の手順で、仮設認証局を設置します。

- 仮設認証局の管理用ファイルを作成
- 仮設認証局自身の証明書を新規発行


### 仮設認証局の管理用ファイルを作成

証明書の発行件数を管理するファイルを生成します。<br>
これは自動生成されないようなので、事前に作成しておきます。

注意点としては、ファイル名、ディレクトリー構成を、openSSLのデフォルト設定と整合させるようにします。<br>
また、後出の鍵／証明書ファイルは、それぞれ `cakey.pem`、`cacert.pem` とします。

```
mkdir provisionalCA
cd provisionalCA
mkdir demoCA
cd demoCA
mkdir private
mkdir certs
mkdir newcerts
touch index.txt
echo 00 > serial
ls -al
```

#### macOSでの実行例
```
MacBookPro-makmorit-jp:Research makmorit$ pwd
/Users/makmorit/GitHub/onecard-fido/Research
MacBookPro-makmorit-jp:Research makmorit$ mkdir provisionalCA
MacBookPro-makmorit-jp:Research makmorit$ cd provisionalCA
MacBookPro-makmorit-jp:provisionalCA makmorit$ mkdir demoCA
MacBookPro-makmorit-jp:provisionalCA makmorit$ cd demoCA
MacBookPro-makmorit-jp:demoCA makmorit$ mkdir private
MacBookPro-makmorit-jp:demoCA makmorit$ mkdir certs
MacBookPro-makmorit-jp:demoCA makmorit$ mkdir newcerts
MacBookPro-makmorit-jp:demoCA makmorit$ touch index.txt
MacBookPro-makmorit-jp:demoCA makmorit$ echo 00 > serial
MacBookPro-makmorit-jp:demoCA makmorit$ ls -al
total 8
drwxr-xr-x  6 makmorit  staff  204  1 28 11:52 .
drwxr-xr-x  3 makmorit  staff  102  1 28 11:52 ..
drwxr-xr-x  2 makmorit  staff   68  1 28 11:52 certs
drwxr-xr-x  2 makmorit  staff  102  1 28 11:52 newcerts
-rw-r--r--  1 makmorit  staff    0  1 28 11:52 index.txt
drwxr-xr-x  2 makmorit  staff   68  1 28 11:52 private
-rw-r--r--  1 makmorit  staff    3  1 28 11:52 serial
MacBookPro-makmorit-jp:demoCA makmorit$
```

### 仮設認証局自身の証明書を新規発行

自己署名証明書を作成する手順と同様になります。
- 秘密鍵生成
- 証明書要求ファイル生成
- 秘密鍵を使用して署名

#### 秘密鍵作成
```
openssl ecparam -out private/cakey.pem -name prime256v1 -genkey
```

#### 証明書要求ファイル作成
```
openssl req -new -sha256 -key private/cakey.pem -out ca.csr
JP
Tokyo
Shinjuku-ku
Diverta Inc.
Provisional CA
ca.diverta.co.jp
```

#### 秘密鍵を使用して署名
自分が発行した秘密鍵を使い、証明書要求に署名をし、自己署名証明書を作成します。<br>
（PEM形式）
```
openssl x509 -days 365 -in ca.csr -req -signkey private/cakey.pem -out cacert.pem
```

#### macOSでの実行例
```
MacBookPro-makmorit-jp:demoCA makmorit$ openssl ecparam -out private/cakey.pem -name prime256v1 -genkey
MacBookPro-makmorit-jp:demoCA makmorit$ openssl req -new -sha256 -key private/cakey.pem -out ca.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:JP
State or Province Name (full name) [Some-State]:Tokyo   
Locality Name (eg, city) []:Shinjuku-ku
Organization Name (eg, company) [Internet Widgits Pty Ltd]:Diverta Inc.
Organizational Unit Name (eg, section) []:Provisional CA
Common Name (e.g. server FQDN or YOUR name) []:ca.diverta.co.jp
Email Address []:

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
An optional company name []:
MacBookPro-makmorit-jp:demoCA makmorit$ openssl x509 -days 365 -in ca.csr -req -signkey private/cakey.pem -out cacert.pem
Signature ok
subject=/C=JP/ST=Tokyo/L=Shinjuku-ku/O=Diverta Inc./OU=Provisional CA/CN=ca.diverta.co.jp
Getting Private key
MacBookPro-makmorit-jp:demoCA makmorit$ ls -al
total 40
drwxr-xr-x  9 makmorit  staff   306  1 28 11:54 .
drwxr-xr-x  4 makmorit  staff   136  1 28 11:53 ..
-rw-r--r--@ 1 makmorit  staff  6148  1 28 11:53 .DS_Store
-rw-r--r--  1 makmorit  staff   501  1 28 11:54 ca.csr
-rw-r--r--  1 makmorit  staff   725  1 28 11:54 cacert.pem
drwxr-xr-x  2 makmorit  staff    68  1 28 11:52 certs
-rw-r--r--  1 makmorit  staff     0  1 28 11:52 index.txt
drwxr-xr-x  3 makmorit  staff   102  1 28 11:53 private
-rw-r--r--  1 makmorit  staff     3  1 28 11:52 serial
MacBookPro-makmorit-jp:demoCA makmorit$
```

## 仮設認証局による開発用証明書発行

おおまかには以下の手順で、開発用証明書を作成（新規発行）します。

- 秘密鍵生成
- 証明書要求ファイル生成
- 証明書拡張の設定
- 認証局の秘密鍵／証明書を使用して署名
- 証明書をPEM形式からDER形式に変換

#### 秘密鍵作成
```
openssl ecparam -out fido2test.pem -name prime256v1 -genkey
```

#### 証明書要求作成
```
openssl req -new -sha256 -key fido2test.pem -out fido2test.csr
JP
Tokyo
Shinjuku-ku
Diverta Inc.
Authenticator Attestation
authr.diverta.co.jp
```


#### 証明書拡張の設定
```
cat <<EOF > v3_client
basicConstraints = CA:false
keyUsage = critical, digitalSignature, keyEncipherment
extendedKeyUsage = clientAuth
subjectAltName=DNS:diverta.co.jp,DNS:ca.diverta.co.jp
EOF
```

#### 認証局の秘密鍵／証明書を使用して署名
認証局が発行した秘密鍵／証明書を使用して証明書要求に署名をし、開発用証明書を作成します。<br>
（PEM形式）
```
openssl ca -days 365 -in fido2test.csr -out fido2test.crt.pem -extfile v3_client
```

#### macOSでの実行例
```
MacBookPro-makmorit-jp:demoCA makmorit$ cd ..
MacBookPro-makmorit-jp:provisionalCA makmorit$ pwd
/Users/makmorit/GitHub/onecard-fido/Research/provisionalCA
MacBookPro-makmorit-jp:provisionalCA makmorit$
MacBookPro-makmorit-jp:provisionalCA makmorit$ openssl ecparam -out fido2test.pem -name prime256v1 -genkey
MacBookPro-makmorit-jp:provisionalCA makmorit$ openssl req -new -sha256 -key fido2test.pem -out fido2test.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:JP
State or Province Name (full name) [Some-State]:Tokyo   
Locality Name (eg, city) []:Shinjuku-ku
Organization Name (eg, company) [Internet Widgits Pty Ltd]:Diverta Inc.
Organizational Unit Name (eg, section) []:Authenticator Attestation
Common Name (e.g. server FQDN or YOUR name) []:authr.diverta.co.jp
Email Address []:

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
An optional company name []:
MacBookPro-makmorit-jp:provisionalCA makmorit$ cat <<EOF > v3_client
> basicConstraints = CA:false
> keyUsage = critical, digitalSignature, keyEncipherment
> extendedKeyUsage = clientAuth
> subjectAltName=DNS:diverta.co.jp,DNS:ca.diverta.co.jp
> EOF
MacBookPro-makmorit-jp:provisionalCA makmorit$
MacBookPro-makmorit-jp:provisionalCA makmorit$ openssl ca -days 365 -in fido2test.csr -out fido2test.crt.pem -extfile v3_client
Using configuration from /System/Library/OpenSSL/openssl.cnf
Check that the request matches the signature
Signature ok
Certificate Details:
        Serial Number: 0 (0x0)
        Validity
            Not Before: Jan 28 02:57:30 2019 GMT
            Not After : Jan 28 02:57:30 2020 GMT
        Subject:
            countryName               = JP
            stateOrProvinceName       = Tokyo
            organizationName          = Diverta Inc.
            organizationalUnitName    = Authenticator Attestation
            commonName                = authr.diverta.co.jp
        X509v3 extensions:
            X509v3 Basic Constraints:
                CA:FALSE
            X509v3 Key Usage: critical
                Digital Signature, Key Encipherment
            X509v3 Extended Key Usage:
                TLS Web Client Authentication
            X509v3 Subject Alternative Name:
                DNS:diverta.co.jp, DNS:ca.diverta.co.jp
Certificate is to be certified until Jan 28 02:57:30 2020 GMT (365 days)
Sign the certificate? [y/n]:y


1 out of 1 certificate requests certified, commit? [y/n]y
Write out database with 1 new entries
Data Base Updated
MacBookPro-makmorit-jp:provisionalCA makmorit$
```

#### 証明書をPEM形式からDER形式に変換
`openssl ca`コマンドで生成された証明書は、PEM形式となっております。<br>
これではFIDO認証器で使用することができないため、下記コマンドを実行してDERエンコードします。

```
openssl x509 -in demoCA/newcerts/00.pem  -inform PEM -out fido2test.crt -outform DER
```

#### macOSでの実行例
```
MacBookPro-makmorit-jp:provisionalCA makmorit$ ls -al
total 56
drwxr-xr-x   9 makmorit  staff   306  1 28 12:31 .
drwxr-xr-x  32 makmorit  staff  1088  1 28 11:52 ..
-rw-r--r--@  1 makmorit  staff  6148  1 28 11:53 .DS_Store
drwxr-xr-x  13 makmorit  staff   442  1 28 11:57 demoCA
-rw-r--r--   1 makmorit  staff   579  1 28 12:31 fido2test.crt
-rw-r--r--   1 makmorit  staff  2408  1 28 11:57 fido2test.crt.pem
-rw-r--r--   1 makmorit  staff   521  1 28 11:56 fido2test.csr
-rw-r--r--   1 makmorit  staff   302  1 28 11:56 fido2test.pem
-rw-r--r--   1 makmorit  staff   167  1 28 11:56 v3_client
MacBookPro-makmorit-jp:provisionalCA makmorit$ openssl x509 -in fido2test.crt.pem -inform PEM -out fido2test.crt -outform DER
MacBookPro-makmorit-jp:provisionalCA makmorit$ ls -al
total 64
drwxr-xr-x  10 makmorit  staff   340  1 28 12:49 .
drwxr-xr-x  32 makmorit  staff  1088  1 28 11:52 ..
-rw-r--r--@  1 makmorit  staff  6148  1 28 11:53 .DS_Store
-rw-r--r--   1 makmorit  staff  2742  8 27 10:12 README.md
drwxr-xr-x  13 makmorit  staff   442  1 28 11:57 demoCA
-rw-r--r--   1 makmorit  staff   579  1 28 12:50 fido2test.crt
-rw-r--r--   1 makmorit  staff  2408  1 28 11:57 fido2test.crt.pem
-rw-r--r--   1 makmorit  staff   521  1 28 11:56 fido2test.csr
-rw-r--r--   1 makmorit  staff   302  1 28 11:56 fido2test.pem
-rw-r--r--   1 makmorit  staff   167  1 28 11:56 v3_client
MacBookPro-makmorit-jp:provisionalCA makmorit$
```

以上で、開発用証明書の発行は完了です。
