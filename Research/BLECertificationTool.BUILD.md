# FIDO U2F認証テストツールビルド手順

FIDOアライアンスから提供されている、FIDO U2F認証取得のための事前テストツール（BLECertificationTool.exe）について、ビルド手順を掲載しています。

## ツールのソースコード

ツールのソースコードは、下記GitHubリポジトリーで無償公開されております。
* Download and build code for Authenticator BLE Transport Layer Testing<br>
https://github.com/fido-alliance/jovasco-u2f-ref-code/tree/master/u2f-tests/BLE

本プロジェクトでは、ソースコードをカスタマイズしないで使用します。<br>
ですのでソースコードは本GitHubリポジトリーでは管理しません。

## ビルド準備

### ビルドツールの導入

Visual Studio 2015をインストールします。

まずはMicrosoftのサイトから「Visual C ++ 再頒布可能パッケージ」をダウンロードします。<br>
URL: https://www.microsoft.com/ja-jp/download/details.aspx?id=48145

ダウンロードした「wdexpress_JPN__1785415392.1497837634.exe」というファイルを実行し、インストールを実施します。<br>
１時間ほどかかります。

### ソースのチェックアウト

GitHubからリポジトリー「jovasco-u2f-ref-code」をチェックアウトし、任意のフォルダー配下にソースプログラムを展開します。<br>
URL: https://github.com/fido-alliance/jovasco-u2f-ref-code.git <br><br>
￼
<img src="../assets/0061.png" width="750">

### メイクファイル修正

３２ビットマシンの場合「jovasco-u2f-ref-code\u2f-tests\BLEMakefile.win」内に記述されているパス「Program Files (x86)」を、単に「Program Files」と修正します。<br>
（下図の通り、３箇所あります）

<img src="../assets/0062.png" width="800">


## ビルド実行

BLECertificationTool.exeのビルドを実行します。

「jovasco-u2f-ref-code\u2f-tests\BLECertificationTools\BLECertificationTools.sln」をダブルクリックすると、前述で導入したVisual Studio 2015が立ち上がります。<br>
ここで、ビルドを実行します。
￼<br><br>
<img src="../assets/0063.png" width="800">

エラーが発生しなければ、ビルドは成功です。
￼<br><br>
<img src="../assets/0064.png" width="800">
￼
### エラー発生時の対処

リビルド時にエラーが発生する場合は「jovasco-u2f-ref-code\u2f-tests\BLE\」配下で直接コマンド「del \*.exe \*.obj」を実行後、ビルドを再試行してみます。<br>
ビルドが成功すると「jovasco-u2f-ref-code\u2f-tests\BLE\」配下に実行ファイルが生成されていることを確認します。
￼

### 初期動作確認

実行ファイル「jovasco-u2f-ref-code\u2f-tests\BLE\BLECertificationTool.exe」を実行してみます。
<br><br>
<img src="../assets/0065.png" width="750">

下図はhelpを表示させたところですが、エラーが発生しなければ、初期動作確認はOKです。
<br><br>
<img src="../assets/0066.png" width="800">

以上で、FIDO U2F認証テストツールのビルド作業は完了です。
