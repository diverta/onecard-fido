# HID U2FデバイスとU2F管理ツールの連携

## 概要

NUCLEO（STM32開発環境）＋mbed OSにより、U2F HID Init／U2F Versionを実行させるためのコマンドを実装するところまで試してみました。<br>
レポートはこちら---><b>[HID U2Fデバイスにコマンドを実装](NUCLEO_HID_U2F_COMMAND.md)</b>

上記のテストで使用したハードウェアで、HID U2FデバイスとU2F管理ツールを連携させるところまで試します。

## サンプルアプリを改修

前述テストで使用した、サンプルのmbedアプリケーションのmain関数に、HID U2FデバイスとU2F管理ツールを連携させるための処理を追加します。

改修したサンプルアプリのソースは[こちらのフォルダー](NucleoF411RE_usbmouse)に格納しておきます。

### U2F管理ツールにデータを転送

U2F RegisterリクエストがHIDデバイスに送信されたら、HIDデバイスで受信したリクエストデータを、無編集でU2F管理ツールに転送します。

ただし、HIDデバイスから１度に送信できるデータは32バイトであるため、ヘッダーを除くデータを分割した上で、U2F管理ツールに送信します。<br>
ヘッダーでは、U2F管理ツールのチャネルIDを0x00と設定します。

```
bool send_response_packet() {
    :
    if (CMD == U2FHID_MSG) {
        :
        if (ins == U2F_REGISTER) {
            // リクエストデータをU2F管理ツールに転送
            if (send_xfer_report(u2f_request_buffer, u2f_request_length) == false) {
                return false;
            }
            :

static bool send_xfer_report(uint8_t *payload_data, size_t payload_length) {
    :
    for (size_t i = 0; i < payload_length; i += xfer_data_len) {
        // データ長
        remaining = payload_length - i;
        xfer_data_max = (i == 0) ? 25 : 27;
        xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;

        // パケットを生成（CIDを0x00000000に設定）
        generate_hid_input_report(payload_data, payload_length, i, xfer_data_len, 0x00, CMD);

        // パケットをU2F管理ツールへ転送
        if (u2fAuthenticator.send2(&send_report) == false) {
            printf("u2fAuthenticator.send failed. \r\n");
            return false;
        }
        :
```

### U2F管理ツールからのデータを受信

U2F RegisterレスポンスがU2F管理ツールから送信されたら、これをHIDデバイスで受信します。<br>
U2F管理ツールからのレスポンス受信には、後述の`readNB2`関数を使用します。

```
int main(void) {
    :
    while (true) {
        :
        if (u2fAuthenticator.readNB2(&recv_report)) {
            if (receive_xfer_response_data() == true) {
                // リクエストを全て受領したらレスポンス
                send_xfer_response_packet();
            }
            :

bool receive_xfer_response_data(void) {
    :
    if (U2FHID_IS_INIT(req->pkt.init.cmd)) {
        // payload長を取得
        payload_len = get_payload_length(req);

        // リクエストデータ領域に格納
        pos = (payload_len < init_payload_size) ? payload_len : init_payload_size;
        memset(&u2f_request_buffer, 0, sizeof(HID_REPORT));
        memcpy(u2f_request_buffer, req->pkt.init.payload, pos);

        dump_hid_init_packet("Recv ", recv_report.length, req, pos);

   } else {
        // リクエストデータ領域に格納
        size_t remain = payload_len - pos;
        size_t cnt = (remain < cont_payload_size) ? remain : cont_payload_size;
        memcpy(u2f_request_buffer + pos, req->pkt.cont.payload, cnt);
        pos += cnt;

        dump_hid_cont_packet("Recv ", recv_report.length, req, cnt);
    }
    :
```

受信したメッセージは、そのまま無編集でU2Fクライアントに送信します。

```
bool send_xfer_response_packet(void) {
    if (CMD == U2FHID_MSG) {
        // レスポンスデータを送信パケットに設定
        generate_u2f_register_response();
        if (send_hid_input_report(u2f_response_buffer, u2f_response_length) == false) {
            return false;
        }
        :

void generate_u2f_register_response(void) {
    // U2F管理ツールから転送されたレスポンスデータを設定
    u2f_response_length = u2f_request_length;
    memcpy(u2f_response_buffer, u2f_request_buffer, u2f_request_length);
}

static bool send_hid_input_report(uint8_t *payload_data, size_t payload_length) {
    :
    for (size_t i = 0; i < payload_length; i += xfer_data_len) {
        :
        // パケットを生成
        generate_hid_input_report(payload_data, payload_length, i, xfer_data_len, CID, CMD);

        // パケットをU2Fクライアントへ転送
        if (u2fAuthenticator.send(&send_report) == false) {
            printf("u2fAuthenticator.send failed. \r\n");
            return false;
        }
        :
```

## HIDインターフェースを２件実装

USBデバイスにHIDインターフェースを１点で実装すると、デバイス（mbedアプリケーション）からU2F管理ツール向けに転送したデータが、ChromeのU2Fクライアントで受信されてしまう問題が確認されました。

こういった不具合は、将来的な本格開発やFIDOアライアンス認証取得のために回避したいので、USBデバイスに、HIDインターフェースを２点実装します。<br>
具体的には、HIDインターフェース＃１をChrome U2Fクライアント通信専用、HIDインターフェース＃２をU2Fクライアント通信専用とします。

この対応により、U2F管理ツールに送信したメッセージ（HID Report）が、U2Fクライアントでは受信されないようになります。

### mbedライブラリーの一部修正

HIDインターフェースの複数実装はmbedの`USBDEVICE`ライブラリーでは想定されていない仕様ですので、ライブラリー・ファイルを直接修正する必要があります。<br>
修正点は以下の通りです。

#### USBDEVICE/USBDevice/USBHAL_STM32F4.cpp

インターフェース＃２のエンドポイントとして使用する`EPBULK_IN`、`EPBULK_OUT`は、mbedではバルク通信に割り当てられていますが、これを`EPINT_IN`、`EPINT_OUT`と同様、割込み通信に割り当てるため、下記のようにtype変数を3に設定するよう修正します。
```
bool USBHAL::realiseEndpoint(uint8_t endpoint, uint32_t maxPacket,
                             uint32_t flags) {
    uint32_t epIndex = endpoint >> 1;

    uint32_t type;
    switch (endpoint) {
        case EP0IN:
        case EP0OUT:
            type = 0;
            break;
        case EPISO_IN:
        case EPISO_OUT:
            type = 1;
        case EPBULK_IN:
        case EPBULK_OUT:
            //
            // mbed original から変更:
            //  EP2を使用するインターフェースの
            //  type を 3 に設定
            //
            // type = 2;
            // break;
        case EPINT_IN:
        case EPINT_OUT:
            type = 3;
            break;
    }
```

#### USBDEVICE/USBHID/USBHID.cpp

HIDインタフェースを２件実装するため、`USBCallback_setConfiguration`関数により、エンドポイントを２件追加します。<br>
前述の通り、mbedでバルク通信用として割り当てられている`EPBULK_IN`、`EPBULK_OUT`をエンドポイントとして使用するものとします。

```
bool USBHID::USBCallback_setConfiguration(uint8_t configuration) {
    :
    addEndpoint(EPINT_IN, MAX_PACKET_SIZE_EPINT);
    addEndpoint(EPINT_OUT, MAX_PACKET_SIZE_EPINT);

    addEndpoint(EPBULK_IN, MAX_PACKET_SIZE_EPINT);  // EP2_IN
    addEndpoint(EPBULK_OUT, MAX_PACKET_SIZE_EPINT); // EP2_OUT

    // We activate the endpoint to be able to recceive data
    readStart(EPINT_OUT, MAX_PACKET_SIZE_EPINT);
    return true;
}
```

前述で追加したインターフェース＃２に対応するReportDescを取得／設定するロジックがないため、インターフェース＃２用のReportDesc内容（USAGE_PAGE／USAGE）を直接書き換えるようにします。

```
bool USBHID::USBCallback_request() {
        :
        case REPORT_DESCRIPTOR:
            if ((reportDesc() != NULL)  && (reportDescLength() != 0)) {
                transfer->remaining = reportDescLength();
                transfer->ptr = reportDesc();

                //
                // mbed original から変更:
                //  EP2を使用するインターフェースの
                //  USAGE_PAGEを
                //  0xff00 に設定
                //
                if (transfer->setup.wIndex == 1) {
                    transfer->ptr[1] = 0x00;
                    transfer->ptr[2] = 0xff;
                }

                transfer->direction = DEVICE_TO_HOST;
                success = true;
            }
            break;
        :
```

USAGE_PAGE = 0xff00 というのは、Vendor Defined Page として知られているものになります。<br>
https://www.itf.co.jp/tech/road-to-usb-master/hid_class

### サンプルアプリでの修正

追加実装したエンドポイントを使用し、U2F管理ツールとの通信を行うための関数を追加します。

受信用の関数です。
```
bool USBU2FAuthenticator::readNB2(HID_REPORT *report) {
    uint32_t bytesRead = 0;
    bool result;
    result = USBDevice::readEP_NB(EPBULK_OUT, report->data, &bytesRead, MAX_HID_REPORT_SIZE);
    // if readEP_NB did not succeed, does not issue a readStart
    if (!result)
        return false;
    report->length = bytesRead;
    if(!readStart(EPBULK_OUT, MAX_HID_REPORT_SIZE))
        return false;
    return result;
}
```

こちらは送信用の関数になります。
```
bool USBU2FAuthenticator::send2(HID_REPORT *report) {
    return write(EPBULK_IN, report->data, report->length, MAX_HID_REPORT_SIZE);
}
```

## U2F管理ツールを改修

U2F管理ツール側でも、HID U2FデバイスとU2F管理ツールを連携させるための処理を追加します。

### HIDデバイスからの転送メッセージを受信

HIDデバイスから転送メッセージを受信するための初期設定を、`ToolHIDHelper`の`initializeHIDManager`関数で実行します。

```
- (void)initializeHIDManager {
    :
    IOReturn ret = IOHIDManagerOpen([self toolHIDManager], kIOHIDOptionsTypeNone);
    :
    // ハンドラー定義
    :
    IOHIDManagerRegisterInputReportCallback(
        [self toolHIDManager], &handleInputReport, (__bridge void *)self);
    :
```

HIDデバイスから転送メッセージが受信されると、`handleInputReport`関数が実行されます。<br>
ここでは、転送されたメッセージがコンソールにデバッグプリントされます。

```
void handleInputReport(void *context, IOReturn result, void *sender, IOHIDReportType type,
                       uint32_t reportID, uint8_t *report, CFIndex reportLength) {
    // 受信メッセージを転送
    ToolHIDHelper *helperSelf = (__bridge ToolHIDHelper *)context;
    [helperSelf HIDManagerDidReceiveMessage:report length:reportLength];
}

- (void)HIDManagerDidReceiveMessage:(uint8_t *)message length:(long)length {
    :
    NSData *reportData = [[NSData alloc] initWithBytes:message length:length];
    NSLog(@"ToolHIDHelper receive: reportLength(%ld) report(%@)", length, reportData);
    :
    // パケットをすべて受信したら、U2Fリクエスト（APDUヘッダー＋データ）をアプリケーションに引き渡す
    remaining -= datalen;
    if (remaining == 0) {
        NSLog(@"hidHelperDidReceive(%lu bytes): %@",
              (unsigned long)[[self hidU2FRequest] length],
              [self hidU2FRequest]);
        [[self delegate] hidHelperDidReceive:[self hidU2FRequest]];
    }
}
```

ここで、U2F管理ツールに制御が渡り、アプリケーション・デリゲートの`hidHelperDidReceive`が呼び出されます。

今回は調査目的のため、転送されたデータをそのまま、HIDデバイスにレスポンスするようにしています。<br>
（これは仮コードであり、後日One Cardの処理を呼び出してそのレスポンスを転送するように変更予定）

```
#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)hidHelperMessages {
        // HIDデバイスから受信したメッセージをエコーバック（仮の実装です）
        [[self toolHIDHelper] hidHelperWillSend:hidHelperMessages];
    }
```

## U2Fクライアントによる確認

### U2F Registerを起動

Googleアカウントの２段階認証設定ページを表示させます。<br>
下図のような画面に遷移させます。

<img src="assets/0015.png" width="720">

画面上の「Security Key」セクションにある「ADD SECURITY KEY」リンクをクリックし、U2F Registerを開始させます。<br>
下図のようなポップアップが表示され、U2F Registerが起動します。

<img src="assets/0016.png" width="300">

### mbedアプリケーションのデバッグプリント

U2Fクライアント上でU2F Registerが起動すると、mbedアプリケーションにチャネルID`0x00003301`でU2Fリクエストが送信されます。<br>

```
Recv ( 64 bytes) CID: 0x00003301, CMD: 0x83, Payload( 73 bytes): 00 01 03 00 00 00 40 a5 68 cc bb 4d b3 5b 66 21 c4 d0 e1 8c 0d 82 3c b6 f3 f6 f9 e3 1a 12 c1 a2 d7 84 b1 02 a0 d7 21 a5 46 72 b2 22 c4 cf 95 e1 51 ed 8d 4d 3c 76 7a 6c c3
Recv ( 64 bytes) CID: 0x00003301, SEQ: 0x00, 49 43 59 43 79 4e 88 4f 3d 02 3a 82 29 fd 00 00
```

mbedアプリケーションは、そのU2Fリクエストを、チャネルID`0x00000000`でU2F管理ツールに転送します。

```
Send ( 32 bytes) CID: 0x00000000, CMD: 0x83, Payload( 73 bytes): 00 01 03 00 00 00 40 a5 68 cc bb 4d b3 5b 66 21 c4 d0 e1 8c 0d 82 3c b6 f3
Send ( 32 bytes) CID: 0x00000000, SEQ: 0x00, f6 f9 e3 1a 12 c1 a2 d7 84 b1 02 a0 d7 21 a5 46 72 b2 22 c4 cf 95 e1 51 ed 8d 4d
Send ( 32 bytes) CID: 0x00000000, SEQ: 0x01, 3c 76 7a 6c c3 49 43 59 43 79 4e 88 4f 3d 02 3a 82 29 fd 00 00
```

U2F管理ツールからは、チャネルID`0x00000000`で、ECHOバックがレスポンスされます。<br>
（こちらは後日、正式な処理に置き換える予定です）

```
Recv ( 32 bytes) CID: 0x00000000, CMD: 0x83, Payload( 73 bytes): 00 01 03 00 00 00 40 a5 68 cc bb 4d b3 5b 66 21 c4 d0 e1 8c 0d 82 3c b6 f3
Recv ( 32 bytes) CID: 0x00000000, SEQ: 0x00, f6 f9 e3 1a 12 c1 a2 d7 84 b1 02 a0 d7 21 a5 46 72 b2 22 c4 cf 95 e1 51 ed 8d 4d
Recv ( 32 bytes) CID: 0x00000000, SEQ: 0x01, 3c 76 7a 6c c3 49 43 59 43 79 4e 88 4f 3d 02 3a 82 29 fd 00 00
```

U2F管理ツールからのレスポンスは、さらにmbedアプリケーションから、チャネルID`0x00003301`を使いU2Fクライアントに転送されます。

```
Send ( 32 bytes) CID: 0x00003301, CMD: 0x83, Payload( 73 bytes): 00 01 03 00 00 00 40 a5 68 cc bb 4d b3 5b 66 21 c4 d0 e1 8c 0d 82 3c b6 f3
Send ( 32 bytes) CID: 0x00003301, SEQ: 0x00, f6 f9 e3 1a 12 c1 a2 d7 84 b1 02 a0 d7 21 a5 46 72 b2 22 c4 cf 95 e1 51 ed 8d 4d
Send ( 32 bytes) CID: 0x00003301, SEQ: 0x01, 3c 76 7a 6c c3 49 43 59 43 79 4e 88 4f 3d 02 3a 82 29 fd 00 00
```


### U2F管理ツールのデバッグプリント

mbedアプリケーションからU2F管理ツールに、U2F Registerリクエストが転送されると、下記のようなデバッグプリントが出力されます。<br>
U2F Registerリクエストが、３分割されて受信されることが確認できます。

```
デフォルト	10:53:17.233576 +0900	U2FMaintenanceTool	ToolHIDHelper receive: reportLength(32) report(<00000000 83004900 01030000 0040a568 ccbb4db3 5b6621c4 d0e18c0d 823cb6f3>)
デフォルト	10:53:17.365483 +0900	U2FMaintenanceTool	ToolHIDHelper receive: reportLength(32) report(<00000000 00f6f9e3 1a12c1a2 d784b102 a0d721a5 4672b222 c4cf95e1 51ed8d4d>)
デフォルト	10:53:17.481541 +0900	U2FMaintenanceTool	ToolHIDHelper receive: reportLength(32) report(<00000000 013c767a 6cc34943 5943794e 884f3d02 3a8229fd 00000000 00000000>)
デフォルト	10:53:17.481666 +0900	U2FMaintenanceTool	hidHelperDidReceive(73 bytes): <00010300 000040a5 68ccbb4d b35b6621 c4d0e18c 0d823cb6 f3f6f9e3 1a12c1a2 d784b102 a0d721a5 4672b222 c4cf95e1 51ed8d4d 3c767a6c c3494359 43794e88 4f3d023a 8229fd00 00>
```

現時点では、U2F Registerリクエストに対しては、ECHOバックでレスポンスするようにしています。<br>
（こちらは後日、正式な処理に置き換える予定です）

```
デフォルト	10:53:17.481731 +0900	U2FMaintenanceTool	hidHelperWillSend(73 bytes): <00010300 000040a5 68ccbb4d b35b6621 c4d0e18c 0d823cb6 f3f6f9e3 1a12c1a2 d784b102 a0d721a5 4672b222 c4cf95e1 51ed8d4d 3c767a6c c3494359 43794e88 4f3d023a 8229fd00 00>
デフォルト	10:53:17.485404 +0900	U2FMaintenanceTool	ToolHIDHelper send: messageLength(32) message(<00000000 83004900 01030000 0040a568 ccbb4db3 5b6621c4 d0e18c0d 823cb6f3>)
デフォルト	10:53:17.493388 +0900	U2FMaintenanceTool	ToolHIDHelper send: messageLength(32) message(<00000000 00f6f9e3 1a12c1a2 d784b102 a0d721a5 4672b222 c4cf95e1 51ed8d4d>)
デフォルト	10:53:17.649414 +0900	U2FMaintenanceTool	ToolHIDHelper send: messageLength(32) message(<00000000 013c767a 6cc34943 5943794e 884f3d02 3a8229fd 00000000 00000000>)
```


これで、Chrome標準サポートのU2Fクライアントから、U2F RegisterリクエストがU2F管理ツールまで流れ、そのレスポンスがHIDデバイスまで戻るところまで確認できました。
