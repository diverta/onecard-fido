# PIVデバイスを使用したmacOSログイン手順

NUCLEO-L432KCに導入したPIVデバイス参考実装「[canokeys/canokey-stm32](https://github.com/canokeys/canokey-stm32)」を使用し、macOSにPINでログインする手順を掲載いたします。

## 準備手順

下記手順書により、canokey-stm32の準備を行います。

- <b>[参考：Yubico PIV Toolビルド手順](../../Research/CCID/BUILDYKPIV.md)</b><br>
[Yubico PIV Tool（command line）](https://www.yubico.com/products/services-software/download/smart-card-drivers-tools/)を一部修正し、macOS環境上で使用できるようにします。

- <b>[Yubico PIV Toolによる初期データ導入手順](../../Research/CCID/SETUPYKPIV.md)</b><br>
Yubico PIV Toolを使用して、canokey-stm32に対し、鍵・証明書・PINなどの機密データを初期導入します。

## 作業手順

#### スマートカードペアリングの実行

canokey-stm32を、PC（MacBook Pro）のUSBポートに装着します。<br>
初回装着の場合、以下のようなダイアログが表示されます。

「PIV認証用証明書」を選択し「ペアリング」ボタンをクリックします。

<img src="reference/assets01/0016.png" width="400">

下図のようなダイアログが表示されます。<br>
macOSユーザーのパスワードを入力します。

<img src="reference/assets01/0017.png" width="400">

下図のようなダイアログが表示されます。<br>
手順書「[Yubico PIV Toolによる初期データ導入手順](../../Research/CCID/SETUPYKPIV.md)」で設定したPIN番号（以下単に「PIN」と称します）を入力します。

<img src="reference/assets01/0018.png" width="400">

下図のようなダイアログが表示されます。<br>
キーチェーンのパスワード（通常はユーザーパスワードと同じ）を入力します。

<img src="reference/assets01/0019.png" width="400">

ダイアログが閉じられ、スマートカードペアリングが完了します。

#### macOSにログイン

いったんmacOSからログオフし、canokey-stm32を、PCのUSBポートから外します。

<img src="reference/assets01/0020.png" width="400">

macOSのログイン画面が表示されます。<br>
この後、ふたたびcanokey-stm32を、PC（MacBook Pro）のUSBポートに装着します。

下図のように、ユーザーパスワードではなく、PINを入力するためのボックスが表示されます。

<img src="reference/assets01/0021.jpeg" width="400">

PINを入力して、ログインを実行します。

<img src="reference/assets01/0022.jpeg" width="400">

通常のパスワードによるログインと同様、macOSにログインできます。

<img src="reference/assets01/0023.jpeg" width="400">

以上で、PIVデバイスを使用したmacOSログインは完了になります。
