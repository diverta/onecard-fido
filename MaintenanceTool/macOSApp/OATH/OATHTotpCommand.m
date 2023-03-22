//
//  OATHTotpCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/17.
//
#import "oath_util.h"

#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "OATHTotpCommand.h"
#import "ToolCCIDHelper.h"

@interface OATHTotpCommand () <ToolCCIDHelperDelegate>

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

@implementation OATHTotpCommand

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
            case 0x04:
                [self doResponseCalculate:resp status:sw];
                break;
            default:
                break;
        }
    }

#pragma mark - Public methods

    - (void)doCalculateForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
        // ワンタイムパスワード生成処理を実行
        [self doRequestCalculate];
    }

#pragma mark - Calculate TOTP

    - (void)doRequestCalculate {
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
