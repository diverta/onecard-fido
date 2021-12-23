//
//  ToolGPGCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/16.
//
#import "ToolAppCommand.h"
#import "ToolCommonMessage.h"
#import "ToolGPGCommand.h"
#import "ToolLogFile.h"

#define GenerateMainKeyScriptName               @"generate_main_key.sh"
#define GenerateMainKeyScriptParamName          @"generate_main_key.param"
#define AddSubKeyScriptName                     @"add_sub_key.sh"
#define AddSubKeyScriptParamName                @"add_sub_key.param"
#define ExportPubkeyAndBackupScriptName         @"export_pubkey_and_backup.sh"
#define ExportedPubkeyFileName                  @"public_key.pgp"
#define ExportedBackupFileName                  @"GNUPGHOME.tgz"
#define TransferSubkeyToCardScriptName          @"transfer_subkey_to_card.sh"
#define TransferSubkeyToCardScriptParamName     @"transfer_subkey_to_card.param"
#define KeyAlreadyStoredWarningMessage          @"such a key has already been stored on the card!"
#define ExecuteScriptSuccessMessage             @"Execute script for gnupg success"

@interface ToolGPGCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand         *toolAppCommand;
    // コマンド種別を保持
    @property (nonatomic) Command                       command;
    // コマンドからの応答データを保持
    @property (nonatomic) NSMutableArray<NSData *>     *commandOutput;
    // 生成された作業用フォルダー名称を保持
    @property (nonatomic) NSString                     *tempFolderPath;
    // 生成された鍵のIDを保持
    @property (nonatomic) NSString                     *generatedMainKeyId;
    // 副鍵が認証器に移動されたかどうかを保持
    @property (nonatomic) bool                          keyStoredSuccess;
    // 副鍵が既に認証器に存在するかどうかを保持
    @property (nonatomic) bool                          keyAlreadyStoredWarning;
    // スクリプトから出力されたログを保持
    @property (nonatomic) NSMutableArray<NSString *>   *scriptOutput;
    // 鍵作成用パラメーターを保持
    @property (nonatomic) NSString                     *realName;
    @property (nonatomic) NSString                     *mailAddress;
    @property (nonatomic) NSString                     *comment;
    @property (nonatomic) NSString                     *passphrase;
    @property (nonatomic) NSString                     *exportFolderPath;

@end

@implementation ToolGPGCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setToolAppCommand:(ToolAppCommand *)delegate];
        }
        return self;
    }

#pragma mark - Public methods

    - (void)setParametersForGeneratePGPKey:(id)sender
            realName:(NSString *)realName mailAddress:(NSString *)mailAddress comment:(NSString *)comment
            passphrase:(NSString *)passphrase exportFolderPath:(NSString *)exportFolderPath {
        // PGP秘密鍵（主鍵）生成のためのパラメーターを指定
        [self setRealName:realName];
        [self setMailAddress:mailAddress];
        [self setComment:comment];
        // PGP公開鍵とバックアップtarの出力先を指定
        [self setExportFolderPath:exportFolderPath];
        // 鍵のpassphraseには、管理用PINを指定（pinentryのloopback使用時、passphraseを複数指定できないための制約）
        [self setPassphrase:passphrase];
    }

    - (void)generatePGPKeyWillStart:(id)sender {
        // スクリプトログ格納配列をクリア
        [self setScriptOutput:[[NSMutableArray alloc] init]];
        // バージョン照会から開始
        [self notifyProcessStarted];
        [self doRequestGPGVersion];
    }

#pragma mark - Private common methods

    - (void)notifyProcessStarted {
        // メイン画面に開始メッセージを出力
        [[self toolAppCommand] commandStartedProcess:COMMAND_OPENPGP_GENERATE_KEYS type:TRANSPORT_NONE];
    }

    - (void)notifyMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] info:message];
        [self notifyToolCommandMessage:message];
    }

    - (void)notifyErrorMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] error:message];
        [self notifyToolCommandMessage:message];
    }

    - (void)notifyToolCommandMessage:(NSString *)message {
        // メイン画面にメッセージ文字列を表示する
        [[[self toolAppCommand] delegate] notifyAppCommandMessage:message];
    }

    - (void)notifyProcessTerminated:(bool)success {
        // メイン画面に制御を戻す
        [[self toolAppCommand] commandDidProcess:COMMAND_OPENPGP_GENERATE_KEYS result:success message:nil];
    }

#pragma mark - Private methods

    - (void)doRequestGPGVersion {
        // MacGPGコマンドが存在するかチェック
        if ([self checkIfFileExist:@"gpg" inFolder:@"/usr/local/bin"] == false) {
            [self notifyErrorMessage:MSG_ERROR_OPENPGP_GPG_VERSION_UNAVAIL];
            [self notifyProcessTerminated:false];
            return;
        }
        // インストールされているMacGPGコマンドのバージョンを照会
        NSString *path = @"/usr/local/bin/gpg";
        NSArray *args = @[@"--version"];
        [self doRequestCommandLine:COMMAND_GPG_VERSION commandPath:path commandArgs:args];
    }

    - (void)doResponseGPGVersion:(NSArray<NSString *> *)response {
        // PCに導入されているGPGが、所定のバージョン以上でない場合は終了
        if ([self checkIfGPGVersionAvailable:response] == false) {
            [self notifyErrorMessage:MSG_ERROR_OPENPGP_GPG_VERSION_UNAVAIL];
            [self notifyProcessTerminated:false];
            return;
        }
        // 次の処理に移行
        [self doRequestMakeTempFolder];
    }

    - (void)doRequestMakeTempFolder {
        // 作業用フォルダーをPC上に生成
        NSString *path = @"/usr/bin/mktemp";
        NSArray *args = @[@"-d"];
        [self doRequestCommandLine:COMMAND_GPG_MAKE_TEMP_FOLDER commandPath:path commandArgs:args];
    }

    - (void)doResponseMakeTempFolder:(NSArray<NSString *> *)response {
        // レスポンスをチェック
        if ([response count] != 1) {
            [self notifyErrorMessage:MSG_ERROR_OPENPGP_CREATE_TEMPDIR_FAIL];
            [self notifyProcessTerminated:false];
            return;
        }
        // 生成された作業用フォルダー名称を保持
        [self setTempFolderPath:[response objectAtIndex:0]];
        [[ToolLogFile defaultLogger] debugWithFormat:MSG_FORMAT_OPENPGP_CREATED_TEMPDIR, [self tempFolderPath]];
        // 次の処理に移行
        [self doRequestGenerateMainKey];
    }

    - (void)doRequestGenerateMainKey {
        // シェルスクリプトの絶対パスを取得
        NSString *scriptPath = [self getResourceFilePath:GenerateMainKeyScriptName];
        // パラメーターテンプレートをファイルから読込み
        NSString *paramTemplContent = [self readParameterTemplateFrom:GenerateMainKeyScriptParamName];
        if (paramTemplContent == nil) {
            [self notifyProcessTerminated:false];
            return;
        }
        // シェルスクリプトのパラメーターファイルを生成
        if ([self writeParameterFile:GenerateMainKeyScriptParamName fromTemplate:paramTemplContent, [self realName], [self mailAddress], [self comment]] == false) {
            [self notifyProcessTerminated:false];
            return;
        }
        // シェルスクリプトを実行
        NSArray *args = @[[self tempFolderPath], [self passphrase], @"--no-tty"];
        [self doRequestCommandLine:COMMAND_GPG_GENERATE_MAIN_KEY commandPath:scriptPath commandArgs:args];
    }

    - (void)doResponseGenerateMainKey:(NSArray<NSString *> *)response {
        // 生成鍵IDをクリア
        [self setGeneratedMainKeyId:nil];
        // レスポンスをチェック
        if ([self checkResponseOfScript:response]) {
            // 生成鍵がCertify機能を有しているかチェック
            NSString *keyid = [self extractMainKeyIdFromResponse:response];
            if (keyid != nil) {
                // チェックOKの場合は鍵IDを保持し、次の処理に移行
                [self setGeneratedMainKeyId:keyid];
                [[ToolLogFile defaultLogger] debugWithFormat:MSG_FORMAT_OPENPGP_GENERATED_MAIN_KEY, [self generatedMainKeyId]];
                [self doRequestAddSubKey];
                return;
            }
        }
        // エラーメッセージを出力し、後処理に移行
        [self notifyErrorMessage:MSG_ERROR_OPENPGP_GENERATE_MAINKEY_FAIL];
        [self doRequestRemoveTempFolder];
    }

    - (void)doRequestAddSubKey {
        // シェルスクリプトの絶対パスを取得
        NSString *scriptPath = [self getResourceFilePath:AddSubKeyScriptName];
        // パラメーターテンプレートをファイルから読込み
        NSString *paramTemplContent = [self readParameterTemplateFrom:AddSubKeyScriptParamName];
        if (paramTemplContent == nil) {
            [self notifyProcessTerminated:false];
            return;
        }
        // シェルスクリプトのパラメーターファイルを生成
        if ([self writeParameterFile:AddSubKeyScriptParamName fromTemplate:paramTemplContent] == false) {
            [self notifyProcessTerminated:false];
            return;
        }
        // シェルスクリプトを実行
        NSArray *args = @[[self tempFolderPath], [self passphrase], [self generatedMainKeyId], @"--no-tty"];
        [self doRequestCommandLine:COMMAND_GPG_ADD_SUB_KEY commandPath:scriptPath commandArgs:args];
    }

    - (void)doResponseAddSubKey:(NSArray<NSString *> *)response {
        // レスポンスをチェック
        if ([self checkResponseOfScript:response]) {
            // 副鍵が３点生成された場合は、次の処理に移行
            if ([self checkIfSubKeysExistFromResponse:response transferred:false]) {
                [[ToolLogFile defaultLogger] debug:MSG_OPENPGP_ADDED_SUB_KEYS];
                [self doRequestExportPubkeyAndBackup];
                return;
            }
        }
        // エラーメッセージを出力し、後処理に移行
        [self notifyErrorMessage:MSG_ERROR_OPENPGP_GENERATE_SUB_KEY_FAIL];
        [self doRequestRemoveTempFolder];
    }

    - (void)doRequestExportPubkeyAndBackup {
        // シェルスクリプトの絶対パスを取得
        NSString *scriptPath = [self getResourceFilePath:ExportPubkeyAndBackupScriptName];
        // シェルスクリプトを実行
        NSArray *args = @[[self tempFolderPath], [self passphrase], [self generatedMainKeyId], [self exportFolderPath]];
        [self doRequestCommandLine:COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP commandPath:scriptPath commandArgs:args];
    }

    - (void)doResponseExportPubkeyAndBackup:(NSArray<NSString *> *)response {
        // レスポンスをチェック
        if ([self checkResponseOfScript:response]) {
            if ([self checkIfPubkeyAndBackupExistIn:[self exportFolderPath]]) {
                // 公開鍵ファイル、バックアップファイルが生成された場合は、次の処理に移行
                [[ToolLogFile defaultLogger] debugWithFormat:MSG_FORMAT_OPENPGP_EXPORT_BACKUP_DONE, [self exportFolderPath]];
                [self doRequestTransferSubkeyToCard];
                return;
            }
        }
        // エラーメッセージを出力し、後処理に移行
        [self notifyErrorMessage:MSG_ERROR_OPENPGP_EXPORT_BACKUP_FAIL];
        [self doRequestRemoveTempFolder];
    }

    - (void)doRequestTransferSubkeyToCard {
        // シェルスクリプトの絶対パスを取得
        NSString *scriptPath = [self getResourceFilePath:TransferSubkeyToCardScriptName];
        // パラメーターテンプレートをファイルから読込み
        NSString *paramTemplContent = [self readParameterTemplateFrom:TransferSubkeyToCardScriptParamName];
        if (paramTemplContent == nil) {
            [self notifyProcessTerminated:false];
            return;
        }
        // シェルスクリプトのパラメーターファイルを生成
        if ([self writeParameterFile:TransferSubkeyToCardScriptParamName fromTemplate:paramTemplContent] == false) {
            [self notifyProcessTerminated:false];
            return;
        }
        // シェルスクリプトを実行
        NSArray *args = @[[self tempFolderPath], [self passphrase], [self generatedMainKeyId], @"--no-tty"];
        [self doRequestCommandLine:COMMAND_GPG_TRANSFER_SUBKEY_TO_CARD commandPath:scriptPath commandArgs:args];
    }

    - (void)doResponseTransferSubkeyToCard:(NSArray<NSString *> *)response {
        // レスポンスをチェック
        if ([self checkResponseOfScript:response]) {
            if ([self checkIfSubKeysExistFromResponse:response transferred:true]) {
                // 副鍵が認証器に移動された場合は後処理に移行
                [[ToolLogFile defaultLogger] debug:MSG_OPENPGP_TRANSFERRED_KEYS_TO_DEVICE];
                [self setKeyStoredSuccess:true];
                [self doRequestRemoveTempFolder];
                return;
            }
        }
        // 副鍵が移動されなかった場合、副鍵が認証器に既に保管されていたかどうかチェック後、後処理に移行
        [self setKeyStoredSuccess:false];
        [self setKeyAlreadyStoredWarning:[self checkIfSubKeyAlreadyStoredFromResponse:response]];
        if ([self keyAlreadyStoredWarning]) {
            [self notifyErrorMessage:MSG_ERROR_OPENPGP_KEYS_ALREADY_STORED];
        } else {
            [self notifyErrorMessage:MSG_ERROR_OPENPGP_TRANSFER_KEYS_FAIL];
        }
        [self doRequestRemoveTempFolder];
    }

    - (void)doRequestRemoveTempFolder {
        // 作業用フォルダーをPC上から削除
        NSString *path = @"/bin/rm";
        NSArray *args = @[@"-rf", [self tempFolderPath]];
        [self doRequestCommandLine:COMMAND_GPG_REMOVE_TEMP_FOLDER commandPath:path commandArgs:args];
    }

    - (void)doResponseRemoveTempFolder:(NSArray<NSString *> *)response {
        // レスポンスをチェック
        if ([response count] > 0) {
            [self notifyErrorMessage:MSG_ERROR_OPENPGP_REMOVE_TEMPDIR_FAIL];
            [self notifyProcessTerminated:false];
            return;
        }
        // 生成された作業用フォルダー名称をクリア
        [self setTempFolderPath:nil];
        [[ToolLogFile defaultLogger] debug:MSG_OPENPGP_REMOVED_TEMPDIR];
        // 処理完了を通知
        [self notifyProcessTerminated:[self keyStoredSuccess]];
    }

#pragma mark - Private functions

    - (bool)checkIfGPGVersionAvailable:(NSArray<NSString *> *)response {
        // メッセージ検索用文字列
        NSString *keyword = @"gpg (GnuPG/MacGPG2)*";
        for (NSString *text in response) {
            if ([text isLike:keyword]) {
                // 改行文字で区切られた文字列を分割
                NSArray *array = [text componentsSeparatedByString:@"\n"];
                // 分割されたメッセージの１件目、バージョン文字列を解析（半角スペース文字で区切られた文字列を分割）
                NSString *versionInfo = [array objectAtIndex:0];
                NSArray *infoCols = [versionInfo componentsSeparatedByString:@" "];
                // 分割されたメッセージの３件目について、鍵の機能を解析
                if ([infoCols count] == 3) {
                    // PCに導入されているGPGのバージョンが2.2.27以上の場合は true
                    NSString *versionStr = [infoCols objectAtIndex:2];
                    int versionDec = [ToolCommon calculateDecimalVersion:versionStr];
                    [[ToolLogFile defaultLogger] debugWithFormat:@"Installed MacGPG: version %@", versionStr];
                    return (versionDec >= 20227);
                }
            }
        }
        [[ToolLogFile defaultLogger] debug:@"MacGPG is not installed yet"];
        return false;
    }

    - (NSString *)extractMainKeyIdFromResponse:(NSArray<NSString *> *)response {
        // メッセージ検索用文字列
        NSString *keyword = @"pub   rsa2048*";
        // メッセージ文字列から鍵IDを抽出
        NSString *keyid = nil;
        for (NSString *text in response) {
            if ([text isLike:keyword]) {
                // 改行文字で区切られた文字列を分割
                NSArray *values = [text componentsSeparatedByString:@"\n"];
                // 分割されたメッセージの１件目について、鍵の機能を解析
                NSString *valueOfFunc = [values objectAtIndex:0];
                if ([valueOfFunc containsString:@"[C]"]) {
                    // 分割されたメッセージの２件目、後ろから16バイトの文字列を、鍵IDとして抽出
                    NSString *valueOfId = [values objectAtIndex:1];
                    keyid = [valueOfId substringWithRange:NSMakeRange([valueOfId length] - 16, 16)];
                }
                break;
            }
        }
        return keyid;
    }

    - (bool)checkIfSubKeysExistFromResponse:(NSArray<NSString *> *)response transferred:(bool)transferred {
        // メッセージ検索用文字列
        NSString *keyword1 = [[NSString alloc] initWithFormat:@"%@/pubring.kbx*", [self tempFolderPath]];
        NSString *keyword2 = transferred ? @"ssb>  rsa2048*" : @"ssb   rsa2048*";
        // 副鍵生成の有無を保持
        bool subKeyS = false;
        bool subKeyE = false;
        bool subKeyA = false;
        // メッセージ文字列から鍵一覧メッセージ（'gpg -K'実行結果）を抽出
        for (NSString *text in response) {
            if ([text isLike:keyword1] == false) {
                continue;
            }
            // 改行文字で区切られた文字列を分割
            NSArray *values = [text componentsSeparatedByString:@"\n"];
            // 副鍵に関するメッセージを解析
            for (NSString *value in values) {
                if ([value isLike:keyword2] == false) {
                    continue;
                }
                // 副鍵の機能を解析
                if ([value containsString:@"[S]"]) {
                    subKeyS = true;
                } else if ([value containsString:@"[E]"]) {
                    subKeyE = true;
                } else if ([value containsString:@"[A]"]) {
                    subKeyA = true;
                }
            }
            break;
        }
        // ３点の副鍵が揃っていれば true を戻す
        if (subKeyS && subKeyE && subKeyA) {
            return true;
        }
        // 揃っていない副鍵についてログを出力
        NSString *str = transferred ? @"transferred" : @"added";
        if (subKeyS == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Sub key (for sign) not %@", str];
        }
        if (subKeyE == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Sub key (for encrypt) not %@", str];
        }
        if (subKeyA == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Sub key (for authenticate) not %@", str];
        }
        // false を戻す
        return false;
    }

    - (bool)checkIfPubkeyAndBackupExistIn:(NSString *)exportPath {
        // 公開鍵ファイルがエクスポート先に存在するかチェック
        if ([self checkIfFileExist:ExportedPubkeyFileName inFolder:exportPath] == false) {
            [[ToolLogFile defaultLogger] error:MSG_ERROR_OPENPGP_EXPORT_PUBKEY_FAIL];
            return false;
        }
        // バックアップファイルがエクスポート先に存在するかチェック
        if ([self checkIfFileExist:ExportedBackupFileName inFolder:exportPath] == false) {
            [[ToolLogFile defaultLogger] error:MSG_ERROR_OPENPGP_BACKUP_FAIL];
            return false;
        }
        return true;
    }

    - (bool)checkIfSubKeyAlreadyStoredFromResponse:(NSArray<NSString *> *)response {
        for (NSString *text in response) {
            if ([text containsString:KeyAlreadyStoredWarningMessage]) {
                return true;
            }
        }
        return false;
    }

#pragma mark - Command line processor

    - (void)doRequestCommandLine:(Command)command commandPath:(NSString*)path commandArgs:(NSArray*)args {
        // コマンド種別を保持
        [self setCommand:command];
        // 標準入力用
        NSTask *task = [[NSTask alloc] init];
        [task setStandardInput:[NSPipe pipe]];
        // 標準出力用
        [task setStandardOutput:[NSPipe pipe]];
        [task setStandardError:[task standardOutput]];
        // 実行するコマンドのパスと引数を設定
        [task setLaunchPath:path];
        [task setArguments:args];
        // 応答文字列の格納用配列を初期化
        [self setCommandOutput:[[NSMutableArray alloc] init]];
        // コマンドからの応答を待機
        ToolGPGCommand * __weak weakSelf = self;
        [[[task standardOutput] fileHandleForReading] waitForDataInBackgroundAndNotify];
        [[NSNotificationCenter defaultCenter]
            addObserverForName:NSFileHandleDataAvailableNotification
                        object:[[task standardOutput] fileHandleForReading] queue:nil
                    usingBlock:^(NSNotification *notification) {
            [weakSelf extractOutputStringWith:task];
        }];
        // コマンドを実行
        [task launch];
    }

    - (void)extractOutputStringWith:(NSTask *)task {
        // 応答がこれ以上無ければ終了
        NSData *output = [[[task standardOutput] fileHandleForReading] availableData];
        if (output == nil || [output length] == 0) {
            [self commandDidTerminated];
            return;
        }
        // 応答データを配列に保持
        [[self commandOutput] addObject:output];
        // 次の応答があれば待機
        [[[task standardOutput] fileHandleForReading] waitForDataInBackgroundAndNotify];
    }

    - (void)commandDidTerminated {
        // 応答データを配列に抽出
        NSMutableArray<NSString *> *outputArray = [[NSMutableArray alloc] init];
        for (NSData *data in [self commandOutput]) {
            // データ末尾に改行文字があれば削除
            NSData *output = data;
            uint8_t *bytes = (uint8_t *)[data bytes];
            if (bytes[[data length] - 1] == 0x0a) {
                output = [data subdataWithRange:NSMakeRange(0, [data length] - 1)];
            }
            // データを文字列に変換し、配列に格納
            NSString *outStr = [[NSString alloc] initWithData:output encoding:NSUTF8StringEncoding];
            [outputArray addObject:outStr];
        }
        // レスポンスを処理
        switch ([self command]) {
            case COMMAND_GPG_VERSION:
                [self doResponseGPGVersion:outputArray];
                break;
            case COMMAND_GPG_MAKE_TEMP_FOLDER:
                [self doResponseMakeTempFolder:outputArray];
                break;
            case COMMAND_GPG_REMOVE_TEMP_FOLDER:
                [self doResponseRemoveTempFolder:outputArray];
                break;
            case COMMAND_GPG_GENERATE_MAIN_KEY:
                [self doResponseGenerateMainKey:outputArray];
                break;
            case COMMAND_GPG_ADD_SUB_KEY:
                [self doResponseAddSubKey:outputArray];
                break;
            case COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP:
                [self doResponseExportPubkeyAndBackup:outputArray];
                break;
            case COMMAND_GPG_TRANSFER_SUBKEY_TO_CARD:
                [self doResponseTransferSubkeyToCard:outputArray];
                break;
            default:
                return;
        }
    }

#pragma mark - Utility functions

    - (NSString *)getResourceFilePath:(NSString *)filename {
        // リソースフォルダー配下にあるファイルの絶対パスを取得
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        NSString *scriptPath = [NSString stringWithFormat:@"%@/%@", resourcePath, filename];
        return scriptPath;
    }

    - (NSString *)readParameterTemplateFrom:(NSString *)filename {
        // パラメーターテンプレートの絶対パスを取得
        NSString *paramTemplPath = [self getResourceFilePath:filename];
        // パラメーターテンプレートを読み込み
        NSError *error;
        NSString *paramTemplContent = [NSString stringWithContentsOfFile:paramTemplPath encoding:NSUTF8StringEncoding error:&error];
        if (error) {
            [[ToolLogFile defaultLogger] error:MSG_ERROR_OPENPGP_READ_PARAM_TEMPL_FAIL];
            return nil;
        }
        return paramTemplContent;
    }

    - (bool)writeParameterFile:(NSString *)filename fromTemplate:(NSString *)paramTemplContent, ... {
        // シェルスクリプトのテンプレートを、指定の内容で置き換え、パラメーターを生成
        va_list args;
        va_start(args, paramTemplContent);
        NSString *paramContent = [[NSString alloc] initWithFormat:paramTemplContent arguments:args];
        va_end(args);
        // パラメーターファイルの書き出し先（絶対パス）を取得
        NSString *paramPath = [NSString stringWithFormat:@"%@/%@", [self tempFolderPath], filename];
        // パラメーターをファイルに書き出し
        NSError *error;
        [paramContent writeToFile:paramPath atomically:YES encoding:NSUTF8StringEncoding error:&error];
        if (error) {
            [[ToolLogFile defaultLogger] error:MSG_ERROR_OPENPGP_WRITE_PARAM_FILE_FAIL];
            return false;
        }
        return true;
    }

    - (bool)checkResponseOfScript:(NSArray<NSString *> *)response {
        bool success = false;
        if ([response count] > 0) {
            for (NSString *text in response) {
                if ([text isEqualToString:ExecuteScriptSuccessMessage]) {
                    // シェルスクリプトから成功メッセージが出力された場合、trueを戻す
                    success = true;
                } else {
                    // シェルスクリプトのログを、格納配列に追加
                    [[self scriptOutput] addObject:text];
                }
            }
        }
        return success;
    }

    - (bool)checkIfFileExist:(NSString *)filename inFolder:(NSString *)folderPath {
        // 指定のフォルダー配下にファイルが存在している場合は true
        NSString *filePath = [NSString stringWithFormat:@"%@/%@", folderPath, filename];
        return [[NSFileManager defaultManager] fileExistsAtPath:filePath];
    }

@end
