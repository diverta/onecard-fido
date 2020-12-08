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

#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPIVCommand.h"
#import "ToolPIVCommon.h"
#import "ToolPIVImporter.h"

@interface ToolPIVCommand () <ToolCCIDHelperDelegate>

    // CCIDインターフェース処理の参照を保持
    @property (nonatomic) ToolCCIDHelper    *toolCCIDHelper;
    // 処理機能名称を保持
    @property (nonatomic) NSString          *processNameOfCommand;
    // コマンドを保持
    @property (nonatomic) Command            command;
    @property (nonatomic) uint8_t            commandIns;
    // コマンドのパラメーターを保持
    @property (nonatomic) NSString          *pinCodeCur;
    @property (nonatomic) NSString          *pinCodeNew;
    @property (nonatomic) ToolPIVImporter   *toolPIVImporter;
    // PIV管理機能認証（往路）のチャレンジを保持
    @property (nonatomic) NSData            *pivAuthChallenge;
    // エラーメッセージテキストを保持
    @property (nonatomic) NSString          *lastErrorMessage;

@end

@implementation ToolPIVCommand

    - (id)init {
        self = [super init];
        if (self) {
            // ToolCCIDHelperのインスタンスを生成
            [self setToolCCIDHelper:[[ToolCCIDHelper alloc] initWithDelegate:self]];
            [self clearCommandParameters];
        }
        return self;
    }

    - (void)clearCommandParameters {
        // コマンドおよびパラメーターを初期化
        [self setProcessNameOfCommand:nil];
        [self setCommand:COMMAND_NONE];
        [self setCommandIns:0x00];
        [self setPinCodeCur:nil];
        [self setPinCodeNew:nil];
        [self setToolPIVImporter:nil];
        [self setPivAuthChallenge:nil];
        [self setLastErrorMessage:nil];
    }

    - (void)ccidHelperWillProcess:(Command)command {
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
            case COMMAND_CCID_PIV_RESET:
            case COMMAND_CCID_PIV_IMPORT_KEY:
            case COMMAND_CCID_PIV_SET_CHUID:
                // 機能実行に先立ち、PIVアプレットをSELECT
                [self doRequestPivInsSelectApplication];
                break;
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)ccidHelperDidProcess:(bool)success response:(NSData *)resp status:(uint16_t)sw {
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
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

#pragma mark - Public methods

    - (void)commandWillChangePin:(Command)command withNewPinCode:(NSString *)pinCodeNew withAuthPinCode:(NSString *)pinCodeCur {
        [self setPinCodeNew:pinCodeNew];
        [self setPinCodeCur:pinCodeCur];
        [self ccidHelperWillProcess:command];
    }

    - (void)commandWillReset:(Command)command {
        [self ccidHelperWillProcess:command];
    }

    - (void)commandWillImportKey:(Command)command withAuthPinCode:(NSString *)pinCodeCur withImporter:(ToolPIVImporter *)importer {
        [self setPinCodeCur:pinCodeCur];
        [self setToolPIVImporter:importer];
        [self ccidHelperWillProcess:command];
    }

    - (void)commandWillSetCHUIDAndCCC:(Command)command withImporter:(ToolPIVImporter *)importer {
        [self setToolPIVImporter:importer];
        [self ccidHelperWillProcess:command];
    }

#pragma mark - Command functions

    - (void)doRequestPivInsSelectApplication {
        [self setCommandIns:PIV_INS_SELECT_APPLICATION];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession:self ins:[self commandIns] p1:0x04 p2:0x00 data:[self getPivAidData] le:0xff];
    }

    - (void)doResponsePivInsSelectApplication:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            [self setLastErrorMessage:MSG_ERROR_PIV_APPLET_SELECT_FAILED];
            [self exitCommandProcess:false];
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
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)doRequestPivInsAuthenticate:(NSData *)apdu {
        // コマンドを実行
        [self setCommandIns:PIV_INS_AUTHENTICATE];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession:self ins:[self commandIns] p1:CRYPTO_ALG_3DES p2:PIV_KEY_CARDMGM data:apdu le:0xff];
    }

    - (void)doResponsePivInsAuthenticate:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            if ([self pivAuthChallenge] == nil) {
                [self setLastErrorMessage:MSG_ERROR_PIV_ADMIN_AUTH_REQ_FAILED];
            } else {
                [self setLastErrorMessage:MSG_ERROR_PIV_ADMIN_AUTH_RES_FAILED];
            }
            [self exitCommandProcess:false];
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
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession:self ins:[self commandIns] p1:0x00 p2:PIV_KEY_PIN data:apdu le:0xff];
    }

    - (void)doResponsePivInsVerify:(NSData *)response status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_IMPORT_KEY:
                [self doYkPivImportKeyProcess];
                break;
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)doRequestYkPivInsImportKey:(NSData *)apdu algorithm:(uint8_t)alg keySlotId:(uint8_t)slotId {
        // コマンドを実行
        [self setCommandIns:YKPIV_INS_IMPORT_ASYMM_KEY];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession:self ins:[self commandIns] p1:alg p2:slotId data:apdu le:0xff];
    }

    - (void)doResponseYkPivInsImportKey:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            [self setLastErrorMessageWithFormat:MSG_ERROR_PIV_IMPORT_PKEY_FAILED withImporter:[self toolPIVImporter]];
            [self exitCommandProcess:false];
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
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession:self ins:[self commandIns] p1:0x3f p2:0xff data:apdu le:0xff];
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
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)setLastErrorMessageWithFormat:(NSString *)format withImporter:(ToolPIVImporter *)importer {
        // インストール先のスロットIDとアルゴリズムを付加してエラーログを生成
        NSString *msg = [[NSString alloc] initWithFormat:format, [importer keySlotId], [importer algorithm]];
        [self setLastErrorMessage:msg];
    }

    - (void)outputLogWithFormat:(NSString *)format withImporter:(ToolPIVImporter *)importer {
        // インストール先のスロットIDとアルゴリズムを付加してログ出力
        [[ToolLogFile defaultLogger] infoWithFormat:format, [importer keySlotId], [importer algorithm]];
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
            [self exitCommandProcess:false];
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
            [self exitCommandProcess:false];
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
                [self exitCommandProcess:false];
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
            [self setLastErrorMessage:MSG_ERROR_PIV_ADMIN_AUTH_CHALLENGE_DIFF];
            return false;
        }
        return true;
    }

#pragma mark - Key and certificate management functions

    - (void)doYkPivImportKey {
        // 処理開始メッセージをログ出力
        [self startCommandProcess];
        // PIV管理機能認証（往路）を実行
        [self doRequestPivAdminAuth];
    }

    - (void)doYkPivImportKeyProcess {
        // 秘密鍵インポート処理を実行
        NSData *apdu = [[self toolPIVImporter] getPrivateKeyAPDUData];
        [self doRequestYkPivInsImportKey:apdu
                               algorithm:[[self toolPIVImporter] algorithm]
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
            [self exitCommandProcess:false];
            return;
        }
        // 処理成功のログを出力
        [self outputLogWithFormat:MSG_PIV_CERT_PEM_IMPORTED withImporter:[self toolPIVImporter]];
        // 制御を戻す
        [self exitCommandProcess:true];
    }

#pragma mark - CHUID and CCC management functions

    - (void)doYkPivSetCHUID {
        // 処理開始メッセージをログ出力
        [self startCommandProcess];
        // PIV管理機能認証（往路）を実行
        [self doRequestPivAdminAuth];
    }

    - (void)doYkPivSetCHUIDProcess {
        // TODO: CHUID設定処理を実行
        // 仮の実装です。
        [self exitCommandProcess:true];
    }

    - (void)doYkPivSetCHUIDProcessContinue:(NSData *)response status:(uint16_t)sw {
        // TODO:エラーハンドリング／制御戻し
        // 仮の実装です。
        [self exitCommandProcess:true];
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
                [self exitCommandProcess:false];
                return;
        }
        // 処理開始メッセージをログ出力
        [self startCommandProcess];
        // コマンドAPDUを生成
        NSData *apdu = [self getPivChangePinData:[self pinCodeCur] withPinCodeNew:[self pinCodeNew]];
        // コマンドを実行
        [self setCommandIns:ins];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession:self ins:[self commandIns] p1:0x00 p2:p2 data:apdu le:0xff];
    }

    - (void)doResponsePivInsChangePin:(NSData *)response status:(uint16_t)sw {
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
            [self setLastErrorMessage:MSG_ERROR_PIV_UNKNOWN];
        }
        // PINブロック or リトライカウンターの状態に応じメッセージを編集
        bool isPinAuth = ([self command] == COMMAND_CCID_PIV_CHANGE_PIN);
        if (isPinBlocked) {
            [self setLastErrorMessage:isPinAuth ? MSG_ERROR_PIV_PIN_LOCKED : MSG_ERROR_PIV_PUK_LOCKED];

        } else if (retries < 3) {
            NSString *name = isPinAuth ? @"PIN" : @"PUK";
            NSString *msg = [[NSString alloc] initWithFormat:MSG_ERROR_PIV_WRONG_PIN, name, name, retries];
            [self setLastErrorMessage:msg];
        }
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self exitCommandProcess:(sw == SW_SUCCESS)];
                break;
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)doYkPivInsReset {
        // 処理開始メッセージをログ出力
        [self startCommandProcess];
        // コマンドを実行
        [self setCommandIns:YKPIV_INS_RESET];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession:self ins:[self commandIns] p1:0x00 p2:0x00 data:nil le:0xff];
    }

    - (void)doResponseYkPivInsReset:(NSData *)response status:(uint16_t)sw {
        // ステータスワードの内容に応じメッセージを編集
        if (sw == SW_SEC_STATUS_NOT_SATISFIED) {
            // PIN／PUKがまだブロックされていない場合
            [self setLastErrorMessage:MSG_ERROR_PIV_RESET_FAIL];

        } else if (sw != SW_SUCCESS) {
            // 不明なエラーが発生時
            [self setLastErrorMessage:MSG_ERROR_PIV_UNKNOWN];
        }
        [self exitCommandProcess:(sw == SW_SUCCESS)];
    }

#pragma mark - Exit function

    - (void)startCommandProcess {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
                [self setProcessNameOfCommand:PROCESS_NAME_CCID_PIV_CHANGE_PIN];
                break;
            case COMMAND_CCID_PIV_CHANGE_PUK:
                [self setProcessNameOfCommand:PROCESS_NAME_CCID_PIV_CHANGE_PUK];
                break;
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self setProcessNameOfCommand:PROCESS_NAME_CCID_PIV_UNBLOCK_PIN];
                break;
            case COMMAND_CCID_PIV_RESET:
                [self setProcessNameOfCommand:PROCESS_NAME_CCID_PIV_RESET];
                break;
            case COMMAND_CCID_PIV_IMPORT_KEY:
                [self setProcessNameOfCommand:PROCESS_NAME_CCID_PIV_IMPORT_KEY];
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                [self setProcessNameOfCommand:PROCESS_NAME_CCID_PIV_SET_CHUID];
                break;
            default:
                break;
        }
        // コマンド開始メッセージをログファイルに出力
        NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE, [self processNameOfCommand]];
        [[ToolLogFile defaultLogger] info:startMsg];
    }

    - (void)exitCommandProcess:(bool)success {
        // コマンド終了メッセージを生成
        NSString *endMsg = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, [self processNameOfCommand],
                                success ? MSG_SUCCESS : MSG_FAILURE];
        if (success == false) {
            // 処理失敗時はエラーメッセージをログ出力
            if ([self lastErrorMessage]) {
                [[ToolLogFile defaultLogger] error:[self lastErrorMessage]];
            }
            // コマンド異常終了メッセージをログ出力
            if ([self processNameOfCommand]) {
                [[ToolLogFile defaultLogger] error:endMsg];
            }
        } else {
            // コマンド正常終了メッセージをログ出力
            if ([self processNameOfCommand]) {
                [[ToolLogFile defaultLogger] info:endMsg];
            }
        }
        // パラメーターを初期化
        [self clearCommandParameters];
        // TODO: 画面に制御を戻す
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

    - (void)setLastErrorMessageWithFuncError:(NSString *)errorMsgTemplate {
        NSString *functionMsg = [[NSString alloc] initWithUTF8String:log_debug_message()];
        NSString *errorMsg = [[NSString alloc] initWithFormat:errorMsgTemplate, functionMsg];
        [self setLastErrorMessage:errorMsg];
    }

@end
