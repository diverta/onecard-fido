# [WIP] Chrome(macOS版)でのBLE U2F対応調査

## 目的

macOS版Chromeブラウザーで、BLE U2F対応が可能かどうか調査します。

具体的にはmacOSでサポートされているChromeの「Web Bluetooth API」を使用し、U2F BLEエクステンションを作成します。

このU2F BLEエクステンションをChromeにインストールし、macOS版Chrome<--->nRF52(One Card)間の疎通確認を行います。

## 進め方

すでに動作確認が取れている、Chrome OS版 BLE U2Fエクステンション（β）をベースに作成します。

* Chrome OS版 BLE U2Fエクステンションはこちら：
https://github.com/diverta/onecard-fido/tree/master/Research/u2f-ble-helper

「chrome.bluetoothLowEnergy」というAPIを使用している部分を、Web Bluetooth API「navigator.bluetooth.requestDevice」で作り変えます。

下記のプロジェクトを参考にします。

* Heart Rate Sensor Demo
https://github.com/WebBluetoothCG/demos/tree/gh-pages/heart-rate-sensor

* Web Bluetooth Samples (Chrome) https://github.com/googlechrome/samples/tree/gh-pages/web-bluetooth
