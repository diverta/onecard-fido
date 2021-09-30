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

ファームウェア更新機能を実装するクラスである[`FirmwareUpgradeManager`](https://github.com/NordicSemiconductor/IOS-nRF-Connect-Device-Manager#firmwareupgrademanager)は、処理の冒頭で初期化が必要です。

初期化時は、BLEトランスポート実装クラス`McuMgrBleTransport`を引数に指定します。<br>
（`McuMgrTransport`は、`McuMgrBleTransport`の抽象クラスになります[注1]）

```
//
// IOS-nRF-Connect-Device-Manager/Example/Example/View Controllers/Manager/FirmwareUpgradeViewController.swift
//
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

## BLEトランスポートの実装

下記内容は、前述`FirmwareUpgradeManager`の動作に必要な、BLEトランスポート`McuMgrBleTransport`の実装について調査したものです。

#### McuMgrTransport

`McuMgrTransport`は、`McuMgrBleTransport`の基底クラスです。<br>
以下のようなメソッドをインターフェースしますが、`McuMgrBleTransport`は、このクラスのBLE実装となっています。

```
//
// IOS-nRF-Connect-Device-Manager/Source/McuMgrTransport.swift
//
public protocol McuMgrTransport: AnyObject {
    func getScheme() -> McuMgrScheme
    func send<T: McuMgrResponse>(data: Data, callback: @escaping McuMgrCallback<T>)
    func connect(_ callback: @escaping ConnectionCallback)
    func close()
    func addObserver(_ observer: ConnectionObserver);
    func removeObserver(_ observer: ConnectionObserver);
}
```

本ドキュメントでは、通常使用しないと思われる`getScheme`、`connect`、`addObserver`、`removeObserver`については調査を省略し、`send`、`close`についての調査内容を掲載します。

#### init

`McuMgrBleTransport`を初期化する関数です。

引数は`CBPeripheral`の参照になります。[注1]<br>
すなわち、ターゲットとなるBLEペリフェラルデバイスに接続でき、かつペリフェラル上で稼働するBLE SMPサービスがディスカバーされている事が前提となります。

```
//
// IOS-nRF-Connect-Device-Manager/Source/Bluetooth/McuMgrBleTransport.swift
//
public class McuMgrBleTransport: NSObject {
    :
    public convenience init(_ target: CBPeripheral) {
        self.init(target.identifier)
    }
    :
```

[注1] このswiftコード内では引数（`CBPeripheral`の参照）は直接使用せず、代わりに内部生成した`CBCentralManager`を使い、UUID（`CBPeripheral.identifier`）が一致する`CBPeripheral`の参照を間接取得しているようです。

#### send

`McuMgrBleTransport`からデータをBLE送信する関数です。

下記は呼び出し例です。<br>
送信データ／イメージ番号[注2]／オフセット、および送信成功／失敗時に呼び出されるコールバック関数が、引数となります。

```
public class ImageManager: McuManager {
    :
    public func upload(data: Data, image: Int, offset: UInt, callback: @escaping McuMgrCallback<McuMgrUploadResponse>) {
        :
        // Build request and send.
        send(op: .write, commandId: ID_UPLOAD, payload: payload, callback: callback)
    }
    :
    private lazy var uploadCallback: McuMgrCallback<McuMgrUploadResponse> = {
        :
    }
}
```

実装は下記の通りです。<br>
送信失敗時はリトライが行われます。[注3]

```
extension McuMgrBleTransport: McuMgrTransport {

    public func send<T: McuMgrResponse>(data: Data, callback: @escaping McuMgrCallback<T>) {
        dispatchQueue.async {
            // Max concurrent opertaion count is set to 1, so operations are
            // executed one after another. A new one will be started when the
            // queue is empty, or the when the last operation finishes.
            self.operationQueue.addOperation {
                for _ in 0..<McuMgrBleTransport.MAX_RETRIES {
                    let retry = self._send(data: data, callback: callback)
                    if !retry {
                        break
                    }
                }
            }
        }
    }
```

サブルーチン`_send`の実装は下記の通りです。

具体的には、送信前のチェック／セットアップを実行[注4]した後、`CBPeripheral.writeValue`を呼び出し、BLEペリフェラルデバイスにデータを転送します。<br>
送信バイト数の上限（MTU）は、別途`CBPeripheral.maximumWriteValueLength`で設定します。

```
extension McuMgrBleTransport: McuMgrTransport {
    private func _send<T: McuMgrResponse>(data: Data, callback: @escaping McuMgrCallback<T>) -> Bool {
        :
        let targetPeripheral: CBPeripheral

        if let existing = peripheral, centralManager.state == .poweredOn {
            targetPeripheral = existing
        } else {
            :
        }
        :
        // Make sure the SMP characteristic is not nil.
        guard let smpCharacteristic = smpCharacteristic else {
            :
            return false
        }

        // Close the lock.
        lock.close(key: Keys.awaitingResponse)

        // Check that data length does not exceed the mtu.
        let mtu = targetPeripheral.maximumWriteValueLength(for: .withoutResponse)
        if data.count > mtu {
            :
            return false
        }

        // Write the value to the characteristic.
        log(msg: "-> \(data.hexEncodedString(options: .prepend0x))", atLevel: .debug)
        targetPeripheral.writeValue(data, for: smpCharacteristic, type: .withoutResponse)

        // Wait for the response.
        let result = lock.block(timeout: DispatchTime.now() + .seconds(McuMgrBleTransport.TRANSACTION_TIMEOUT))
        :
```

転送処理が成功または失敗した場合は、所定のコールバックが実行されます。

```
extension McuMgrBleTransport: McuMgrTransport {
    private func _send<T: McuMgrResponse>(data: Data, callback: @escaping McuMgrCallback<T>) -> Bool {
        :
        switch result {
        case .timeout:
            log(msg: "Request timed out", atLevel: .error)
            fail(error: McuMgrTransportError.sendTimeout, callback: callback)
        case .error(let error):
            log(msg: "Request failed: \(error)", atLevel: .error)
            fail(error: error, callback: callback)
        case .success:
            do {
                // Build the McuMgrResponse.
                log(msg: "<- \(responseData?.hexEncodedString(options: .prepend0x) ?? "0 bytes")",
                    atLevel: .debug)
                let response: T = try McuMgrResponse.buildResponse(scheme: getScheme(),
                                                                   data: responseData)
                success(response: response, callback: callback)
            } catch {
                fail(error: error, callback: callback)
            }
        }
        return false
    }

    private func success<T: McuMgrResponse>(response: T, callback: @escaping McuMgrCallback<T>) {
        :
        DispatchQueue.main.async {
            callback(response, nil)
        }
    }

    private func fail<T: McuMgrResponse>(error: Error, callback: @escaping McuMgrCallback<T>) {
        :
        DispatchQueue.main.async {
            callback(nil, error)
        }
    }
```

[注2] ファームウェア更新イメージファイルに更新イメージが複数同梱されている場合、特定のイメージ番号を指定するユースケースを想定しているようです。本プロジェクトでは、１点の更新イメージしか搭載しないため、０固定となります。<br>
[注3] 送信リトライ回数（`McuMgrBleTransport.MAX_RETRIES`）は３回となっています。<br>
[注4] 接続されていない場合は接続まで待ち、BLE SMPサービスがディスカバーされていない場合はディスカバーを実行します。
