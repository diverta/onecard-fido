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

#pragma mark - 共通
#define MSG_INVALID_FIELD           @"入力値が不正です。"
#define MSG_INVALID_FILE_PATH       @"ファイルが存在しません。"
#define MSG_BUTTON_SELECT           @"選択"

#pragma mark - FIDO鍵・証明書設定
#define MSG_PROMPT_USB_PORT_SET     @"FIDO認証器をUSBポートに装着してから実行してください。"
#define MSG_PROMPT_SELECT_PKEY_PATH @"秘密鍵ファイル(PEM)のパスを選択してください"
#define MSG_PROMPT_SELECT_CRT_PATH  @"証明書ファイル(CRT)のパスを選択してください"
#define MSG_INSTALL_SKEY_CERT       @"FIDO認証器に鍵・証明書をインストールします。"
#define MSG_PROMPT_INSTL_SKEY_CERT  @"インストールを実行しますか？"
#define MSG_ERASE_SKEY_CERT         @"FIDO認証器から鍵・証明書・ユーザー登録情報をすべて削除します。"
#define MSG_PROMPT_ERASE_SKEY_CERT  @"削除後はFIDO認証器によるユーザー登録／ログインができなくなります。\n削除処理を実行しますか？"

#endif /* AppCommonMessage_h */
