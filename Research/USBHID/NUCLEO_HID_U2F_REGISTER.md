# HID U2FデバイスからU2F Registerを実行

## 概要

NUCLEO（STM32開発環境）＋mbed OSにより、HID U2Fデバイスを、既存のU2F管理ツールと連携させるところまで試してみました。<br>
レポートはこちら---><b>[HID U2FデバイスとU2F管理ツールの連携](NUCLEO_HID_U2FMNT.md)</b>

上記のテストで使用したハードウェアで、HID U2Fデバイスから、U2F管理ツールを経て、One CardのU2F Registerコマンドを実行させるところまで試します。

## U2F管理ツールを改修

U2F管理ツールに、HID U2FデバイスとOne Cardを連携させるための処理を追加します。

### HIDデバイスからのメッセージをOne Cardへ転送

HIDデバイスから転送されたAPDU（デバイスから転送されたデータから、ヘッダー＆フッターを削除したデータ）を、`ToolCommand`の`bleRequestArray`により引き渡します。<br>
その後、`ToolCommand`の該当コマンド`COMMAND_U2F_HID_PROCESS`を実行します。

```
#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)hidHelperMessages {
        // HIDデバイスから受信したメッセージをToolCommandに引き渡し
        [[self toolCommand] setBleRequestArray:hidHelperMessages];
        // ToolCommandの該当コマンドを実行する
        [self enableButtons:false];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_U2F_HID_PROCESS];
    }
```

コマンドでは、APDUからBLEへ転送するデータ（INIT/CONT各フレーム）を生成します。

```
- (void)toolCommandWillCreateBleRequest:(Command)command {
    // コマンドに応じ、以下の処理に分岐
    [self setCommand:command];
    switch (command) {
        :
        case COMMAND_U2F_HID_PROCESS:
            [self createCommandU2FHIDProcess];
            break;
        :

- (void)createCommandU2FHIDProcess {
    // HIDデバイスから転送されたAPDUより、分割送信のために64バイトごとのコマンド配列を作成
    NSData *dataForRequest = [self bleRequestArray] objectAtIndex:0];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest]];
}
```
以降は、他のU2F管理ツールにおけるコマンドと同様、BLE接続後、One Card側で処理が実行されます。

### One CardからのレスポンスをHIDデバイスへ転送

One Card側での処理が完了すると、U2Fレスポンスが`ToolCommand`の`bleResponseData`に格納され戻ってきます。

```
- (void)centralManagerDidReceive:(NSData *)bleResponse {
        :
        // レスポンスを次処理に引き渡す
        [self.toolCommand toolCommandWillProcessBleResponse];
        :

- (void)toolCommandWillProcessBleResponse {
    :
    // コマンドに応じ、以下の処理に分岐
    switch ([self command]) {
        :
        case COMMAND_U2F_PROCESS:
            [self toolCommandDidProcess:true message:@"U2F response received"];
            break;
            :

- (void)toolCommandDidProcess:(bool)result message:(NSString *)message {
    :
    if ([self command] == COMMAND_U2F_HID_PROCESS) {
        // U2Fレスポンスデータをアプリケーションへ戻す
        [[self delegate] toolCommandDidReceiveResponse:[self command]
            responseData:[self bleResponseData]];
        :
```

アプリケーション・デリゲートでは、U2Fレスポンスを無編集でHIDデバイスへ転送します。


```
- (void)toolCommandDidReceiveResponse:(Command)command responseData:(NSData *)responseData {
    // BLEデバイス接続を切断
    [[self toolBLECentral] centralManagerWillDisconnect];
    // U2FレスポンスをHIDデバイスに転送
    [[self toolHIDHelper] hidHelperWillSend:responseData];
}
```

## U2Fクライアントによる確認

### U2F Registerを起動

<b>後報</b>
