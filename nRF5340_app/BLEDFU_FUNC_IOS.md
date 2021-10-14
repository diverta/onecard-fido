# ファームウェア更新機能についての調査

最終更新日：2021/10/01

## 概要

nRF Connectで提供しているBLE経由のファームウェア更新機能（[BLE DFU機能](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/dfu.html#mcuboot)）について、Nordic社からソースコードが提供されているようです。

BLE経由のファームウェア更新機能を、[管理ツール](../MaintenanceTool/macOSApp)に組み込めるよう、ソースコードの調査・解析を行います。

#### 参照したコード
下記リポジトリーのコードを参照しました。<br>
搭載先はiOS、言語はSwiftのようです。

<b>・[nRF Connect Device Manager](https://github.com/NordicSemiconductor/IOS-nRF-Connect-Device-Manager)</b>

残念ながら、[管理ツール](../MaintenanceTool/macOSApp)と同じプラットフォーム（macOS、Objective-C）で実装された版は、Nordic社から公開されていないようです。<br>
管理ツールに搭載する場合は、Swift --> Objective-Cへの移植作業が必要となります。

#### コードの参照方法

リポジトリー[`nRF Connect Device Manager`](https://github.com/NordicSemiconductor/IOS-nRF-Connect-Device-Manager)をPCにチェックアウトした後、CocoaPodを使用し、依存ライブラリーをダウンロードします。<br>
ダウンロードに必要なPodfileは、リポジトリーの`Example`配下に`Podfile`という名称で配置されています。

以下はCocoaPodsの実行例になります。<br>
（CocoaPodsの導入コマンドを含みます）

```
bash-3.2$ sudo gem install cocoapods
Password:
Fetching i18n-1.8.10.gem
:
Installing ri documentation for cocoapods-1.11.2
Done installing documentation for concurrent-ruby, i18n, tzinfo, zeitwerk, activesupport, nap, fuzzy_match, httpclient, algoliasearch, ffi, ethon, typhoeus, netrc, public_suffix, addressable, cocoapods-core, claide, cocoapods-deintegrate, cocoapods-downloader, cocoapods-plugins, cocoapods-search, cocoapods-trunk, cocoapods-try, molinillo, atomos, colored2, nanaimo, rexml, xcodeproj, escape, fourflusher, gh_inspector, ruby-macho, cocoapods after 20 seconds
34 gems installed
bash-3.2$
bash-3.2$ pod setup
Setup completed
bash-3.2$
bash-3.2$ cd ${HOME}/GitHub/IOS-nRF-Connect-Device-Manager/Example
bash-3.2$ pod install
Analyzing dependencies
Adding spec repo `trunk` with CDN `https://cdn.cocoapods.org/`
Downloading dependencies
Installing McuManager (0.12.0)
Installing SwiftCBOR (0.4.3)
Generating Pods project
Integrating client project
Pod installation complete! There is 1 dependency from the Podfile and 2 total pods installed.
bash-3.2$
```

これで、Xcodeからサンプルコードを参照／ビルドする事ができるようになります。

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

ボタン押下＋モード選択（下記例では`Test and confirm`）により、`dfuManager.start()`でファームウェア更新機能が起動されます。

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
`dfuManager.start()`の実装は下記になります。

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

#### ファームウェア更新イメージ転送前の処理

`FirmwareUpgradeManager.start()`では、以下の処理が行われます。

- ファームウェア更新イメージのハッシュを計算
- ファームウェア更新イメージのチェック
- ファームウェア更新イメージの転送を開始

まず最初に、ファームウェア更新イメージのハッシュを計算します。

```
    public func start(data: Data) throws {
        :
        imageData = data
        hash = try McuMgrImage(data: imageData).hash
        :
```

次に、ファームウェア更新イメージについてのチェックを行うため、BLEペリフェラルデバイスにステータスを照会（`validate()`）します。

```
    public func start(data: Data) throws {
        :
        validate()
```

`validate()`では、BLEペリフェラルデバイスにリクエスト`ID_STATE`を送信します。

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    :
    private func validate() {
        setState(.validate)
        if !paused {
            imageManager.list(callback: validateCallback)
        }
    }
    :

public class ImageManager: McuManager {
    :
    public func list(callback: @escaping McuMgrCallback<McuMgrImageStateResponse>) {
        send(op: .read, commandId: ID_STATE, payload: nil, callback: callback)
    }
    :
```

デバイスから受領するレスポンスは、以下のようなイメージになっています。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0","hash":"QW8thibgCorU5iJir/u8gjaUju28TulJwyBdhSlq7pg=",
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0","hash":"QW8thibgCorU5iJir/u8gjaUju28TulJwyBdhSlq7pg=",
            "bootable":true,"pending":false,"confirmed":false,"active":false,"permanent":false
        }
    ],
    "splitStatus":0
}
```

レスポンス`McuMgrImageStateResponse`を参照し、ファームウェア更新イメージについて、諸々チェックを行います。

- 更新ファームウェアが、現在デバイスで稼働中のファームウェアと同一か？
- デバイスのスロット#1に格納されたイメージが`confirmed`か？
- デバイスのスロット#1に格納されたイメージが`pending`か？
- 更新ファームウェアが、すでにデバイスに転送済みか？

上記４点のチェックに該当しない場合は、ファームウェア更新イメージの転送が開始されます。<br>
（`upload()`が実行されます）

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    private lazy var validateCallback: McuMgrCallback<McuMgrImageStateResponse> =
    { [weak self] (response: McuMgrImageStateResponse?, error: Error?) in
        :
        // Check if the new firmware is different then the active one.
        if Data(images[0].hash) == self.hash {
            :
            return
        }

        // If the image in slot 1 is confirmed, we won't be able to erase or
        // test the slot. Therefore, we confirm the image in slot 0 to allow us
        // to modify the image in slot 1.
        if images.count > 1 && images[1].confirmed {
            self.validationConfirm(hash: images[0].hash)
            return
        }

        // If the image in slot 1 is pending, we won't be able to
        // erase or test the slot. Therefore, We must reset the device and
        // revalidate the new image state.
        if images.count > 1 && images[1].pending {
            self.defaultManager.transporter.addObserver(self)
            self.defaultManager.reset(callback: self.resetCallback)
            return
        }

        // Check if the firmware has already been uploaded.
        if images.count > 1 && Data(images[1].hash) == self.hash {
            // Firmware is identical to the one in slot 1. No need to send
            // anything.
            :
        }

        // Validation successful, begin with image upload.
        self.upload()
    }
```

#### ファームウェア更新イメージの転送

`FirmwareUpgradeManager.upload()`では、ファームウェア更新イメージの転送が行われます。

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    private func upload() {
        setState(.upload)
        if !paused {
            _ = imageManager.upload(data: imageData, delegate: self)
        }
    }
```

実処理は、`ImageManager.upload()`で行われます。

```
public class ImageManager: McuManager {
    public func upload(data: Data, image: Int = 0, delegate: ImageUploadDelegate?) -> Bool {
        :
        // Set image data.
        imageData = data

        // Set the slot we're uploading the image to.
        imageNumber = image
        :
        log(msg: "Uploading image (\(data.count) bytes)...", atLevel: .application)
        upload(data: imageData!, image: imageNumber, offset: 0, callback: uploadCallback)
        return true
    }
```

転送処理では、後述の`McuMgrBleTransport.send()`１回あたり送信バイト数上限（MTU）が存在するため、数回に分割送信されます。<br>
下記の同名のサブルーチン`upload()`が、分割送信のたびに呼び出されます。

```
    public func upload(data: Data, image: Int, offset: UInt, callback: @escaping McuMgrCallback<McuMgrUploadResponse>) {
        // Calculate the number of remaining bytes.
        let remainingBytes: UInt = UInt(data.count) - offset
        :
        // Get the length of image data to send.
        let maxDataLength: UInt = UInt(mtu) - UInt(packetOverhead)
        let dataLength: UInt = min(maxDataLength, remainingBytes)

        // Build the request payload.
        var payload: [String:CBOR] = ["data": CBOR.byteString([UInt8](data[offset..<(offset+dataLength)])),
                                      "off": CBOR.unsignedInt(UInt64(offset))]
        :
        // Build request and send.
        send(op: .write, commandId: ID_UPLOAD, payload: payload, callback: callback)
    }
```

`send`による転送が完了すると、`uploadCallback()`がコールバックされ、次の分割送信が実行されます。

```
    private lazy var uploadCallback: McuMgrCallback<McuMgrUploadResponse> = {
        [weak self] (response: McuMgrUploadResponse?, error: Error?) in
        :
        // Get the offset from the response.
        if let offset = response.off {
            // Set the image upload offset.
            self.offset = offset
            :
            // Check if the upload has completed.
            if offset == imageData.count {
                self.log(msg: "Upload finished", atLevel: .application)
                self.resetUploadVariables()
                self.uploadDelegate?.uploadDidFinish()
                :
                return
            }

            // Send the next packet of data.
            self.sendNext(from: UInt(offset))
            :
    }

    private func sendNext(from offset: UInt) {
        :
        upload(data: imageData!, image: imageNumber, offset: offset, callback: uploadCallback)
    }
```

全ての分割送信が完了した場合は、`self.uploadDelegate?.uploadDidFinish()`が呼び出されます。<br>
（実体は`FirmwareUpgradeManager.uploadDidFinish()`になります）

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    public func uploadDidFinish() {
        // On a successful upload move to the next state.
        switch mode {
        case .confirmOnly:
            confirm()
        case .testOnly, .testAndConfirm:
            test()
        }
    }
```

#### ファームウェア更新イメージ転送後の処理

前述の通り、転送後の処理として、`FirmwareUpgradeManager.test()`が実行されます。

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    private func test() {
        setState(.test)
        if !paused {
            imageManager.test(hash: [UInt8](hash), callback: testCallback)
        }
    }
```

呼び出し先の`ImageManager.test()`では、`ID_STATE`コマンドが送信されます。

```
public class ImageManager: McuManager {
    public func test(hash: [UInt8], callback: @escaping McuMgrCallback<McuMgrImageStateResponse>) {
        let payload: [String:CBOR] = ["hash": CBOR.byteString(hash),
                                      "confirm": CBOR.boolean(false)]
        send(op: .write, commandId: ID_STATE, payload: payload, callback: callback)
    }
```

送信リクエストおよびレスポンスは以下のイメージになります。

```
//
// request payload
//   転送したファームウェア更新イメージファイルのハッシュが送信されます。
//
{
    "confirm":false,"hash":"n5/abwxPAbcaIAsYgsRMJSJsec1HIafpIq0j4N8Goso="
}

//
// response payload
//   requestに対する応答として、スロットの状況が戻ります。
//
{
    "images":[
        {
            "slot":0,"version":"0.0.0","hash":"QW8thibgCorU5iJir/u8gjaUju28TulJwyBdhSlq7pg=",
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0","hash":"n5/abwxPAbcaIAsYgsRMJSJsec1HIafpIq0j4N8Goso=",
            "bootable":true,"pending":true,"confirmed":false,"active":false,"permanent":false
        }
    ],
    "splitStatus":0
}
```

#### デバイスのリセット

続いて、`FirmwareUpgradeManager.testCallback()`がコールバックされます。

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    private lazy var testCallback: McuMgrCallback<McuMgrImageStateResponse> =
    { [weak self] (response: McuMgrImageStateResponse?, error: Error?) in
        :
        // Check that the image in slot 1 is pending (i.e. test succeeded).
        if !images[1].pending {
            self.fail(error: FirmwareUpgradeError.unknown("Tested image is not in a pending state."))
            return
        }
        // Test image succeeded. Begin device reset.
        self.reset()
    }
```

呼び出し先の`FirmwareUpgradeManager.reset()`では、デバイスのリセット要求が行われます。<br>
同時に、`addObserver()`で、リセット完了の監視を開始します。

```
    private func reset() {
        setState(.reset)
        if !paused {
            defaultManager.transporter.addObserver(self)
            defaultManager.reset(callback: resetCallback)
        }
    }
```

呼び出し先の`DefaultManager`では、コマンド`ID_RESET`の送信が行われます。<br>
リクエスト、レスポンスともにブランクとなっております。

```
public class DefaultManager: McuManager {
    public func reset(callback: @escaping McuMgrCallback<McuMgrResponse>) {
        send(op: .write, commandId: ID_RESET, payload: nil, callback: callback)
    }
```

リセット要求時は、まず`resetCallback()`が呼び出されます。

```
    private lazy var resetCallback: McuMgrCallback<McuMgrResponse> =
    { [weak self] (response: McuMgrResponse?, error: Error?) in
        :
        // Check for McuMgrReturnCode error.
        if !response.isSuccess() {
            self.fail(error: FirmwareUpgradeError.mcuMgrReturnCodeError(response.returnCode))
            return
        }
        self.resetResponseTime = Date()
        self.log(msg: "Reset request sent. Waiting for reset...", atLevel: .application)
    }
```

デバイスのリセットには10秒程度かかりますので、その間は待ちとなります。<br>
当然のことながら、BLE接続は切れてしまいます。

リセット後のBLE切断を検知すると、`McuMgrBleTransport.notifyStateChanged()`を経由し、`ConnectionObserver.transport()`が呼び出されます。<br>
（後述、`ConnectionObserver`に関する記述をご参照）

```
public class McuMgrBleTransport: NSObject {

    private func notifyStateChanged(_ state: McuMgrTransportState) {
        // The list of observers may be modified by each observer.
        // Better iterate a copy of it.
        let array = [ConnectionObserver](observers)
        for observer in array {
            observer.transport(self, didChangeStateTo: state)
        }
    }
```

`ConnectionObserver.transport()`の実体は、下記`FirmwareUpgradeManager.transport()`となっております。<br>
ここで、再接続が10秒後に実行されます。<br>
（`dfuManager.estimatedSwapTime = 10.0`）

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    public func transport(_ transport: McuMgrTransport, didChangeStateTo state: McuMgrTransportState) {
        transport.removeObserver(self)
        // Disregard connected state.
        guard state == .disconnected else {
            return
        }
        self.log(msg: "Device has disconnected (reset). Reconnecting...", atLevel: .info)
        :
        let remainingTime = estimatedSwapTime - timeSinceReset
        if remainingTime > 0 {
            DispatchQueue.main.asyncAfter(deadline: .now() + remainingTime) { [weak self] in
                self?.reconnect()
            }
        } else {
            reconnect()
        }
    }

    private func reconnect() {
        imageManager.transporter.connect { [weak self] result in
            guard let self = self else {
                return
            }
            switch result {
            case .connected:
                self.log(msg: "Reconnect successful.", atLevel: .info)
            :
            }

            // Continue the upgrade after reconnect.
            switch self.state {
            :
            case .reset:
                switch self.mode {
                :
                default:
                    self.log(msg: "Upgrade complete", atLevel: .application)
                    self.success()
                }
            :
            }
        }
    }
```

最終的には`delegate?.upgradeDidComplete()`（実体は`FirmwareUpgradeViewController.upgradeDidComplete()`）が呼び出され、画面に制御が戻ります。

```
    private func success() {
        :
        delegate?.upgradeDidComplete()
        :
    }
```

以上で、ファームウェア更新イメージ転送処理が完了します。

#### ファームウェア更新イメージの反映

転送されたファームウェア更新イメージを、nRF5340に反映します。<br>
再度、先述の`FirmwareUpgradeManager.start()`から処理を実行します。

```
//
// IOS-nRF-Connect-Device-Manager/Source/Managers/DFU/FirmwareUpgradeManager.swift
//
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    public func start(data: Data) throws {
        :
        imageData = data
        hash = try McuMgrImage(data: imageData).hash
        :
        validate()
        :
    }

    private func validate() {
        setState(.validate)
        if !paused {
            imageManager.list(callback: validateCallback)
        }
    }
```

転送処理時と同様、下記のようなレスポンスが戻ります。

```
{
    "images":[
        {
            "slot":0,"version":"0.0.0","hash":"n5/abwxPAbcaIAsYgsRMJSJsec1HIafpIq0j4N8Goso=",
            "bootable":true,"pending":false,"confirmed":false,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0","hash":"QW8thibgCorU5iJir/u8gjaUju28TulJwyBdhSlq7pg=",
            "bootable":true,"pending":false,"confirmed":true,"active":false,"permanent":false
        }
    ],
    "splitStatus":0
}
```

すでにファームウェア更新イメージが転送済みですので、下記コード<br>
`Data(images[0].hash) == self.hash`かつ`images[0].confirmed == false`<br>
のケースに該当します。

ですので、`self.confirm()`が実行されます。

```
    private lazy var validateCallback: McuMgrCallback<McuMgrImageStateResponse> =
    { [weak self] (response: McuMgrImageStateResponse?, error: Error?) in
        :
        // Check if the new firmware is different then the active one.
        if Data(images[0].hash) == self.hash {
            if images[0].confirmed {
                :
            } else {
                // The new firmware is in test mode.
                switch self.mode {
                case .confirmOnly, .testAndConfirm:
                    self.confirm()
                    :
                }
            }
            return
        }
    }

    private func confirm() {
        setState(.confirm)
        if !paused {
            imageManager.confirm(hash: [UInt8](hash), callback: confirmCallback)
        }
    }
```

`confirm`では、`ID_STATE`コマンドが送信されます。

```
    public func confirm(hash: [UInt8]? = nil, callback: @escaping McuMgrCallback<McuMgrImageStateResponse>) {
        var payload: [String:CBOR] = ["confirm": CBOR.boolean(true)]
        if let hash = hash {
            payload.updateValue(CBOR.byteString(hash), forKey: "hash")
        }
        send(op: .write, commandId: ID_STATE, payload: payload, callback: callback)
    }
```

送信リクエストおよびレスポンスは以下のイメージになります。

```
//
// request payload
//   転送したファームウェア更新イメージファイルのハッシュが送信されます。
//
{
    "confirm":true,"hash":"n5/abwxPAbcaIAsYgsRMJSJsec1HIafpIq0j4N8Goso="
}

//
// response payload
//   requestに対する応答として、スロットの状況が戻ります。
//
{
    "images":[
        {
            "slot":0,"version":"0.0.0","hash":"n5/abwxPAbcaIAsYgsRMJSJsec1HIafpIq0j4N8Goso=",
            "bootable":true,"pending":false,"confirmed":true,"active":true,"permanent":false
        },{
            "slot":1,"version":"0.0.0","hash":"QW8thibgCorU5iJir/u8gjaUju28TulJwyBdhSlq7pg=",
            "bootable":true,"pending":false,"confirmed":false,"active":false,"permanent":false
        }
    ],
    "splitStatus":0
}
```

最後に、`confirmCallback()`がコールバックされます。<br>
デバイスの再リセットが不要であることに注意します。

ここでも`FirmwareUpgradeManager.success()`が呼び出され、最終的に制御が画面に戻ります。

```
    private lazy var confirmCallback: McuMgrCallback<McuMgrImageStateResponse> =
    { [weak self] (response: McuMgrImageStateResponse?, error: Error?) in
        :
        switch self.mode {
        :
        case .testAndConfirm:
            // Check that the upgrade image has successfully booted.
            if Data(images[0].hash) != self.hash {
                self.fail(error: FirmwareUpgradeError.unknown("Device failed to boot into new image."))
                return
            }
            // Check that the new image is in confirmed state.
            if !images[0].confirmed {
                self.fail(error: FirmwareUpgradeError.unknown("Image is not in a confirmed state."))
                return
            }
            // Confirm successful.
            self.log(msg: "Upgrade complete", atLevel: .application)
            self.success()
        :
        }
    }
}
```

以上で、ファームウェア更新イメージ反映処理が完了します。

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

#### addObserver／removeObserver

`McuMgrTransport`の`addObserver()`、`removeObserver()`は、接続状態変更監視を開始／終了させるための関数です。

```
//
// IOS-nRF-Connect-Device-Manager/Source/McuMgrTransport.swift
//
/// The connection state observer protocol.
public protocol McuMgrTransport: AnyObject {
    func addObserver(_ observer: ConnectionObserver);
    func removeObserver(_ observer: ConnectionObserver);
}
```

`McuMgrBleTransport`における実装は以下になります。

```
//
// IOS-nRF-Connect-Device-Manager/Source/Bluetooth/McuMgrBleTransport.swift
//
public class McuMgrBleTransport: NSObject {
    /// An array of observers.
    private var observers: [ConnectionObserver]

    public func addObserver(_ observer: ConnectionObserver) {
        observers.append(observer)
    }

    public func removeObserver(_ observer: ConnectionObserver) {
        if let index = observers.firstIndex(where: {$0 === observer}) {
            observers.remove(at: index)
        }
    }

    private func notifyStateChanged(_ state: McuMgrTransportState) {
        // The list of observers may be modified by each observer.
        // Better iterate a copy of it.
        let array = [ConnectionObserver](observers)
        for observer in array {
            observer.transport(self, didChangeStateTo: state)
        }
    }
}    
```

`notifyStateChanged()`が呼び出されると、`addObserver`によって設定された`ConnectionObserver`を経由して`ConnectionObserver.transport()`が呼び出され、接続状態の変更内容に応じた処理を行う事ができます。

#### ConnectionObserver

デバイスが接続／切断状態になったことを監視するために用意されています。

```
//
// IOS-nRF-Connect-Device-Manager/Source/McuMgrTransport.swift
//
/// The connection state observer protocol.
public protocol ConnectionObserver: AnyObject {
    /// Called whenever the peripheral state changes.
    ///
    /// - parameter transport: the Mcu Mgr transport object.
    /// - parameter state: The new state of the peripheral.
    func transport(_ transport: McuMgrTransport, didChangeStateTo state: McuMgrTransportState)
}
```

`ConnectionObserver.transport()`は、クラス`FirmwareUpgradeManager`に実装されています。

```
public class FirmwareUpgradeManager : FirmwareUpgradeController, ConnectionObserver {
    public func transport(_ transport: McuMgrTransport, didChangeStateTo state: McuMgrTransportState) {
        transport.removeObserver(self)
        // Disregard connected state.
        guard state == .disconnected else {
            return
        }
        self.log(msg: "Device has disconnected (reset). Reconnecting...", atLevel: .info)
        :
    }
}
```

一方、`ConnectionObserver.transport()`は、前述の`McuMgrBleTransport.notifyStateChanged()`経由で呼び出されます。<br>
すなわち、デバイスが接続／切断された時点（下記関数が実行された時点）で、`FirmwareUpgradeManager.transport()`が実行されます。

```
extension McuMgrBleTransport: CBCentralManagerDelegate {
    public func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        :
        state = .disconnected
        notifyStateChanged(.disconnected)
    }
}

extension McuMgrBleTransport: CBPeripheralDelegate {
    public func peripheral(_ peripheral: CBPeripheral,
                           didUpdateNotificationStateFor characteristic: CBCharacteristic,
                           error: Error?) {
        :
        state = .connected
        notifyStateChanged(.connected)
        :
    }
}
```
