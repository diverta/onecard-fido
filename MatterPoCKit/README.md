# Matter評価キット

## 概要
新しいスマートホーム共通規格「[Matter](https://buildwithmatter.com)」についての評価用ハードウェア／ソフトウェアです。

## 構成

本キットは、Matterコントローラー／デバイス／ハブの３点から構成されます。

<img src="assets01/0000.jpg" width="700">

#### Matterコントローラー
Matterコマンドを実行させる側の、Androidスマートフォン用のアプリです。

<img src="assets01/0001.jpg" width="160">

#### Matterデバイス
Matterコマンドを実行する側となる、nRF52840を組み込んだデバイスです。<br>
以前製作した「[MDBT50Q Dongle](../FIDO2Device/MDBT50Q_Dongle/README.md)」を、当面使用する想定です。

<img src="assets01/0002.jpg" width="320">

#### Matterハブ
Matterコントローラー〜Matterデバイス間の中継役（ハブ）となるデバイスです。<br>
Raspberry Pi 3 Model Bを使用しています。

<img src="assets01/0003.jpg" width="480">

#### Matterコマンド
この評価用キットでは、数ある[Matterコマンド](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/data_model/gen/CHIPClusters.h)のうち、`OnOffCluster`というコマンド群（クラスター）をサポートします。<br>
以下のコマンドがあります。
- `OffCommand`
- `OnCommand`
- `ToggleCommand`

## 手順書

- <b>[Matterコントローラー導入手順]()</b><br>
コントローラーアプリを、Android環境にインストールする手順について掲載します。

- <b>[Matterデバイスアプリ導入手順](../MatterPoCKit/INSTALLFW.md)</b><br>
Matterデバイスアプリ（ファームウェア）をnRF52840環境にインストールする手順について掲載します。

- <b>[Matterハブ導入手順](../MatterPoCKit/SETUPHUB.md)</b><br>
Matterハブを構築する手順について掲載します。

- <b>[Matterコマンド実行手順]()</b><br>
コントローラーアプリを使用し、デバイスにMatterコマンドを送信する手順について掲載します。
