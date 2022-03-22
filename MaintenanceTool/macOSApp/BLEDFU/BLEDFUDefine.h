//
//  BLEDFUDefine.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/04.
//
#ifndef BLEDFUDefine_h
#define BLEDFUDefine_h

// 更新対象アプリケーション＝version 0.4.0
#define DFU_UPD_TARGET_APP_VERSION      400
// 処理タイムアウト（転送／反映チェック処理）
#define TIMEOUT_SEC_DFU_PROCESS         150.0
// イメージ反映所要時間（秒）
#define DFU_WAITING_SEC_ESTIMATED       25
// イメージ反映モード　true＝テストモード[Swap type: test]、false＝通常モード[Swap type: perm]
#define IMAGE_UPDATE_TEST_MODE          false

#endif /* BLEDFUDefine_h */
