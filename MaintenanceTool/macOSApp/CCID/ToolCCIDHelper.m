//
//  ToolCCIDHelper.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#import <CryptoTokenKit/CryptoTokenKit.h>

#import "debug_log.h"
#import "usb_cdc_util.h"
#import "ToolCommonMessage.h"
#import "ToolCCIDHelper.h"
#import "ToolLogFile.h"
#import "ToolCCIDCommon.h"

@interface ToolCCIDHelper ()

    // 接続されたデバイスの情報を保持
    @property (nonatomic) NSString *slotName;
    // 送信パラメーターを保持
    @property (nonatomic) uint8_t   sendIns;
    @property (nonatomic) uint8_t   sendP1;
    @property (nonatomic) uint8_t   sendP2;
    @property (nonatomic) NSData   *sendData;
    @property (nonatomic) uint16_t  sendLe;

@end

@implementation ToolCCIDHelper

    - (id)init {
        self = [super init];
        return self;
    }

    - (void)setSendParameters:(uint8_t)ins p1:(uint8_t)p1 p2:(uint8_t)p2 data:(NSData *)data le:(uint16_t)le {
        [self setSendIns:ins];
        [self setSendP1:p1];
        [self setSendP2:p2];
        [self setSendData:data];
        [self setSendLe:le];
    }

    - (bool)SCardSlotManagerWillBeginSession {
        // CCIDデバイスとセッションを開始
        TKSmartCardSlotManager *mngr = [TKSmartCardSlotManager defaultManager];
        if ([[mngr slotNames] count] == 0) {
            [[ToolLogFile defaultLogger] error:MSG_CCID_INTERFACE_UNAVAILABLE];
            return false;
        }
        NSString *slotName = [mngr slotNames][0];
        [mngr getSlotWithName:slotName reply:^(TKSmartCardSlot *slot) {
            [self SCardSlotManagerDidGetSlot:slot withName:slotName];
        }];
        return true;
    }

#pragma 

    - (void)SCardSlotManagerDidGetSlot:(TKSmartCardSlot *)slot withName:(NSString *)slotName {
        // 接続デバイス名を保持し、接続を試行
        [self setSlotName:slotName];
        TKSmartCard *card = [slot makeSmartCard];
        if (card == nil) {
            [[ToolLogFile defaultLogger] errorWithFormat:MSG_CCID_DEVICE_UNAVAILABLE, [self slotName]];
            return;
        }
        [card beginSessionWithReply:^(BOOL success, NSError *error) {
            [self SCardSlotManagerDidBeginSession:card withReply:success error:error];
        }];
    }

    - (void)SCardSlotManagerDidBeginSession:(TKSmartCard *)card withReply:(bool)success error:(NSError *)error {
        if (success == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:MSG_CCID_DEVICE_CONNECT_ERROR, [self slotName], [error description]];
            return;
        }
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_CCID_DEVICE_CONNECTED, [self slotName]];
        // コマンドを実行
        uint16_t sw;
        NSNumber *le = [[NSNumber alloc] initWithUnsignedChar:[self sendLe]];
        NSData *response = [card sendIns:[self sendIns] p1:[self sendP1] p2:[self sendP2] data:[self sendData] le:le sw:&sw error:&error];
        if (error) {
            NSLog(@"sendIns error: %@", error);
        } else {
            NSLog(@"Response: %@ 0x%04X", response, sw);
        }
    }

@end
