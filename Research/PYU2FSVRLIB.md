# U2Fライブラリーサーバー導入手順

## 概要

Yubikeyの製造元であるYubico社が、無償で公開している、PythonベースのU2Fライブラリーサーバーの導入手順を掲載します。

以下のドキュメントを参考にいたしました。<br>
[Python based U2F server library](https://developers.yubico.com/python-u2flib-server/)

以下は、macOS上に導入する手順になります。

## pipコマンドのアップデート

macOSに導入されているpipコマンドのバージョンが古い場合は、依存パッケージが正しくインストールできない可能性があります。<br>
具体的には以下のようなエラーが発生したりします。
```
`Could not fetch URL https://xxxxxxxx/xxx/xxx/: There was a problem confirming the ssl certificate: [SSL: TLSV1_ALERT_PROTOCOL_VERSION] tlsv1 alert protocol version (_ssl.c:646) - skipping
```

pipコマンドのバージョンが古いために発生するSSL障害のようですので、この場合はpipコマンドをアップデートします。<br>
以下は、pipコマンドのアンインストール--->インストールを実行した例になります。

#### アンインストール
コマンド`sudo pip uninstall pip`を実行します。

```
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ sudo pip uninstall pip
Password:
The directory '/Users/makmorit/Library/Caches/pip/http' or its parent directory is not owned by the current user and the cache has been disabled. Please check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
Uninstalling pip-9.0.1:
  /Library/Python/2.7/site-packages/pip-9.0.1.dist-info/DESCRIPTION.rst
  /Library/Python/2.7/site-packages/pip-9.0.1.dist-info/INSTALLER
  /Library/Python/2.7/site-packages/pip-9.0.1.dist-info/METADATA
  :
  /usr/local/bin/pip
  /usr/local/bin/pip2
  /usr/local/bin/pip2.7
Proceed (y/n)? y
  Successfully uninstalled pip-9.0.1
The directory '/Users/makmorit/Library/Caches/pip/http' or its parent directory is not owned by the current user and the cache has been disabled. Please check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
MacBookPro-makmorit-jp:python-u2flib-server makmorit$
```

#### インストールスクリプトの取得
`curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py`を実行します。
```
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 1603k  100 1603k    0     0  1115k      0  0:00:01  0:00:01 --:--:-- 1116k
MacBookPro-makmorit-jp:python-u2flib-server makmorit$
```

#### インストール実行
`python get-pip.py --user`を実行すると、インストールがスタートします。
```
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ python get-pip.py --user
Collecting pip
  Using cached https://files.pythonhosted.org/packages/0f/74/ecd13431bcc456ed390b44c8a6e917c1820365cbebcb6a8974d1cd045ab4/pip-10.0.1-py2.py3-none-any.whl
matplotlib 1.3.1 requires nose, which is not installed.
matplotlib 1.3.1 requires tornado, which is not installed.
Installing collected packages: pip
Successfully installed pip-10.0.1
MacBookPro-makmorit-jp:python-u2flib-server makmorit$
```

`.bash_profile`にPythonパスを設定して、pipコマンドが実行できるようにします。
```
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ echo 'export PATH="$HOME/Library/Python/2.7/bin:$PATH"' >> ~/.bash_profile
MacBookPro-makmorit-jp:python-u2flib-server makmorit$
```

## U2Fライブラリーサーバーの導入

#### ソースコード取得

以下のリポジトリーから丸ごとチェックアウトします。<br>
[Yubico/python-u2flib-server](https://github.com/Yubico/python-u2flib-server.git)

本例では、`ホームディレクトリー/GitHub/python-u2flib-server`にチェックアウトしています。

```
MacBookPro-makmorit-jp:~ makmorit$ ls -al ~/GitHub/python-u2flib-server
total 136
drwxr-xr-x@ 21 makmorit  staff   714  5 23 09:56 .
drwxr-xr-x  25 makmorit  staff   850  5 23 09:56 ..
-rw-r--r--   1 makmorit  staff   107  5 23 09:56 .coveragerc
drwxr-xr-x  14 makmorit  staff   476  5 23 09:56 .git
-rw-r--r--   1 makmorit  staff   166  5 23 09:56 .gitignore
-rw-r--r--   1 makmorit  staff   135  5 23 09:56 .pre-commit-config.yaml
-rw-r--r--   1 makmorit  staff   494  5 23 09:56 .travis.yml
-rw-r--r--   1 makmorit  staff   221  5 23 09:56 BLURB
-rw-r--r--   1 makmorit  staff  1304  5 23 09:56 COPYING
-rw-r--r--   1 makmorit  staff    85  5 23 09:56 MANIFEST.in
-rw-r--r--   1 makmorit  staff  2309  5 23 09:56 NEWS
-rw-r--r--   1 makmorit  staff  4542  5 23 09:56 README
lrwxr-xr-x   1 makmorit  staff     6  5 23 09:56 README.adoc -> README
-rw-r--r--   1 makmorit  staff    19  5 23 09:56 dev-requirements.txt
drwxr-xr-x   3 makmorit  staff   102  5 23 09:56 examples
-rw-r--r--   1 makmorit  staff  6934  5 23 09:56 release.py
-rw-r--r--   1 makmorit  staff   116  5 23 09:56 setup.cfg
-rwxr-xr-x   1 makmorit  staff  2846  5 23 09:56 setup.py
drwxr-xr-x   9 makmorit  staff   306  5 23 09:56 test
-rw-r--r--   1 makmorit  staff   222  5 23 09:56 tox.ini
drwxr-xr-x   7 makmorit  staff   238  5 23 09:56 u2flib_server
MacBookPro-makmorit-jp:~ makmorit$
```

#### 依存パッケージの導入

次にwebobというパッケージを追加インストールします。<br>
`sudo pip install webob`を実行します。

```
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ sudo pip install webob
Password:
The directory '/Users/makmorit/Library/Caches/pip/http' or its parent directory is not owned by the current user and the cache has been disabled. Please check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
The directory '/Users/makmorit/Library/Caches/pip' or its parent directory is not owned by the current user and caching wheels has been disabled. check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
Collecting webob
  Downloading https://files.pythonhosted.org/packages/6f/f8/b2ce2bacd1e63840224af7169536ef0f8c2da7fcf2085bfb0b0d0a6edf4a/WebOb-1.8.1-py2.py3-none-any.whl (115kB)
    100% |████████████████████████████████| 122kB 1.0MB/s
matplotlib 1.3.1 requires nose, which is not installed.
matplotlib 1.3.1 requires tornado, which is not installed.
Installing collected packages: webob
Successfully installed webob-1.8.1
MacBookPro-makmorit-jp:python-u2flib-server makmorit$
```

## U2Fライブラリーサーバーの実行確認

#### U2Fライブラリーサーバーの起動
先述`~/GitHub/python-u2flib-server`のexamplesフォルダーに格納されている`u2f_server.py`を、リポジトリーのルートディレクトリーにコピーし、Pythonで実行すれば、U2Fサーバーが動作します。

```
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ pwd
/Users/makmorit/GitHub/python-u2flib-server
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ cp -p examples/u2f_server.py .
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ python ./u2f_server.py
[23/May/2018 10:01:17] Starting server on http://localhost:8081
```

U2Fライブラリーサーバーはポート8081でHTTPリクエストを待機します。

#### enrollコマンドで動作確認

試しにenrollコマンドを実行します。
U2F Registerで使用するチャレンジ（Web-safe B64デコードされた文字列）がレスポンスされます。

```
MacBookPro-makmorit-jp:~ makmorit$ curl http://localhost:8081/enroll
{"registeredKeys": [], "registerRequests": [{"challenge": "k9rWGblE3LhnEOWDkKHyOTsj68kt8tZ4nUSkpdwF0BY", "version": "U2F_V2"}], "appId": "http://localhost:8081"}
```

サーバーを起動したターミナル画面には、下記のようなアクセスログが出力されます。

```
127.0.0.1 - - [23/May/2018 10:02:16] "GET /enroll HTTP/1.1" 200 161
```

以上で、U2Fライブラリーサーバー導入は完了です。
