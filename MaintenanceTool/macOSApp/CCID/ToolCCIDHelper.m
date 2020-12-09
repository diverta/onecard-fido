//
//  ToolCCIDHelper.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#import "debug_log.h"
#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface ToolCCIDHelper ()

    @property (nonatomic, weak) id<ToolCCIDHelperDelegate> delegate;

    // 接続されたデバイスの情報を保持
    @property (nonatomic) NSString *slotName;
    // 送信パラメーターを保持
    @property (nonatomic) id        commandRef;
    @property (nonatomic) uint8_t   sendIns;
    @property (nonatomic) uint8_t   sendP1;
    @property (nonatomic) uint8_t   sendP2;
    @property (nonatomic) NSData   *sendData;
    @property (nonatomic) uint16_t  sendLe;

@end

@implementation ToolCCIDHelper

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolCCIDHelperDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self clearSendParameters];
        }
        return self;
    }

    - (void)clearSendParameters {
        [self setCommandRef:nil];
        [self setSendIns:0];
        [self setSendP1:0];
        [self setSendP2:0];
        [self setSendData:nil];
        [self setSendLe:0];
    }

    - (void)setSendParameters:(id)ref ins:(uint8_t)ins p1:(uint8_t)p1 p2:(uint8_t)p2 data:(NSData *)data le:(uint16_t)le {
        [self setCommandRef:ref];
        [self setSendIns:ins];
        [self setSendP1:p1];
        [self setSendP2:p2];
        [self setSendData:data];
        [self setSendLe:le];
    }

    - (void)SCardSlotManagerWillBeginSession:(id)ref ins:(uint8_t)ins p1:(uint8_t)p1 p2:(uint8_t)p2 data:(NSData *)data le:(uint16_t)le {
        // セッション開始済みの場合は終了
        if ([self commandRef] != nil) {
            [[ToolLogFile defaultLogger] error:MSG_CCID_SESSION_ALREADY_EXIST];
            [self exitHelperProcess:false response:nil status:0];
            return;
        }
        // パラメーターを退避
        [self setSendParameters:ref ins:ins p1:p1 p2:p2 data:data le:le];
        // CCIDデバイスとセッションを開始
        TKSmartCardSlotManager *mngr = [TKSmartCardSlotManager defaultManager];
        if ([[mngr slotNames] count] == 0) {
            // デバイス名称が取得できない場合は、以降の処理を行わず、コマンドに制御を戻す
            [[ToolLogFile defaultLogger] error:MSG_CCID_INTERFACE_UNAVAILABLE];
            [self exitHelperProcess:false response:nil status:0];
            return;
        }
        NSString *slotName = [mngr slotNames][0];
        ToolCCIDHelper * __weak weakSelf = self;
        [mngr getSlotWithName:slotName reply:^(TKSmartCardSlot *slot) {
            [weakSelf SCardSlotManagerDidGetSlot:slot withName:slotName];
        }];
    }

    - (void)SCardSlotManagerDidGetSlot:(TKSmartCardSlot *)slot withName:(NSString *)slotName {
        // 接続デバイス名を保持し、接続を試行
        [self setSlotName:slotName];
        TKSmartCard *card = [slot makeSmartCard];
        if (card == nil) {
            // 接続されなかった場合は、以降の処理を行わず、コマンドに制御を戻す
            [[ToolLogFile defaultLogger] errorWithFormat:MSG_CCID_DEVICE_UNAVAILABLE, [self slotName]];
            [self exitHelperProcess:false response:nil status:0];
            return;
        }
        ToolCCIDHelper * __weak weakSelf = self;
        [card beginSessionWithReply:^(BOOL success, NSError *error) {
            [weakSelf SCardSlotManagerDidBeginSession:card withReply:success error:error];
        }];
    }

    - (void)SCardSlotManagerDidBeginSession:(TKSmartCard *)card withReply:(bool)success error:(NSError *)error {
        // セッション終了済みの場合は終了
        if ([self commandRef] == nil) {
            return;
        }
        // 接続されなかった場合は、以降の処理を行わず、コマンドに制御を戻す
        if (success == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:MSG_CCID_DEVICE_CONNECT_ERROR, [self slotName], [error description]];
            [self exitHelperProcess:false response:nil status:0];
            return;
        }
        // リクエスト送信-->レスポンス受信
        size_t sizeAlreadySent = 0;
        size_t sizeToSend = (size_t)[[self sendData] length];
        uint16_t sw = 0;
        NSMutableData *mutableResponse = [[NSMutableData alloc] init];
        do {
            // 送信サイズとCLA値を設定
            size_t thisSendSize = 0xff;
            if(sizeAlreadySent + thisSendSize < sizeToSend) {
                // 最終フレームでない場合
                [card setCla:0x10];
            } else {
                // 最終フレームの場合
                thisSendSize = sizeToSend - sizeAlreadySent;
                [card setCla:0x00];
            }
            // 今回送信分のAPDUデータを抽出し、送信処理を実行
            NSData *thisSendData = [[self sendData] subdataWithRange:NSMakeRange(sizeAlreadySent, thisSendSize)];
            NSNumber *le = [[NSNumber alloc] initWithUnsignedChar:[self sendLe]];
            sw = 0;
            NSData *response = [card sendIns:[self sendIns] p1:[self sendP1] p2:[self sendP2] data:thisSendData le:le sw:&sw error:&error];
            if (response == nil && error) {
                // レスポンスが無く、かつerrorが設定されている場合は、送信エラー発生と判断し、内容をログ出力
                [[ToolLogFile defaultLogger] errorWithFormat:MSG_CCID_REQUEST_SEND_FAILED, [self slotName], [error description]];
                break;
            }
            // 受信データがある場合は連結
            if (response != nil && [response length] > 0) {
                [mutableResponse appendData:response];
            }
            // 送信済みサイズを更新
            sizeAlreadySent += thisSendSize;
        } while (sizeAlreadySent < sizeToSend);
        // コマンドに制御を戻す
        [self exitHelperProcess:(error == nil) response:mutableResponse status:sw];
    }

#pragma mark - Exit function

    - (void)exitHelperProcess:(bool)success response:(NSData *)response status:(uint16_t)sw {
        // パラメーターを初期化
        id commandRef = [self commandRef];
        [self clearSendParameters];
        // セッションが存在する場合はコマンドに制御を戻す
        if (commandRef != nil) {
            [[self delegate] ccidHelperDidProcess:success response:response status:sw];
        }
    }

@end
