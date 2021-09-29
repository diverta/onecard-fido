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

#### FirmwareUpgradeManagerクラスの初期化

ファームウェア更新機能を実装するクラスである[`FirmwareUpgradeManager`](https://github.com/NordicSemiconductor/IOS-nRF-Connect-Device-Manager#firmwareupgrademanager)は、処理の冒頭で初期化が必要です。<br>
また、初期化を実行するためには、別途、`McuMgrTransport`というトランスポート実装クラスが必要となります。

今回事例で使用する`McuMgrBleTransport`は、`McuMgrTransport`のBLE実装クラスです。<br>
冒頭で、nRF5340アプリケーションに同梱されているBLE SMPサービス／キャラクタリスティックUUIDが定義されています。

```
//
// IOS-nRF-Connect-Device-Manager/Source/Bluetooth/McuMgrBleTransport.swift
//
public class McuMgrBleTransport: NSObject {

    public static let SMP_SERVICE = CBUUID(string: "8D53DC1D-1DB7-4CD3-868B-8A527460AA84")
    public static let SMP_CHARACTERISTIC = CBUUID(string: "DA2E7828-FBCE-4E01-AE9E-261174997C48")
    :
    public convenience init(_ target: CBPeripheral) {
        self.init(target.identifier)
    }
    :

extension McuMgrBleTransport: McuMgrTransport {
    :
    public func send<T: McuMgrResponse>(data: Data, callback: @escaping McuMgrCallback<T>) {
        :
    }

    public func connect(_ callback: @escaping ConnectionCallback) {
        :
    }

    public func close() {
        :
    }
    :
```


`FirmwareUpgradeManager`を、BLEトランスポート実装クラス`McuMgrBleTransport`を使い初期化します。<br>
（`McuMgrTransport`は、`McuMgrBleTransport`の抽象クラスになります）

```
class FirmwareUpgradeViewController: UIViewController, McuMgrViewController {
    :
    private var dfuManager: FirmwareUpgradeManager!
    var transporter: McuMgrTransport! {
        didSet {
            dfuManager = FirmwareUpgradeManager(transporter: transporter, delegate: self)
            :
        }
    }
```

`FirmwareUpgradeManager`の`transporter`に、`McuMgrBleTransport`が設定されるのは、画面が表示された時点のようです。<br>
下記の`ImageController`は、`FirmwareUpgradeManager`（`McuMgrViewController`の継承クラス）の親コンポーネントになります。

```
//
// IOS-nRF-Connect-Device-Manager/Example/Example/View Controllers/Manager/ImageController.swift
//
class ImageController: UITableViewController {
    :
    override func viewDidAppear(_ animated: Bool) {
        showModeSwitch()

        // Set the connection status label as transport delegate.
        let baseController = parent as! BaseViewController
        let bleTransporter = baseController.transporter as? McuMgrBleTransport
        bleTransporter?.delegate = connectionStatus
    }
    :
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        let baseController = parent as! BaseViewController
        let transporter = baseController.transporter!

        var destination = segue.destination as? McuMgrViewController
        destination?.transporter = transporter
        :
```

`baseController.transporter`の正体ですが、事前に他の画面上でスキャンしておいたBLEペリフェラルデバイスの情報（`DiscoveredPeripheral`）を保持したものと思われます。<br>
（`BaseViewController`は、`ImageController`のさらに親コンポーネントになります）

なお、`peripheral.basePeripheral`とあるのは、BLEペリフェラルクラス`CBPeripheral`の参照です。[注1]

```
//
// IOS-nRF-Connect-Device-Manager/Example/Example/View Controllers/Manager/BaseViewController.swift
//
class BaseViewController: UITabBarController {

    var transporter: McuMgrTransport!
    var peripheral: DiscoveredPeripheral! {
        didSet {
            let bleTransporter = McuMgrBleTransport(peripheral.basePeripheral)
            bleTransporter.logDelegate = UIApplication.shared.delegate as? McuMgrLogDelegate
            transporter = bleTransporter
        }
    }
    :
```

[注1] 管理ツールでは、スキャンしたBLEペリフェラルデバイスに接続された後、コールバック関数`connectPeripheral`の引数として渡ってくる参照（`(CBPeripheral *)peripheral`）になります。

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

処理の本体である`dfuManager`は、前述の`FirmwareUpgradeManager`クラスです。<br>
`dfuManager.start`の実装は下記になります。

```
//
// IOS-nRF-Connect-Device-Manager/Source/Managers/DFU/FirmwareUpgradeManager.swift
//
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {

  private var dfuManager: FirmwareUpgradeManager!

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
