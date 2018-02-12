# 鍵・証明書インストール手順

BLE U2Fサービスを動作させるためには、秘密鍵と署名済み証明書が必要になります。

秘密鍵と署名済み証明書を、U2F管理ツールを使用してインストールし、動作確認（ヘルスチェック）を実行するまでの手順を、以下に掲載いたします。

## 鍵・証明書の作成

以下の手順で、秘密鍵ファイル(.pem)、証明書ファイル(.crt)を作成します。

### 秘密鍵ファイル(.pem)の作成

秘密鍵は、Nordicが提供しているnrfutilというツールにより、PEM形式のファイルで作成します。

#### nrfutilをインストールする

nrfutilは、pythonで稼働するソフトウェアです。<br>
最新コードをGitHubからチェックアウトの上、直接ビルドします。

以下、macOSにおけるオペレーション例になります。

```
MacBookPro-makmorit-jp:GitHub makmorit$ sudo -H pip install --ignore-installed six
Collecting six
  Using cached six-1.10.0-py2.py3-none-any.whl
Installing collected packages: six
Successfully installed six-1.10.0
MacBookPro-makmorit-jp:GitHub makmorit$ sudo -H pip install nrfutil
Collecting nrfutil
Collecting pc-ble-driver-py>=0.8.1 (from nrfutil)
Requirement already satisfied: enum34>=1.0.4 in /Library/Python/2.7/site-packages (from nrfutil)
Collecting behave (from nrfutil)
  Using cached behave-1.2.5-py2.py3-none-any.whl
Requirement already satisfied: click>=6.0 in /Library/Python/2.7/site-packages (from nrfutil)
Requirement already satisfied: six>=1.9 in /Library/Python/2.7/site-packages (from nrfutil)
Collecting pyserial>=2.7 (from nrfutil)
  Using cached pyserial-3.3-py2.py3-none-any.whl
Collecting ecdsa>=0.13 (from nrfutil)
  Using cached ecdsa-0.13-py2.py3-none-any.whl
Collecting protobuf (from nrfutil)
Collecting future (from pc-ble-driver-py>=0.8.1->nrfutil)
Collecting wrapt (from pc-ble-driver-py>=0.8.1->nrfutil)
Collecting parse>=1.6.3 (from behave->nrfutil)
Collecting parse-type>=0.3.4 (from behave->nrfutil)
Requirement already satisfied: setuptools in /System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python (from protobuf->nrfutil)
Installing collected packages: future, wrapt, pc-ble-driver-py, parse, parse-type, behave, pyserial, ecdsa, protobuf, nrfutil
Successfully installed behave-1.2.5 ecdsa-0.13 future-0.16.0 nrfutil-2.3.0 parse-1.8.2 parse-type-0.3.4 pc-ble-driver-py-0.9.0 protobuf-3.3.0 pyserial-3.3 wrapt-1.10.10
MacBookPro-makmorit-jp:GitHub makmorit$
MacBookPro-makmorit-jp:GitHub makmorit$ which nrfutil
/usr/local/bin/nrfutil
MacBookPro-makmorit-jp:GitHub makmorit$
```

#### 秘密鍵を生成する

nrfutil keys generateコマンドを使用し、秘密鍵ファイル（private.pem）を作成します。

```
MacBookPro-makmorit-jp:FIDO makmorit$ nrfutil keys generate private.pem
Generated private key and stored it in: private.pem
MacBookPro-makmorit-jp:FIDO makmorit$ cat private.pem
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIOL8DUdOhRKWnF6rcjKHwP5YTFgMunt7rXk/uGuOw6c4oAoGCCqGSM49
AwEHoUQDQgAEWmyHfUclTVTuXb7OdGXbL5BgT6OoHqY63RscJCFFTA9UTZ0rZNkk
8fQ3hoB1jrmh/HfoMYEFLmEEhEiX5VPK0A==
-----END EC PRIVATE KEY-----
MacBookPro-makmorit-jp:FIDO makmorit$
```

### 証明書ファイル(.crt)の作成

署名済み証明書は、本来、ベリサインやグローバルサインのようなパブリック認証局に署名／作成を依頼するのですが、その手順につきましては、ここでは触れません。

ここでは、開発用に使用できる「自己署名証明書」の作成手順について、掲載いたします。

以下、macOSに導入されているopenSSLコマンドを使用した例になります。

#### CSRファイルを生成する

秘密鍵ファイル（private.pem）を入力とし、CSRファイル（ca.csr）を生成します。

```
MacBookPro-makmorit-jp:FIDO makmorit$ openssl req -new -key private.pem -out ca.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:JP
State or Province Name (full name) [Some-State]:Tokyo
Locality Name (eg, city) []:Shinjyuku-ku
Organization Name (eg, company) [Internet Widgits Pty Ltd]:Diverta inc.
Organizational Unit Name (eg, section) []:Dev
Common Name (e.g. server FQDN or YOUR name) []:self-ca
Email Address []:

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:password
An optional company name []:Diverta
MacBookPro-makmorit-jp:FIDO makmorit$
MacBookPro-makmorit-jp:FIDO makmorit$ cat ca.csr
-----BEGIN CERTIFICATE REQUEST-----
MIIBVjCB/gIBADBrMQswCQYDVQQGEwJKUDEOMAwGA1UECBMFVG9reW8xFTATBgNV
BAcTDFNoaW5qeXVrdS1rdTEVMBMGA1UEChMMRGl2ZXJ0YSBpbmMuMQwwCgYDVQQL
EwNEZXYxEDAOBgNVBAMTB3NlbGYtY2EwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNC
AARabId9RyVNVO5dvs50ZdsvkGBPo6gepjrdGxwkIUVMD1RNnStk2STx9DeGgHWO
uaH8d+gxgQUuYQSESJflU8rQoDEwFgYJKoZIhvcNAQkCMQkTB0RpdmVydGEwFwYJ
KoZIhvcNAQkHMQoTCHBhc3N3b3JkMAkGByqGSM49BAEDSAAwRQIhAIm21nNNcFUw
BHlORZYf6ZPj0okm9A2Pnr1XzZ/WX0MpAiAavc9VBJOMsQN9qE9LTNvI2nu59xSH
/jlDHIAO2eegqg==
-----END CERTIFICATE REQUEST-----
MacBookPro-makmorit-jp:FIDO makmorit$
```

#### CSRファイルから、自己署名証明書を作成

CSRファイル（ca.csr）を入力とし、X509を使用して署名します。<br>
出力形式は、秘密鍵(PEM形式)との混同を回避するため、DER形式としてください。

```
MacBookPro-makmorit-jp:FIDO makmorit$ openssl x509 -in ca.csr -days 3650 -req -signkey private.pem -out cacert.crt -outform der
Signature ok
subject=/C=JP/ST=Tokyo/L=Shinjyuku-ku/O=Diverta inc./OU=Dev/CN=self-ca
Getting Private key
MacBookPro-makmorit-jp:FIDO makmorit$ ls -al cacert.crt
-rw-r--r--  1 makmorit  staff  456 Jul  4 12:16 cacert.crt
MacBookPro-makmorit-jp:FIDO makmorit$ openssl x509 -in cacert.crt -inform der
-----BEGIN CERTIFICATE-----
MIIBxDCCAWsCCQDTGItjNaeweTAJBgcqhkjOPQQBMGsxCzAJBgNVBAYTAkpQMQ4w
DAYDVQQIEwVUb2t5bzEVMBMGA1UEBxMMU2hpbmp5dWt1LWt1MRUwEwYDVQQKEwxE
aXZlcnRhIGluYy4xDDAKBgNVBAsTA0RldjEQMA4GA1UEAxMHc2VsZi1jYTAeFw0x
NzA3MDQwMzE2MTFaFw0yNzA3MDIwMzE2MTFaMGsxCzAJBgNVBAYTAkpQMQ4wDAYD
VQQIEwVUb2t5bzEVMBMGA1UEBxMMU2hpbmp5dWt1LWt1MRUwEwYDVQQKEwxEaXZl
cnRhIGluYy4xDDAKBgNVBAsTA0RldjEQMA4GA1UEAxMHc2VsZi1jYTBZMBMGByqG
SM49AgEGCCqGSM49AwEHA0IABFpsh31HJU1U7l2+znRl2y+QYE+jqB6mOt0bHCQh
RUwPVE2dK2TZJPH0N4aAdY65ofx36DGBBS5hBIRIl+VTytAwCQYHKoZIzj0EAQNI
ADBFAiBIt2iguenzw0/q0Ar2IzJMWnk06tiIiCp6/S62EPpe9wIhAJ4Ffuqg9fia
sm6gY3d9V+RDrUNDhwgGpdlIOyudMMWF
-----END CERTIFICATE-----
MacBookPro-makmorit-jp:FIDO makmorit$
```

【ご注意】macOSに導入されているopenSSLは古いバージョンですので、開発目的だけで使用するようにします。
```
MacBookPro-makmorit-jp:~ makmorit$ which openssl
/usr/bin/openssl
MacBookPro-makmorit-jp:~ makmorit$ openssl version
OpenSSL 0.9.8zh 14 Jan 2016
MacBookPro-makmorit-jp:~ makmorit$
```

## U2F管理ツールの準備

[U2F管理ツール](../U2FMaintenanceTool/) をGitHubから取得します。

macOS環境の場合は [U2FMaintenanceTool.pkg](../U2FMaintenanceTool/macOSApp/U2FMaintenanceTool.pkg) をダウンロード後、インストールを実行します。

Windows環境の場合は [U2FMaintenanceToolGUI.exe](../U2FMaintenanceTool/WindowsExe/BLE/U2FMaintenanceToolGUI.exe) をダウンロードします。

## 鍵・証明書のインストール

U2F管理ツールを使用して、秘密鍵ファイル(.pem)、証明書ファイル(.crt)を、One Cardにインストールします。

### macOS環境の場合

U2F管理ツール（U2FMaintenanceTool.app）を起動します。<br>
表示された画面の「鍵・証明書消去／AES暗号生成」ボタンをクリックします。

<img src="../assets/0027.png" width="550">

One Card側の処理が成功すると「鍵・証明書削除処理が成功しました。」と表示されます。

<img src="../assets/0028.png" width="550">

続いて、秘密鍵ファイル(.pem)、証明書ファイル(.crt)をそれぞれ「参照」ボタンをクリックして選択します。

<img src="../assets/0029.png" width="550">

U2F管理ツール画面の「鍵・証明書ファイルのインストール」ボタンをクリックします。

<img src="../assets/0030.png" width="550">

One Card側の処理が成功すると「鍵・証明書インストール処理が成功しました。」と表示されます。

<img src="../assets/0031.png" width="550">

これで、鍵・証明書のインストールは完了です。

### Windows環境の場合

U2F管理ツール（U2FMaintenanceToolGUI.exe）を起動します。<br>
表示された画面の「鍵・証明書消去／AES暗号生成」ボタンをクリックします。

<img src="../assets/0023.png" width="550">

One Card側の処理が成功すると「鍵・証明書削除処理が成功しました。」と表示されます。

<img src="../assets/0024.png" width="550">

続いて、秘密鍵ファイル(.pem)、証明書ファイル(.crt)をそれぞれ「参照」ボタンをクリックして選択します。

<img src="../assets/0025.png" width="550">

U2F管理ツール画面の「鍵・証明書ファイルのインストール」ボタンをクリックします。

<img src="../assets/0026.png" width="550">

One Card側の処理が成功すると「鍵・証明書インストール処理が成功しました。」と表示されます。

<img src="../assets/0067.png" width="550">

これで、鍵・証明書のインストールは完了です。

## U2F管理ツールによる動作確認（ヘルスチェック）

One Cardにインストールされた秘密鍵と署名済み証明書を使用し、U2F管理ツールを使用して動作確認（ヘルスチェック）を実行することができます。

### macOS環境の場合

U2F管理ツール（U2FMaintenanceTool.app）を起動します。<br>
表示された画面の「ヘルスチェック実行」ボタンをクリックします。

<img src="../assets/0032.png" width="550">

One Card側で処理が進み、ほどなくOne Card上の３番目のLEDが<font color=ff0000><b>点灯</b></font>します。<br>
（ユーザー所在確認を求めるため、One Card側の処理が一時的に中断されます）

ここでMAIN SWを１回押しますと、再びOne Card側の処理が再開されます。

<img src="../assets/0033.png" width="550">

One Card側の処理が成功すると「ヘルスチェックが成功しました。」と表示されます。

<img src="../assets/0034.png" width="550">

これでヘルスチェックは完了です。

### windows環境の場合

U2F管理ツール（U2FMaintenanceToolGUI.exe）を起動します。<br>
表示された画面の「ヘルスチェック実行」ボタンをクリックします。

<img src="../assets/0068.png" width="550">

One Card側で処理が進み、ほどなくOne Card上の３番目のLEDが<font color=ff0000><b>点灯</b></font>します。<br>
（ユーザー所在確認を求めるため、One Card側の処理が一時的に中断されます）

ここでMAIN SWを１回押しますと、再びOne Card側の処理が再開されます。

<img src="../assets/0033.png" width="550">

One Card側の処理が成功すると「ヘルスチェックが成功しました。」と表示されます。

<img src="../assets/0069.png" width="550">

これでヘルスチェックは完了です。
