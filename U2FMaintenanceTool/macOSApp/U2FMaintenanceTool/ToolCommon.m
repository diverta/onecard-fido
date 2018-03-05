//
//  ToolCommon.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/03/05.
//
#import <Foundation/Foundation.h>

#import "ToolCommon.h"

@interface ToolCommon ()

@end

@implementation ToolCommon

    + (NSString *)processNameOfCommand:(Command)command {
        // コマンド種別に対応する名称を戻す
        NSString *processName;
        switch (command) {
            case COMMAND_ERASE_BOND:
                processName = @"ペアリング情報削除処理";
                break;
            case COMMAND_ERASE_SKEY_CERT:
                processName = @"鍵・証明書削除処理";
                break;
            case COMMAND_INSTALL_SKEY:
            case COMMAND_INSTALL_CERT:
                processName = @"鍵・証明書インストール";
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                processName = @"ヘルスチェック";
                break;
            case COMMAND_U2F_PROCESS:
                processName = @"Chromeブラウザから要求された処理";
                break;
            case COMMAND_SETUP_CHROME_NATIVE_MESSAGING:
                processName = @"Chrome Native Messaging有効化設定";
                break;
            case COMMAND_CREATE_KEYPAIR_PEM:
                processName = @"鍵ファイル作成";
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                processName = @"証明書要求ファイル作成";
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                processName = @"自己署名証明書ファイル作成";
                break;
            default:
                processName = NULL;
                break;
        }
        return processName;
    }

@end
