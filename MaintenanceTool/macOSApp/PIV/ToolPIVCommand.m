//
//  ToolPIVCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#import "debug_log.h"
#import "tool_crypto_common.h"
#import "tool_crypto_des.h"
#import "tool_piv_admin.h"

#import "AppCommonMessage.h"
#import "ToolAppCommand.h"
#import "PIVPreferenceWindow.h"
#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolInfoWindow.h"
#import "ToolLogFile.h"
#import "ToolPIVCommand.h"
#import "ToolPIVCommon.h"
#import "ToolPIVImporter.h"
#import "ToolPIVSetting.h"

@implementation ToolPIVParameter

@end

@interface ToolPIVCommand () <ToolCCIDHelperDelegate>

    // CCIDインターフェース処理の参照を保持
    @property (nonatomic) ToolCCIDHelper    *toolCCIDHelper;
    // 処理機能名称を保持
    @property (nonatomic) NSString          *nameOfCommand;
    // コマンドを保持
    @property (nonatomic) Command            command;
    @property (nonatomic) uint8_t            commandIns;
    // コマンドのパラメーターを保持
    @property (nonatomic) NSString          *pinCodeCur;
    @property (nonatomic) NSString          *pinCodeNew;
    @property (nonatomic) ToolPIVImporter   *toolPIVImporter;
    // エラーメッセージテキストを保持
    @property (nonatomic) NSString          *errorMessageOfCommand;
    // PIV管理機能認証（往路）のチャレンジを保持
    @property (nonatomic) NSData            *pivAuthChallenge;
    // CCCインポート処理が実行中かどうかを保持
    @property (nonatomic) bool               cccImportProcessing;
    // 現在取得中のPIVオブジェクトIDを保持
    @property (nonatomic) unsigned int       objectIdToFetch;
    // PIV設定情報クラスの参照を保持
    @property (nonatomic) ToolPIVSetting    *toolPIVSetting;
    // 画面の参照を保持
    @property (nonatomic, weak) ToolAppCommand  *toolAppCommand;
    @property (nonatomic) PIVPreferenceWindow   *pivPreferenceWindow;

@end

@implementation ToolPIVCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 画面の参照を保持
            [self setToolAppCommand:delegate];
            // ToolCCIDHelperのインスタンスを生成
            [self setToolCCIDHelper:[[ToolCCIDHelper alloc] initWithDelegate:self]];
            [self clearCommandParameters];
            // PIV設定画面のインスタンスを生成
            [self setPivPreferenceWindow:[[PIVPreferenceWindow alloc] initWithWindowNibName:@"PIVPreferenceWindow"]];
        }
        return self;
    }

    - (void)clearCommandParameters {
        // コマンドおよびパラメーターを初期化
        [self setNameOfCommand:nil];
        [self setCommand:COMMAND_NONE];
        [self setCommandIns:0x00];
        [self setPinCodeCur:nil];
        [self setPinCodeNew:nil];
        [self setToolPIVImporter:nil];
        [self setPivAuthChallenge:nil];
    }

#pragma mark - For CCID interface

    - (void)ccidHelperWillProcess {
        // コマンド実行時のエラーテキストをクリア
        [self setErrorMessageOfCommand:nil];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
            case COMMAND_CCID_PIV_RESET:
            case COMMAND_CCID_PIV_IMPORT_KEY:
            case COMMAND_CCID_PIV_SET_CHUID:
            case COMMAND_CCID_PIV_STATUS:
                // CCIDデバイスに接続
                if ([[self toolCCIDHelper] ccidHelperWillConnect]) {
                    // 機能実行に先立ち、PIVアプレットをSELECT
                    [self doRequestPivInsSelectApplication];
                    return;
                } else {
                    // PIV機能を認識できなかった旨のエラーメッセージを設定
                    [self notifyErrorMessage:MSG_ERROR_PIV_SELECTING_CARD_FAIL];
                }
                break;
            default:
                break;
        }
        [self notifyProcessTerminated:false];
    }

    - (void)ccidHelperDidReceiveResponse:(NSData *)resp status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self commandIns]) {
            case PIV_INS_SELECT_APPLICATION:
                [self doResponsePivInsSelectApplication:resp status:sw];
                break;
            case PIV_INS_AUTHENTICATE:
                [self doResponsePivInsAuthenticate:resp status:sw];
                break;
            case PIV_INS_VERIFY:
                [self doResponsePivInsVerify:resp status:sw];
                break;
            case PIV_INS_CHANGE_REFERENCE:
            case PIV_INS_RESET_RETRY:
                [self doResponsePivInsChangePin:resp status:sw];
                break;
            case YKPIV_INS_RESET:
                [self doResponseYkPivInsReset:resp status:sw];
                break;
            case YKPIV_INS_IMPORT_ASYMM_KEY:
                [self doResponseYkPivInsImportKey:resp status:sw];
                break;
            case PIV_INS_PUT_DATA:
                [self doResponsePivInsPutData:resp status:sw];
                break;
            case PIV_INS_GET_DATA:
                [self doResponsePivInsGetData:resp status:sw];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

#pragma mark - For reset firmware

    - (void)commandWillResetFirmware {
        // HIDインターフェース経由でファームウェアをリセット
        [self notifyProcessStarted];
        [[self toolAppCommand] doCommandFirmwareResetForCommandRef:self];
    }

    - (void)commandDidResetFirmware:(bool)success {
        if (success == false) {
            [self notifyErrorMessage:MSG_FIRMWARE_RESET_UNSUPP];
        }
        [self notifyProcessTerminated:success];
    }

#pragma mark - For PIVPreferenceWindow open/close

    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent {
        // PIV機能設定画面を表示（親画面＝メイン画面）
        [[self pivPreferenceWindow] windowWillOpenWithCommandRef:self parentWindow:parent];
    }

    - (void)commandDidClosePreferenceWindow {
        // メイン画面に制御を戻す
    }

#pragma mark - Public methods

    - (void)commandWillPerformPIVProcess:(Command)command withParameter:(ToolPIVParameter *)parameter {
        // 実行コマンド／パラメーターを保持
        [self setCommand:command];
        // コマンドにより分岐
        switch (command) {
            case COMMAND_HID_FIRMWARE_RESET:
                [self commandWillResetFirmware];
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                [self setToolPIVImporter:[[ToolPIVImporter alloc] init]];
                [[self toolPIVImporter] generateChuidAndCcc];
                [self ccidHelperWillProcess];
                break;
            case COMMAND_CCID_PIV_RESET:
            case COMMAND_CCID_PIV_STATUS:
                [self ccidHelperWillProcess];
                break;
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self setPinCodeNew:[parameter renewalPin]];
                [self setPinCodeCur:[parameter currentPin]];
                [self ccidHelperWillProcess];
                break;
            case COMMAND_CCID_PIV_IMPORT_KEY:
                [self commandwillimportKeyWith:parameter];
                break;
            default:
                // 画面に制御を戻す
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (void)commandwillimportKeyWith:(ToolPIVParameter *)parameter {
        ToolPIVImporter *importer = [[ToolPIVImporter alloc] initForKeySlot:[parameter keySlotId]];
        if ([importer readPrivateKeyPemFrom:[parameter pkeyPemPath]] == false) {
            [self notifyErrorMessage:MSG_PIV_LOAD_PKEY_FAILED];
            [self notifyProcessTerminated:false];
            return;
        }
        if ([importer readCertificatePemFrom:[parameter certPemPath]] == false) {
            [self notifyErrorMessage:MSG_PIV_LOAD_CERT_FAILED];
            [self notifyProcessTerminated:false];
            return;
        }
        // 鍵・証明書のアルゴリズムが異なる場合は、エラーメッセージを表示し処理中止
        if ([importer keyAlgorithm] != [importer certAlgorithm]) {
            NSString *info = [[NSString alloc] initWithFormat:MSG_FORMAT_PIV_PKEY_CERT_ALGORITHM,
                              [importer keyAlgorithm], [importer certAlgorithm]];
            [self notifyErrorMessage:info];
            [self notifyProcessTerminated:false];
            return;
        }
        // コマンド実行
        [self setPinCodeCur:[parameter authPin]];
        [self setToolPIVImporter:importer];
        [self ccidHelperWillProcess];
    }

#pragma mark - Private common methods

    - (void)notifyProcessStarted {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
                [self setNameOfCommand:PROCESS_NAME_CCID_PIV_CHANGE_PIN];
                break;
            case COMMAND_CCID_PIV_CHANGE_PUK:
                [self setNameOfCommand:PROCESS_NAME_CCID_PIV_CHANGE_PUK];
                break;
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self setNameOfCommand:PROCESS_NAME_CCID_PIV_UNBLOCK_PIN];
                break;
            case COMMAND_CCID_PIV_RESET:
                [self setNameOfCommand:PROCESS_NAME_CCID_PIV_RESET];
                break;
            case COMMAND_CCID_PIV_IMPORT_KEY:
                [self setNameOfCommand:PROCESS_NAME_CCID_PIV_IMPORT_KEY];
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                [self setNameOfCommand:PROCESS_NAME_CCID_PIV_SET_CHUID];
                break;
            case COMMAND_CCID_PIV_STATUS:
                [self setNameOfCommand:PROCESS_NAME_CCID_PIV_STATUS];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self setNameOfCommand:PROCESS_NAME_FIRMWARE_RESET];
                break;
            default:
                break;
        }
        // コマンド開始メッセージをログファイルに出力
        NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE, [self nameOfCommand]];
        [[ToolLogFile defaultLogger] info:startMsg];
    }

    - (void)notifyErrorMessage:(NSString *)message {
        // エラーメッセージをログファイルに出力（出力前に改行文字を削除）
        NSString *logMessage = [message stringByReplacingOccurrencesOfString:@"\n" withString:@""];
        [[ToolLogFile defaultLogger] error:logMessage];
        // 戻り先画面に表示させるためのエラーメッセージを保持
        [self setErrorMessageOfCommand:message];
    }

    - (void)notifyProcessTerminated:(bool)success {
        // CCIDデバイスから切断
        [[self toolCCIDHelper] ccidHelperWillDisconnect];
        // コマンド終了メッセージを生成
        NSString *endMsg = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, [self nameOfCommand],
                                success ? MSG_SUCCESS : MSG_FAILURE];
        if (success == false) {
            // コマンド異常終了メッセージをログ出力
            if ([self nameOfCommand]) {
                [[ToolLogFile defaultLogger] error:endMsg];
            }
        } else {
            // コマンド正常終了メッセージをログ出力
            if ([self nameOfCommand]) {
                [[ToolLogFile defaultLogger] info:endMsg];
            }
        }
        // パラメーターを初期化
        Command command = [self command];
        [self clearCommandParameters];
        // 画面に制御を戻す
        [[self pivPreferenceWindow] toolPIVCommandDidProcess:command withResult:success withErrorMessage:[self errorMessageOfCommand]];
    }

#pragma mark - Command functions

    - (void)doRequestPivInsSelectApplication {
        [self setCommandIns:PIV_INS_SELECT_APPLICATION];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x04 p2:0x00 data:[self getPivAidData] le:0xff];
    }

    - (void)doResponsePivInsSelectApplication:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            [self notifyErrorMessage:MSG_ERROR_PIV_APPLET_SELECT_FAILED];
            [self notifyProcessTerminated:false];
            return;
        }
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self doPivInsChangePIN];
                break;
            case COMMAND_CCID_PIV_RESET:
                [self doYkPivInsReset];
                break;
            case COMMAND_CCID_PIV_IMPORT_KEY:
                [self doYkPivImportKey];
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                [self doYkPivSetCHUID];
                break;
            case COMMAND_CCID_PIV_STATUS:
                [self doYkPivStatusProcess];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (void)doRequestPivInsAuthenticate:(NSData *)apdu {
        // コマンドを実行
        [self setCommandIns:PIV_INS_AUTHENTICATE];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:CRYPTO_ALG_3DES p2:PIV_KEY_CARDMGM data:apdu le:0xff];
    }

    - (void)doResponsePivInsAuthenticate:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            if ([self pivAuthChallenge] == nil) {
                [self notifyErrorMessage:MSG_ERROR_PIV_ADMIN_AUTH_REQ_FAILED];
            } else {
                [self notifyErrorMessage:MSG_ERROR_PIV_ADMIN_AUTH_RES_FAILED];
            }
            [self notifyProcessTerminated:false];
            return;
        }
        // PIV管理機能認証レスポンスに対する処理を続行
        [self doResponsePivAdminAuth:response];
    }

    - (void)doRequestPivInsVerify:(NSString *)pinCode {
        // コマンドAPDUを生成
        NSData *apdu = nil;
        if (pinCode != nil) {
            apdu = [self getPivPinVerifyData:pinCode];
        }
        // コマンドを実行
        [self setCommandIns:PIV_INS_VERIFY];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:PIV_KEY_PIN data:apdu le:0xff];
    }

    - (void)doResponsePivInsVerify:(NSData *)response status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_IMPORT_KEY:
                [self doYkPivImportKeyProcessWithResponse:response status:sw];
                break;
            case COMMAND_CCID_PIV_STATUS:
                [self doYkPivStatusProcessWithPinRetryResponse:response status:sw];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (void)doRequestYkPivInsImportKey:(NSData *)apdu algorithm:(uint8_t)alg keySlotId:(uint8_t)slotId {
        // コマンドを実行
        [self setCommandIns:YKPIV_INS_IMPORT_ASYMM_KEY];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:alg p2:slotId data:apdu le:0xff];
    }

    - (void)doResponseYkPivInsImportKey:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            [self setLastErrorMessageWithFormat:MSG_ERROR_PIV_IMPORT_PKEY_FAILED withImporter:[self toolPIVImporter]];
            [self notifyProcessTerminated:false];
            return;
        }
        // 処理成功のログを出力
        [self outputLogWithFormat:MSG_PIV_PKEY_PEM_IMPORTED withImporter:[self toolPIVImporter]];
        // 証明書インポート処理を実行
        [self doYkPivImportCertProcess];
    }

    - (void)doRequestPivInsPutData:(NSData *)apdu {
        // コマンドを実行
        [self setCommandIns:PIV_INS_PUT_DATA];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x3f p2:0xff data:apdu le:0xff];
    }

    - (void)doResponsePivInsPutData:(NSData *)response status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_IMPORT_KEY:
                [self doYkPivImportCertTerminate:response status:sw];
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                [self doYkPivSetCHUIDProcessContinue:response status:sw];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (void)doRequestPivInsGetData:(NSData *)apdu {
        // コマンドを実行
        [self setCommandIns:PIV_INS_GET_DATA];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x3f p2:0xff data:apdu le:0xff];
    }

    - (void)doResponsePivInsGetData:(NSData *)response status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_STATUS:
                [self doResponseYkPivStatusFetchObjects:response status:sw];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (void)setLastErrorMessageWithFormat:(NSString *)format withImporter:(ToolPIVImporter *)importer {
        // インストール先のスロットIDとアルゴリズムを付加してエラーログを生成
        NSString *msg = [[NSString alloc] initWithFormat:format, [importer keySlotId], [importer keyAlgorithm]];
        [self notifyErrorMessage:msg];
    }

    - (void)outputLogWithFormat:(NSString *)format withImporter:(ToolPIVImporter *)importer {
        // インストール先のスロットIDとアルゴリズムを付加してログ出力
        [[ToolLogFile defaultLogger] infoWithFormat:format, [importer keySlotId], [importer keyAlgorithm]];
    }

#pragma mark - PIV administrative authentication

    - (void)doRequestPivAdminAuth {
        // コマンドAPDUを生成
        NSData *apdu = [self getPivAdminAuthRequestData];
        // コマンドを実行
        [self setPivAuthChallenge:nil];
        [self doRequestPivInsAuthenticate:apdu];
    }

    - (void)doRequestPivAdminAuthSecond:(NSData *)insAuthResp {
        // PIV管理機能認証（往路）のレスポンスから、暗号化された受信チャレンジを抽出（５バイト目から８バイト分）
        NSData *encrypted = [insAuthResp subdataWithRange:NSMakeRange(4, DES_LEN_DES)];
        // PIV管理パスワードを使用し、受信チャレンジを復号化
        NSData *witness = [self decryptPivAdminAuthWitness:encrypted];
        if (witness == nil) {
            [self setLastErrorMessageWithFuncError:MSG_ERROR_PIV_ADMIN_AUTH_FUNC_FAILED];
            [self notifyProcessTerminated:false];
            return;
        }
        // 8バイトのランダムベクターを送信チャレンジに設定
        [self setPivAuthChallenge:[self generateRandom:DES_LEN_DES]];
        // コマンドAPDUを生成
        NSData *apdu = [self getPivAdminAuthResponseData:witness withChallenge:[self pivAuthChallenge]];
        // コマンドを実行
        [self doRequestPivInsAuthenticate:apdu];
    }

    - (void)doResponsePivAdminAuth:(NSData *)insAuthResp {
        if ([self pivAuthChallenge] == nil) {
            // PIV管理機能認証（復路）を実行
            [self doRequestPivAdminAuthSecond:insAuthResp];
            return;
        }
        // 送受信チャレンジの内容一致チェック
        if ([self verifyPivAuthAdminChallenge:insAuthResp] == false) {
            [self notifyProcessTerminated:false];
            return;
        }
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_IMPORT_KEY:
                // PIN番号認証を実行
                [self doRequestPivInsVerify:[self pinCodeCur]];
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                // CHUID／CCC設定処理を実行
                [self doYkPivSetCHUIDProcess];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (bool)verifyPivAuthAdminChallenge:(NSData *)insAuthResp {
        // PIV管理機能認証（復路）のレスポンスから、暗号化された受信チャレンジを抽出（５バイト目から８バイト分）
        NSData *encrypted = [insAuthResp subdataWithRange:NSMakeRange(4, DES_LEN_DES)];
        // PIV管理パスワードを使用し、受信チャレンジを復号化
        NSData *challenge = [self decryptPivAdminAuthWitness:encrypted];
        if (challenge == nil) {
            [self setLastErrorMessageWithFuncError:MSG_ERROR_PIV_ADMIN_AUTH_FUNC_FAILED];
            return false;
        }
        if ([challenge isEqualToData:[self pivAuthChallenge]] == false) {
            // 送信チャレンジと受信チャレンジの内容が異なる場合はPIV管理認証失敗
            [self notifyErrorMessage:MSG_ERROR_PIV_ADMIN_AUTH_CHALLENGE_DIFF];
            return false;
        }
        return true;
    }

#pragma mark - Key and certificate management functions

    - (void)doYkPivImportKey {
        // 処理開始メッセージをログ出力
        [self notifyProcessStarted];
        // PIV管理機能認証（往路）を実行
        [self doRequestPivAdminAuth];
    }

    - (void)doYkPivImportKeyProcessWithResponse:(NSData *)response status:(uint16_t)sw {
        // ステータスワードをチェックし、PIN認証の成否を判定
        if ([self checkPivInsReplyUsingPinOrPukWithStatus:sw isPinAuth:true] == false) {
            // PIN認証が失敗した場合は処理終了
            [self notifyProcessTerminated:false];
            return;
        }
        // 秘密鍵インポート処理を実行
        NSData *apdu = [[self toolPIVImporter] getPrivateKeyAPDUData];
        [self doRequestYkPivInsImportKey:apdu
                               algorithm:[[self toolPIVImporter] keyAlgorithm]
                               keySlotId:[[self toolPIVImporter] keySlotId]];
    }

    - (void)doYkPivImportCertProcess {
        // 証明書インポート処理を実行
        NSData *apdu = [[self toolPIVImporter] getCertificateAPDUData];
        [self doRequestPivInsPutData:apdu];
    }

    - (void)doYkPivImportCertTerminate:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            [self setLastErrorMessageWithFormat:MSG_ERROR_PIV_IMPORT_CERT_FAILED withImporter:[self toolPIVImporter]];
            [self notifyProcessTerminated:false];
            return;
        }
        // 処理成功のログを出力
        [self outputLogWithFormat:MSG_PIV_CERT_PEM_IMPORTED withImporter:[self toolPIVImporter]];
        // 制御を戻す
        [self notifyProcessTerminated:true];
    }

#pragma mark - CHUID and CCC management functions

    - (void)doYkPivSetCHUID {
        // 処理開始メッセージをログ出力
        [self notifyProcessStarted];
        // PIV管理機能認証（往路）を実行
        [self doRequestPivAdminAuth];
    }

    - (void)doYkPivSetCHUIDProcess {
        // フラグをクリア
        [self setCccImportProcessing:false];
        // CHUIDインポート処理を実行
        NSData *apdu = [[self toolPIVImporter] getChuidAPDUData];
        [self doRequestPivInsPutData:apdu];
    }

    - (void)doYkPivSetCHUIDProcessContinue:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            // 処理失敗ログを出力し、制御を戻す
            [self outputYkPivSetCHUIDProcessLog:false isCccImportProcessing:[self cccImportProcessing]];
            [self notifyProcessTerminated:false];
            return;
        }
        // 処理成功ログを出力
        [self outputYkPivSetCHUIDProcessLog:true isCccImportProcessing:[self cccImportProcessing]];
        // CCCインポート処理実行結果の場合
        if ([self cccImportProcessing]) {
            // フラグをクリア
            [self setCccImportProcessing:false];
            // 制御を戻す
            [self notifyProcessTerminated:true];
        } else {
            // フラグを設定
            [self setCccImportProcessing:true];
            // CCCインポート処理を実行
            NSData *apdu = [[self toolPIVImporter] getCccAPDUData];
            [self doRequestPivInsPutData:apdu];
        }
    }

    - (void)outputYkPivSetCHUIDProcessLog:(bool)success isCccImportProcessing:(bool)ccc {
        // 処理の成否ログを出力
        if (success) {
            [[ToolLogFile defaultLogger] info:(ccc ? MSG_PIV_CCC_IMPORTED : MSG_PIV_CHUID_IMPORTED)];
        } else {
            [[ToolLogFile defaultLogger] error:(ccc ? MSG_ERROR_PIV_IMPORT_CCC_FAILED : MSG_ERROR_PIV_IMPORT_CHUID_FAILED)];
        }
    }

#pragma mark - PIV setting reference functions

    - (void)doYkPivStatusProcess {
        // 処理開始メッセージをログ出力
        [self notifyProcessStarted];
        // PINリトライカウンターを照会
        [self doRequestPivInsVerify:nil];
    }

    - (void)doYkPivStatusProcessWithPinRetryResponse:(NSData *)response status:(uint16_t)sw {
        if ((sw >> 8) == 0x63) {
            // PIV設定情報クラスを生成
            [self setToolPIVSetting:[[ToolPIVSetting alloc] initWithSlotName:[[self toolCCIDHelper] getConnectingSlotName]]];
            // PINリトライカウンターを取得
            uint8_t retries = sw & 0x0f;
            [[ToolLogFile defaultLogger] infoWithFormat:MSG_PIV_PIN_RETRY_CNT_GET, retries];
            [[self toolPIVSetting] setRetryCount:retries];
            // PIVオブジェクトを取得
            [self doYkPivStatusFetchObjects:PIV_OBJ_CHUID];

        } else {
            // 不明エラーが発生時は処理失敗ログを出力し、制御を戻す
            [[ToolLogFile defaultLogger] error:MSG_ERROR_PIV_PIN_RETRY_CNT_GET_FAILED];
            [self notifyProcessTerminated:false];
        }
    }

    - (void)doYkPivStatusFetchObjects:(unsigned int)objectId {
        // 取得対象のオブジェクトIDを退避
        [self setObjectIdToFetch:objectId];
        // オブジェクト取得処理を実行
        NSData *apdu = [self getPivInsGetApdu:objectId];
        [self doRequestPivInsGetData:apdu];
    }

    - (void)doResponseYkPivStatusFetchObjects:(NSData *)response status:(uint16_t)sw {
        if (sw != SW_SUCCESS) {
            // 処理失敗ログを出力（エラーではなく警告扱いとする）
            [[ToolLogFile defaultLogger] warnWithFormat:MSG_ERROR_PIV_DATA_OBJECT_GET_FAILED, [self objectIdToFetch]];
            // ブランクデータをPIV設定情報クラスに設定
            [[self toolPIVSetting] setDataObject:[[NSData alloc] init] forObjectId:[self objectIdToFetch]];
        } else {
            // 処理成功ログを出力
            [[ToolLogFile defaultLogger] infoWithFormat:MSG_PIV_DATA_OBJECT_GET, [self objectIdToFetch]];
            // 取得したデータをPIV設定情報クラスに設定
            [[self toolPIVSetting] setDataObject:response forObjectId:[self objectIdToFetch]];
        }
        // オブジェクトIDに応じて後続処理分岐
        switch ([self objectIdToFetch]) {
            case PIV_OBJ_CHUID:
                [self doYkPivStatusFetchObjects:PIV_OBJ_CAPABILITY];
                break;
            case PIV_OBJ_CAPABILITY:
                [self doYkPivStatusFetchObjects:PIV_OBJ_AUTHENTICATION];
                break;
            case PIV_OBJ_AUTHENTICATION:
                [self doYkPivStatusFetchObjects:PIV_OBJ_SIGNATURE];
                break;
            case PIV_OBJ_SIGNATURE:
                [self doYkPivStatusFetchObjects:PIV_OBJ_KEY_MANAGEMENT];
                break;
            case PIV_OBJ_KEY_MANAGEMENT:
                [self notifyProcessTerminated:true];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (NSString *)getPIVSettingDescriptionString {
        return [[self toolPIVSetting] getDescriptionString];
    }

#pragma mark - PIN management functions

    - (void)doPivInsChangePIN {
        // INS、P2を設定
        uint8_t ins, p2;
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
                ins = PIV_INS_CHANGE_REFERENCE;
                p2 = PIV_KEY_PIN;
                break;
            case COMMAND_CCID_PIV_CHANGE_PUK:
                ins = PIV_INS_CHANGE_REFERENCE;
                p2 = PIV_KEY_PUK;
                break;
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                ins = PIV_INS_RESET_RETRY;
                p2 = PIV_KEY_PIN;
                break;
            default:
                [self notifyProcessTerminated:false];
                return;
        }
        // 処理開始メッセージをログ出力
        [self notifyProcessStarted];
        // コマンドAPDUを生成
        NSData *apdu = [self getPivChangePinData:[self pinCodeCur] withPinCodeNew:[self pinCodeNew]];
        // コマンドを実行
        [self setCommandIns:ins];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:p2 data:apdu le:0xff];
    }

    - (bool)checkPivInsReplyUsingPinOrPukWithStatus:(uint16_t)sw isPinAuth:(bool)isPinAuth {
        // ステータスワードをチェックし、エラーの種類を判定
        uint8_t retries = 3;
        bool isPinBlocked = false;
        if ((sw >> 8) == 0x63) {
            // リトライカウンターが戻された場合（入力PIN／PUKが不正時）
            retries = sw & 0xf;
            if (retries < 1) {
                isPinBlocked = true;
            }

        } else if (sw == SW_ERR_AUTH_BLOCKED) {
            // 入力PIN／PUKがすでにブロックされている場合
            isPinBlocked = true;

        } else if (sw != SW_SUCCESS) {
            // 不明なエラーが発生時
            NSString *msg = [NSString stringWithFormat:MSG_ERROR_PIV_UNKNOWN, sw];
            [self notifyErrorMessage:msg];
        }
        // PINブロック or リトライカウンターの状態に応じメッセージを編集
        if (isPinBlocked) {
            [self notifyErrorMessage:isPinAuth ? MSG_ERROR_PIV_PIN_LOCKED : MSG_ERROR_PIV_PUK_LOCKED];

        } else if (retries < 3) {
            NSString *name = isPinAuth ? @"PIN" : @"PUK";
            NSString *msg = [[NSString alloc] initWithFormat:MSG_ERROR_PIV_WRONG_PIN, name, name, retries];
            [self notifyErrorMessage:msg];
        }
        return (sw == SW_SUCCESS);
    }

    - (void)doResponsePivInsChangePin:(NSData *)response status:(uint16_t)sw {
        // ステータスワードをチェックし、PIN認証の成否を判定
        bool isPinAuth = ([self command] == COMMAND_CCID_PIV_CHANGE_PIN);
        bool ret = [self checkPivInsReplyUsingPinOrPukWithStatus:sw isPinAuth:isPinAuth];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self notifyProcessTerminated:ret];
                break;
            default:
                [self notifyProcessTerminated:false];
                break;
        }
    }

    - (void)doYkPivInsReset {
        // 処理開始メッセージをログ出力
        [self notifyProcessStarted];
        // コマンドを実行
        [self setCommandIns:YKPIV_INS_RESET];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:0x00 data:nil le:0xff];
    }

    - (void)doResponseYkPivInsReset:(NSData *)response status:(uint16_t)sw {
        // ステータスワードの内容に応じメッセージを編集
        if (sw == SW_SEC_STATUS_NOT_SATISFIED) {
            // PIN／PUKがまだブロックされていない場合
            [self notifyErrorMessage:MSG_ERROR_PIV_RESET_FAIL];

        } else if (sw != SW_SUCCESS) {
            // 不明なエラーが発生時
            NSString *msg = [NSString stringWithFormat:MSG_ERROR_PIV_UNKNOWN, sw];
            [self notifyErrorMessage:msg];
        }
        [self notifyProcessTerminated:(sw == SW_SUCCESS)];
    }

#pragma mark - Utility functions

    - (NSData *)getPivAidData {
        static uint8_t piv_aid[] = {0xa0, 0x00, 0x00, 0x03, 0x08};
        return [NSData dataWithBytes:piv_aid length:sizeof(piv_aid)];
    }

    - (NSData *)getPivAdminAuthRequestData {
        // PIV管理機能認証（往路）のリクエストデータを生成
        static uint8_t apdu[] = {TAG_DYNAMIC_AUTH_TEMPLATE, 2, TAG_AUTH_WITNESS, 0};
        return [NSData dataWithBytes:apdu length:sizeof(apdu)];
    }

    - (NSData *)getPivAdminAuthResponseData:(NSData *)witness withChallenge:(NSData *)challenge {
        // 引数のチャレンジをバイト変換
        uint8_t *w = (uint8_t *)[witness bytes];
        size_t w_size = [witness length];
        uint8_t *c = (uint8_t *)[challenge bytes];
        size_t c_size = [challenge length];
        // PIV管理機能認証（復路）のリクエストデータを生成
        uint8_t apdu[22];
        uint8_t offset = 0;
        apdu[offset++] = TAG_DYNAMIC_AUTH_TEMPLATE;
        apdu[offset++] = 20;
        // copy witness
        apdu[offset++] = TAG_AUTH_WITNESS;
        apdu[offset++] = w_size;
        memcpy(apdu + offset, w, w_size);
        // copy challenge
        offset += w_size;
        apdu[offset++] = TAG_AUTH_CHALLENGE;
        apdu[offset++] = c_size;
        memcpy(apdu + offset, c, c_size);
        return [NSData dataWithBytes:apdu length:sizeof(apdu)];
    }

    - (NSData *)decryptPivAdminAuthWitness:(NSData *)encryptedWitness {
        // デフォルトのPIV管理パスワードを取得
        unsigned char *pw = tool_piv_admin_des_default_key();
        if (tool_crypto_des_import_key(pw, DES_LEN_3DES) == false) {
            return nil;
        }
        // PIV管理パスワードを使用し、チャレンジを復号化
        const unsigned char *encrypted = [encryptedWitness bytes];
        unsigned char decrypted[DES_LEN_DES];
        size_t decryptedSize = DES_LEN_DES;
        if (tool_crypto_des_decrypt(encrypted, DES_LEN_DES, decrypted, &decryptedSize) == false) {
            return nil;
        }
        // 復号化されたチャレンジを戻す
        NSData *challenge = [[NSData alloc] initWithBytes:decrypted length:decryptedSize];
        return challenge;
    }

    - (NSData *)generateRandom:(size_t)size {
        uint8_t u[size];
        for (size_t i = 0; i < size; i++) {
            uint32_t r = arc4random_uniform(0xff);
            u[i] = (uint8_t)r;
        }
        return [[NSData alloc] initWithBytes:u length:size];
    }

    - (NSData *)getPivPinVerifyData:(NSString *)pinCode {
        // PINコードを配列にセット
        uint8_t pin_code[8];
        memset(pin_code, 0xff, sizeof(pin_code));
        if (pinCode != nil) {
            uint8_t *c = (uint8_t *)[pinCode UTF8String];
            size_t s = [pinCode length];
            memcpy(pin_code, c, s);
        }
        // NSData形式に変換
        return [NSData dataWithBytes:pin_code length:sizeof(pin_code)];
    }

    - (NSData *)getPivChangePinData:(NSString *)pinCodeCur withPinCodeNew:(NSString *)pinCodeNew {
        // 認証用PINコード、更新用PINコードの順で配列にセット
        uint8_t pin_code[16];
        memset(pin_code, 0xff, sizeof(pin_code));
        if (pinCodeCur != nil) {
            uint8_t *c = (uint8_t *)[pinCodeCur UTF8String];
            size_t s = [pinCodeCur length];
            memcpy(pin_code, c, s);
        }
        if (pinCodeNew != nil) {
            uint8_t *c = (uint8_t *)[pinCodeNew UTF8String];
            size_t s = [pinCodeNew length];
            memcpy(pin_code + 8, c, s);
        }
        // NSData形式に変換（16バイト固定長）
        return [NSData dataWithBytes:pin_code length:sizeof(pin_code)];
    }

    - (NSData *)getPivInsGetApdu:(unsigned int)objectID {
        unsigned char apdu[5];
        size_t size = tool_piv_admin_set_object_header(objectID, apdu);
        // NSData形式に変換
        return [NSData dataWithBytes:apdu length:size];
    }

    - (void)setLastErrorMessageWithFuncError:(NSString *)errorMsgTemplate {
        NSString *functionMsg = [[NSString alloc] initWithUTF8String:log_debug_message()];
        NSString *errorMsg = [[NSString alloc] initWithFormat:errorMsgTemplate, functionMsg];
        [self notifyErrorMessage:errorMsg];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolAppCommand] checkUSBHIDConnection];
    }

@end
