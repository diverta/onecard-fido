//
//  ToolCCIDHelper.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#import "debug_log.h"
#import "tool_pcsc.h"

#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface ToolCCIDHelper ()

    @property (nonatomic, weak) id<ToolCCIDHelperDelegate> delegate;

    // 接続デバイス名を保持
    @property (nonatomic) NSString *slotName;

@end

@implementation ToolCCIDHelper

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolCCIDHelperDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        return self;
    }

    - (bool)ccidHelperWillConnect {
        // すでに接続済みの場合は終了
        if (tool_pcsc_scard_connected()) {
            [[ToolLogFile defaultLogger] error:MSG_CCID_SESSION_ALREADY_EXIST];
            return false;
        }
        // 接続デバイス名をクリア
        [self setSlotName:nil];
        // CCIDデバイスと接続
        tool_pcsc_scard_init();
        if (tool_pcsc_scard_connect() == false) {
            [self logErrorMessageWithFuncError:MSG_CCID_DEVICE_CONNECT_ERROR];
            return false;
        }
        // デバイス名をログ出力
        NSString *slotName = [[NSString alloc] initWithUTF8String:tool_pcsc_scard_slot_name()];
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_CCID_DEVICE_CONNECTED, slotName];
        // トランザクションを開始
        if (tool_pcsc_scard_begin_transaction() == false) {
            [self logErrorMessageWithFuncError:MSG_CCID_DEVICE_UNAVAILABLE];
            return false;
        }
        // 接続デバイス名を退避
        [self setSlotName:slotName];
        return true;
    }

    - (NSString *)getConnectingSlotName {
        // 接続デバイス名を戻す
        return [self slotName];
    }

    - (void)logErrorMessageWithFuncError:(NSString *)errorMsgTemplate {
        NSString *functionMsg = [[NSString alloc] initWithUTF8String:log_debug_message()];
        NSString *errorMsg = [[NSString alloc] initWithFormat:errorMsgTemplate, functionMsg];
        [[ToolLogFile defaultLogger] error:errorMsg];
    }

    - (void)ccidHelperWillDisconnect {
        // トランザクションを終了
        tool_pcsc_scard_end_transaction();
        // 接続されていた場合は切断
        tool_pcsc_scard_disconnect();
    }

    - (void)ccidHelperWillSendIns:(uint8_t)sendIns p1:(uint8_t)sendP1 p2:(uint8_t)sendP2 data:(NSData *)sendData le:(uint16_t)sendLe {
        // リクエスト送信-->レスポンス受信
        size_t sizeAlreadySent = 0;
        size_t sizeToSend = (size_t)[sendData length];
        uint16_t sw = 0;
        uint8_t sendCla;
        NSMutableData *mutableResponse = [[NSMutableData alloc] init];
        do {
            // 送信サイズとCLA値を設定
            size_t thisSendSize = 0xff;
            if (sizeAlreadySent + thisSendSize < sizeToSend) {
                // 最終フレームでない場合
                sendCla = 0x10;
            } else {
                // 最終フレームの場合
                thisSendSize = sizeToSend - sizeAlreadySent;
                sendCla = 0x00;
            }
            // 今回送信分のAPDUデータを抽出し、送信処理を実行
            NSData *thisSendData = [sendData subdataWithRange:NSMakeRange(sizeAlreadySent, thisSendSize)];
            tool_pcsc_scard_set_command_apdu(sendCla, sendIns, sendP1, sendP2, thisSendSize, (uint8_t *)[thisSendData bytes], sendLe);
            if (tool_pcsc_scard_send_command_apdu() == false) {
                // 送信エラーが発生した場合、エラー内容をログ出力
                [self logErrorMessageWithFuncError:MSG_CCID_REQUEST_SEND_FAILED];
                [self receivedResponse:nil status:SW_UNABLE_TO_PROCESS];
                return;
            }
            // 受信データがある場合は連結
            size_t response_apdu_size;
            uint8_t *response_apdu_data = tool_pcsc_scard_response_apdu_data(&response_apdu_size, &sw);
            if (response_apdu_size > 0) {
                [mutableResponse appendData:[[NSData alloc] initWithBytes:response_apdu_data length:response_apdu_size]];
            }
            // 送信済みサイズを更新
            sizeAlreadySent += thisSendSize;
        } while (sizeAlreadySent < sizeToSend);
        while (sw >> 8 == 0x61) {
            // GET RESPONSE APDU
            tool_pcsc_scard_set_command_apdu(0x00, 0xc0, 0x00, 0x00, 0, NULL, 0xff);
            if (tool_pcsc_scard_send_command_apdu() == false) {
                // 送信エラーが発生した場合、エラー内容をログ出力
                [self logErrorMessageWithFuncError:MSG_CCID_REQUEST_SEND_FAILED];
                [self receivedResponse:nil status:SW_UNABLE_TO_PROCESS];
                return;
            }
            // 受信データがある場合は連結
            size_t response_apdu_size;
            uint8_t *response_apdu_data = tool_pcsc_scard_response_apdu_data(&response_apdu_size, &sw);
            if (sw != SW_SUCCESS && sw >> 8 != 0x61) {
                // ステータスワードが不正の場合は制御を戻す
                [self receivedResponse:nil status:sw];
                return;
            }
            if (response_apdu_size > 0) {
                [mutableResponse appendData:[[NSData alloc] initWithBytes:response_apdu_data length:response_apdu_size]];
            }
        }
        // コマンドに制御を戻す
        [self receivedResponse:mutableResponse status:sw];
    }

    - (void)receivedResponse:(NSData *)response status:(uint16_t)sw {
        [[self delegate] ccidHelperDidReceiveResponse:response status:sw];
    }

@end
