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
    @property (readwrite) CHIPDeviceController     *chipController;
    @property (nonatomic) CHIPNetworkCommissioning *cluster;
    @property (nonatomic) CHIPOnOff                *clusterOnOff;
    // コミッショニングに必要なパラメーターを保持
    @property (nonatomic) NSData               *operationalDataset;
    @property (nonatomic) NSData               *networkId;

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
            [self setupCommissioningParameters];
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

    - (void)setupCommissioningParameters {
        // コミッショニングに必要なパラメーターを設定
        //  channel         0x00000f
        //  Network PANID   0x1234
        //  Network XPANID  0x1111111122222222
        //  Master Key      0x00112233445566778899aabbccddeeff
        [self setOperationalDataset:[self generateHexBytesFrom:@"000300000f0102123402081111111122222222051000112233445566778899aabbccddeeff"]];
        [self setNetworkId:[self generateHexBytesFrom:@"1111111122222222"]];
    }

    - (NSData *)generateHexBytesFrom:(NSString *)hexString {
        unsigned int  hexInt;
        unsigned char byte;
        
        // 与えられたHEX文字列を２文字ずつ切り出し、バイトデータに変換する
        NSMutableData *convertedBytes = [[NSMutableData alloc] init];
        for (int i = 0; i < [hexString length]; i+=2) {
            NSString *tmp = [hexString substringWithRange:NSMakeRange(i, 2)];
            [[NSScanner scannerWithString:tmp] scanHexInt:&hexInt];
            byte = (unsigned char)hexInt;
            [convertedBytes appendBytes:&byte length:sizeof(byte)];
        }
        
        return convertedBytes;
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

    - (void)startBLEConnection {
        // 処理開始ログ
        NSLog(@"Matter device commissioning start");
        uint32_t setupPINCode = 20202021;
        uint16_t discriminator = 0x0f00;
        [self handleRendezVousBLE:discriminator setupPINCode:setupPINCode];
    }

    - (void)performOffCommand {
        // 処理開始ログ
        NSLog(@"Matter device off command start");
        [self performCommandOnOff:true endpoint:1];
    }

    - (void)performOnCommand {
        // 処理開始ログ
        NSLog(@"Matter device on command start");
        [self performCommandOnOff:false endpoint:1];
    }

#pragma mark - Pairing & Commissioning

    - (void)pairingCompleted:(bool)success {
        if (success == false) {
            // ペアリング失敗時は画面に制御を戻す
            [[self delegate] didPairingComplete:false];
            return;
        }
        // For debug
        [self printDeviceList];
        // コミッショニング処理を続行
        if ([self addThreadNetwork] == false) {
            NSLog(@"Status: Failed to trigger the connection with the device");
            [[self delegate] didPairingComplete:false];
            return;
        }
    }

    - (bool)addThreadNetwork {
        bool success = CHIPGetConnectedDevice(^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice == nil) {
                NSLog(@"Status: Failed to establish a connection with the device");
                [[self delegate] didPairingComplete:false];
                return;
            }

            [self setCluster:[[CHIPNetworkCommissioning alloc] initWithDevice:chipDevice endpoint:0 queue:dispatch_get_main_queue()]];

            __weak typeof(self) weakSelf = self;
            uint64_t breadcrumb = 0;
            uint32_t timeoutMs = 3000;
            [[self cluster] addThreadNetwork:[self operationalDataset]
                breadcrumb:breadcrumb timeoutMs:timeoutMs
                responseHandler:^(NSError *error, NSDictionary *values) {
                [weakSelf onAddNetworkResponse:error isWiFi:NO];
            }];
        });
        return success;
    }

    - (void)onAddNetworkResponse:(NSError *)error isWiFi:(BOOL)isWiFi {
        if (error != nil) {
            NSLog(@"Error adding network: %@", error);
            [[self delegate] didPairingComplete:false];
            return;
        }

        NSData * networkId;
        if (isWiFi) {
            NSString * ssid = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkSSIDDefaultsKey);
            networkId = [ssid dataUsingEncoding:NSUTF8StringEncoding];
        } else {
            networkId = [self networkId];
        }

        __weak typeof(self) weakSelf = self;
        uint64_t breadcrumb = 0;
        uint32_t timeoutMs = 3000;
        [[self cluster] enableNetwork:networkId
            breadcrumb:breadcrumb timeoutMs:timeoutMs
            responseHandler:^(NSError * err, NSDictionary * values) {
            [weakSelf onEnableNetworkResponse:err];
        }];
    }

    - (void)onEnableNetworkResponse:(NSError *)error {
        if (error != nil) {
            NSLog(@"Error enabling network: %@", error);
            [[self delegate] didPairingComplete:false];
            return;
        }
        // コミッション全工程完了
        uint64_t deviceId = CHIPGetNextAvailableDeviceID() - 1;
        CHIPDeviceController *controller = [CHIPDeviceController sharedController];
        [controller updateDevice:deviceId fabricId:0];
        [[self delegate] didPairingComplete:true];
    }

    - (void)printDeviceList {
        // For debug
        uint64_t nextDeviceID = CHIPGetNextAvailableDeviceID();
        for (uint64_t i = 0; i < nextDeviceID; i++) {
            if (CHIPIsDevicePaired(i)) {
                NSLog(@"Paired Matter device (%d)", (int)i);
            }
        }
    }

#pragma mark - Perform on & off command

    - (void)performCommandOnOff:(bool)isOff endpoint:(uint16_t)endpoint {
        // コミッション済みデバイスのIDを取得
        uint64_t deviceId = CHIPGetNextAvailableDeviceID() - 1;
        // 接続処理を実行
        if ([self connectForCommandOnOff:isOff deviceId:deviceId endpoint:endpoint] == false) {
            NSLog(@"Failed to trigger the connection with the device (id=%d)", (int)deviceId);
            [[self delegate] didPerformCommandComplete:false];
            return;
        }
        // 接続処理完了待ち
        NSLog(@"Waiting for connection with the device (id=%d)", (int)deviceId);
    }

    - (bool)connectForCommandOnOff:(bool)isOff deviceId:(uint64_t)deviceId endpoint:(uint16_t)endpoint {
        // 接続完了ハンドラーを設定
        __weak typeof(self) weakSelf = self;
        CHIPDeviceConnectionCallback handler = ^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice == nil) {
                NSLog(@"Status: Failed to establish a connection with the device");
                [[weakSelf delegate] didPerformCommandComplete:false];
                return;
            }
            // 接続処理完了時
            [weakSelf onConnectForCommandOnOff:chipDevice error:error isOff:isOff endpoint:endpoint];
        };
        // 接続処理を実行
        bool success = CHIPGetConnectedDeviceWithID(deviceId, handler);
        return success;
    }

    - (void)onConnectForCommandOnOff:(CHIPDevice *)chipDevice error:(NSError *)error isOff:(bool)isOff endpoint:(uint16_t)endpoint{
        if (error != nil) {
            NSLog(@"Error connecting device for on/off command: %@", error);
            [[self delegate] didPerformCommandComplete:false];
            return;
        }
        // 実行コマンドの完了ハンドラーを設定
        __weak typeof(self) weakSelf = self;
        ResponseHandler handler = ^(NSError * error, NSDictionary * values) {
            [weakSelf onCommandOnOffResponse:error values:values isOff:isOff];
        };
        // コマンド実行
        [self setClusterOnOff:[[CHIPOnOff alloc] initWithDevice:chipDevice endpoint:endpoint queue:dispatch_get_main_queue()]];
        if (isOff) {
            [[self clusterOnOff] off:handler];
        } else {
            [[self clusterOnOff] on:handler];
        }
    }

    - (void)onCommandOnOffResponse:(NSError *)error values:(NSDictionary *)values isOff:(bool)isOff {
        if (error != nil) {
            NSLog(@"Error performing on/off command: %@", error);
            [[self delegate] didPerformCommandComplete:false];
            return;
        }
        // コマンド実行完了
        [[self delegate] didPerformCommandComplete:true];
    }

@end
