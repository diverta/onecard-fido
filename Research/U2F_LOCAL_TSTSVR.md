# U2Fテスト用ローカルサーバー構築手順

macOS上のApacheで稼働するローカルWebサーバーに、U2Fライブラリーサーバーを組み込む手順を掲載いたします。

以下の手順によりすでにApacheとU2Fライブラリーサーバーが稼働していることを前提といたします。

- [Apache ローカルSSLサーバー構築手順](APACHE_LOCAL_TSTSVR.md)

- [U2Fライブラリーサーバー導入手順](PYU2FSVRLIB.md)

## RP名（ホスト名）

今回は仮に `www.makmorit.jp` とします。

これを一時的に`/etc/hosts`ファイルに記述しておけば、ドメイン作成やDNS設定は不要なようです。

```
MacBookPro-makmorit-jp:~ makmorit$ cat /etc/hosts
（中略）
192.168.100.100 www.makmorit.jp
```

## U2FライブラリーサーバーをApacheに組込

#### バーチャルホスト設定ファイルの変更

`/private/etc/apache2/extra/httpd-ssl.conf`を変更します。<br>
具体的には`ProxyPass`に、U2Fライブラリーサーバーの稼働パスを指定します。

```
MacBookPro-makmorit-jp:~ makmorit$ diff /private/etc/apache2/extra/httpd-ssl.conf.original /private/etc/apache2/extra/httpd-ssl.conf
120d119
<
125c124
< ServerName www.example.com:443
---
> ServerName www.makmorit.jp:443
129a129,130
> ProxyPass /python-u2flib-server/ http://www.makmorit.jp:8081/
>
MacBookPro-makmorit-jp:~ makmorit$
```

#### U2Fライブラリーサーバーを始動

U2Fライブラリーサーバーを、上記で設定したパスで稼働するようにします。
具体的には`u2f_server.py`に、引数`-i www.makmorit.jp`を与えて起動させるようにします。

```
MacBookPro-makmorit-jp:python-u2flib-server makmorit$ python ./u2f_server.py -i www.makmorit.jp
[23/May/2018 11:02:48] Starting server on http://www.makmorit.jp:8081
```

その後、Apacheを再起動させます。

```
MacBookPro-makmorit-jp:~ makmorit$ sudo apachectl restart
```

#### 動作確認

Chromeブラウザーを起動し、確認用のコマンド`enroll`を実行してみます。<br>
アドレスバーにURL`https://<RP名>/python-u2flib-server/enroll`を指定して実行します。

<img src="assets01/0018.png" width="700">

サーバー証明書が自己署名なので、いくつかの警告が表示されてしまいますが、テストなので静観します。

<img src="assets01/0019.png" width="700">

下図のように、`enroll`の実行結果がブラウザー上に表示されれば成功です。

<img src="assets01/0020.png" width="700">
