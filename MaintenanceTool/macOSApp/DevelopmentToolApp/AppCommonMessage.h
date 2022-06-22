//
//  AppCommonMessage.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/07.
//
#ifndef AppCommonMessage_h
#define AppCommonMessage_h

#pragma mark - ホーム画面
#define MSG_APP_NAME                @"FIDO認証器開発ツール"
#define MSG_APP_LAUNCHED            @"FIDO認証器開発ツールを起動しました: Version %@"
#define MSG_APP_TERMINATED          @"FIDO認証器開発ツールを終了しました。"
#define MSG_APP_FUNC_NOT_SUPPORTED  @"この機能は実行できません。"
#define MSG_APP_COPYRIGHT           @"Copyright (c) 2017-2022 Diverta Inc."
#define MSG_FORMAT_APP_VERSION      @"Version %@"
#define MSG_FORMAT_START_MESSAGE    @"%1$@を開始します。"
#define MSG_FORMAT_END_MESSAGE      @"%1$@が%2$@しました。"

#pragma mark - 共通
#define MSG_INVALID_FIELD           @"入力値が不正です。"
#define MSG_INVALID_FILE_PATH       @"ファイルが存在しません。"
#define MSG_BUTTON_SELECT           @"選択"
#define MSG_SUCCESS                 @"成功"
#define MSG_FAILURE                 @"失敗"

#pragma mark - FIDO鍵・証明書設定
#define MSG_PROMPT_USB_PORT_SET     @"FIDO認証器をUSBポートに装着してから実行してください。"
#define MSG_PROMPT_SELECT_PKEY_PATH @"秘密鍵ファイル(PEM)のパスを選択してください"
#define MSG_PROMPT_SELECT_CRT_PATH  @"証明書ファイル(CRT)のパスを選択してください"
#define MSG_INSTALL_SKEY_CERT       @"FIDO認証器に鍵・証明書をインストールします。"
#define MSG_PROMPT_INSTL_SKEY_CERT  @"インストールを実行しますか？"
#define MSG_ERASE_SKEY_CERT         @"FIDO認証器から鍵・証明書・ユーザー登録情報をすべて削除します。"
#define MSG_PROMPT_ERASE_SKEY_CERT  @"削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？"

#pragma mark - ToolCommandクラス専用メッセージ
#define MSG_INVALID_SKEY_LENGTH_IN_PEM      @"鍵ファイルに格納された秘密鍵の長さが不正です。"
#define MSG_INVALID_SKEY_HEADER_IN_PEM      @"鍵ファイルに格納された秘密鍵のヘッダーが不正です。"
#define MSG_CANNOT_READ_SKEY_PEM_FILE       @"鍵ファイルを読み込むことができません。"
#define MSG_INVALID_SKEY_CONTENT_IN_PEM     @"鍵ファイルの内容が不正です。"
#define MSG_CANNOT_READ_CERT_CRT_FILE       @"証明書ファイルを読み込むことができません。"
#define MSG_INVALID_CERT_LENGTH_IN_CRT      @"証明書ファイルに格納されたデータの長さが不正です。"
#define MSG_READ_NBYTES_FROM_CRT_FILE       @"証明書ファイル(%ldバイト)を読込みました。"
#define MSG_CANNOT_CRYPTO_SKEY_CERT_DATA    @"鍵・証明書の転送データを暗号化できませんでした。"
#define MSG_CANNOT_RECV_DEVICE_PUBLIC_KEY   @"公開鍵を認証器から受け取ることができませんでした。"
#define MSG_INVALID_SKEY_OR_CERT            @"秘密鍵または公開鍵の内容が不正です。"

#pragma mark - コマンド種別に対応する処理名称
#define PROCESS_NAME_ERASE_SKEY_CERT                @"鍵・証明書の削除"
#define PROCESS_NAME_INSTALL_SKEY_CERT              @"鍵・証明書インストール"

#endif /* AppCommonMessage_h */
