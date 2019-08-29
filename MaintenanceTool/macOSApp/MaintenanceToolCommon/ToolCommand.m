//
//  ToolCommand.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#import <Foundation/Foundation.h>
#import "ToolCommand.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"

@interface ToolCommand ()
    // 送信PINGデータを保持
    @property(nonatomic) NSData *pingData;
@end

@implementation ToolCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            self.delegate = delegate;
        }
        return self;
    }

#pragma mark - Private methods

- (void)createCommandPairing {
    NSLog(@"Pairing start");
    
    // 書き込むコマンドを編集
    unsigned char arr[] = {0x83, 0x00, 0x04, 0x00, 0x45, 0x00, 0x00};
    NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
    [self setBleRequestArray:[NSArray arrayWithObject:commandData]];
}

- (void)createCommandPing {
    NSLog(@"BLE ping start");
    // 100バイトのランダムなPINGデータを生成
    [self setPingData:[ToolCommon generateRandomBytesDataOf:100]];
    // 分割送信のために64バイトごとのコマンド配列を作成する
    [self setBleRequestArray:[self generateCommandArrayFrom:[self pingData] cmd:0x81]];
}

- (NSArray<NSData *> *)generateCommandArrayFrom:(NSData *)dataForCommand cmd:(uint8_t)cmd {
    unsigned char initHeader[] = {cmd, 0x00, 0x00};
    unsigned char contHeader[] = {0x00};

    NSUInteger dataForCommandLength = [dataForCommand length];
    NSUInteger start    = 0;
    char       sequence = 0;

    NSMutableArray<NSData *> *array = [[NSMutableArray alloc] init];
    while (start < dataForCommandLength) {
        NSMutableData *dataRequest = [NSMutableData alloc];
        NSData *dataHeader;
        
        NSUInteger strlen = dataForCommandLength - start;
        if (start == 0) {
            // 最大61バイト分取得する
            if (strlen > 61) {
                strlen = 61;
            }
            // BLEヘッダーにリクエストデータ長を設定する
            initHeader[1] = dataForCommandLength / 256;
            initHeader[2] = dataForCommandLength % 256;
            dataHeader = [[NSData alloc] initWithBytes:initHeader length:sizeof(initHeader)];

        } else {
            // 最大63バイト分取得する
            if (strlen > 63) {
                strlen = 63;
            }
            // BLEヘッダーにシーケンス番号を設定する
            contHeader[0] = sequence++;
            dataHeader = [[NSData alloc] initWithBytes:contHeader length:sizeof(contHeader)];
        }

        // スタート位置からstrlen文字分切り出して、ヘッダーに連結
        [dataRequest appendData:dataHeader];
        [dataRequest appendData:[dataForCommand subdataWithRange:NSMakeRange(start, strlen)]];
        [array addObject:dataRequest];
        
        // スタート位置を更新
        start += strlen;
    }
    
    return array;
}

- (NSData *)generateAPDUDataFrom:(NSData *)data INS:(unsigned char)ins P1:(unsigned char)p1 {
    // APDUを編集するための一時領域
    unsigned char apduHeader[] = {0x00, ins, p1, 0x00, 0x00, 0x00, 0x00};
    unsigned char apduFooter[] = {0x00, 0x00};
    
    // リクエストデータ長を設定
    NSUInteger dataCertLength = [data length];
    apduHeader[sizeof(apduHeader)-2] = dataCertLength / 256;
    apduHeader[sizeof(apduHeader)-1] = dataCertLength % 256;
    
    // ヘッダー＋データ＋フッターを連結し、APDUを作成
    NSMutableData *dataForRequest =
        [[NSMutableData alloc] initWithBytes:apduHeader length:sizeof(apduHeader)];
    [dataForRequest appendData:data];
    [dataForRequest appendBytes:apduFooter length:sizeof(apduFooter)];
    
    return dataForRequest;
}

- (NSMutableData *)createTestRequestData {
    // テストデータから、リクエストデータの先頭部分を生成
    NSData *challenge =
        [ToolCommon generateHexBytesFrom:
            @"124dc843bb8ba61f035a7d0938251f5dd4cbfc96f5453b130d890a1cdbae3220"];
    NSData *appIDHash =
        [ToolCommon generateHexBytesFrom:
            @"23be84e16cd6ae529049f1f1bbe9ebb3a6db3c870c3e99245e0d1c06b747deb3"];

    NSMutableData *requestData = [[NSMutableData alloc] initWithData:challenge];
    [requestData appendData:appIDHash];
    return requestData;
}

- (void)createCommandTestRegister {
    NSLog(@"Health check start");

    // テストデータを編集
    NSMutableData *requestData = [self createTestRequestData];
    
    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x01 P1:0x00];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest cmd:0x83]];
}

- (NSData *)getKeyHandleDataFrom:(NSData *)registerResponse {
    // Registerレスポンスからキーハンドル(67バイト目から65バイト)を切り出し
    return [registerResponse subdataWithRange:NSMakeRange(66, 65)];
}

- (void)createCommandTestAuthFrom:(NSData *)registerResponse P1:(unsigned char)p1 {
    // Registerレスポンスからキーハンドルを切り出し、テストデータに連結
    NSMutableData *requestData = [self createTestRequestData];
    [requestData appendData:[self getKeyHandleDataFrom:registerResponse]];
    
    // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
    NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x02 P1:p1];
    [self setBleRequestArray:[self generateCommandArrayFrom:dataForRequest cmd:0x83]];
}

- (bool)commandArrayIsBlank {
    if ([self bleRequestArray]) {
        if ([[self bleRequestArray] count]) {
            return false;
        }
    }
    return true;
}

- (NSUInteger)getStatusWordFrom:(NSData *)bleResponseData {
    // BLEレスポンスデータから、ステータスワードを取得する
    NSUInteger length = [bleResponseData length];
    NSData *responseStatusWord = [bleResponseData subdataWithRange:NSMakeRange(length-2, 2)];
    unsigned char *statusWordChar = (unsigned char *)[responseStatusWord bytes];
    NSUInteger statusWord = statusWordChar[0] * 256 + statusWordChar[1];
    
    return statusWord;
}

#pragma mark - Public methods

- (void)toolCommandWillCreateBleRequest:(Command)command {
    // コマンドに応じ、以下の処理に分岐
    [self setCommand:command];
    switch (command) {
        case COMMAND_TEST_REGISTER:
            [self createCommandTestRegister];
            break;
        case COMMAND_PAIRING:
            [self createCommandPairing];
            break;
        case COMMAND_TEST_BLE_PING:
            [self createCommandPing];
            break;
        default:
            [self setBleRequestArray:nil];
            break;
    }
    // コマンド生成時
    if ([self commandArrayIsBlank] == false) {
        // コマンド開始メッセージを画面表示し、デリゲートに制御を戻す
        NSString *message = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE,
                             [ToolCommon processNameOfCommand:command]];
        [[self delegate] notifyToolCommandMessage:message];
        [[self delegate] toolCommandDidCreateBleRequest];
    }
}

- (bool)isResponseCompleted:(NSData *)responseData {
    // 受信データおよび長さを保持
    static NSUInteger     totalLength;
    static NSMutableData *receivedData;
    
    // 後続データの存在有無をチェック
    NSData *dataBLEHeader = [responseData subdataWithRange:NSMakeRange(0, 3)];
    unsigned char *bytesBLEHeader = (unsigned char *)[dataBLEHeader bytes];
    if (bytesBLEHeader[0] == 0x82) {
        // キープアライブの場合は引き続き次のレスポンスを待つ
        receivedData = nil;
        
    } else if (bytesBLEHeader[0] == 0x81 || bytesBLEHeader[0] == 0x83) {
        // ヘッダーから全受信データ長を取得
        totalLength  = bytesBLEHeader[1] * 256 + bytesBLEHeader[2];
        // 4バイト目から後ろを切り出して連結
        NSData *tmp  = [responseData subdataWithRange:NSMakeRange(3, [responseData length] - 3)];
        receivedData = [[NSMutableData alloc] initWithData:tmp];
        
    } else {
        // 2バイト目から後ろを切り出して連結
        NSData *tmp  = [responseData subdataWithRange:NSMakeRange(1, [responseData length] - 1)];
        [receivedData appendData:tmp];
    }
    NSLog(@"Received response %@", responseData);
    
    if (receivedData && ([receivedData length] == totalLength)) {
        // 全受信データを保持
        [self setBleResponseData:[[NSData alloc] initWithData:receivedData]];
        receivedData = nil;
        // 後続レスポンスがない
        return false;
        
    } else {
        // 後続レスポンスがある
        [self setBleResponseData:nil];
        return true;
    }
}

- (bool)checkStatusWordOfResponse {
    // レスポンスデータが揃っていない場合は終了
    if (![self bleResponseData]) {
        return false;
    }
    
    // PINGコマンドの場合は成功
    if ([self command] == COMMAND_TEST_BLE_PING) {
        return true;
    }
    
    // ステータスワード(レスポンスの末尾２バイト)を取得
    NSUInteger statusWord = [self getStatusWordFrom:[self bleResponseData]];
    
    // 成功判定は、キーハンドルチェックの場合0x6985、それ以外は0x9000
    if (statusWord == 0x6985) {
        return true;
    } else if (statusWord == 0x9000) {
        return true;
    }
    
    // invalid keyhandleエラーである場合はその旨を通知
    if (statusWord == 0x6a80) {
        [self toolCommandDidProcess:false message:MSG_OCCUR_KEYHANDLE_ERROR];
        return false;
    }
    
    // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
    if (statusWord == 0x9402) {
        [self toolCommandDidProcess:false message:MSG_OCCUR_SKEYNOEXIST_ERROR];
        return false;
    }

    // ペアリングモード時はペアリング以外の機能を実行できない旨を通知
    if (statusWord == 0x9601) {
        [self toolCommandDidProcess:false message:MSG_OCCUR_PAIRINGMODE_ERROR];
        return false;
    }
    
    // ステータスワードチェックがNGの場合
    [self toolCommandDidProcess:false message:MSG_OCCUR_UNKNOWN_BLE_ERROR];
    return false;
}

- (void)checkPingResponseData {
    // PINGレスポンスの内容をチェックし、画面に制御を戻す
    bool result = [[self bleResponseData] isEqualToData:[self pingData]];
    [self toolCommandDidProcess:result message:@"BLE ping end"];
}

- (void)toolCommandWillProcessBleResponse {
    // Registerレスポンスは、３件のテストケースで共通使用するため、
    // ここで保持しておく必要がある
    static NSData *registerReponseData;
    
    // レスポンスをチェックし、内容がNGであれば処理終了
    if ([self checkStatusWordOfResponse] == false) {
        return;
    }
    
    // コマンドに応じ、以下の処理に分岐
    switch ([self command]) {
        case COMMAND_PAIRING:
            [self toolCommandDidProcess:true message:@"Pairing end"];
            break;
        case COMMAND_TEST_BLE_PING:
            // PINGレスポンスの内容をチェックし、画面に制御を戻す
            [self checkPingResponseData];
            break;
        case COMMAND_TEST_REGISTER:
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_REGISTER_SUCCESS];
            NSLog(@"Register test success");
            // Registerレスポンスを内部で保持して後続処理を実行
            registerReponseData = [[NSData alloc] initWithData:[self bleResponseData]];
            [self setCommand:COMMAND_TEST_AUTH_CHECK];
            [self createCommandTestAuthFrom:registerReponseData P1:0x07];
            break;
        case COMMAND_TEST_AUTH_CHECK:
            NSLog(@"Authenticate test (check) success");
            [self setCommand:COMMAND_TEST_AUTH_NO_USER_PRESENCE];
            [self createCommandTestAuthFrom:registerReponseData P1:0x08];
            break;
        case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            NSLog(@"Authenticate test (dont-enforce-user-presence-and-sign) success");
            [self setCommand:COMMAND_TEST_AUTH_USER_PRESENCE];
            [self createCommandTestAuthFrom:registerReponseData P1:0x03];
            // 後続のU2F Authenticateを開始する前に、
            // 基板上のMAIN SWを押してもらうように促すメッセージを表示
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_START];
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT1];
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT2];
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT3];
            break;
        case COMMAND_TEST_AUTH_USER_PRESENCE:
            [[self delegate] notifyToolCommandMessage:MSG_HCHK_U2F_AUTHENTICATE_SUCCESS];
            registerReponseData = nil;
            NSLog(@"Authenticate test (enforce-user-presence-and-sign) success");
            [self toolCommandDidProcess:true message:@"Health check end"];
            break;
        default:
            break;
    }
    // コマンド生成時は後続処理を実行させる
    if ([self commandArrayIsBlank] == false) {
        [self.delegate toolCommandDidCreateBleRequest];
    }
}

- (void)toolCommandDidProcess:(bool)result message:(NSString *)message {
    // コマンド配列をブランクに初期化
    [self setBleRequestArray:nil];
    
    // 引数のメッセージをコンソール出力
    if (message) {
        NSLog(@"%@", message);
    }
    
    // 画面処理時は、処理終了とメッセージ文言をAppDelegateに戻す
    [[self delegate] toolCommandDidProcess:[self command] result:result
                                   message:message];
}

@end
