# OATH-TOTPについての実装調査

最終更新日：2022/4/27

## 概要
OATH-TOTPの実装に必要な要素技術の調査内容／結果を掲載しています。

### Secret導入方法の調査
OATHで発行するパスワードの元となるシード（Secret）を、認証器に導入する方法について調査します。

#### QRコードによる導入
Secret導入方法としては、Webサイトが表示するQRコードから、URIに含まれるSecretを解析して導入するケースが多いようです。<br>
例として、下図のようなQRコードが、Webサイトに表示されていたとします。

<img src="assets01/qrcode.png" width="150">

こちらを解析すると、以下のようなURIになっています。
```
otpauth://totp/Example:alice@google.com?secret=JBSWY3DPEHPK3PXP&issuer=Example
```

#### macOS環境での解析
macOS環境におけるQRコード解析処理は、下記のようなコードで実装できます。

```
#import <CoreImage/CoreImage.h>
:
@implementation AppDelegate
    :
    - (IBAction)buttonDidPress:(id)sender {
        NSURL *url = [NSURL fileURLWithPath:@"/Users/makmorit/Desktop/qrcode.png"];
        NSDictionary *options = @{CIDetectorAccuracy: CIDetectorAccuracyHigh};
        CIContext *context = [[CIContext alloc] init];
        CIImage *ciImage = [[CIImage alloc] initWithContentsOfURL:url];
        CIDetector *ciDetector = [CIDetector detectorOfType:CIDetectorTypeQRCode context:context options:options];
        NSArray<CIFeature *> *features = [ciDetector featuresInImage:ciImage options:options];
        if ([features count] == 0) {
            [[ToolLogFile defaultLogger] debug:@"No image detector feature"];
            return;
        }
        for (CIFeature *feature in features) {
            if ([[feature type] isNotEqualTo:CIFeatureTypeQRCode]) {
                continue;
            }
            CIQRCodeFeature *qrCodeFeature = (CIQRCodeFeature *)feature;
            NSString *string = [qrCodeFeature messageString];
            [[ToolLogFile defaultLogger] debugWithFormat:@"QR code detected: %@", string];
        }
    }
```

実行結果は以下になります。

```
2022-04-27 14:55:25.109 [debug] QR code detected: otpauth://totp/Example:alice@google.com?secret=JBSWY3DPEHPK3PXP&issuer=Example
```

#### Windows環境での解析
（後報）
