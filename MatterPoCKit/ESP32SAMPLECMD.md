# ESP32版サンプルアプリの動作確認手順

開発ボード「ESP32-DevKitC」に書き込まれた[ESP32版サンプルアプリ](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/esp32)の動作確認手順について掲載します。

以下のガイドを参考に作業を進めます。<br>
<b>・[CHIP ESP32 Lock Example](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/esp32/README.md)</b>

## 事前準備

ESP32版サンプルアプリの動作確認に必要な物件を準備します。

- Matterコントローラーの準備
- Matterデバイスの準備
- Matterハブの準備

### Matterコントローラーの準備

Matterコントローラーとなる、コマンドラインベースの制御ツール[`Python CHIP Device Controller`](https://github.com/project-chip/connectedhomeip/tree/master/src/controller/python)を、PCにインストールします。

#### 制御ツールのインストール

[MatterのGitHubリポジトリー](https://github.com/project-chip/connectedhomeip)に含まれている`Python CHIP Device Controller`を、提供されているシェル（[`build_python.sh`](https://github.com/project-chip/connectedhomeip/blob/master/scripts/build_python.sh)）を使用してビルド／インストールします。

下記のコマンドを実行します。

```
cd ${HOME}/GitHub/connectedhomeip
./scripts/build_python.sh -m platform
```

下記は実行例になります。

```
chip_detail_logging = false , chip_mdns = "platform"

  WELCOME TO...

  ▄███▒  ░▓█  ░▓█ ░▓█▓ ▒█████▄
 ██▒ ▀█▒  ▒█   ▒█  ░█▒  ▒█░  █░
 █▓░      ▒██████  ░█▒  ▒█▄▄▄█░
 ▓█   █▒  ▒█   ▒█  ░█░  ▒█▀
 ░▓███▀  ░▓███░▓█▒ ░█░  ▒█

  ACTIVATOR! This sets your shell environment variables.

Activating environment (setting environment variables):

  Setting environment variables for CIPD package manager...done
  Setting environment variables for Python environment.....done
  Setting environment variables for Host tools.............done

Checking the environment:
：
[331/331] stamp obj/src/controller/python/python.stamp
created virtual environment CPython3.8.2.final.0-64 in 453ms
  creator CPython3Posix(dest=/Users/makmorit/GitHub/connectedhomeip/out/python_env, clear=True, no_vcs_ignore=False, global=False)
  seeder FromAppData(download=False, pip=bundle, setuptools=bundle, wheel=bundle, via=copy, app_data_dir=/Users/makmorit/Library/Application Support/virtualenv)
    added seed packages: pip==21.0.1, setuptools==54.1.2, wheel==0.36.2
  activators BashActivator,CShellActivator,FishActivator,PowerShellActivator,PythonActivator,XonshActivator
Requirement already satisfied: pip in ./out/python_env/lib/python3.8/site-packages (21.0.1)
Collecting pip
  Using cached pip-21.2.4-py3-none-any.whl (1.6 MB)
Installing collected packages: pip
：
Collecting pyobjc-framework-Cocoa>=7.3
  Downloading pyobjc_framework_Cocoa-7.3-cp38-cp38-macosx_10_9_x86_64.whl (273 kB)
     |████████████████████████████████| 273 kB 2.8 MB/s
Building wheels for collected packages: construct
  Building wheel for construct (setup.py) ... done
  Created wheel for construct: filename=construct-2.10.67-py3-none-any.whl size=59037 sha256=694392ec7bf9a4b5a545b8f3847a78a1bd232027b38846181b2afdd9ea44a5c2
  Stored in directory: /private/var/folders/hn/087_vn3j0lq1sbh89zfjl0640000gp/T/pip-ephem-wheel-cache-1kew7_qp/wheels/e3/4e/be/d56811047059833c9eaa777779b57d6e86b34c836f050a7745
Successfully built construct
Installing collected packages: ipython-genutils, wcwidth, traitlets, pyobjc-core, ptyprocess, parso, setuptools, pyobjc-framework-Cocoa, pygments, prompt-toolkit, pickleshare, pexpect, matplotlib-inline, jedi, humanfriendly, decorator, backcall, appnope, pyobjc-framework-corebluetooth, ipython, construct, coloredlogs, chip
  Attempting uninstall: setuptools
    Found existing installation: setuptools 54.1.2
    Uninstalling setuptools-54.1.2:
      Successfully uninstalled setuptools-54.1.2
Successfully installed appnope-0.1.2 backcall-0.2.0 chip-0.0 coloredlogs-15.0.1 construct-2.10.67 decorator-5.0.9 humanfriendly-9.2 ipython-7.26.0 ipython-genutils-0.2.0 jedi-0.18.0 matplotlib-inline-0.1.2 parso-0.8.2 pexpect-4.8.0 pickleshare-0.7.5 prompt-toolkit-3.0.19 ptyprocess-0.7.0 pygments-2.10.0 pyobjc-core-7.3 pyobjc-framework-Cocoa-7.3 pyobjc-framework-corebluetooth-7.3 setuptools-57.4.0 traitlets-5.0.5 wcwidth-0.2.5

Compilation completed and WHL package installed in:
  ./out/python_env

To use please run:
  source ./out/python_env/bin/activate
bash-5.1$
```

#### 制御ツールの起動確認

制御ツールがインストールされたら、正常に起動するか確認します。<br>
以下のコマンドを実行します。

```
source ${HOME}/GitHub/connectedhomeip/out/python_env/bin/activate
chip-device-ctrl
```

以下は実行例になります。<br>
この例では、試しにBLEデバイスをスキャンしています。

```
bash-3.2$ source ${HOME}/GitHub/connectedhomeip/out/python_env/bin/activate
(python_env) bash-3.2$ chip-device-ctrl
[1629160314608] [813:11332] CHIP: [IN] local node id is 0x000000000001B669
[1629160314608] [813:11332] CHIP: [ZCL] Using ZAP configuration...
[1629160314609] [813:11332] CHIP: [ZCL] deactivate report event
[1629160314609] [813:11332] CHIP: [CTL] Getting operational keys
[1629160314609] [813:11332] CHIP: [CTL] Getting root certificate for the controller from the issuer
[1629160314609] [813:11332] CHIP: [CTL] Generating operational certificate for the controller
[1629160314609] [813:11332] CHIP: [CTL] Getting intermediate CA certificate from the issuer
[1629160314609] [813:11332] CHIP: [CTL] GetIntermediateCACertificate returned 0
[1629160314609] [813:11332] CHIP: [CTL] Generating credentials
[1629160314609] [813:11332] CHIP: [CTL] Loaded credentials successfully
[1629160314612] [813:11357] CHIP: [DL] Platform main loop started.
Chip Device Controller Shell
Bluetooth adapter set to hciNone

chip-device-ctrl > ble-scan
2021-08-17 09:32:22,968 ChipBLEMgr   INFO     BLE is ready!
2021-08-17 09:32:30,755 ChipBLEMgr   INFO     adding to scan list:
2021-08-17 09:32:30,755 ChipBLEMgr   INFO     
2021-08-17 09:32:30,755 ChipBLEMgr   INFO     Name            = None                                                                            
2021-08-17 09:32:30,755 ChipBLEMgr   INFO     ID              = 282D9DB3-D25D-4E49-884B-5E64A74F337D                                            
2021-08-17 09:32:30,755 ChipBLEMgr   INFO     RSSI            = -33                                                                             
2021-08-17 09:32:30,756 ChipBLEMgr   INFO     Pairing State   = 0
2021-08-17 09:32:30,756 ChipBLEMgr   INFO     Discriminator   = 3840
2021-08-17 09:32:30,756 ChipBLEMgr   INFO     Vendor Id       = 9050
2021-08-17 09:32:30,756 ChipBLEMgr   INFO     Product Id      = 65279
2021-08-17 09:32:30,756 ChipBLEMgr   INFO     ADV data: {
    kCBAdvDataChannel = 37;
    kCBAdvDataIsConnectable = 1;
    kCBAdvDataServiceData =     {
        FFF6 = <00000f5a 23fffe>;
    };
}
2021-08-17 09:32:30,756 ChipBLEMgr   INFO     
2021-08-17 09:32:32,969 ChipBLEMgr   INFO     scanning stopped
chip-device-ctrl > exit
(python_env) bash-3.2$ deactivate
bash-3.2$
```

以上で、Matterコントローラーの準備は完了です。

### Matterデバイスの準備

Matterデバイスとなる、開発ボード「ESP32-DevKitC」に、LED、ボタンを２点ずつ装着します。

#### LED／ボタンの装着

「ESP32-DevKitC」を、適宜ブレッドボードに取り付けた後、ワイヤー類を使用してLED、ボタンと接続します。

下図は装着・配線の例になります。<br>
（ご注意：LED／ボタンの装着・配線は、ボードをPCのUSBポートに接続していない＝電源が投入されていない状態で実施してください）

<img src="assets04/0002.jpg" width="400">

##### 装着部品

|名称|ESP32-DevKitC| |装着部品[注1]|備考|
|:--|:-:|:-:|:-|:--|
|System State LED|IO25  | --> |緑色LED|Anode側に接続[注2]|
|Lock State LED |IO26  | -->  |赤色LED|Anode側に接続[注2]|
|Lock Button |IO34  | <-- |タクトスイッチ（橙）|High-active[注3]|
|Function Button |IO35  | <--  |タクトスイッチ（黒）|High-active[注3]|

[注1] LED・ボタンの色は適宜決めています<br>
[注2] Cathode側には抵抗器を経由し0Vに接続します。抵抗器の定数は、使用LEDの性能によって適宜決めます<br>
[注3] High-active＝スイッチで3.3Vに接続するとOn（3.3Vには抵抗器を介して接続）、未接続でOff

#### 基板の接続

LED／ボタンの装着が終わりましたら、ボードをPCのUSBポートに接続します。<br>
基板上の赤色LEDが点灯／緑色LEDが点滅を開始します。

<img src="assets04/0001.jpg" width="400">

#### ターミナルの準備

開発ボードからのデバッグプリントを表示させるため、ターミナルを起動し、開発ボードと接続しておきます。<br>
以下のコマンドを実行します。

```
screen `ls /dev/tty.usbserial*` 115200
```

以下は実行例になります。<br>
（下図は開発ボード上のリセットボタンを押下した時のデバッグプリントです）

<img src="assets04/0003.jpg" width="400">

以上で、Matterデバイスの準備は完了です。

### Matterハブの準備

別途手順書「[Matterハブ構築手順](../MatterPoCKit/SETUPHUB.md)」の「Matterハブ開始手順」に従い、Matterハブを開始させます。

<img src="assets04/0005.jpg" width="400">

なお、Matterハブは後述「ペアリング／コミッショニング」で使用する、Wi-Fiネットワークのルーターとなります。<br>
コミッショニング処理において、MatterデバイスはWi-Fi経由でMatterハブに接続する必要があるため、Matterハブが準備できていないと、ペアリング／コミッショニングが正常に完了しません。

## 動作確認

ESP32版サンプルアプリの動作確認を実施します。

- ペアリング／コミッショニング実行
- Matterコマンド実行

### ペアリング／コミッショニング実行

#### Matterデバイスのリセット

ESP32-DevKitCのボタン（黒色）を７秒以上長押しすると、途中（約３秒経過後）基板上の全LEDが点滅した後、リセット処理が実行され、Matterデバイスが工場出荷状態に戻ります。<br>
デバッグ出力には、以下のようにプリントされます。

```
I (2189235) lock-app: Factory Reset Triggered. Release button within 3000ms to cancel.
I (2192245) chip[DL]: NVS set: chip-counters/life-count = 17 (0x11)
I (2192245) chip[DL]: Performing factory reset
I (2192255) wifi:state: run -> init (0)
I (2192255) wifi:pm stop, total sleep time: 2055966167 us / 2189037594 us

W (2192255) wifi:<ba-del>idx
I (2192255) wifi:new:<6,0>, old:<6,0>, ap:<255,255>, sta:<6,0>, prof:1
W (2192265) wifi:hmac tx: ifx0 stop, discard
I (2192325) wifi:flush txq
I (2192325) wifi:stop sw txq
I (2192325) wifi:lmac stop hw txq
I (2192375) chip[DL]: System restarting
：
I (1584) lock-app: App Task started

I (1594) chip[DL]: CHIPoBLE advertising started
E (1594) chip[DL]: Long dispatch time: 700 ms
I (1604) chip[DL]: Starting ESP WiFi layer
I (1614) wifi:mode : sta (08:3a:f2:22:b9:3c)
I (1614) wifi:enable tsf
W (1614) wifi:Haven't to connect to a suitable AP now!
I (1614) chip[DL]: Done driving station state, nothing else to do...
W (1624) wifi:Haven't to connect to a suitable AP now!
I (1634) chip[DL]: Done driving station state, nothing else to do...
I (1634) lock-devicecallbacks: Current free heap: 124152

I (1644) chip[DL]: WIFI_EVENT_STA_START
W (1644) wifi:Haven't to connect to a suitable AP now!
I (1654) chip[DL]: Done driving station state, nothing else to do...
：
```

#### Matterコントローラーの起動

Matterコントローラーとなる`Python CHIP Device Controller`を、ターミナルから起動します。<br>
以下のコマンドを実行します。

```
source ${HOME}/GitHub/connectedhomeip/out/python_env/bin/activate
chip-device-ctrl
```

`chip-device-ctrl`が起動すると、コマンドプロンプト`chip-device-ctrl > `が表示されます。

```
bash-3.2$ source ${HOME}/GitHub/connectedhomeip/out/python_env/bin/activate
(python_env) bash-3.2$ chip-device-ctrl
[1629251576258] [2756:28279] CHIP: [IN] local node id is 0x000000000001B669
[1629251576258] [2756:28279] CHIP: [ZCL] Using ZAP configuration...
：
[1629251576262] [2756:28299] CHIP: [DL] Platform main loop started.
Chip Device Controller Shell
Bluetooth adapter set to hciNone

chip-device-ctrl >
```

#### ペアリング／コミッショニング実行

`chip-device-ctrl`のコマンドプロンプトから、以下のコマンドを実行します。

```
ble-scan
connect -ble 3840 20202021 135246
zcl NetworkCommissioning AddWiFiNetwork 135246 0 0 ssid=str:<SSID> credentials=str:<password> breadcrumb=0 timeoutMs=1000 [注1]
zcl NetworkCommissioning EnableNetwork 135246 0 0 networkID=str:<SSID> breadcrumb=0 timeoutMs=1000 [注2]
close-ble
resolve 0 135246
```

以下は実行例になります。

```
chip-device-ctrl > ble-scan
2021-08-17 15:27:19,041 ChipBLEMgr   INFO     BLE is ready!
2021-08-17 15:27:19,066 ChipBLEMgr   INFO     adding to scan list:
2021-08-17 15:27:19,066 ChipBLEMgr   INFO     
2021-08-17 15:27:19,066 ChipBLEMgr   INFO     Name            = None                                                                            
2021-08-17 15:27:19,066 ChipBLEMgr   INFO     ID              = 282D9DB3-D25D-4E49-884B-5E64A74F337D                                            
2021-08-17 15:27:19,066 ChipBLEMgr   INFO     RSSI            = -33                                                                             
2021-08-17 15:27:19,067 ChipBLEMgr   INFO     Pairing State   = 0
2021-08-17 15:27:19,067 ChipBLEMgr   INFO     Discriminator   = 3840
2021-08-17 15:27:19,067 ChipBLEMgr   INFO     Vendor Id       = 9050
2021-08-17 15:27:19,067 ChipBLEMgr   INFO     Product Id      = 65279
2021-08-17 15:27:19,067 ChipBLEMgr   INFO     ADV data: {
    kCBAdvDataChannel = 37;
    kCBAdvDataIsConnectable = 1;
    kCBAdvDataServiceData =     {
        FFF6 = <00000f5a 23fffe>;
    };
}
2021-08-17 15:27:19,067 ChipBLEMgr   INFO     
2021-08-17 15:27:29,042 ChipBLEMgr   INFO     scanning stopped
chip-device-ctrl >
chip-device-ctrl >
chip-device-ctrl > connect -ble 3840 20202021 135246
Device is assigned with nodeid = 135246
[1629181658466] [825:23809] CHIP: [BLE] NewConnection
[1629181658485] [825:24635] CHIP: [BLE] Connecting to device with discriminator: 3840
[1629181658895] [825:24635] CHIP: [BLE] subscribe complete, ep = 0x112c089e8
[1629181658907] [825:24344] CHIP: [BLE] peripheral chose BTP version 3; central expected between 2 and 3
[1629181658907] [825:24344] CHIP: [BLE] using BTP fragment sizes rx 20 / tx 128.
[1629181658907] [825:24344] CHIP: [BLE] local and remote recv window size = 3
[1629181659458] [825:24344] CHIP: [EM] Received message of type 33 and protocolId 0 on exchange 44249
：
[1629181665544] [825:24635] CHIP: [EM] Received message of type 9 and protocolId 5 on exchange 44252
SetCommandIndexStatus commandHandle=1 commandIndex=1
[1629181665544] [825:24635] CHIP: [ZCL] DefaultResponse:
[1629181665544] [825:24635] CHIP: [ZCL]   Transaction: 0x112c0ae00
[1629181665544] [825:24635] CHIP: [ZCL]   status: EMBER_ZCL_STATUS_SUCCESS (0x00)
[1629181665544] [825:24635] CHIP: [CTL] Device confirmed that it has received the operational certificate
[1629181665544] [825:24635] CHIP: [CTL] Operational credentials provisioned on device 0x7fa308008028
Secure Session to Device Established
Device temporary node id (**this does not match spec**): 135246
chip-device-ctrl >
chip-device-ctrl >
chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 135246 0 0 ssid=str:BorderRouter-AP credentials=str:12345678 breadcrumb=0 timeoutMs=1000
[1629181672743] [825:23809] CHIP: [IN] Encrypted message 0x7ffee387efb0 from 0x000000000001B669 to 0x000000000002104E of type 8 and protocolId 5 on exchange 44253.
[1629181672743] [825:23809] CHIP: [IN] Sending msg 0x7ffee387efb0 to 0x000000000002104E at utc time: 3696743 msec
[1629181672743] [825:23809] CHIP: [IN] Sending secure msg on generic transport
[1629181672743] [825:23809] CHIP: [IN] Secure msg send status No Error
[1629181672957] [825:24344] CHIP: [IN] Secure transport received message destined to fabric 0, node 0x000000000001B669. Key ID 1
[1629181672957] [825:24344] CHIP: [EM] Received message of type 9 and protocolId 5 on exchange 44253
SetCommandIndexStatus commandHandle=1 commandIndex=1
[1629181672958] [825:24344] CHIP: [ZCL] DefaultResponse:
[1629181672958] [825:24344] CHIP: [ZCL]   Transaction: 0x112c0ae00
[1629181672958] [825:24344] CHIP: [ZCL]   status: EMBER_ZCL_STATUS_SUCCESS (0x00)
Received command status response:
Container:
    ProtocolId = 5
    ProtocolCode = 0
    EndpointId = 0
    ClusterId = 49
    CommandId = 2
    CommandIndex = 1
chip-device-ctrl >
chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 135246 0 0 networkID=str:BorderRouter-AP breadcrumb=0 timeoutMs=1000
[1629181684612] [825:23809] CHIP: [IN] Encrypted message 0x7ffee387f060 from 0x000000000001B669 to 0x000000000002104E of type 8 and protocolId 5 on exchange 44254.
[1629181684612] [825:23809] CHIP: [IN] Sending msg 0x7ffee387f060 to 0x000000000002104E at utc time: 3708612 msec
[1629181684612] [825:23809] CHIP: [IN] Sending secure msg on generic transport
[1629181684612] [825:23809] CHIP: [IN] Secure msg send status No Error
[1629181685130] [825:24635] CHIP: [IN] Secure transport received message destined to fabric 0, node 0x000000000001B669. Key ID 1
[1629181685130] [825:24635] CHIP: [EM] Received message of type 9 and protocolId 5 on exchange 44254
SetCommandIndexStatus commandHandle=1 commandIndex=1
[1629181685130] [825:24635] CHIP: [ZCL] DefaultResponse:
[1629181685130] [825:24635] CHIP: [ZCL]   Transaction: 0x112c0ae00
[1629181685130] [825:24635] CHIP: [ZCL]   status: EMBER_ZCL_STATUS_SUCCESS (0x00)
Received command status response:
Container:
    ProtocolId = 5
    ProtocolCode = 0
    EndpointId = 0
    ClusterId = 49
    CommandId = 12
    CommandIndex = 1
chip-device-ctrl >
chip-device-ctrl > close-ble
[1629181702476] [825:23809] CHIP: [BLE] Auto-closing end point's BLE connection.
chip-device-ctrl >
chip-device-ctrl > resolve 0 135246
[1629181721557] [825:24344] CHIP: [DIS] Node ID resolved for 0x000000000002104E to [fe80::a3a:f2ff:fe22:b93c]:11097
Node address has been updated
[1629181721557] [825:24344] CHIP: [CTL] OperationalDiscoveryComplete for device ID 135246
[1629181723081] [825:24344] CHIP: [EM] Received message of type 49 and protocolId 0 on exchange 44255
Commissioning complete
Current address: fe80::a3a:f2ff:fe22:b93c:11097
[1629181726025] [825:24344] CHIP: [EM] Received message of type 16 and protocolId 0 on exchange 44255
chip-device-ctrl >
```

[注1] `<SSID>`、`<password>`には、Matterハブ構築時のWi-FiアクセスポイントのSSID、パスワードを指定します（デフォルトは、前述実行例に記載の通りです）。<br>
[注2] 消費電力が大きめのLEDを使用した場合、コマンド`zcl NetworkCommissioning EnableNetwork`実行中に、電圧降下によるリセットが実行されてしまうことがあります。その場合はお手数ですがいったんLEDを外し、再度「Matterデバイスのリセット」から実行願います。

以上で、ペアリング／コミッショニング実行は完了になります。

### Matterコマンド実行

このサンプルアプリでは、数あるMatterコマンドのうち、`OnOff`コマンドが実行できます。

#### Offコマンド実行

Matterコントローラーから「Offコマンド」を実行します。<br>
`chip-device-ctrl`コマンドプロンプトから、コマンド`zcl OnOff Off 135246 1 0`を実行します。

ターミナルからは以下のようにプリントされます。

```
chip-device-ctrl > zcl OnOff Off 135246 1 0
[1629181757804] [825:23809] CHIP: [IN] Encrypted message 0x7fa2fb00ea58 from 0x000000000001B669 to 0x000000000002104E of type 8 and protocolId 5 on exchange 44256.
[1629181757804] [825:23809] CHIP: [IN] Sending msg 0x7fa2fb00ea58 to 0x000000000002104E at utc time: 3781804 msec
[1629181757804] [825:23809] CHIP: [IN] Sending secure msg on generic transport
[1629181757804] [825:23809] CHIP: [IN] Secure msg send status No Error
[1629181757979] [825:25130] CHIP: [IN] Secure transport received message destined to fabric 0, node 0x000000000001B669. Key ID 2
[1629181757979] [825:25130] CHIP: [EM] Received message of type 9 and protocolId 5 on exchange 44256
SetCommandIndexStatus commandHandle=1 commandIndex=1
[1629181757979] [825:25130] CHIP: [ZCL] DefaultResponse:
[1629181757979] [825:25130] CHIP: [ZCL]   Transaction: 0x112c0ae00
[1629181757979] [825:25130] CHIP: [ZCL]   status: EMBER_ZCL_STATUS_SUCCESS (0x00)
[1629181757979] [825:25130] CHIP: [IN] Encrypted message 0x700005795b60 from 0x000000000001B669 to 0x000000000002104E of type 16 and protocolId 0 on exchange 44256.
[1629181757979] [825:25130] CHIP: [IN] Sending msg 0x700005795b60 to 0x000000000002104E at utc time: 3781979 msec
[1629181757979] [825:25130] CHIP: [IN] Sending secure msg on generic transport
Received command status response:
[1629181757979] [825:25130] CHIP: [IN] Secure msg send status No Error
Container:
    ProtocolId = 5
    ProtocolCode = 0
    EndpointId = 1
    ClusterId = 6
    CommandId = 0
    CommandIndex = 1
chip-device-ctrl >
```

Offコマンドが実行され、赤色LEDが消灯します。<br>
（Off状態を示しています）

<img src="assets04/0004.jpg" width="400">

#### Onコマンド実行

Matterコントローラーから「Onコマンド」を実行します。<br>
`chip-device-ctrl`コマンドプロンプトから、コマンド`zcl OnOff On 135246 1 0`を実行します。

ターミナルからは以下のようにプリントされます。

```
chip-device-ctrl > zcl OnOff On 135246 1 0
[1629181772789] [825:23809] CHIP: [IN] Encrypted message 0x7fa2fb00ea58 from 0x000000000001B669 to 0x000000000002104E of type 8 and protocolId 5 on exchange 44257.
[1629181772789] [825:23809] CHIP: [IN] Sending msg 0x7fa2fb00ea58 to 0x000000000002104E at utc time: 3796789 msec
[1629181772789] [825:23809] CHIP: [IN] Sending secure msg on generic transport
[1629181772789] [825:23809] CHIP: [IN] Secure msg send status No Error
[1629181772928] [825:25130] CHIP: [IN] Secure transport received message destined to fabric 0, node 0x000000000001B669. Key ID 2
[1629181772928] [825:25130] CHIP: [EM] Received message of type 9 and protocolId 5 on exchange 44257
SetCommandIndexStatus commandHandle=1 commandIndex=1
[1629181772928] [825:25130] CHIP: [ZCL] DefaultResponse:
[1629181772928] [825:25130] CHIP: [ZCL]   Transaction: 0x112c0ae00
[1629181772928] [825:25130] CHIP: [ZCL]   status: EMBER_ZCL_STATUS_SUCCESS (0x00)
[1629181772929] [825:25130] CHIP: [IN] Encrypted message 0x700005795b60 from 0x000000000001B669 to 0x000000000002104E of type 16 and protocolId 0 on exchange 44257.
[1629181772929] [825:25130] CHIP: [IN] Sending msg 0x700005795b60 to 0x000000000002104E at utc time: 3796929 msec
[1629181772929] [825:25130] CHIP: [IN] Sending secure msg on generic transport
Received command status response:
[1629181772929] [825:25130] CHIP: [IN] Secure msg send status No Error
Container:
    ProtocolId = 5
    ProtocolCode = 0
    EndpointId = 1
    ClusterId = 6
    CommandId = 1
    CommandIndex = 1
chip-device-ctrl >
```

Onコマンドが実行され、赤色LEDが再び点灯します。<br>
（On状態を示しています）

<img src="assets04/0001.jpg" width="400">

以上で、Matterコマンド実行の確認は完了です。

以降も、Matterコントローラー（`chip-device-ctrl`）が起動中の間は、Matterデバイスに対し、Matterコマンドを実行することが可能です。
