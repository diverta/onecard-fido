//
//  MatterCommand.m
//  mattercontroller
//
//  Created by Makoto Morita on 2021/07/28.
//
#import <Foundation/Foundation.h>
#import "DefaultsUtils.h"
#import "MatterCommand.h"

@interface MatterCommand ()

    // 画面の参照を保持
    @property (nonatomic, weak) id<MatterCommandDelegate> delegate;
    // 非同期処理用のキュー（画面用）
    @property (nonatomic) dispatch_queue_t mainQueue;
    // Matterライブラリーのオブジェクト参照
    @property (readwrite) CHIPDeviceController *chipController;

@end

@implementation MatterCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<MatterCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            // 画面の参照を保持
            [self setDelegate:delegate];
            // 画面スレッドにバインドされるデフォルトキューを取得
            [self setMainQueue:dispatch_get_main_queue()];
            // Matter関連の初期化
            [self setupChipDeviceController];
        }
        NSLog(@"initWithViewControllerRef done");
        return self;
    }

    - (void)setupChipDeviceController {
        // ChipDeviceControllerを初期化
        dispatch_queue_t callbackQueue = dispatch_queue_create("jp.co.diverta.matter.vc.callback", DISPATCH_QUEUE_SERIAL);
        [self setChipController:InitializeCHIP()];
        [[self chipController] setPairingDelegate:self queue:callbackQueue];
    }

    - (void)handleRendezVousBLE:(uint16_t)discriminator setupPINCode:(uint32_t)setupPINCode {
        NSError * error;
        uint64_t deviceID = CHIPGetNextAvailableDeviceID();
        if ([[self chipController] pairDevice:deviceID discriminator:discriminator setupPINCode:setupPINCode csrNonce:nil error:&error]) {
            deviceID++;
            CHIPSetNextAvailableDeviceID(deviceID);
        }
    }

#pragma mark - for CHIPDevicePairingDelegate

    - (void)onPairingComplete:(NSError *)error {
        bool success = true;
        if (error.code != CHIPSuccess) {
            NSLog(@"Got pairing error back %@", error);
            success = false;
        }
        dispatch_async([self mainQueue], ^{
            // ペアリング完了後の処理を画面スレッドで実行
            [self pairingCompleted:success];
        });
    }

#pragma mark - Public method

    - (void)startBLEConnection:(id)ref {
        // 処理開始ログ
        NSLog(@"Matter device commissioning start");
        uint32_t setupPINCode = 20202021;
        uint16_t discriminator = 0x0f00;
        [self handleRendezVousBLE:discriminator setupPINCode:setupPINCode];
    }

    - (void)pairingCompleted:(bool)success {
        // 画面に制御を戻す
        [[self delegate] didPairingComplete:success];
    }

@end
