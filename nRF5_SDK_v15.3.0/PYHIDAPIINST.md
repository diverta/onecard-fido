# pyhidapiインストール手順

USB HIDデバイス通信用のPython3ライブラリー「[pyhidapi](https://github.com/apmorton/pyhidapi)」を導入する手順を記載します。

## 前提
macOSに初期導入されているPython3環境を利用します。<br>
また、事前にXcodeコマンドラインツールの導入が必要となります。

#### 動作確認環境
macOS Catalina (Version 10.15.5)<br>
Command Line Tools for Xcode 11.5

## インストール

### Python3ライブラリーのインストール

以下のコマンドを実行します。

```
sudo pip3 install hidapi
```

下記は実行例になります。

```
makmorit@iMac-makmorit-jp ~ % sudo pip3 install hidapi                                 
The directory '/Users/makmorit/Library/Caches/pip/http' or its parent directory is not owned by the current user and the cache has been disabled. Please check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
The directory '/Users/makmorit/Library/Caches/pip' or its parent directory is not owned by the current user and caching wheels has been disabled. check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
Collecting hidapi
  Downloading https://files.pythonhosted.org/packages/ee/e9/b2ec08690c280a0eaa4777bf829db6b5d269903d4e8e9ce82f079c837d5a/hidapi-0.9.0.post3.tar.gz (57kB)
    100% |████████████████████████████████| 61kB 425kB/s
Requirement already satisfied: setuptools>=19.0 in /Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.7/lib/python3.7/site-packages (from hidapi) (40.8.0)
Installing collected packages: hidapi
  Running setup.py install for hidapi ... done
Successfully installed hidapi-0.9.0.post3
You are using pip version 19.0.3, however version 20.2b1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
makmorit@iMac-makmorit-jp ~ %
```

### HomeBrewのインストール

PCにHomeBrewがインストールされていない場合は、追加でインストールします。<br>
以下のコマンドを実行します。

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

下記は実行例になります。

```
makmorit@iMac-makmorit-jp ~ % /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
Password:
==> This script will install:
/usr/local/bin/brew
/usr/local/share/doc/homebrew
（中略）
Cloning into '/usr/local/Homebrew/Library/Taps/homebrew/homebrew-core'...
remote: Enumerating objects: 740074, done.
remote: Total 740074 (delta 0), reused 0 (delta 0), pack-reused 740074
Receiving objects: 100% (740074/740074), 298.04 MiB | 1.10 MiB/s, done.
Resolving deltas: 100% (489870/489870), done.
Tapped 2 commands and 5086 formulae (5,359 files, 326.8MB).
Already up-to-date.
==> Installation successful!

==> Homebrew has enabled anonymous aggregate formulae and cask analytics.
Read the analytics documentation (and how to opt-out) here:
  https://docs.brew.sh/Analytics
No analytics data has been sent yet (or will be during this `install` run).

==> Homebrew is run entirely by unpaid volunteers. Please consider donating:
  https://github.com/Homebrew/brew#donations

==> Next steps:
- Run `brew help` to get started
- Further documentation:
    https://docs.brew.sh
makmorit@iMac-makmorit-jp ~ % echo $?
0
makmorit@iMac-makmorit-jp ~ %
```


### 基底ライブラリーのインストール

先述のPython3ライブラリーを稼働させるために必要な、基底ライブラリー「hidapi」を、追加でインストールします。<br>
以下のコマンドを実行します。

```
brew install hidapi
```

下記は実行例になります。

```
makmorit@iMac-makmorit-jp ~ % brew install hidapi
Updating Homebrew...
==> Downloading https://homebrew.bintray.com/bottles/hidapi-0.9.0.catalina.bottle.tar.gz
######################################################################## 100.0%
==> Pouring hidapi-0.9.0.catalina.bottle.tar.gz
🍺  /usr/local/Cellar/hidapi/0.9.0: 17 files, 140KB
makmorit@iMac-makmorit-jp ~ % echo $?
0
makmorit@iMac-makmorit-jp ~ %
```

以上で、pyhidapiのインストールは完了です。
