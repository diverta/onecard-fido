# OpenThread Border Router導入手順

`Raspberry Pi 3 Type B`を使用し、OpenThread Border Router（以下、本ドキュメントにおいて <b>OTBR</b> と略します。）として構築する手順を記載しています。

## Linuxシステム（ラズベリーパイ）の準備

EthernetとWi-Fiが同時に使用できるPCに、Linuxシステムを構築します。

今回の例では、`Raspberry Pi 3 Model B`（ラズベリーパイ）という基板を使用します。

<img src="assets01/0007.jpg" width=500>

#### システムイメージのコピー

下記サイトから`Raspberry Pi OS with desktop`をダウンロードします。<br>
[https://www.raspberrypi.org/software/operating-systems/](https://www.raspberrypi.org/software/operating-systems/)

<img src="assets01/0010.jpg" width=500>

ダウンロードしたOSのイメージファイルを、SDカードにコピーします。<br>
以下のコマンドを実行します。

```
diskutil list
diskutil unmountDisk /dev/disk2
sudo dd bs=1m if=/Users/makmorit/Downloads/2021-03-04-raspios-buster-armhf.img of=/dev/rdisk2; sync
sudo diskutil eject /dev/rdiskN
```

下記は実行例になります。

```
bash-3.2$ diskutil list
/dev/disk0 (internal, physical):
   #:                       TYPE NAME                    SIZE       IDENTIFIER
   0:      GUID_partition_scheme                        *500.3 GB   disk0
   1:                        EFI EFI                     314.6 MB   disk0s1
   2:                 Apple_APFS Container disk1         500.0 GB   disk0s2

/dev/disk1 (synthesized):
   #:                       TYPE NAME                    SIZE       IDENTIFIER
   0:      APFS Container Scheme -                      +500.0 GB   disk1
                                 Physical Store disk0s2
   1:                APFS Volume Macintosh HD            11.0 GB    disk1s1
   2:                APFS Volume Macintosh HD - Data     161.9 GB   disk1s2
   3:                APFS Volume Preboot                 82.6 MB    disk1s3
   4:                APFS Volume Recovery                525.4 MB   disk1s4
   5:                APFS Volume VM                      2.1 GB     disk1s5

/dev/disk2 (external, physical):
   #:                       TYPE NAME                    SIZE       IDENTIFIER
   0:                            NO NAME                *15.5 GB    disk2

bash-3.2$ diskutil unmountDisk /dev/disk2
Unmount of all volumes on disk2 was successful
bash-3.2$ sudo dd bs=1m if=/Users/makmorit/Downloads/2021-03-04-raspios-buster-armhf.img of=/dev/rdisk2; sync
Password:
3784+0 records in
3784+0 records out
3967811584 bytes transferred in 348.658687 secs (11380217 bytes/sec)
bash-3.2$
bash-3.2$ sudo diskutil eject /dev/rdisk2
Password:
Disk /dev/rdisk2 ejected
bash-3.2$
```

イメージのコピーが完了したら、SDカードをラズベリーパイの基板にセット後、電源を入れてシステムを起動します。<br>
システムが起動したら、以下のコマンドを次々と実行します。

#### システム更新

```
sudo apt-get update
sudo apt-get upgrade
```

下記は実行例になります。

```
pi@raspberrypi:~ $ sudo apt-get update
Get:1 http://archive.raspberrypi.org/debian buster InRelease [32.7 kB]                             
Get:2 http://raspbian.raspberrypi.org/raspbian buster InRelease [15.0 kB]                          
Get:3 http://archive.raspberrypi.org/debian buster/main armhf Packages [376 kB]
Get:4 http://raspbian.raspberrypi.org/raspbian buster/main armhf Packages [13.0 MB]
Fetched 13.4 MB in 25s (547 kB/s)                                                                  
Reading package lists... Done
pi@raspberrypi:~ $ sudo apt-get upgrade
Reading package lists... Done
Building dependency tree       
Reading state information... Done
Calculating upgrade... Done
The following packages will be upgraded:
  agnostics alsa-utils apt apt-utils avahi-daemon base-files bind9-host chromium-browser
  chromium-browser-l10n chromium-codecs-ffmpeg-extra curl groff-base gstreamer1.0-alsa
  gstreamer1.0-libav gstreamer1.0-plugins-bad gstreamer1.0-plugins-base gstreamer1.0-plugins-good
  gstreamer1.0-x iputils-ping libapt-inst2.0 libapt-pkg5.0 libavahi-client3 libavahi-common-data
：
Processing triggers for desktop-file-utils (0.23-4) ...
Processing triggers for mime-support (3.62) ...
Processing triggers for hicolor-icon-theme (0.17-2) ...
Processing triggers for gnome-menus (3.31.4-3) ...
Processing triggers for libc-bin (2.28-10+rpi1) ...
Processing triggers for man-db (2.8.5-2) ...
Processing triggers for dbus (1.12.20-0+deb10u1) ...
Processing triggers for install-info (6.5.0.dfsg.1-4+b1) ...
Processing triggers for initramfs-tools (0.133+deb10u1) ...
Processing triggers for libvlc-bin:armhf (3.0.12-0+deb10u1+rpt2) ...
pi@raspberrypi:~ $
```

#### Dockerのインストール

```
curl -sSL https://get.docker.com | sh
sudo usermod -aG docker $USER
```

Dockerのインストールが完了したら、ラズパイを再起動させます。

```
sudo reboot
```

ラズパイが再起動したら、コマンド`systemctl status docker`により、Dockerがサービスとして立ち上がっていることを確認します。<br>
下記は実行例になります。

```
pi@raspberrypi:~ $ systemctl status docker
● docker.service - Docker Application Container Engine
   Loaded: loaded (/lib/systemd/system/docker.service; enabled; vendor preset: enabled)
   Active: active (running) since Tue 2021-05-25 13:46:31 JST; 4min 17s ago
     Docs: https://docs.docker.com
 Main PID: 1307 (dockerd)
    Tasks: 11
   CGroup: /system.slice/docker.service
           └─1307 /usr/bin/dockerd -H fd:// --containerd=/run/containerd/containerd.sock

 5月 25 13:46:28 raspberrypi dockerd[1307]: time="2021-05-25T13:46:28.346994945+09:00" level=warning
 5月 25 13:46:28 raspberrypi dockerd[1307]: time="2021-05-25T13:46:28.347219847+09:00" level=warning
 5月 25 13:46:28 raspberrypi dockerd[1307]: time="2021-05-25T13:46:28.347278911+09:00" level=warning
 5月 25 13:46:28 raspberrypi dockerd[1307]: time="2021-05-25T13:46:28.348862808+09:00" level=info ms
 5月 25 13:46:29 raspberrypi dockerd[1307]: time="2021-05-25T13:46:29.577920159+09:00" level=info ms
 5月 25 13:46:30 raspberrypi dockerd[1307]: time="2021-05-25T13:46:30.086066919+09:00" level=info ms
 5月 25 13:46:30 raspberrypi dockerd[1307]: time="2021-05-25T13:46:30.657677883+09:00" level=info ms
 5月 25 13:46:30 raspberrypi dockerd[1307]: time="2021-05-25T13:46:30.658105134+09:00" level=info ms
 5月 25 13:46:31 raspberrypi systemd[1]: Started Docker Application Container Engine.
 5月 25 13:46:31 raspberrypi dockerd[1307]: time="2021-05-25T13:46:31.914400663+09:00" level=info ms
pi@raspberrypi:~ $
```

#### gitのインストール

```
sudo apt install git
```

下記は実行例になります。

```
pi@raspberrypi:~ $
pi@raspberrypi:~ $ sudo apt install git
パッケージリストを読み込んでいます... 完了
依存関係ツリーを作成しています                
状態情報を読み取っています... 完了
git はすでに最新バージョン (1:2.20.1-2+deb10u3) です。
アップグレード: 0 個、新規インストール: 0 個、削除: 0 個、保留: 0 個。
pi@raspberrypi:~ $
```

## OTBRの構築

#### ソースコードの取得

GitHubから、OTBRのソースコードを取得します。<br>
下記は実行例になります。

```
pi@raspberrypi:~ $ pwd
/home/pi
pi@raspberrypi:~ $ git clone https://github.com/openthread/ot-br-posix
Cloning into 'ot-br-posix'...
remote: Enumerating objects: 76975, done.
remote: Counting objects: 100% (10289/10289), done.
remote: Compressing objects: 100% (465/465), done.
remote: Total 76975 (delta 9993), reused 9979 (delta 9823), pack-reused 66686
Receiving objects: 100% (76975/76975), 46.31 MiB | 3.29 MiB/s, done.
Resolving deltas: 100% (52163/52163), done.
pi@raspberrypi:~ $
```

#### インストールの準備

依存ライブラリー／パッケージの導入を実行します。<br>
下記は実行例になります。

```
pi@raspberrypi:~ $ cd ot-br-posix
pi@raspberrypi:~/ot-br-posix $
pi@raspberrypi:~/ot-br-posix $ ./script/bootstrap
+++ dirname ./script/bootstrap
++ cd ./script/..
++ [[ ! -n '' ]]
++ grep -s 'BeagleBone Black' /sys/firmware/devicetree/base/model
++ case "${OSTYPE}" in
++ have_or_die lsb_release
++ have lsb_release
++ command -v lsb_release
：
+ main
+ . /dev/null
+ git submodule update --init --recursive --depth 1
Submodule 'third_party/cJSON/repo' (https://github.com/DaveGamble/cJSON.git) registered for path 'third_party/cJSON/repo'
Submodule 'third_party/http-parser/repo' (https://github.com/nodejs/http-parser.git) registered for path 'third_party/http-parser/repo'
Submodule 'third_party/openthread/repo' (https://github.com/openthread/openthread.git) registered for path 'third_party/openthread/repo'
Cloning into '/home/pi/ot-br-posix/third_party/cJSON/repo'...
Cloning into '/home/pi/ot-br-posix/third_party/http-parser/repo'...
Cloning into '/home/pi/ot-br-posix/third_party/openthread/repo'...
：
node-cacache (11.3.2-2) を設定しています ...
node-gyp (3.8.0-6) を設定しています ...
node-libnpx (10.2.0+repack-1) を設定しています ...
npm (5.8.0+ds6-4+deb10u2) を設定しています ...
man-db (2.8.5-2) のトリガを処理しています ...
libc-bin (2.28-10+rpi1) のトリガを処理しています ...
+ . examples/platforms/raspbian/after_bootstrap
++ . /etc/os-release
+++ PRETTY_NAME='Raspbian GNU/Linux 10 (buster)'
+++ NAME='Raspbian GNU/Linux'
+++ VERSION_ID=10
+++ VERSION='10 (buster)'
+++ VERSION_CODENAME=buster
+++ ID=raspbian
+++ ID_LIKE=debian
+++ HOME_URL=http://www.raspbian.org/
+++ SUPPORT_URL=http://www.raspbian.org/RaspbianForums
+++ BUG_REPORT_URL=http://www.raspbian.org/RaspbianBugs
++ [[ 10 -ge 10 ]]
pi@raspberrypi:~/ot-br-posix $
```

#### インストールの実行

OTBRのビルド／インストールを実行します。<br>
下記は実行例になります。

```
pi@raspberrypi:~/ot-br-posix $ INFRA_IF_NAME=eth0 ./script/setup
+++ dirname ./script/setup
++ cd ./script/..
++ [[ ! -n '' ]]
++ grep -s 'BeagleBone Black' /sys/firmware/devicetree/base/model
++ case "${OSTYPE}" in
++ have_or_die lsb_release
++ have lsb_release
：
+ ./script/cmake-build -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/usr -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOTBR_MDNS=mDNSResponder -DOTBR_INFRA_IF_NAME=eth0 -DOTBR_WEB=ON -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_BACKBONE_ROUTER=ON
+++ dirname ./script/cmake-build
++ cd ./script/..
++ [[ ! -n '' ]]
++ grep -s 'BeagleBone Black' /sys/firmware/devicetree/base/model
++ case "${OSTYPE}" in
++ have_or_die lsb_release
++ have lsb_release
++ command -v lsb_release
+++ lsb_release -i
+++ cut -c17-
：
[6/7] Install the project...
-- Install configuration: ""
-- Installing: /usr/sbin/otbr-agent
-- Installing: /usr/sbin/ot-ctl
-- Installing: /etc/dbus-1/system.d/otbr-agent.conf
-- Installing: /lib/systemd/system/otbr-agent.service
-- Installing: /etc/default/otbr-agent
：
+++ WEB_GUI=1
+++ REST_API=1
++ eval echo '${WEB_GUI-}'
+++ echo 1
+ value=1
+ [[ 1 == 1 ]]
+ sudo systemctl is-enabled otbr-web
enabled
+ [[ 0 == \1 ]]
+ . /dev/null
pi@raspberrypi:~/ot-br-posix $
```

#### RCPの設定

別途、nRF52840を使いセットアップしたRCP（Radio Co-Processor）を、ラズパイのOTBRに割り当てます。<br>
今回の例では、nRF52840デバイスとして、以前に製作した「MDBT50Q Dongle」を使用しております。

まずはラズパイのUSBポートに、nRF52840を装着します。

<img src="assets01/0008.jpg" width=500>

ラズパイに装着されたnRF52840のデバイス名を取得します。<br>
以下は、nRF52840が`/dev/ttyACM0`として認識された例です。

```
pi@raspberrypi:~ $ ls -al /dev/tty*0
crw--w---- 1 root tty       4,  0  5月 25 14:57 /dev/tty0
crw--w---- 1 root tty       4, 10  5月 25 14:57 /dev/tty10
crw--w---- 1 root tty       4, 20  5月 25 14:57 /dev/tty20
crw--w---- 1 root tty       4, 30  5月 25 14:57 /dev/tty30
crw--w---- 1 root tty       4, 40  5月 25 14:57 /dev/tty40
crw--w---- 1 root tty       4, 50  5月 25 14:57 /dev/tty50
crw--w---- 1 root tty       4, 60  5月 25 14:57 /dev/tty60
crw-rw---- 1 root dialout 166,  0  5月 25 15:45 /dev/ttyACM0
crw-rw---- 1 root dialout 204, 64  5月 25 14:57 /dev/ttyAMA0
pi@raspberrypi:~ $
```

`/etc/default/otbr-agent`というファイルの内容を確認し、記述されているデバイス名が、上記のデバイス名と整合していることを確認します。

```
pi@raspberrypi:~ $ cat /etc/default/otbr-agent
# Default settings for otbr-agent. This file is sourced by systemd

# Options to pass to otbr-agent
OTBR_AGENT_OPTS="-I wpan0 -B eth0 spinel+hdlc+uart:///dev/ttyACM0 trel://eth0"
pi@raspberrypi:~ $
```

ここまで確認できたら、nRF52840をUSBポートに装着したまま、ラズパイを再起動します。

```
sudo reboot
```

#### 各サービスの稼働確認

再起動が完了したら、RCP（nRF52840）および各サービス（`otbr-agent.service`、`otbr-web.service`）の稼働状況を確認します。<br>
以下は実行例になります。

```
pi@raspberrypi:~ $ sudo systemctl status
● raspberrypi
    State: running
     Jobs: 0 queued
   Failed: 0 units
    Since: Thu 1970-01-01 09:00:03 JST; 51 years 4 months ago
   CGroup: /
           ├─user.slice
           │ └─user-1000.slice
           │   ├─session-3.scope
           │   │ ├─ 695 lightdm --session-child 14 17
           │   │ ├─ 699 /usr/bin/lxsession -s LXDE-pi -e LXDE
           ：
           ├─init.scope
           │ └─1 /sbin/init splash
           └─system.slice
             ├─lightdm.service
             │ ├─531 /usr/sbin/lightdm
             │ └─574 /usr/lib/xorg/Xorg :0 -seat seat0 -auth /var/run/lightdm/root/:0 -nolisten tcp
             ：
             ├─docker.service
             │ └─625 /usr/bin/dockerd -H fd:// --containerd=/run/containerd/containerd.sock
             ├─avahi-daemon.service
             │ ├─398 avahi-daemon: running [raspberrypi.local]
             │ └─448 avahi-daemon: chroot helper
             ├─otbr-web.service
             │ └─421 /usr/sbin/otbr-web
             ├─wpa_supplicant.service
             │ └─447 /sbin/wpa_supplicant -u -s -O /run/wpa_supplicant
             ├─triggerhappy.service
             │ └─451 /usr/sbin/thd --triggers /etc/triggerhappy/triggers.d/ --socket /run/thd.socket
             ├─systemd-logind.service
             │ └─404 /lib/systemd/systemd-logind
             ├─rtkit-daemon.service
             │ └─936 /usr/lib/rtkit/rtkit-daemon
             ├─cups.service
             │ └─417 /usr/sbin/cupsd -l
             ├─polkit.service
             │ └─510 /usr/lib/policykit-1/polkitd --no-debug
             ├─otbr-agent.service
             │ └─420 /usr/sbin/otbr-agent -I wpan0 -B eth0 spinel+hdlc+uart:///dev/ttyACM0 trel://et
             ：
pi@raspberrypi:~ $
```

以下は個別の動作確認になります。<br>
まずはRCPの状況確認を実行します。

```
sudo ot-ctl state
```

以下は実行例です。<br>
`disable`の文言は、RCPおよび`otbr-agent.service`は正常に稼働しているが、Threadネットワークには参加されていない状態であることを示します。

```
pi@raspberrypi:~ $ sudo ot-ctl state
disabled
Done
pi@raspberrypi:~ $
```

次に、Webブラウザーを起動し、`http://localhost/`を実行します。<br>
下図のような管理画面が表示されれば、`otbr-web.service`が正常に稼働していることを示します。

<img src="assets01/0009.jpg" width=500>
