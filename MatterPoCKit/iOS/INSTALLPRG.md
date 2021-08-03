# Matterコントローラーアプリ導入手順

最終更新日：2021/08/03

## 概要
MatterコントローラーアプリをiOS環境（iPhone）にインストールする手順について掲載しています。

#### 制約

最終更新日現在、開発者がApple Developer Programメンバーシップ（Apple社から承認されている開発アカウント）を保有していないため、アプリをインターネットからダウンロード-->インストールという通常の導入手順が利用できません。

したがって、iPhoneとPCをUSB接続した上で、Xcodeを使用してアプリのインストールを行う必要があります。

#### 動作確認時の環境

- iPhone SE (1st generation) - iOS 13.3

- iMac Pro (2019) - macOS Catalina (Version 10.15.5)

- Xcode (Version 11.5)

## Xcodeの準備

iOS環境では、Apple社が承認していないアプリケーションのインストールができないようになっています。<br>
従いまして、本プロジェクトで制作したMatterコントローラーアプリは、開発ツール「Xcode」を使用し、インストールを行う必要があります。

アプリのインストールでは、Xcodeを使用し、ソースコードのビルドからiPhoneへのアプリ転送までを一気通貫で行います。

#### Xcodeアカウントの準備

Xcodeに開発用のユーザーアカウントを設定しておきます。<br>
Xcodeのメニューから「`Preferences`」を実行し、設定画面を開きます。

<img src="../assets03/0015.jpg" width="300">

設定画面のタブ「`Accounts`」で、任意のApple IDを追加します。<br>
`Team`欄に、`Personal Team`が表示されていれば、準備は完了です。

<img src="../assets03/0016.jpg" width="400">

## アプリの準備

[Matterコントローラーアプリ](../../MatterPoCKit/iOS)を、[GitHubリポジトリー](https://github.com/diverta/onecard-fido/tree/research-Matter-20210726)からチェックアウトします。<br>
下図は、チェックアウトされたGitHubリポジトリーにおけるMatterコントローラーのソースコードの位置を示しています。

<img src="../assets03/0010.jpg" width="400">

macOSにインストールされたXcodeにより、ワークスペースを開きます。<br>
上図のファイル「`app.xcworkspace`」をダブルクリックします。

Xcodeのワークスペース画面が開きます。<br>
この時点ではPCにiPhoneが接続されていないので、ターゲットのデバイス名は「Generic iOS Device」となっています。

<img src="../assets03/0011.jpg" width="600">

#### Xcodeアカウントの設定

開いたXcodeワークスペース画面の左ペインで、プロジェクト`mattercontroller`を選択後、画面右側のタブ`Signing & Capabilities`を開きます。<br>
`Team`欄が`None`となっています。

<img src="../assets03/0017.jpg" width="600">

前述「Xcodeアカウントの準備」でXcodeに設定しておいた`Personal Team`を、`Team`欄のプルダウンから選択し、設定します。

<img src="../assets03/0018.jpg" width="600">

## アプリのインストール

iPhoneをPCに接続します。<br>
その後Xcode画面上で、ターゲットを、接続したiPhoneに切り替えます。

<img src="../assets03/0012.jpg" width="300">

画面左上部の黒い三角形のアイコンをクリックすると、ビルド-->インストールが開始されます。

<img src="../assets03/0013.jpg" width="300">

ほどなくアプリのインストールが完了します。<br>
iPhone上でアプリが自動的に始動し、デバッグプリントがXcode画面下部に出力されます。

<img src="../assets03/0014.jpg" width="600">

以上で、アプリのインストールは完了になります。
