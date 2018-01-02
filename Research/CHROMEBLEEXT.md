# [WIP] ブラウザーエクステンションの調査

PC環境で動作するFIDO U2F BLEエクステンションを、Chromeエクステンションで実装できるかどうかの継続調査です。<br>
調査時の手順および結果を、以下に掲載いたします。

## ChromeのNative Messagingによる実装 ---> 対応不可

Chrome<--->サブプロセス間通信（Native Messaging）がGoogle Chromeで用意されていました。

参考URL：<br>
https://developer.chrome.com/extensions/nativeMessaging

具体的には、Chromeから起動したサブプロセスと、エクステンションの間で、標準入出力（STDIO）経由でやり取りをすることができるようです。

ただし（動作確認中に発覚したのですが）確認用に使用したmacOS版Chromeでは、2018年度以降サポートされなくなる（＝Chromeストアからの提供ができなくなってしまう）のことです。<br>
下記URLに、正式アナウンスがあります。<br>
https://blog.chromium.org/2016/08/from-chrome-apps-to-web.html

ご参考までに、マイナビニュースというサイトが、[Googleアプリのサポート終了の旨を報じています](https://news.mynavi.jp/article/20160822-a034/)。<br>
<img src="assets/0024.png" width="600">

### サンプルによる動作確認

下記URLで公開されているサンプルを使用して動作確認します。<br>
https://developer.chrome.com/extensions/nativeMessaging#examples

#### サンプルの取得

こちらからダウンロードできます。<br>
[The examples/api/nativeMessaging directory](https://chromium.googlesource.com/chromium/src/+/master/chrome/common/extensions/docs/examples/api/nativeMessaging)
<img src="assets/0012.png" width="800">

#### サブプロセスの導入

ダウンロードしたファイルを任意のフォルダーに展開します。<br>
<img src="assets/0013.png" width="500">

フォルダー「host」に格納されているインストール用スクリプト「install_host.sh」を実行して、サブプロセスをインストールします。<br>
<img src="assets/0014.png" width="750">

#### エクステンションの導入

拡張機能ページの「Load unpacked extension...」をクリックします。<br>
<img src="assets/0015.png" width="750">

先述のエクステンション配置フォルダー内の「app」を選択します。<br>
<img src="assets/0016.png" width="500">

拡張機能ページにエクステンション「Native Messaging Example」が追加されます。<br>
<img src="assets/0017.png" width="750">

#### Chromeアプリの実行

Chromeアプリページ（[chrome://apps](chrome://apps)）の「Native Messaging...」をクリックします。<br>
<img src="assets/0018.png" width="750">

HTMLページの「Connect」ボタンをクリックします。<br>
<img src="assets/0019.png" width="750">

HTMLページにテキストボックスと「Send」ボタンが表示されます。<br>
メッセージを入力し「Send」ボタンをクリックします。<br>
<img src="assets/0020.png" width="750">

HTMLページに実行結果が表示されます。<br>
<img src="assets/0021.png" width="750">

裏で立ち上がっているサブプロセス（PythonのGUIプログラム）の画面を見ると、先ほど「Send」したメッセージが表示されていることが確認できます。<br>
その後、サブプロセス側からメッセージを送信してみます。<br>
画面にメッセージを入力して「Send」ボタンをクリックします。<br>
<img src="assets/0022.png" width="750">

HTMLページ、サブプロセス画面の両方に実行結果が表示されます。<br>
<img src="assets/0023.png" width="750">

これで、Chrome<--->サブプロセス間で通信（Native Messaging）ができていることが確認できました。

#### 確認結果からの考察

別途用意している[U2F管理ツール](../../U2FMaintenanceTool/macOSApp)を改修することにより、ChromeブラウザーとNative Messagingを介した通信は可能と考えられます。

他方、先述の通り、macOS上でNative Messagingが使用できるChromeアプリが、2018年度以降Chromeストアで正式提供できなくなるとのことです。

なので、Native Messagingによる対応の検討は見送りとし、[他の代替え手段](https://developers.chrome.com/apps/migration)による検討を継続、とさせていただきます。
