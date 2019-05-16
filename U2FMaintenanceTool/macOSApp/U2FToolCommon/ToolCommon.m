//
//  ToolCommon.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/03/05.
//
#import <Foundation/Foundation.h>

#import "ToolCommon.h"
#import "ToolCommonMessage.h"

@interface ToolCommon ()

@end

@implementation ToolCommon

    + (NSString *)processNameOfCommand:(Command)command {
        // コマンド種別に対応する処理名称を戻す
        NSString *processName;
        switch (command) {
            case COMMAND_PAIRING:
                processName = PROCESS_NAME_PAIRING;
                break;
            case COMMAND_ERASE_SKEY_CERT:
                processName = PROCESS_NAME_ERASE_SKEY_CERT;
                break;
            case COMMAND_INSTALL_SKEY:
            case COMMAND_INSTALL_CERT:
                processName = PROCESS_NAME_INSTALL_SKEY_CERT;
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                processName = PROCESS_NAME_HEALTHCHECK;
                break;
            case COMMAND_U2F_HID_PROCESS:
                processName = PROCESS_NAME_U2F_HID_PROCESS;
                break;
            case COMMAND_CREATE_KEYPAIR_PEM:
                processName = PROCESS_NAME_CREATE_KEYPAIR_PEM;
                break;
            case COMMAND_CREATE_CERTREQ_CSR:
                processName = PROCESS_NAME_CREATE_CERTREQ_CSR;
                break;
            case COMMAND_CREATE_SELFCRT_CRT:
                processName = PROCESS_NAME_CREATE_SELFCRT_CRT;
                break;
            case COMMAND_TEST_CTAPHID_INIT:
                processName = PROCESS_NAME_TEST_CTAPHID_INIT;
                break;
            default:
                processName = nil;
                break;
        }
        return processName;
    }

@end
