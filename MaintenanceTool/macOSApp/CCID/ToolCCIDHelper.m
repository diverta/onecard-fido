//
//  ToolCCIDHelper.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#import "debug_log.h"
#import "ToolCCIDCommand.h"
#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface ToolCCIDHelper ()

    // 接続されたデバイスの情報を保持
    @property (nonatomic) NSString *slotName;
    // 送信パラメーターを保持
    @property (nonatomic) ToolCCIDCommand *commandRef;
    @property (nonatomic) uint8_t   sendIns;
    @property (nonatomic) uint8_t   sendP1;
    @property (nonatomic) uint8_t   sendP2;
    @property (nonatomic) NSData   *sendData;
    @property (nonatomic) uint16_t  sendLe;

@end

@implementation ToolCCIDHelper

    - (id)init {
        self = [super init];
        [self clearSendParameters];
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

    - (void)setSendParameters:(ToolCCIDCommand *)ref ins:(uint8_t)ins p1:(uint8_t)p1 p2:(uint8_t)p2 data:(NSData *)data le:(uint16_t)le {
        [self setCommandRef:ref];
        [self setSendIns:ins];
        [self setSendP1:p1];
        [self setSendP2:p2];
        [self setSendData:data];
        [self setSendLe:le];
    }

    - (void)SCardSlotManagerWillBeginSession {
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
        uint16_t sw = 0;
        NSNumber *le = [[NSNumber alloc] initWithUnsignedChar:[self sendLe]];
        NSData *response = [card sendIns:[self sendIns] p1:[self sendP1] p2:[self sendP2] data:[self sendData] le:le sw:&sw error:&error];
        if (error) {
            [[ToolLogFile defaultLogger] errorWithFormat:MSG_CCID_REQUEST_SEND_FAILED, [self slotName], [error description]];
        }
        // コマンドに制御を戻す
        [self exitHelperProcess:(error == nil) response:response status:sw];
    }

#pragma mark - Exit function

    - (void)exitHelperProcess:(bool)success response:(NSData *)response status:(uint16_t)sw {
        // パラメーターを初期化
        ToolCCIDCommand *commandRef = [self commandRef];
        [self clearSendParameters];
        // コマンドの参照が無い場合は終了
        if (commandRef == nil) {
            return;
        }
        // コマンドに制御を戻す
        [commandRef ccidHelperDidProcess:success response:response status:sw];
    }

@end
