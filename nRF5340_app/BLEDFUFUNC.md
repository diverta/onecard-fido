# ファームウェア更新機能についての調査

最新更新日：2021/09/29

## 概要

nRF Connectで提供しているBLE経由のファームウェア更新機能（[BLE DFU機能](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/dfu.html#mcuboot)）について、Nordic社からソースコードが提供されているようです。

BLE経由のファームウェア更新機能を、[管理ツール](../MaintenanceTool/macOSApp)に組み込めるよう、ソースコードの調査・解析を行います。

#### 参照したコード
下記リポジトリーのコードを参照しました。<br>
搭載先はiOS、言語はSwiftのようです。

<b>・[nRF Connect Device Manager](https://github.com/NordicSemiconductor/IOS-nRF-Connect-Device-Manager)</b>

残念ながら、[管理ツール](../MaintenanceTool/macOSApp)と同じプラットフォーム（macOS、Objective-C）で実装された版は、Nordic社から公開されていないようです。<br>
管理ツールに搭載する場合は、Swift --> Objective-Cへの移植作業が必要となります。

## ファームウェア更新機能の実装

下記内容は、ソースコードを追跡し、iOSにおけるファームウェア更新機能の挙動を調査したものです。

#### 画面からの起動

ボタン押下＋モード選択（下記例では`Test and confirm`）により、`dfuManager.start`でファームウェア更新機能が起動されます。

```
//
// IOS-nRF-Connect-Device-Manager/Example/Example/View Controllers/Manager/FirmwareUpgradeViewController.swift
//
class FirmwareUpgradeViewController: UIViewController, McuMgrViewController {

  @IBAction func start(_ sender: UIButton) {
      selectMode(for: imageData!)
  }

  private func selectMode(for imageData: Data) {
      let alertController = UIAlertController(title: "Select mode", message: nil, preferredStyle: .actionSheet)
      alertController.addAction(UIAlertAction(title: "Test and confirm", style: .default) {
          action in
          self.dfuManager!.mode = .testAndConfirm
          self.startFirmwareUpgrade(imageData: imageData)
      })
      :
    }

    private func startFirmwareUpgrade(imageData: Data) {
        do {
            try dfuManager.start(data: imageData)
        } catch {
            print("Error reading hash: \(error)")
            status.textColor = .systemRed
            status.text = "ERROR"
            actionStart.isEnabled = false
        }
    }
```

引数になっている`imageData`は、おそらくファームウェア更新イメージのバイナリーデータ（`NSData`に相当するもの）と思われます。<br>
ファイル選択ダイアログから選択されたデータのバイナリーをそのまま`imageData`放り込んでいるようです。

なお、ファームウェア更新イメージは、[Zephyrアプリケーション](../nRF5340_app)のビルドにより生成された[`app_update.bin`](../nRF5340_app/firmwares/secure_device_app/app_update.bin)を想定しております。<br>
（このイメージには、[BLE DFU機能](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/dfu.html#mcuboot)の動作に必要となるブートローダーが同梱されています）

```
private var imageData: Data?

extension FirmwareUpgradeViewController: UIDocumentMenuDelegate, UIDocumentPickerDelegate {
    :
    func documentPicker(_ controller: UIDocumentPickerViewController, didPickDocumentAt url: URL) {
        if let data = dataFrom(url: url) {
            fileName.text = url.lastPathComponent
            fileSize.text = "\(data.count) bytes"

            do {
                let hash = try McuMgrImage(data: data).hash

                imageData = data
                fileHash.text = hash.hexEncodedString(options: .upperCase)
                status.textColor = .primary
                status.text = "READY"
                actionStart.isEnabled = true
                :
            }
        }
    }

    /// Get the image data from the document URL
    private func dataFrom(url: URL) -> Data? {
        do {
            return try Data(contentsOf: url)
            :
        }
    }
}
```

処理の本体である`dfuManager`は、[`FirmwareUpgradeManager`](https://github.com/NordicSemiconductor/IOS-nRF-Connect-Device-Manager#firmwareupgrademanager)というモジュール（クラス）のようです。

```
private var dfuManager: FirmwareUpgradeManager!
```

`dfuManager.start`が実行される部分は下記になります。

```
//
// IOS-nRF-Connect-Device-Manager/Source/Managers/DFU/FirmwareUpgradeManager.swift
//
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
  :
  public init(transporter: McuMgrTransport, delegate: FirmwareUpgradeDelegate?) {
      self.imageManager = ImageManager(transporter: transporter)
      self.defaultManager = DefaultManager(transporter: transporter)
      self.delegate = delegate
      self.state = .none
  }

  public func start(data: Data) throws {
      :
      imageData = data
      hash = try McuMgrImage(data: imageData).hash
      :
      delegate?.upgradeDidStart(controller: self)
      validate()
      :
  }
```
