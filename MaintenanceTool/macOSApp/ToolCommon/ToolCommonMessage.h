//
//  ToolCommonMessage.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/17.
//
#ifndef ToolCommonMessage_h
#define ToolCommonMessage_h

#pragma mark - USB HID関連
#define MSG_USB_DETECT_FAILED                   @"USBデバイス検知の開始に失敗しました。"
#define MSG_USB_DETECT_STARTED                  @"USBデバイス検知を開始しました。"
#define MSG_HID_REMOVED                         @"USB HIDデバイスが取り外されました。"
#define MSG_HID_CONNECTED                       @"USB HIDデバイスに接続されました。"
#define MSG_HID_CMD_RESPONSE_TIMEOUT            @"認証器からの応答が受信できませんでした。"

#endif /* ToolCommonMessage_h */
