# FIDO2ローカルテストサーバー構築手順

Yubico社が無償公開している「python-fido2」をmacOSに導入する手順を掲載しております。

## インストール

### pipenvの導入

この「python-fido2」を稼働させるために必要な、pipenvをインストールします。

下記URLを参考にしてインストールを進めます。<br>
https://pipenv.readthedocs.io/en/latest/

下記は実行例になります。
```
MacBookPro-makmorit-jp:~ makmorit$ brew install pipenv
Updating Homebrew...
==> Auto-updated Homebrew!
Updated 1 tap (homebrew/core).
==> New Formulae
aom                 easyengine          kubespy             php@7.2             qalculate-gtk       up
astrometry-net      fx                  minica              pict                rargs               websocat
curl-openssl        i386-elf-gdb        opa                 postgresql@10       sng
==> Updated Formulae
mandoc ✔                      fluxctl                       libtorrent-rasterbar          rbspy
openssl@1.1 ✔                 flyway                        libuv                         rclone
:
autosuggestions
==> Renamed Formulae
gutenberg -> zola                       hh -> hstr                              mat -> mat2
==> Deleted Formulae
apple-gcc42                   aptly-completion              php@7.0                       pldebugger

Error: The following directories are not writable by your user:
/usr/local/share/man/man3

You should change the ownership of these directories to your user.
  sudo chown -R $(whoami) /usr/local/share/man/man3
MacBookPro-makmorit-jp:~ makmorit$
```

最後の方で、
```
Error: The following directories are not writable by your user:
```
というエラーが表示された場合は、上記メッセージの通り `sudo chown -R $(whoami) /usr/local/share/man/man3` を実行し、その後 `brew install pipenv` を再実行させます。

```
MacBookPro-makmorit-jp:~ makmorit$ sudo chown -R $(whoami) /usr/local/share/man/man3
Password:
MacBookPro-makmorit-jp:~ makmorit$ brew install pipenv
==> Installing dependencies for pipenv: gdbm, openssl, readline, sqlite, xz and python
==> Installing pipenv dependency: gdbm
==> Downloading https://homebrew.bintray.com/bottles/gdbm-1.18.1.sierra.bottle.tar.gz
:
==> pipenv
Bash completion has been installed to:
  /usr/local/etc/bash_completion.d
MacBookPro-makmorit-jp:~ makmorit$
```

`Bash completion has been installed to: /usr/local/etc/bash_completion.d`というメッセージが表示されれば、インストールは完了となります。

### 「python-fido2」のダウンロードと配置

GitHubからリポジトリー「python-fido2」を全量チェックアウトして、適宜別のフォルダーにコピーします。
https://github.com/Yubico/python-fido2

今回は `~/GitHub/onecard-fido/FIDO2DemoServer` 配下に、チェックアウトしたファイル全量ではなく、必要なファイルだけをコピーしました。

```
MacBookPro-makmorit-jp:python-fido2 makmorit$ pwd
/Users/makmorit/GitHub/onecard-fido/FIDO2DemoServer/python-fido2
MacBookPro-makmorit-jp:python-fido2 makmorit$ ls -al
total 8
drwxr-xr-x@  5 makmorit  staff   170 12 11 15:31 .
drwxr-xr-x   5 makmorit  staff   170 12 11 15:18 ..
drwxr-xr-x   7 makmorit  staff   238 12 11 14:30 examples
drwxr-xr-x  17 makmorit  staff   578 12 11 14:30 fido2
-rwxr-xr-x   1 makmorit  staff  3154 12 11 14:19 setup.py
MacBookPro-makmorit-jp:python-fido2 makmorit$
```

- examples - サーバーソフトウェアが格納されています。
- fido2 - Yubico社が制作したFIDO2サーバーライブラリーが格納されています。
- setup.py - ローカルテストサーバー起動用のvirtualenv作成時に必要となるスクリプト


## サーバーの作成／始動確認

### サーバーの作成

サブフォルダー `examples/server` に移動し、コマンド `pipenv install` を実行します。

```
MacBookPro-makmorit-jp:~ makmorit$ cd /Users/makmorit/GitHub/onecard-fido/FIDO2DemoServer/python-fido2/examples/server
MacBookPro-makmorit-jp:server makmorit$ ls -al
total 72
drwxr-xr-x  8 makmorit  staff   272 12 11 14:41 .
drwxr-xr-x  7 makmorit  staff   238 12 11 14:30 ..
-rw-r--r--@ 1 makmorit  staff  6148 12 11 14:56 .DS_Store
-rw-r--r--  1 makmorit  staff   202 12 11 14:19 Pipfile
-rw-r--r--  1 makmorit  staff  9119 12 11 14:19 Pipfile.lock
-rw-r--r--  1 makmorit  staff  1628 12 11 14:19 README.adoc
-rw-r--r--  1 makmorit  staff  4351 12 11 14:19 server.py
drwxr-xr-x  6 makmorit  staff   204 12 11 14:19 static
MacBookPro-makmorit-jp:server makmorit$ pipenv install
Installing dependencies from Pipfile.lock (ccdd4d)…
Ignoring enum34: markers 'python_version < "3"' don't match your environment
Ignoring ipaddress: markers 'python_version < "3"' don't match your environment
  🐍   ▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉▉ 16/16 — 00:00:03
To activate this project's virtualenv, run pipenv shell.
Alternatively, run a command inside the virtualenv with pipenv run.
MacBookPro-makmorit-jp:server makmorit$
```

上記の通り、正しくvirtualenvが作成できれば、サーバー作成は完了となります。

### サーバーの始動確認

コマンド `pipenv run server` でvirtualenvを起動し、FIDO2ローカルテストサーバーを始動させます。

```
MacBookPro-makmorit-jp:~ makmorit$ cd /Users/makmorit/GitHub/onecard-fido/FIDO2DemoServer/python-fido2/examples/server
MacBookPro-makmorit-jp:server makmorit$ pipenv run server

Example demo server to use a supported web browser to call the WebAuthn APIs
to register and use a credential.

See the file README.adoc in this directory for details.

Navigate to https://localhost:5000 in a supported web browser.

 * Serving Flask app "server" (lazy loading)
 * Environment: production
   WARNING: Do not use the development server in a production environment.
   Use a production WSGI server instead.
 * Debug mode: on
 * Running on https://127.0.0.1:5000/ (Press CTRL+C to quit)
 * Restarting with stat

Example demo server to use a supported web browser to call the WebAuthn APIs
to register and use a credential.

See the file README.adoc in this directory for details.

Navigate to https://localhost:5000 in a supported web browser.

 * Debugger is active!
 * Debugger PIN: 258-954-445
```

正しくサーバーの始動が確認できたら、FIDO2ローカルテストサーバー構築は完了となります。
