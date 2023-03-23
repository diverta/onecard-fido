//
//  OATHAccountCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/17.
//
#import "base32_util.h"
#import "oath_util.h"

#import "AppCommonMessage.h"
#import "OATHAccountCommand.h"
#import "OATHCommand.h"
#import "ToolCCIDHelper.h"

@interface OATHAccountCommand () <ToolCCIDHelperDelegate>

    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolCCIDHelper       *toolCCIDHelper;
    // パラメーターの参照を保持
    @property (nonatomic) OATHCommandParameter *parameter;
    // コマンド完了後に継続される処理を保持
    @property (nonatomic) id                    targetForContinue;
    @property (nonatomic) SEL                   selectorForContinue;
    // CCID I/Fにリクエストしたコマンドバイトを保持
    @property (nonatomic) uint8_t               commandIns;

@end

@implementation OATHAccountCommand

    - (id)init {
        self = [super init];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setToolCCIDHelper:[[ToolCCIDHelper alloc] initWithDelegate:self]];
            // パラメーターの参照を保持
            [self setParameter:[[OATHCommand instance] parameter]];
        }
        return self;
    }

    - (void)ccidHelperDidReceiveResponse:(NSData *)resp status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self commandIns]) {
            case 0x01:
                [self doResponseAccountAdd:resp status:sw];
                break;
            case 0x02:
                [self doResponseAccountDelete:resp status:sw];
                break;
            case 0x03:
                [self doResponseAccountList:resp status:sw];
                break;
            case 0x04:
                [self doResponseCalculate:resp status:sw];
                break;
            default:
                break;
        }
    }

#pragma mark - Account add

    - (void)doAccountAddForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
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
        // 上位クラスに制御を戻す
        [self notifyProcessTerminated:true withInformative:MSG_NONE];
    }

    - (NSData *)GenerateAccountAddAPDU {
        // アカウント、Secretを入力とし、APDUバイト配列を生成
        NSString *account = [[self parameter] oathAccount];
        NSString *base32_secret = [[self parameter] oathBase32Secret];
        if (generate_account_add_apdu([account UTF8String], [account length], [base32_secret UTF8String], [base32_secret length]) == false) {
            return nil;
        }
        return [[NSData alloc] initWithBytes:generated_oath_apdu_bytes() length:generated_oath_apdu_size()];
    }

#pragma mark - Account list

    - (void)doAccountListForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
        // アカウント一覧取得コマンドを実行
        NSData *apduBytes = [[NSData alloc] init];
        [self setCommandIns:0x03];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:0x00 data:apduBytes le:0xff];
    }

    - (void)doResponseAccountList:(NSData *)responseData status:(uint16_t)responseSW {
        // 不明なエラーが発生時は以降の処理を行わない
        if (responseSW != 0x9000) {
            NSString *message = [NSString stringWithFormat:MSG_ERROR_OATH_LIST_ACCOUNT_FAILED, responseSW];
            [self notifyProcessTerminated:false withInformative:message];
            return;
        }
        // レスポンスからアカウント名一覧を抽出
        [self parseAccountListBytes:responseData];
        // 上位クラスに制御を戻す
        [self notifyProcessTerminated:true withInformative:MSG_NONE];
    }

    - (void)parseAccountListBytes:(NSData *)accountListData {
        // 領域を初期化
        NSMutableArray<NSString *> *array = [[NSMutableArray alloc] init];
        uint8_t *accountListBytes = (uint8_t *)[accountListData bytes];
        size_t size = [accountListData length];
        size_t i = 0;
        while (i < size) {
            // 0x71（アカウント名）出現まで走査
            if (accountListBytes[i++] != 0x71) {
                continue;
            }
            // アカウント名の長さを取得
            int nameLength = accountListBytes[i++];
            if (nameLength == 0 || i > size) {
                continue;
            }
            // アカウント名を抽出し、配列に格納
            uint8_t *nameBytes = accountListBytes + i;
            NSData *nameData = [[NSData alloc] initWithBytes:nameBytes length:nameLength];
            NSString *nameString = [[NSString alloc] initWithData:nameData encoding:NSUTF8StringEncoding];
            [array addObject:nameString];
            // 後続バイトを走査
            i += nameLength;
        }
        // パラメーターに配列を格納
        [[self parameter] setAccountList:array];
    }

#pragma mark - Account delete

    - (void)doAccountDeleteForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
        // アカウント削除処理用APDUを生成
        NSData *apduBytes = [self generateAPDUForDelete];
        if (apduBytes == nil) {
            [self notifyProcessTerminated:false withInformative:MSG_ERROR_OATH_ACCOUNT_DELETE_APDU_FAILED];
            return;
        }
        // アカウント削除コマンドを実行
        [self setCommandIns:0x02];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:0x00 data:apduBytes le:0xff];
    }

    - (void)doResponseAccountDelete:(NSData *)responseData status:(uint16_t)responseSW {
        // 不明なエラーが発生時は以降の処理を行わない
        if (responseSW != 0x9000) {
            NSString *message = [NSString stringWithFormat:MSG_ERROR_OATH_ACCOUNT_DELETE_FAILED, responseSW];
            [self notifyProcessTerminated:false withInformative:message];
            return;
        }
        // 上位クラスに制御を戻す
        [self notifyProcessTerminated:true withInformative:MSG_NONE];
    }

    - (NSData *)generateAPDUForDelete {
        // アカウントをバイト配列化
        NSString *account = [[self parameter] oathAccount];
        const char *accountBytes = [account UTF8String];
        size_t accountSize = [account length];
        // 変数初期化
        size_t apduBytesSize = 2 + accountSize;
        uint8_t apduBytes[apduBytesSize];
        size_t offset = 0;
        // アカウント
        apduBytes[offset++] = 0x71;
        apduBytes[offset++] = (uint8_t)accountSize;
        // アカウントをコピー
        memcpy(apduBytes + offset, accountBytes, accountSize);
        // APDUを戻す
        return [[NSData alloc] initWithBytes:apduBytes length:apduBytesSize];
    }

#pragma mark - Calculate TOTP

    - (void)doCalculateForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
        // ワンタイムパスワード生成処理用APDUを生成
        NSData *apduBytes = [self generateAPDUForCalculate];
        if (apduBytes == nil) {
            [self notifyProcessTerminated:false withInformative:MSG_ERROR_OATH_CALCULATE_APDU_FAILED];
            return;
        }
        // ワンタイムパスワード生成コマンドを実行
        [self setCommandIns:0x04];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:0x00 data:apduBytes le:0xff];
    }

    - (void)doResponseCalculate:(NSData *)responseData status:(uint16_t)responseSW {
        // 不明なエラーが発生時は以降の処理を行わない
        if (responseSW != 0x9000) {
            NSString *message = [NSString stringWithFormat:MSG_ERROR_OATH_CALCULATE_FAILED, responseSW];
            [self notifyProcessTerminated:false withInformative:message];
            return;
        }
        // レスポンスの4～7バイト目をエンディアン変換し、ワンタイムパスワードを生成（下６桁を抽出）
        uint8_t *responseBytes = (uint8_t *)[responseData bytes];
        uint32_t totpSrcInt = [ToolCommon getLENumber32FromBEBytes:(responseBytes + 3)];
        [[self parameter] setOathTotpValue:(totpSrcInt % 1000000)];
        // 上位クラスに制御を戻す
        [self notifyProcessTerminated:true withInformative:MSG_NONE];
    }

    - (NSData *)generateAPDUForCalculate {
        // アカウントを入力とし、APDUバイト配列を生成
        NSString *account = [[self parameter] oathAccount];
        if (generate_apdu_for_calculate([account UTF8String], [account length]) == false) {
            return nil;
        }
        return [[NSData alloc] initWithBytes:generated_oath_apdu_bytes() length:generated_oath_apdu_size()];
    }

#pragma mark - Private common methods

    - (void)notifyProcessTerminated:(bool)success withInformative:(NSString *)informative {
        // 結果を退避
        [[self parameter] setCommandSuccess:success];
        [[self parameter] setResultInformativeMessage:informative];
        // 戻り先がある場合は制御を戻す
        if ([self targetForContinue] && [self selectorForContinue]) {
            [[self targetForContinue] performSelector:[self selectorForContinue] withObject:nil afterDelay:0.0];
        }
    }

@end
