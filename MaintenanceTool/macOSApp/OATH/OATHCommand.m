//
//  OATHCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//
#import "base32_util.h"
#import "oath_util.h"

#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "QRCodeUtil.h"
#import "ToolCCIDHelper.h"
#import "ToolLogFile.h"

// コマンドクラスのインスタンスを保持
static OATHCommand *sharedInstance;

@implementation OATHCommandParameter

    - (NSString *)oathAccount {
        return [NSString stringWithFormat:@"%@:%@", [self oathAccountIssuer], [self oathAccountName]];
    }

@end

@interface OATHCommand () <ToolCCIDHelperDelegate>

    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolCCIDHelper       *toolCCIDHelper;
    // コマンド完了後に継続される処理を保持
    @property (nonatomic) id                    targetForContinue;
    @property (nonatomic) SEL                   selectorForContinue;
    // CCID I/Fにリクエストしたコマンドバイトを保持
    @property (nonatomic) uint8_t               commandIns;

@end

@implementation OATHCommand

#pragma mark - Methods for singleton

    + (OATHCommand *)instance {
        // このクラスのインスタンス化を１度だけ行う
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [[self alloc] init];
        });
        // インスタンスの参照を戻す
        return sharedInstance;
    }

    + (id)allocWithZone:(NSZone *)zone {
        // このクラスのインスタンス化を１度だけ行う
        __block id ret = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [super allocWithZone:zone];
            ret = sharedInstance;
        });
        
        // インスタンスの参照を戻す（２回目以降の呼び出しではnilが戻る）
        return ret;
    }

    - (id)copyWithZone:(NSZone *)zone{
        return self;
    }

#pragma mark - Methods of this instance

    - (id)init {
        self = [super init];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setToolCCIDHelper:[[ToolCCIDHelper alloc] initWithDelegate:self]];
            [self setParameter:[[OATHCommandParameter alloc] init]];
        }
        return self;
    }

    - (bool)isUSBCCIDCanConnect {
        // USB CCIDインターフェースに接続可能でない場合は false
        return [[self toolCCIDHelper] checkHelperCanConnect];
    }

    - (void)ccidHelperDidReceiveResponse:(NSData *)resp status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self commandIns]) {
            case 0x01:
                [self doResponseAccountAdd:resp status:sw];
                break;
            default:
                [self doResponseInsSelectApplication:resp status:sw];
                break;
        }
    }

#pragma mark - Scanning QR code

    - (bool)scanQRCode {
        // QRコードのスキャンを実行
        NSString *message = [QRCodeUtil scanQRCodeFromScreenShot];
        if (message == nil) {
            [[self parameter] setResultInformativeMessage:MSG_ERROR_OATH_QRCODE_SCAN_FAILED];
            return false;
        }
        // スキャンしたアカウント情報の項目有無をチェック
        QRCodeUtil *qrCodeUtil = [[QRCodeUtil alloc] initWithQRMessageString:message];
        if ([self checkScannedAccountInfo:qrCodeUtil] == false) {
            [[self parameter] setResultInformativeMessage:MSG_ERROR_OATH_SCANNED_ACCOUNT_INFO_INVALID];
            return false;
        }
        // アカウント情報の各項目をパラメーターに設定
        [[self parameter] setOathAccountName:[qrCodeUtil valueForKey:@"account"]];
        [[self parameter] setOathAccountIssuer:[qrCodeUtil valueForKey:@"issuer"]];
        [[self parameter] setOathBase32Secret:[qrCodeUtil valueForKey:@"secret"]];
        // 処理正常終了
        [[ToolLogFile defaultLogger] debug:@"Scan account info from QR code success"];
        return true;
    }

    - (bool)checkScannedAccountInfo:(QRCodeUtil *)qrCodeUtil {
        // スキャンしたアカウント情報の項目有無をチェック
        if ([self checkScannedAccountInfoItem:qrCodeUtil forKey:@"protocol"] == false) {
            return false;
        }
        if ([self checkScannedAccountInfoItem:qrCodeUtil forKey:@"method"] == false) {
            return false;
        }
        if ([self checkScannedAccountInfoItem:qrCodeUtil forKey:@"account"] == false) {
            return false;
        }
        if ([self checkScannedAccountInfoItem:qrCodeUtil forKey:@"issuer"] == false) {
            return false;
        }
        if ([self checkScannedAccountInfoItem:qrCodeUtil forKey:@"secret"] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkScannedAccountInfoItem:(QRCodeUtil *)qrCodeUtil forKey:(NSString *)key {
        if ([qrCodeUtil valueForKey:key] == nil) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Scanned account info invalid: %@ not exist", key];
            return false;
        }
        return true;
    }

#pragma mark - Public methods

    - (void)commandWillPerformForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];

        // 処理開始を通知
        [self notifyProcessStarted];

        // CCIDインタフェース経由で認証器に接続
        if ([[self toolCCIDHelper] ccidHelperWillConnect] == false) {
            // OATH機能を認識できなかった旨のエラーメッセージを設定し、上位クラスに制御を戻す
            [self notifyProcessTerminated:false withInformative:MSG_ERROR_OATH_APPLET_SELECT_FAILED];
            return;
        }
        // 機能実行に先立ち、アプレットをSELECT
        [self doRequestInsSelectApplication];
    }

#pragma mark - Private methods

    - (void)doRequestInsSelectApplication {
        // アプレットIDを生成
        static uint8_t aid[] = {0xa0, 0x00, 0x00, 0x05, 0x27, 0x21, 0x01};
        NSData *oathAidBytes = [NSData dataWithBytes:aid length:sizeof(aid)];
        // コマンドを実行
        [self setCommandIns:0xa4];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x04 p2:0x00 data:oathAidBytes le:0xff];
    }

    - (void)doResponseInsSelectApplication:(NSData *)responseData status:(uint16_t)responseSW {
        // 不明なエラーが発生時は以降の処理を行わない
        if (responseSW != 0x9000) {
            NSString *message = [NSString stringWithFormat:MSG_OCCUR_UNKNOWN_ERROR_SW, responseSW];
            [self notifyProcessTerminated:false withInformative:message];
            return;
        }
        // アカウント登録処理-->ワンタイムパスワード生成処理を一息に実行
        if ([[[self parameter] commandTitle] isEqualToString:MSG_LABEL_COMMAND_OATH_GENERATE_TOTP]) {
            [self doRequestAccountAdd];
            return;
        }
    }

#pragma mark - Account functions

    - (void)doRequestAccountAdd {
        // アカウント登録処理用APDUを生成
        NSData *apduBytes = [self GenerateAccountAddAPDU];
        if (apduBytes == nil) {
            [self notifyProcessTerminated:false withInformative:MSG_ERROR_OATH_ACCOUNT_ADD_APDU_FAILED];
            return;
        }
        // アカウント登録コマンドを実行
        [self setCommandIns:0x01];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:0x00 data:apduBytes le:0xff];
    }

    - (void)doResponseAccountAdd:(NSData *)responseData status:(uint16_t)responseSW {
        // 不明なエラーが発生時は以降の処理を行わない
        if (responseSW != 0x9000) {
            NSString *message = [NSString stringWithFormat:MSG_ERROR_OATH_ACCOUNT_ADD_FAILED, responseSW];
            [self notifyProcessTerminated:false withInformative:message];
            return;
        }
        // 処理成功のログを出力
        [[ToolLogFile defaultLogger] info:MSG_INFO_OATH_ACCOUNT_ADD_SUCCESS];
        // 上位クラスに制御を戻す
        [self notifyProcessTerminated:true withInformative:MSG_NONE];
    }

    - (NSData *)GenerateAccountAddAPDU {
        // アカウント、Secretを入力とし、APDUバイト配列を生成
        NSString *account = [[self parameter] oathAccount];
        NSString *base32_secret = [[self parameter] oathBase32Secret];
        if (generate_account_add_apdu([account UTF8String], [base32_secret UTF8String]) == false) {
            return nil;
        }
        return [[NSData alloc] initWithBytes:generated_oath_apdu_bytes() length:generated_oath_apdu_size()];
    }

#pragma mark - Private common methods

    - (void)notifyProcessStarted {
        // コマンド処理結果を初期化
        [[self parameter] setCommandSuccess:false];
        // コマンド開始メッセージをログファイルに出力
        if ([[self parameter] commandTitle]) {
            NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE, [[self parameter] commandTitle]];
            [[ToolLogFile defaultLogger] info:startMsg];
        }
    }

    - (void)notifyProcessTerminated:(bool)success withInformative:(NSString *)informative {
        // CCIDデバイスから切断
        [[self toolCCIDHelper] ccidHelperWillDisconnect];
        // エラーメッセージを画面＆ログ出力
        if (success == false && [informative length] > 0) {
            // ログ出力する文言からは、改行文字を除去
            NSString *logMessage = [informative stringByReplacingOccurrencesOfString:@"\n" withString:@""];
            [[ToolLogFile defaultLogger] error:logMessage];
            [[self parameter] setResultInformativeMessage:informative];
        }
        // コマンド終了メッセージを生成
        NSString *endMsg = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, [[self parameter] commandTitle],
                            success ? MSG_SUCCESS : MSG_FAILURE];
        if (success == false) {
            // コマンド異常終了メッセージをログ出力
            [[ToolLogFile defaultLogger] error:endMsg];
        } else {
            // コマンド正常終了メッセージをログ出力
            [[ToolLogFile defaultLogger] info:endMsg];
        }
        // 結果を退避
        [[self parameter] setCommandSuccess:success];
        [[self parameter] setResultMessage:endMsg];
        // 戻り先がある場合は制御を戻す
        if ([self targetForContinue] && [self selectorForContinue]) {
            [[self targetForContinue] performSelector:[self selectorForContinue] withObject:nil afterDelay:0.0];
        }
    }

@end
