//
//  ToolCommonMessage.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/26.
//
#ifndef ToolCommonMessage_h
#define ToolCommonMessage_h

#pragma mark - 共通
#define MSG_INVALID_FIELD           @"入力項目が不正です。"
#define MSG_BUTTON_SELECT           @"選択"
#define MSG_BUTTON_CREATE           @"作成"
#define MSG_SUCCESS                 @"成功"
#define MSG_FAILURE                 @"失敗"

#pragma mark - ホーム画面
#define MSG_PROMPT_SELECT_PKEY_PATH @"秘密鍵ファイル(PEM)のパスを選択してください"
#define MSG_PROMPT_SELECT_CRT_PATH  @"証明書ファイル(CRT)のパスを選択してください"
#define MSG_SETUP_CHROME            @"ChromeでBLE U2Fトークンが使用できるよう設定します。"
#define MSG_PROMPT_SETUP_CHROME     @"ChromeでBLE U2Fトークンを使用時、このU2F管理ツールがChromeのサブプロセスとして起動します。\n設定を実行しますか？"
#define MSG_OCCUR_BLECONN_ERROR     @"BLE接続エラーが発生しました。"
#define MSG_OCCUR_UNKNOWN_ERROR     @"不明なエラーが発生しました。"
#define MSG_FORMAT_END_MESSAGE      @"%1$@が%2$@しました。"

#pragma mark - 証明書要求ファイル作成画面
#define MSG_PROMPT_SELECT_PEM_PATH  @"証明書要求時に使用する秘密鍵ファイル(PEM)を選択してください。"
#define MSG_PROMPT_CREATE_CSR_PATH  @"作成する証明書要求ファイル(CSR)名を指定してください。"
#define MSG_PROMPT_CREATE_PEM_PATH  @"作成する秘密鍵ファイル(PEM)名を指定してください。"
#define MSG_PROMPT_INPUT_CN         @"実際に接続されるURLのFQDN（例：www.diverta.co.jp）を入力してください。"
#define MSG_PROMPT_INPUT_O          @"申請組織の名称（例：Diverta Inc.）を入力してください。"
#define MSG_PROMPT_INPUT_L          @"申請組織の事業所住所の市区町村名（例：Shinjuku-ku、Yokohama-shi等）を入力してください。"
#define MSG_PROMPT_INPUT_ST         @"申請組織の事業所住所の都道府県名（例：Tokyo、Kanagawa）を入力してください。"
#define MSG_PROMPT_INPUT_C          @"申請組織の事業所住所の国名（例：JP）を入力してください。"

#pragma mark - プロセス名称
#define PROCESS_NAME_ERASE_BOND     @"ペアリング情報削除処理"
#define PROCESS_NAME_ERASE_KEYCRT   @"鍵・証明書削除処理"
#define PROCESS_NAME_INSTALL_KEYCRT @"鍵・証明書インストール"
#define PROCESS_NAME_HEALTHCHECK    @"ヘルスチェック"
#define PROCESS_NAME_SETUP_CHROME   @"Chrome Native Messaging有効化設定"
#define PROCESS_NAME_CREATE_KEYPAIR @"鍵ファイル作成"
#define PROCESS_NAME_CREATE_CERTREQ @"証明書要求ファイル作成"
#define PROCESS_NAME_CREATE_SELFCRT @"自己署名証明書ファイル作成"

#endif /* ToolCommonMessage_h */
